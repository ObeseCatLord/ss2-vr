"""Offline negative controls for request correlation; never runtime VR proof."""
import sys
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from assess_runtime import complete_pairs_for_pose,first_person_depth_probe

HEAD={'p':[0,0,0],'q':[0,0,0,1]}

def fixture(count=30,camera_sequence_offset=0,camera_tracking=7,presentation=1,eyes=(0,1)):
    game=[];host=[]
    for sequence in range(1,count+1):
        for eye in eyes:
            game.append(f'Lab native camera eye={eye} request={sequence+camera_sequence_offset} session=2 reference=3 tracking={camera_tracking} head=0,0,0,0,0,0,1 camera={eye*.064},0,0,0,0,0,1')
        game.append(f'Lab native pair request={sequence} session=2 reference=3 tracking=7 presentation={presentation}')
        host.append(f'Lab projection submitted request={sequence} session=2 reference=3 tracking=7 presentation={presentation}')
    return '\n'.join(game),'\n'.join(host)

class CorrelationChecks(unittest.TestCase):
    def test_distinct_complete_requests(self):
        self.assertEqual(len(complete_pairs_for_pose(*fixture(),HEAD)),30)

    def test_same_pose_other_requests_cannot_certify_capture(self):
        self.assertFalse(complete_pairs_for_pose(*fixture(camera_sequence_offset=100),HEAD))

    def test_repeated_submission_is_one_native_pair(self):
        game,host=fixture(count=1)
        self.assertEqual(len(complete_pairs_for_pose(game,host*100,HEAD)),1)

    def test_generation_and_session_are_not_interchangeable(self):
        self.assertFalse(complete_pairs_for_pose(*fixture(camera_tracking=8),HEAD))
        game,host=fixture()
        self.assertFalse(complete_pairs_for_pose(game,host.replace('session=2','session=4'),HEAD))
        self.assertFalse(complete_pairs_for_pose(game,host.replace('reference=3','reference=4'),HEAD))

    def test_world_only_or_single_eye_is_incomplete(self):
        self.assertFalse(complete_pairs_for_pose(*fixture(presentation=0),HEAD))
        self.assertFalse(complete_pairs_for_pose(*fixture(eyes=(0,)),HEAD))

    def test_changed_pose_does_not_match_held_baseline(self):
        game,host=fixture()
        self.assertFalse(complete_pairs_for_pose(game.replace('head=0,0,0,','head=0.1,0,0,'),host,HEAD))

class DepthProbeChecks(unittest.TestCase):
    @staticmethod
    def fixture(wrong_eye=None):
        rows=[]
        for eye in (0,1,-1):
            rows.append(f'Lab world draw attempts request=3 eye={eye} aborted=0 calls=10 primitives=20 readFailures=0')
            wrong=eye==wrong_eye
            rows.append(f'Lab world draw state request=3 eye={eye} z=1 write=1 alpha=1 blend=0 calls=10 primitives=20 fullRange={10 if wrong else 0} worldRange={0 if wrong else 10} otherRange=0')
        return '\n'.join(rows)

    def test_right_eye_counterexample(self):
        self.assertTrue(first_person_depth_probe(self.fixture())['coherent'])
        report=first_person_depth_probe(self.fixture(wrong_eye=1))
        self.assertFalse(report['coherent'])
        self.assertEqual(report['wrong_ranges'],[{'request':3,'eye':1,'full_range_opaque_calls':10}])

    def test_partial_or_uncertain_observation_cannot_pass(self):
        self.assertFalse(first_person_depth_probe('')['coherent'])
        self.assertFalse(first_person_depth_probe(self.fixture().replace('eye=-1','eye=2'))['coherent'])
        for before,after in [('aborted=0','aborted=1'),('readFailures=0','readFailures=1'),('primitives=20','primitives=21')]:
            self.assertFalse(first_person_depth_probe(self.fixture().replace(before,after,1))['coherent'])
        self.assertFalse(first_person_depth_probe(self.fixture()+'\n'+self.fixture())['coherent'])

if __name__=='__main__':unittest.main()
