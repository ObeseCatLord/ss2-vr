import sys
from pathlib import Path
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from assess_idle_weapon import assess

SOURCE='a'*64
DRAW=f'Lab idle draw source={SOURCE} ipc=10 wire=7 request=100 input=90 owner=1 weapon=2 model=3 generation=4 hand=0 eye=1 stage=4 cfg=20 file=30 resource=5 contributors=1 matrices=1 historicalBytes=0 grasp=0'
ANIM='Lab idle animation request=100 eye=1 hand=0 index=0 raw=00000001,00000002,00000003,00000004,00000005,00000006,00000007,00001000 header=00000037,00000000,0000000a,3f800000'
MATRICES=[f'Lab idle matrix request=100 eye=1 hand=0 kind={kind} index=0 values=1,0,0,0,0,1,0,0,0,0,1,0' for kind in ['world','nativePlacement','trackedPlacement','controller','canonical']]
STRETCH='Lab idle stretch request=100 eye=1 hand=0 values=-1,1,1'
VALID='\n'.join([DRAW,ANIM,*MATRICES,STRETCH])

class Checks(unittest.TestCase):
    def test_complete_copy_is_not_alignment(self):
        r=assess(VALID,SOURCE)
        self.assertEqual(len(r['copied_event_pose_observations']),1)
        self.assertFalse(r['alignment_accepted']);self.assertFalse(r['positive_grasp_verified'])
        self.assertFalse(r['historical_loaded_bytes_verified'])
        self.assertEqual(r['copied_event_pose_observations'][0]['stretch'],[-1,1,1])
    def test_rejection_is_not_truncated_success(self):
        r=assess(DRAW.replace('stage=4','stage=3'),SOURCE)
        self.assertFalse(r['copied_event_pose_observations'])
        self.assertEqual(len(r['rejected_or_missing_observations']),1)
    def test_native_numeric_boundaries(self):
        r=assess(VALID.replace('request=100','request=18446744073709551615')
            .replace('file=30','file=4294967295').replace('resource=5','resource=-2147483648')
            .replace('values=-1,1,1','values=-3.40282347e38,1.40129846e-45,0'),SOURCE)
        self.assertEqual(len(r['copied_event_pose_observations']),1)
    def test_hostile_or_incomplete(self):
        variants=[VALID.replace(SOURCE,'b'*64),VALID.replace('grasp=0','grasp=1'),
                  VALID.replace('historicalBytes=0','historicalBytes=1'),VALID.replace('matrices=1','matrices=65'),
                  VALID.replace(STRETCH,''),VALID.replace(ANIM,''),VALID.replace(MATRICES[-1],''),
                  VALID+'\n'+ANIM,VALID+'\n'+DRAW,VALID.replace('index=0 raw=','index=1 raw='),
                  VALID.replace('kind=world','kind=unknown'),VALID.replace('values=-1,1,1','values=nan,1,1'),
                  VALID.replace('eye=1 hand=0 index=0 raw=','eye=0 hand=0 index=0 raw='),
                  ANIM+'\n'+VALID,VALID.replace('stage=4','stage=3'),
                  VALID.replace('cfg=20','cfg=0'),VALID.replace('wire=7','wire=6'),
                  VALID.replace('request=100','request=18446744073709551616'),
                  VALID.replace('file=30','file=4294967296'),VALID.replace('resource=5','resource=2147483648'),
                  VALID.replace('values=-1,1,1','values=1e100,1,1'),
                  VALID.replace('values=-1,1,1','values=1e-400,1,1'),VALID+'\nLab idle',VALID+'\nLab idle ']
        for v in variants:
            with self.subTest(v=v[:100]):
                with self.assertRaises((ValueError,KeyError)):assess(v,SOURCE)

if __name__=='__main__':unittest.main()
