"""Alter actual x86 instruction bytes in memory; never execute the object."""
from pathlib import Path
import re
import struct
import subprocess
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from verify_ride_control_abi import bodies, decoded_nodes, extent, instructions, symbol_offset, verify_extent

OBJECT = Path(sys.argv.pop(1)).resolve()


class Checks(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.assembly = subprocess.check_output(
            ['objdump', '-drC', '-Mintel', '--insn-width=16', str(OBJECT)], text=True)
        cls.symbols = subprocess.check_output(['objdump', '-tC', str(OBJECT)], text=True)
        cls.table = bodies(cls.assembly)

    def changed(self, body, address, replacement):
        pattern = r'(^\s*' + format(address, 'x') + r':\s+)((?:[0-9a-f]{2} )+)(\s+[^\n]+)$'
        matches = list(re.finditer(pattern, body, re.M))
        self.assertEqual(len(matches), 1)
        match = matches[0]
        self.assertEqual(len(bytes.fromhex(match[2])), len(replacement))
        altered = body[:match.start(2)] + replacement.hex(' ') + ' ' + body[match.end(2):]
        return self.assembly.replace(body, altered)

    def rejects(self, altered):
        with self.assertRaises(ValueError):
            verify_extent(altered, self.symbols)

    def test_actual_production_paths(self):
        result = verify_extent(self.assembly, self.symbols)
        self.assertTrue(result['normal_paths_forward_once'])
        self.assertTrue(result['outer_tls_restored_each_cleanup_return'])
        self.assertEqual(result['native_reference_stack_bytes'], 4)
        self.assertFalse(result['runtime_executed'])

    def test_actual_forward_and_callee_pop_mutations(self):
        for name, entry_name, slot_name in (
            ('observedRideLookClamp', '@_ZN5ss2vr4gameL21observedRideLookClamp', 'originalRideLookClamp'),
            ('mountedLookClamp', '@_ZN5ss2vr4gameL16mountedLookClamp', 'originalLookClamp'),
        ):
            with self.subTest(wrapper=name):
                run = extent(self.table, name + '(void*', '::Context::run(void*)')
                slot = symbol_offset(self.symbols, slot_name)
                calls = [a for a, mn, op in instructions(run)
                         if mn == 'call' and op == f'dword ptr [{hex(slot)}]']
                self.assertEqual(len(calls), 1)
                self.rejects(self.changed(run, calls[0], b'\xff\x15' + struct.pack('<I', slot + 4)))
                entry = extent(self.table, entry_name, '@12')
                returns = [a for a, mn, op in instructions(entry) if mn == 'ret' and op == '4']
                self.assertTrue(returns)
                self.rejects(self.changed(entry, returns[0], b'\xc2\x08\x00'))

    def test_each_outer_tls_restore_and_unknown_branch_reject(self):
        finish = extent(self.table, 'mountedLookClamp(void*', '::Context::finish(void*, int)')
        stores = [(a, op) for a, mn, op in instructions(finish)
                  if mn == 'mov' and re.fullmatch(r'dword ptr \[(?:edx|esi)\], ecx', op)]
        self.assertEqual(len(stores), 2)
        for address, op in stores:
            with self.subTest(restore=op):
                # Keep size/offset; store EBX instead of saved outer ECX.
                replacement = bytes.fromhex('89 9a 00 00 00 00' if '[edx]' in op else '89 9e 00 00 00 00')
                self.rejects(self.changed(finish, address, replacement))
        branch = next(a for a, mn, _ in instructions(finish) if mn == 'je')
        self.rejects(self.changed(finish, branch, b'\xff\xe0'))
        reload = next(a for a, mn, op in instructions(finish)
                      if (mn, op) == ('mov', 'esi, dword ptr [ebp - 4]'))
        # At this exact reload ESI still addresses TLS; corrupt its low byte
        # after the valid DWORD restore. A restore count alone cannot catch it.
        self.rejects(self.changed(finish, reload, bytes.fromhex('c6 06 01')))

    def test_cleanup_invalidation_and_callback_flags_reject(self):
        for name in ('observedRideLookClamp', 'mountedLookClamp'):
            finish = extent(self.table, name + '(void*', '::Context::finish(void*, int)')
            # Mutate all copies of a required flag instruction, not another thunk.
            for before, after in (('c6 40 14 01', 'c6 40 14 00'),
                                  ('c6 40 12 00', 'c6 40 12 01')):
                with self.subTest(wrapper=name, flag=before):
                    self.assertIn(before, finish)
                    self.rejects(self.assembly.replace(finish, finish.replace(before, after)))
        finish = extent(self.table, 'observedRideLookClamp(void*', '::Context::finish(void*, int)')
        for before, after in (('c6 40 10 00', 'c6 40 10 01'),
                              ('c6 40 13 01', 'c6 40 13 00')):
            with self.subTest(callback_flag=before):
                self.assertIn(before, finish)
                self.rejects(self.assembly.replace(finish, finish.replace(before, after)))

    def test_astra_case_and_relocation_counterexamples_reject(self):
        finish = extent(self.table, 'mountedLookClamp(void*', '::Context::finish(void*, int)')
        condition = next(a for a, mn, op in instructions(finish) if (mn, op) == ('test', 'ebx, ebx'))
        self.rejects(self.changed(finish, condition, bytes.fromhex('31 c9')))
        branch = next((a, op) for a, mn, op in instructions(finish) if mn == 'je')
        displacement = int(branch[1], 16) - branch[0] - 2
        self.rejects(self.changed(finish, branch[0], b'\xeb' + struct.pack('b', displacement)))
        run = extent(self.table, 'observedRideLookClamp(void*', '::Context::run(void*)')
        slot = symbol_offset(self.symbols, 'originalRideLookClamp')
        call = next(item for item in decoded_nodes(run) if item[1:3] == ('call', f'dword ptr [{hex(slot)}]'))
        self.assertEqual(call[3], [(call[0] + 2, 'dir32', '.bss')])
        relocation = f'{call[0]+2:x}: dir32\t.bss'
        self.assertEqual(run.count(relocation), 1)
        self.rejects(self.assembly.replace(run, run.replace(relocation, relocation.replace('.bss', '.data'))))

    def test_entry_stack_repair_mutations_reject(self):
        entry = extent(self.table, '@_ZN5ss2vr4gameL21observedRideLookClamp', '@12')
        repairs = [a for a, mn, op in instructions(entry) if (mn, op) == ('add', 'esp, 0x4c')]
        self.assertEqual(len(repairs), 2)
        for address in repairs:
            with self.subTest(epilogue=address):
                self.rejects(self.changed(entry, address, bytes.fromhex('83 c4 48')))


if __name__ == '__main__':
    unittest.main()
