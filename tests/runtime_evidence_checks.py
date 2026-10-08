"""Offline negative controls for request correlation; never runtime VR proof."""
import sys
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from assess_runtime import (complete_pairs_for_pose,first_person_depth_probe,dual_topologies,
    DualPhaseEvidence,require_dual_capture,dual_weapon_events,successful_dual_fire)

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

class DualTopologyChecks(unittest.TestCase):
    def test_topology_is_two_hands_and_exact_identity(self):
        row='Lab native primary topology input=20 session=2 reference=3 tracking=7 owner=9 weapons=10,11 receivers=100,110 hands=3 combo=1 dual=1 flip=0 buttons=1,0 topology_compatible=1'
        self.assertEqual(len(dual_topologies(row,(2,3,7),10,30)),1)
        for before,after in [('tracking=7','tracking=0'),('session=2','session=4'),('reference=3','reference=4'),
            ('hands=3','hands=1'),('weapons=10,11','weapons=10,10'),('receivers=100,110','receivers=100,100'),
            ('combo=1','combo=0'),('buttons=1,0','buttons=0,0'),('topology_compatible=1','topology_compatible=0')]:
            self.assertFalse(dual_topologies(row.replace(before,after),(2,3,7),10,30))

class DualPhaseChecks(unittest.TestCase):
    @staticmethod
    def sample(sequence,tick,ui,trigger=(0,0),fire=(2,3)):
        return {'input_sequence':sequence,'input_tick_ms':tick,'ui_tick_ms':ui,
            'trigger':list(trigger),'fire_sequence':list(fire),'session':2,'reference':3,
            'tracking_generation':7,'primary_generations':[8,9],
            'hand_valid':[1,1],'primary_active_mask':3}

    def test_convergence_can_wait_but_later_contradiction_fails(self):
        phase=DualPhaseEvidence(10,[0,1])
        phase.observe(self.sample(11,100,90))
        phase.observe(self.sample(12,110,100,(0,1)))
        with self.assertRaises(RuntimeError):phase.observe(self.sample(13,120,110))

    def test_repeated_inputs_or_regression_cannot_certify_progress(self):
        phase=DualPhaseEvidence(10,[0,0])
        phase.observe(self.sample(11,100,90));phase.observe(self.sample(11,100,90))
        with self.assertRaises(RuntimeError):phase.require_progress()
        phase.observe(self.sample(13,120,110))
        with self.assertRaises(RuntimeError):phase.observe(self.sample(12,110,100))

    def test_neutral_uses_post_input_ui_and_rejects_resumed_fire(self):
        phase=DualPhaseEvidence(10,[0,0])
        for n in range(3):phase.observe(self.sample(11+n,100+n,97+n))
        with self.assertRaises(RuntimeError):phase.require_stopped()
        for n in range(3):phase.observe(self.sample(14+n,110+n,105+n))
        phase.require_stopped()
        phase.observe(self.sample(17,115,109,fire=(3,3)))
        with self.assertRaises(RuntimeError):phase.require_stopped()

    def test_eye_capture_requires_confirmed_phase_and_frozen_identity(self):
        phase=DualPhaseEvidence(10,[0,1])
        first=self.sample(12,110,100,(0,1));after=self.sample(15,130,120,(0,1))
        phase.observe(first);phase.observe(after)
        metadata=self.sample(14,120,110,(0,1))
        identity=(2,3,7,8,9)
        require_dual_capture(metadata,phase,identity,after)
        for key,value in [('input_sequence',11),('input_sequence',16),('input_tick_ms',109),
            ('trigger',[1,1]),('session',4),('reference',4),('tracking_generation',8),
            ('primary_generations',[9,9]),('hand_valid',[1,0]),('primary_active_mask',1)]:
            with self.assertRaises(RuntimeError):require_dual_capture(metadata|{key:value},phase,identity,after)
        with self.assertRaises(RuntimeError):require_dual_capture(metadata,phase,identity,after|{'trigger':[0,0]})

    def test_charged_release_can_fire_then_requires_a_measured_quiet_interval(self):
        phase=DualPhaseEvidence(10,[0,0])
        phase.observe(self.sample(11,100,110,fire=(2,3)))
        phase.observe(self.sample(12,120,130,fire=(3,3)))
        for n in range(4):phase.observe(self.sample(13+n,200+n*400,210+n*400,fire=(3,3)))
        phase.require_quiet_after(100)
        with self.assertRaises(RuntimeError):phase.require_quiet_after(1000)
        phase.observe(self.sample(17,1800,1810,fire=(4,3)))
        with self.assertRaises(RuntimeError):phase.require_quiet_after(100)

    def test_zap_receipts_bind_release_controls_identity_and_native_pair(self):
        topology=[{'owner':9,'weapon':[10,11],'receiver':[100,110]}]
        row='Lab native weapon event=release input=20 session=2 reference=3 tracking=7 owner=9 weaponHint=10 receiver=100 id=1 hand=0 stateBefore=5 returned=1 aborted=0 result=0 tick=100 actions=8,9 trigger=0,1 completion=120'
        args=((2,3,7,8,9),10,30,topology,'release')
        self.assertEqual(dual_weapon_events(row,*args)[0]['trigger'],[0,1])
        for before,after in [('actions=8,9','actions=8,10'),('id=1','id=12'),('returned=1','returned=0'),
            ('aborted=0','aborted=1'),('owner=9','owner=8'),('receiver=100','receiver=110'),
            ('result=0','result=1'),('trigger=0,1','trigger=-1,1'),('completion=120','completion=99')]:
            self.assertFalse(dual_weapon_events(row.replace(before,after),*args))

    def test_fire_before_confirmed_rise_cannot_supply_new_cycle(self):
        phase=DualPhaseEvidence(10,[0,1])
        phase.observe(self.sample(11,100,90))
        phase.observe(self.sample(12,110,100,(0,1)))
        phase.observe(self.sample(13,120,110,(0,1)))
        topology=[{'owner':9,'weapon':[10,11],'receiver':[100,110]}]
        row='Lab native weapon event=fire input=11 session=2 reference=3 tracking=7 owner=9 weaponHint=11 receiver=110 id=1 hand=1 stateBefore=5 returned=1 aborted=0 result=1 tick=100 actions=8,9 trigger=0,0 completion=120'
        args=((2,3,7,8,9),phase.confirmed_boundary(),21,topology)
        self.assertFalse(successful_dual_fire(row,*args))
        self.assertEqual(successful_dual_fire(row.replace('input=11','input=12').replace('trigger=0,0','trigger=0,1'),*args),{1})

if __name__=='__main__':unittest.main()
