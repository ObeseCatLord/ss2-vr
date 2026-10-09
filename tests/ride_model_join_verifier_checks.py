"""Counterexamples in decoded production-object bytes; no native execution."""
from pathlib import Path
import re
import struct
import subprocess
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from verify_ride_model_join_abi import verify_extent
from verify_ride_control_abi import bodies,decoded_nodes,extent,instructions,symbol_offset
OBJECT=Path(sys.argv.pop(1)).resolve()


class Checks(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.assembly=subprocess.check_output(['objdump','-drC','-Mintel','--insn-width=16',str(OBJECT)],text=True)
        cls.symbols=subprocess.check_output(['objdump','-tC',str(OBJECT)],text=True)
        cls.table=bodies(cls.assembly)

    def changed(self,body,address,replacement):
        matches=list(re.finditer(r'(^\s*'+format(address,'x')+r':\s+)((?:[0-9a-f]{2} )+)(\s+[^\n]+)$',body,re.M))
        self.assertEqual(len(matches),1);m=matches[0]
        self.assertEqual(len(bytes.fromhex(m[2])),len(replacement))
        changed=body[:m.start(2)]+replacement.hex(' ')+' '+body[m.end(2):]
        return self.assembly.replace(body,changed)

    def rejects(self,assembly):
        with self.assertRaises(ValueError):verify_extent(assembly,self.symbols)

    def test_actual_object(self):
        result=verify_extent(self.assembly,self.symbols)
        self.assertEqual(result['cleanup_cases_checked'],12)
        self.assertEqual(result['return_opcode_stack_pop_bytes'],0)
        self.assertTrue(result['outer_tls_final_value_verified'])
        self.assertFalse(result['runtime_executed'])

    def test_original_identity_and_eax_store_controls(self):
        for name,slot_name in (('observedRideRenderable','originalRideRenderable'),
                               ('observedRideModelInstance','originalRideModelInstance')):
            with self.subTest(getter=name):
                run=extent(self.table,name+'(void*','::Context::run(void*)')
                slot=symbol_offset(self.symbols,slot_name);code=instructions(run)
                i=next(i for i,item in enumerate(code) if item[1:]==('call',f'dword ptr [{hex(slot)}]'))
                self.rejects(self.changed(run,code[i][0],b'\xff\x15'+struct.pack('<I',slot+4)))
                self.assertIn(code[i+1][1:],(('mov','dword ptr [esi], eax'),('mov','dword ptr [ebx], eax')))
                # Store ECX instead of EAX immediately after the original returns.
                replacement=b'\x89\x0e' if '[esi]' in code[i+1][2] else b'\x89\x0b'
                self.rejects(self.changed(run,code[i+1][0],replacement))
                relocation=f'{code[i][0]+2:x}: dir32\t.bss'
                self.assertEqual(run.count(relocation),1)
                self.rejects(self.assembly.replace(run,run.replace(relocation,relocation.replace('.bss','.data'))))

    def test_outer_cleanup_branch_value_and_overwrite_controls(self):
        finish=extent(self.table,'observedRideModelInstance(void*','::Context::finish(void*, int)')
        code=instructions(finish)
        condition=next(a for a,mn,op in code if (mn,op)==('test','ebx, ebx'))
        self.rejects(self.changed(finish,condition,bytes.fromhex('31 c9')))
        branch=next((a,op) for a,mn,op in code if mn=='je')
        self.rejects(self.changed(finish,branch[0],b'\xeb'+struct.pack('b',int(branch[1],16)-branch[0]-2)))
        offset=symbol_offset(self.symbols,'activeRideModelJoin')
        if offset==0:
            reload=next(a for a,mn,op in code if (mn,op)==('mov','esi, dword ptr [ebp - 4]'))
            self.rejects(self.changed(finish,reload,bytes.fromhex('c6 06 01')))
        else:
            self.assertLess(offset,128)
            flag=next(a for a,mn,op in code if (mn,op)==('mov','byte ptr [eax + 0x28], 1'))
            self.rejects(self.changed(finish,flag,b'\xc6\x42'+bytes([offset,1])))

    def test_finally_function_entry_addend_for_both_getters(self):
        for name in ('@_ZN5ss2vr4gameL22observedRideRenderable','@_ZN5ss2vr4gameL25observedRideModelInstance'):
            entry=extent(self.table,name,'@8')
            call=next(a for a,mn,op,relocs in decoded_nodes(entry)
                      if mn=='call' and relocs==[(a+1,'DISP32','ss2vrNativeFinally')])
            with self.subTest(entry=name):
                self.rejects(self.changed(entry,call,bytes.fromhex('e8 01 00 00 00')))

    def test_each_abnormal_token_clear_and_inner_return_flag(self):
        for name in ('observedRideRenderable','observedRideModelInstance'):
            finish=extent(self.table,name+'(void*','::Context::finish(void*, int)')
            for address,mn,op in instructions(finish):
                if mn=='mov' and op in ('dword ptr [eax + 0x1c], 0','dword ptr [eax + 0x20], 0'):
                    displacement=0x1c if '0x1c' in op else 0x20
                    with self.subTest(getter=name,clear=address):
                        self.rejects(self.changed(finish,address,b'\xc7\x40'+bytes([displacement])+struct.pack('<I',1)))
        finish=extent(self.table,'observedRideRenderable(void*','::Context::finish(void*, int)')
        returned=next(a for a,mn,op in instructions(finish) if (mn,op)==('mov','byte ptr [eax + 0x26], 1'))
        self.rejects(self.changed(finish,returned,bytes.fromhex('c6 40 26 00')))


if __name__=='__main__':unittest.main()
