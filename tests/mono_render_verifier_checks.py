"""Mutate decoded production-object bytes in memory; never execute native code."""
from pathlib import Path
import re
import struct
import subprocess
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from verify_mono_render_abi import verify_extent,verify_completion_sequence
from verify_ride_control_abi import bodies,decoded_nodes,extent,instructions,symbol_offset
OBJECT=Path(sys.argv.pop(1)).resolve()


class Checks(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.assembly=subprocess.check_output(['objdump','-drC','-Mintel','--insn-width=16',str(OBJECT)],text=True)
        cls.symbols=subprocess.check_output(['objdump','-tC',str(OBJECT)],text=True)
        cls.table=bodies(cls.assembly)

    def rejects(self,body,address,replacement):
        matches=list(re.finditer(r'(^\s*'+format(address,'x')+r':\s+)((?:[0-9a-f]{2} )+)(\s+[^\n]+)$',body,re.M))
        self.assertEqual(len(matches),1);m=matches[0]
        self.assertEqual(len(bytes.fromhex(m[2])),len(replacement))
        changed=body[:m.start(2)]+replacement.hex(' ')+' '+body[m.end(2):]
        with self.assertRaises(ValueError):
            verify_extent(self.assembly.replace(body,changed),self.symbols)

    def test_actual_object(self):
        result=verify_extent(self.assembly,self.symbols)
        self.assertTrue(result['native_thiscall_ecx_capture_verified'])
        self.assertTrue(result['once_only_original_normal_paths'])
        self.assertFalse(result['runtime_executed'])

    def test_entry_receiver_finally_addend_and_stack_pop(self):
        entry=self.table['ss2vr::game::presentedRender(void*)']
        capture=next(a for a,mn,op in instructions(entry) if (mn,op)==('mov','dword ptr [esp + 0xc], ecx'))
        self.rejects(entry,capture,bytes.fromhex('89 54 24 0c')) # Save EDX instead of ECX.
        delegate=next(a for a,mn,op,relocs in decoded_nodes(entry)
                      if relocs==[(a+1,'DISP32','ss2vrNativeFinally')])
        self.rejects(entry,delegate,bytes.fromhex('e8 01 00 00 00'))
        # Replace ret+padding with ret4: a thiscall callback with no stack args.
        ret=next(a for a,mn,op in instructions(entry) if mn=='ret')
        changed=re.sub(r'(^\s*'+format(ret,'x')+r':\s+)c3(\s+ret)',r'\g<1>c2 04 00  ret',entry,flags=re.M)
        with self.assertRaises(ValueError):verify_extent(self.assembly.replace(entry,changed),self.symbols)

    def test_original_receiver_and_slot(self):
        run=extent(self.table,'presentedRender(void*)','::Context::run(void*)')
        slot=symbol_offset(self.symbols,'originalRender')
        call=next(a for a,mn,op in instructions(run) if (mn,op)==('call',f'dword ptr [{hex(slot)}]'))
        self.rejects(run,call,b'\xff\x15'+struct.pack('<I',slot+4))
        receiver=next(a for a,mn,op in instructions(run) if (mn,op)==('mov','ecx, dword ptr [eax]'))
        self.rejects(run,receiver,bytes.fromhex('8b 10')) # Load EDX instead of ECX.

    def test_actual_callbacks_closure_mutation_and_retirement_target(self):
        entry=self.table['ss2vr::game::presentedRender(void*)']
        run=extent(self.table,'presentedRender(void*)','::Context::run(void*)')
        finish=extent(self.table,'presentedRender(void*)','::Context::finish(void*, int)')
        for location,body,prefix in (('esp',run,b'\xc7\x04\x24'),
                                    ('esp + 4',finish,b'\xc7\x44\x24\x04')):
            start=instructions(body)[0][0]
            address=next(a for a,mn,op in instructions(entry)
                         if (mn,op)==('mov',f'dword ptr [{location}], {hex(start)}'))
            self.rejects(entry,address,prefix+struct.pack('<I',start+1))
        # This backedge follows mono admission and precedes the native call.
        # Changing the no-op LEA to ADD EBX corrupts the closure base.
        padding=next(a for a,mn,op in instructions(run) if (mn,op)==('lea','esi, [esi]'))
        self.rejects(run,padding,bytes.fromhex('83 c3 01'))
        self.rejects(run,padding,bytes.fromhex('80 cb 01')) # OR BL corrupts EBX too.
        late=next(a for a,mn,op in instructions(entry)
                  if (mn,op)==('mov','dword ptr [esp + 0x28], 0'))
        for displacement in (0,4):
            self.rejects(entry,late,b'\xc7\x44\x24'+bytes([displacement])+bytes(4))
        delegate=next(a for a,mn,op,relocs in decoded_nodes(finish)
                      if relocs==[(a+1,'DISP32','ss2vr::game::remote_render::retirePresentation(unsigned int)')])
        self.rejects(finish,delegate,bytes.fromhex('e9 01 00 00 00'))

    def test_completion_owner_flag_and_relocation(self):
        run=extent(self.table,'presentedRender(void*)','::Context::run(void*)')
        target='ss2vr::game::remote_render::completeMonoPresentation(unsigned int, bool)'
        complete=next(a for a,mn,op,relocs in decoded_nodes(run)
                      if relocs==[(a+1,'DISP32',target)])
        self.rejects(run,complete,bytes.fromhex('e8 01 00 00 00'))
        flag=next(a for a,mn,op in instructions(run) if (mn,op)==('mov','dword ptr [esp + 4], 1'))
        self.rejects(run,flag,bytes.fromhex('c7 44 24 04 00 00 00 00'))
        owner=next(a for a,mn,op in instructions(run) if (mn,op)==('mov','eax, dword ptr [ebx]'))
        self.rejects(run,owner,bytes.fromhex('8b 13')) # Wrong completion value register.

    def test_branch_cannot_enter_argument_setup_midway(self):
        run=extent(self.table,'presentedRender(void*)','::Context::run(void*)')
        branch=next(a for a,mn,op in instructions(run) if mn=='js')
        middle=next(a for a,mn,op in instructions(run) if (mn,op)==('mov','eax, dword ptr [ebx]'))
        self.assertTrue(-128<=middle-(branch+2)<=127)
        self.rejects(run,branch,bytes([0x78,(middle-(branch+2))&255]))

    def test_address_order_and_one_call_each_do_not_prove_execution_order(self):
        # Encoded bounded graph executes all six events once, but setup and
        # completion precede the original. Numerical addresses remain ordered.
        body=''' 100: eb 0e jmp 110
 105: 90 nop
 106: c3 ret
 110: 90 nop
 111: 90 nop
 112: 90 nop
 113: 90 nop
 114: 90 nop
 115: eb ee jmp 105
'''
        normal=body.replace('eb 0e jmp 110','eb 03 jmp 105').replace('106: c3 ret','106: eb 08 jmp 110').replace('115: eb ee jmp 105','115: c3 ret')
        events=[0x105,0x110,0x111,0x112,0x113,0x114]
        verify_completion_sequence(normal,events)
        with self.assertRaisesRegex(ValueError,'skips/reorders'):
            verify_completion_sequence(body,events)


if __name__=='__main__':unittest.main()
