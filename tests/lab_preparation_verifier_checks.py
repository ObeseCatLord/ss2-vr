"""In-memory instruction mutations of the actual production object; no native run."""
from pathlib import Path
import re
import subprocess
import struct
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from verify_lab_preparation_abi import verify,verify_extent,descriptor,instructions,symbol_offset,callback_index
OBJECT=Path(sys.argv.pop(1)).resolve()

class Checks(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.assembly=subprocess.check_output(['objdump','-drC','-Mintel','--insn-width=16',str(OBJECT)],text=True)
        cls.symbols=subprocess.check_output(['objdump','-tC',str(OBJECT)],text=True)
        cls.targets=dict(descriptor(OBJECT,n) for n in ('LabAutoShotgunTarget','LabSniperTarget'))
        parts=re.split(r'^[0-9a-f]+ <(.+)>:\n',cls.assembly,flags=re.M)
        bodies=dict(zip(parts[1::2],parts[2::2]))
        cls.body=next(b for n,b in bodies.items() if n.endswith('runPostSimulationSniper(ss2vr::game::SimulationInterval&)::{lambda()#1}::operator()() const'))

    def test_production_object(self):
        result=verify(OBJECT)
        self.assertEqual(result['selection_arguments'],4)
        self.assertEqual(result['selection_callee_pop_bytes'],16)
        self.assertTrue(result['id2_sniper_zoom_excluded']);self.assertFalse(result['runtime_executed'])

    def test_actual_argument_stack_and_dispatch_mutations_reject(self):
        mutations=(
            ('c7 44 24 04 01 00 00 00','c7 44 24 04 02 00 00 00'), # Wrong right-hand argument.
            ('83 ec 10','83 ec 0c'), # Wrong four-argument repair.
            ('8b 41 0c','8b 41 08'), # Save basename instead of native resource path.
            ('80 78 14 00','80 78 10 00'), # Wrong target zoom flag.
        )
        for before,after in mutations:
            with self.subTest(instruction=before):
                self.assertEqual(self.body.count(before),2 if before=='80 78 14 00' else 1)
                mutated=self.assembly.replace(self.body,self.body.replace(before,after,1))
                with self.assertRaises(ValueError):verify_extent(mutated,self.symbols,self.targets)
        # Replace only the final player load before the actual selection call.
        pattern=r'(8b 8d [0-9a-f ]+\s+mov\s+ecx,DWORD PTR \[ebp-[^]]+\]\n)(\s*[0-9a-f]+:\s+ff 15 [^\n]+\n\s*[^\n]+dir32\s+\.bss)'
        matches=list(re.finditer(pattern,self.body))
        # Resolve by the compiled four-argument staging directly before it.
        selected=[m for m in matches if 'c7 44 24 04 01 00 00 00' in self.body[max(0,m.start()-300):m.start()]]
        self.assertEqual(len(selected),1)
        m=selected[0];piece=m[1].replace('8b 8d','8b 85')
        body=self.body[:m.start(1)]+piece+self.body[m.end(1):]
        with self.assertRaises(ValueError):verify_extent(self.assembly.replace(self.body,body),self.symbols,self.targets)
        for targets in ({2:True,13:True},{2:False,13:False},{3:False,13:True}):
            with self.subTest(targets=targets),self.assertRaises(ValueError):
                verify_extent(self.assembly,self.symbols,targets)

    def test_senior_review_branch_and_receiver_counterexamples_reject(self):
        code=instructions(self.body)
        zoom=callback_index(code,symbol_offset(self.symbols,'nativeZoomFlag'))
        guard=max(i for i in range(zoom) if code[i][1:]==('cmp','byte ptr [eax + 0x14], 0'))
        alternate=int(code[guard+1][2],16)
        start=next(i for i,item in enumerate(code) if item[0]==alternate)
        mismatch=code[start+2][0]
        self.assertEqual(code[start+2][1],'jne')
        spill=code[zoom-1][0]
        self.assertEqual(code[zoom-2][1:],('mov','ecx, edx'))
        changes=(
            (mismatch,b'\x0f\x84'+struct.pack('<i',int(code[start+2][2],16)-mismatch-6)),
            (mismatch,b'\x0f\x85'+struct.pack('<i',code[guard+1][0]-mismatch-6)),
            (spill,bytes.fromhex('81 c9 01 00 00 00')),
        )
        for address,replacement in changes:
            with self.subTest(address=address,replacement=replacement.hex()):
                pattern=r'(^\s*'+format(address,'x')+r':\s+)((?:[0-9a-f]{2} )+)(\s+[^\n]+)$'
                matches=list(re.finditer(pattern,self.body,re.M));self.assertEqual(len(matches),1)
                match=matches[0];self.assertEqual(len(bytes.fromhex(match[2])),len(replacement))
                body=self.body[:match.start(2)]+replacement.hex(' ')+' '+self.body[match.end(2):]
                with self.assertRaises(ValueError):verify_extent(self.assembly.replace(self.body,body),self.symbols,self.targets)

if __name__=='__main__':unittest.main()
