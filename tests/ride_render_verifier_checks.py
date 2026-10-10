"""Finite production-byte rejection controls; no native code is executed."""
from pathlib import Path
import re
import struct
import subprocess
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from verify_remote_render_unwind import verify_declined_ride_entries
from verify_ride_control_abi import bodies,decoded_nodes
OBJECT=Path(sys.argv.pop(1)).resolve()

class Checks(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.assembly=subprocess.check_output(['objdump','-drC','-Mintel','--insn-width=16',str(OBJECT)],text=True)
        cls.table=bodies(cls.assembly)

    def test_actual_consumers(self):
        self.assertEqual(verify_declined_ride_entries(self.assembly),3)

    def test_owner_call_test_and_rejection_branch(self):
        for name,body in self.table.items():
            if 'clone' in name or not any(n in name for n in
                    ('::copyRidePaletteBookend(', '::observeRidePalette()', '::publishRideObservation(')):continue
            nodes=decoded_nodes(body)
            call=next(i for i,(_,mn,op,_) in enumerate(nodes) if mn=='call')
            test=call+1
            if nodes[test][1:3]==('mov','edx, eax'):test+=1
            self.assertEqual(nodes[test][1:3],('test','al, al'))
            self.assertEqual(nodes[test+1][1],'je')
            for index,kind in ((call,'call'),(test,'test'),(test+1,'branch')):
                address=nodes[index][0]
                m=re.search(r'(^\s*'+format(address,'x')+r':\s+)((?:[0-9a-f]{2} )+)',body,re.M)
                self.assertIsNotNone(m)
                raw=bytearray.fromhex(m[2])
                if kind=='call':raw[1]^=1 # Changes the actual local target.
                elif kind=='test':raw[:]=bytes.fromhex('84 d2') # TEST DL, not returned AL.
                elif raw[0]==0x74:raw[0]=0x75
                else:
                    self.assertEqual(raw[:2],bytes.fromhex('0f 84'));raw[1]=0x85
                changed=body[:m.start(2)]+raw.hex(' ')+' '+body[m.end(2):]
                with self.assertRaises(ValueError):
                    verify_declined_ride_entries(self.assembly.replace(body,changed))

    def test_entry_branch_cannot_skip_admission(self):
        body=next(b for n,b in self.table.items() if '::copyRidePaletteBookend(' in n and 'clone' not in n)
        nodes=decoded_nodes(body)
        gate=next(i for i,(_,mn,op,_) in enumerate(nodes) if mn=='call')
        # Replace the seven-byte private TLS load with a same-width conditional
        # branch (CS prefix plus near JE), bypassing the owner gate.
        before=next(row for row in nodes[:gate]
                    if row[1]=='mov' and row[2]=='edx, dword ptr fs:[0x2c]')
        address=before[0]
        m=re.search(r'(^\s*'+format(address,'x')+r':\s+)((?:[0-9a-f]{2} )+)',body,re.M)
        self.assertEqual(len(bytes.fromhex(m[2])),7)
        target=nodes[gate+4][0] # Beyond the admission call/test/rejection.
        raw=b'\x2e\x0f\x84'+struct.pack('<i',target-(address+7))
        changed=body[:m.start(2)]+raw.hex(' ')+' '+body[m.end(2):]
        with self.assertRaises(ValueError):verify_declined_ride_entries(self.assembly.replace(body,changed))

if __name__=='__main__':unittest.main()
