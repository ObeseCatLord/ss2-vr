"""Alter actual x86 instruction bytes in memory; never execute the object."""
from pathlib import Path
import re
import os
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
        self.assertEqual(result['mounted_capture_layout_verified'],
                         {'previousObservation': 0, 'previousGrip': 4, 'grip': 8, 'observation': 12})
        self.assertEqual(result['mounted_dual_tls_restores'], 4)
        self.assertEqual(result['ride_grasp_interrupt_offsets'], {'mask': 0x134, 'keyed': 0x150})
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

    def test_each_dual_tls_restore_and_unknown_branch_reject(self):
        finish = extent(self.table, 'mountedLookClamp(void*', '::Context::finish(void*, int)')
        stores = [(a, op) for a, mn, op, relocs in decoded_nodes(finish)
                  if mn == 'mov' and relocs == [(a + 2, 'secrel32', '.tls$')]]
        self.assertEqual(len(stores), 4)
        for address, op in stores:
            with self.subTest(restore=op):
                # Preserve width/base/displacement; corrupt the captured value register.
                offset = int(re.search(r'\+ (0x[0-9a-f]+|[0-9]+)\]', op)[1], 0)
                replacement = bytes([0x89, 0x82 if '[edx' in op else 0x86]) + struct.pack('<I', offset)
                self.rejects(self.changed(finish, address, replacement))
        branch = next(a for a, mn, _ in instructions(finish) if mn == 'je')
        self.rejects(self.changed(finish, branch, b'\xff\xe0'))

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

    def test_compact_capture_interrupt_and_global_destination_controls(self):
        finish = extent(self.table, 'mountedLookClamp(void*', '::Context::finish(void*, int)')
        run = extent(self.table, 'observedRideLookClamp(void*', '::Context::run(void*)')
        slot = symbol_offset(self.symbols, 'originalRideLookClamp')
        call = next(item for item in decoded_nodes(run) if item[1:3] == ('call', f'dword ptr [{hex(slot)}]'))
        self.assertEqual(call[3], [(call[0] + 2, 'dir32', '.bss')])
        relocation = f'{call[0]+2:x}: dir32\t.bss'
        self.assertEqual(run.count(relocation), 1)
        self.rejects(self.assembly.replace(run, run.replace(relocation, relocation.replace('.bss', '.data'))))
        mounted_run = extent(self.table, 'mountedLookClamp(void*', '::Context::run(void*)')
        capture = next(a for a, mn, op in instructions(mounted_run)
                       if (mn, op) == ('mov', 'edx, dword ptr [ebx + 4]'))
        self.rejects(self.changed(mounted_run, capture, bytes.fromhex('8b 53 08')))
        code = instructions(finish)
        abnormal = next(a for a, mn, op in code if (mn, op) == ('mov', 'byte ptr [eax + 0x14], 1'))
        bypass = next(a for a, mn, op in code if mn == 'jmp' and op == hex(abnormal))
        epilogue = next(a for a, mn, op in code if (mn, op) == ('mov', 'ebx, dword ptr [ebp - 0xc]'))
        self.rejects(self.changed(finish, bypass,
                                  b'\xe9' + struct.pack('<i', epilogue - bypass - 5)))
        ride_grasp = symbol_offset(self.symbols, 'rideGrasp')
        keyed = next(a for a, mn, op in instructions(finish)
                     if (mn, op) == ('mov', f'byte ptr [{hex(ride_grasp + 0x150)}], 0'))
        self.rejects(self.changed(finish, keyed,
                                  b'\xc6\x05' + struct.pack('<I', ride_grasp + 0x154) + b'\x00'))

    def test_compact_tls_index_and_stack_probe_identity_reject(self):
        finish = extent(self.table, 'mountedLookClamp(void*', '::Context::finish(void*, int)')
        indices = [node for node in decoded_nodes(finish)
                   if any(target == '_tls_index' for _, _, target in node[3])]
        self.assertEqual(len(indices), 2)
        for address, _, _, relocs in indices:
            offset, kind, target = relocs[0]
            original = f'{offset:x}: {kind}\t{target}'
            self.assertIn(original, finish)
            self.rejects(self.assembly.replace(finish, finish.replace(original, original.replace('_tls_index', '.bss'))))
        entry = extent(self.table, '@_ZN5ss2vr4gameL16mountedLookClamp', '@12')
        probe = next(node for node in decoded_nodes(entry)
                     if any(target == '__chkstk_ms' for _, _, target in node[3]))
        relocation = f'{probe[0]+1:x}: DISP32\t__chkstk_ms'
        self.rejects(self.assembly.replace(entry, entry.replace(relocation, relocation.replace('__chkstk_ms', 'unknown_callback'))))
        self.rejects(self.changed(entry, probe[0], bytes.fromhex('e8 01 00 00 00')))

    def test_actual_constructor_capture_and_finish_pointer_reject(self):
        entry = extent(self.table, '@_ZN5ss2vr4gameL16mountedLookClamp', '@12')
        nodes = decoded_nodes(entry)
        reference = next(a for a, mn, op, _ in nodes if (mn, op) == ('lea', 'eax, [ebp - 0x1378]'))
        # Swap the actual saved-observation capture for the saved-grip reference.
        self.rejects(self.changed(entry, reference, b'\x8d\x85' + struct.pack('<i', -0x1374)))
        finish = extent(self.table, 'mountedLookClamp(void*', '::Context::finish(void*, int)')
        run = extent(self.table, 'mountedLookClamp(void*', '::Context::run(void*)')
        finish_address = decoded_nodes(finish)[0][0];run_address = decoded_nodes(run)[0][0]
        argument = next(a for a, mn, op, _ in nodes if (mn, op) == ('mov', f'dword ptr [esp + 4], {hex(finish_address)}'))
        self.rejects(self.changed(entry, argument, b'\xc7\x44\x24\x04' + struct.pack('<I', run_address)))

    def with_added_coff_relocation(self, address, kind):
        # Mutate an actual mod COFF object in anonymous memory; disk products and
        # their relocation tables remain unchanged. This is offline decoding.
        data = bytearray(OBJECT.read_bytes())
        machine, count = struct.unpack_from('<HH', data);self.assertEqual(machine, 0x14c)
        optional = struct.unpack_from('<H', data, 16)[0]
        sections = []
        for index in range(count):
            offset = 20 + optional + index * 40
            if bytes(data[offset:offset + 8]).rstrip(b'\x00') == b'.text':sections.append(offset)
        self.assertEqual(len(sections), 1);section = sections[0]
        relocation_start = struct.unpack_from('<I', data, section + 24)[0]
        relocation_count = struct.unpack_from('<H', data, section + 32)[0]
        self.assertLess(relocation_count, 65535)
        symbol_start, symbol_count = struct.unpack_from('<II', data, 8)
        symbol = None;index = 0
        while index < symbol_count:
            offset = symbol_start + index * 18
            if bytes(data[offset:offset + 8]).rstrip(b'\x00') == b'.data':symbol = index;break
            index += 1 + data[offset + 17]
        self.assertIsNotNone(symbol)
        relocations = bytes(data[relocation_start:relocation_start + relocation_count * 10])
        new_start = len(data)
        data.extend(relocations + struct.pack('<IIH', address, symbol, kind))
        struct.pack_into('<I', data, section + 24, new_start)
        struct.pack_into('<H', data, section + 32, relocation_count + 1)
        descriptor = os.memfd_create('ss2vr-coff-rejection', os.MFD_CLOEXEC)
        try:
            with os.fdopen(os.dup(descriptor), 'wb') as stream:stream.write(data)
            return subprocess.check_output(['objdump', '-drC', '-Mintel', '--insn-width=16',
                f'/proc/self/fd/{descriptor}'], text=True, pass_fds=(descriptor,))
        finally:
            os.close(descriptor)

    def test_actual_coff_unexpected_tls_relocations_reject(self):
        for suffix, kind in (('::Context::run(void*)', 6), ('::Context::finish(void*, int)', 7)):
            body = extent(self.table, 'mountedLookClamp(void*', suffix)
            address = next(a for a, mn, op, _ in decoded_nodes(body)
                           if mn == 'mov' and op.endswith('dword ptr fs:[0x2c]'))
            altered = self.with_added_coff_relocation(address + 3, kind)
            self.assertIn('dir32' if kind == 6 else 'rva32', altered)
            self.rejects(altered)

    def test_entry_stack_repair_mutations_reject(self):
        entry = extent(self.table, '@_ZN5ss2vr4gameL21observedRideLookClamp', '@12')
        repairs = [a for a, mn, op in instructions(entry) if (mn, op) == ('add', 'esp, 0x4c')]
        self.assertEqual(len(repairs), 2)
        for address in repairs:
            with self.subTest(epilogue=address):
                self.rejects(self.changed(entry, address, bytes.fromhex('83 c4 48')))

    def test_finally_delegate_addend_controls(self):
        for name in ('@_ZN5ss2vr4gameL21observedRideLookClamp','@_ZN5ss2vr4gameL16mountedLookClamp'):
            entry = extent(self.table, name, '@12')
            call = next(a for a, mn, op, relocs in decoded_nodes(entry)
                        if mn == 'call' and relocs == [(a+1, 'DISP32', 'ss2vrNativeFinally')])
            with self.subTest(entry=name):
                self.rejects(self.changed(entry, call, bytes.fromhex('e8 01 00 00 00')))


if __name__ == '__main__':
    unittest.main()
