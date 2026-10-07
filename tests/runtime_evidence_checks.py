"""Offline negative controls for request correlation; never runtime VR proof."""
import sys
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from assess_runtime import complete_pairs_for_pose

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

if __name__=='__main__':unittest.main()
