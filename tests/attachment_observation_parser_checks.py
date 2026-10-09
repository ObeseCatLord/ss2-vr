import sys
from pathlib import Path
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from collect_attachment_observation import parse
HEADER='Lab sniper attachment schema=1 input=10 generation=2 owner=3 weapon=4 hand=1 nativeId=13 model=5 instance=6 ident=123 calls=1 result=1 calibrationSerial=7 drawRequest=8 drawInput=9 drawEye=0 cacheAge=44 cfg=11 file=12 resource=1 nativeReturn=1,2,3,0,0,0,1'
WORDS=','.join(['00000000']*12)
MATRICES='\n'.join('Lab sniper attachment matrix schema=1 calibrationSerial=7 kind='+kind+' words='+WORDS for kind in ('nativeAttachment','calibrationNativeModel'))
VALID=HEADER+'\n'+MATRICES
class Checks(unittest.TestCase):
    def test_complete(self):
        result=parse(VALID)
        self.assertTrue(result['same_original_call_provenance'])
        for flag in ('draw_animation_equivalence_verified','visual_muzzle_alignment_verified','native_firing_verified','bound_changed'):
            self.assertFalse(result[flag])
        self.assertEqual(len(result['matrices']['nativeAttachment']['values']),12)
    def test_rejected_receipts(self):
        cases=[VALID+'\n'+HEADER,HEADER,VALID.replace('hand=1','hand=0'),VALID.replace('calls=1','calls=2'),
               VALID.replace('result=1','result=0'),VALID.replace('nativeId=13','nativeId=1'),
               VALID.replace('cacheAge=44','cacheAge=101'),VALID.replace('drawEye=0','drawEye=-1'),
               VALID.replace('words='+WORDS,'words=7fc00000,'+','.join(['00000000']*11),1),
               VALID.replace('kind=calibrationNativeModel','kind=nativeAttachment'),
               VALID.replace('calibrationSerial=7 kind=nativeAttachment','calibrationSerial=8 kind=nativeAttachment'),
               VALID.replace('nativeReturn=1,2,3,0,0,0,1','nativeReturn=nan,2,3,0,0,0,1'),
               VALID.replace('input=10','input=10 input=11')]
        cases.extend(VALID+'\n'+record for record in [HEADER.replace('schema=1','schema=2'),
            'Lab sniper attachment schema=1','Lab sniper attachment matrix schema=1',
            'Lab sniper attachment matrix schema=2 words='+WORDS,
            'Lab sniper attachment schema=1 ',HEADER+' '+HEADER])
        for case in cases:
            with self.subTest(case=case),self.assertRaises(ValueError):parse(case)
if __name__=='__main__':unittest.main()
