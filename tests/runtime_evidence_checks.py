"""Offline negative controls for request correlation; never runtime VR proof."""
import sys
import unittest
import hashlib
import json
import tempfile
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from assess_runtime import (complete_pairs_for_pose,first_person_depth_probe,dual_topologies,
    DualPhaseEvidence,require_dual_capture,dual_weapon_events,successful_dual_fire)
from runtime_lab import (native_grip_resource_receipts,validate_idle_probe,validate_idle_preparation,idle_probe_weapon,validate_sniper_destination,sniper_preparation_receipt,
                         idle_configuration_digest,IDLE_FIXED_FILES,IDLE_TOOLS)

HEAD={'p':[0,0,0],'q':[0,0,0,1]}

class IdleProbeSelectionChecks(unittest.TestCase):
    def test_stationary_pinned_collection_and_mode_separation(self):
        cfg={'idle_weapon_probe':True,'expected_product_source':'a'*64,
             'baseline_head':[0,1.6,0,0,0,0],
             'pose_steps':[{'name':'baseline','head':[0,1.6,0,0,0,0]}]}
        validate_idle_probe(cfg)
        for key,value in [('idle_weapon_probe',1),('renderer_mode','stock'),('expected_product_source','a'),
                          ('native_dual_probe','zap-initial-inventory'),('background_controls_probe','joystick-native-movement'),
                          ('native_input_probe','read-only-three-samples'),('grip_resource_probe',True),
                          ('pose_steps',cfg['pose_steps']+[{'name':'turned','head':[0,1.6,0,1,0,0]}])]:
            with self.subTest(key=key),self.assertRaises(ValueError):validate_idle_probe({**cfg,key:value})
        validate_idle_probe({})

    def test_exact_idle_weapon_selector_and_disabled_rejection(self):
        cfg={'idle_weapon_probe':True,'expected_product_source':'a'*64,
             'baseline_head':[0,1.6,0,0,0,0],
             'pose_steps':[{'name':'baseline','head':[0,1.6,0,0,0,0]}]}
        self.assertEqual(idle_probe_weapon(cfg),1)
        self.assertIsNone(idle_probe_weapon({}))
        for selected in (1,13):
            with self.subTest(selected=selected):
                validate_idle_probe({**cfg,'idle_native_id':selected})
                self.assertEqual(idle_probe_weapon({**cfg,'idle_native_id':selected}),selected)
        for selected in (True,False,0,2,14,13.0,'13',None):
            with self.subTest(selected=selected),self.assertRaises(ValueError):
                validate_idle_probe({**cfg,'idle_native_id':selected})
        for enabled in (False,None):
            disabled={'idle_native_id':13}
            if enabled is not None:disabled['idle_weapon_probe']=enabled
            with self.assertRaises(ValueError):validate_idle_probe(disabled)

    def test_sniper_preparation_needs_exact_neutral_selector(self):
        cfg={'idle_weapon_probe':True,'idle_native_id':13,'prepare_sniper_fixture':True,
             'expected_product_source':'a'*64,'baseline_head':[0,1.6,0,0,0,0],
             'pose_steps':[{'name':'baseline','head':[0,1.6,0,0,0,0]}]}
        validate_idle_probe(cfg)
        for change in ({'idle_native_id':1},{'idle_weapon_probe':False},
                       {'prepare_sniper_fixture':1},{'prepare_sniper_fixture':None},
                       {'native_dual_probe':'zap-initial-inventory'}):
            with self.subTest(change=change),self.assertRaises(ValueError):validate_idle_probe({**cfg,**change})

    def test_sniper_save_destination_collision_and_redirection(self):
        with tempfile.TemporaryDirectory() as d:
            lab=Path(d)/'game';lab.mkdir();destination=lab/'Temp/SS2VR'
            with self.assertRaises(ValueError):validate_sniper_destination(lab)
            destination.mkdir(parents=True);validate_sniper_destination(lab)
            for name in ('sniper-id13.sav','sniper-id13.sav.preload'):
                p=destination/name;p.write_bytes(b'preserve')
                with self.assertRaises(ValueError):validate_sniper_destination(lab)
                self.assertEqual(p.read_bytes(),b'preserve');p.unlink()
                p.symlink_to(destination/'absent')
                with self.assertRaises(ValueError):validate_sniper_destination(lab)
                p.unlink()
            destination.rmdir();destination.symlink_to(Path(d))
            with self.assertRaises(ValueError):validate_sniper_destination(lab)

    def test_sniper_receipt_requires_order_owner_and_actual_private_files(self):
        with tempfile.TemporaryDirectory() as d:
            lab=Path(d)/'game';destination=lab/'Temp/SS2VR';destination.mkdir(parents=True)
            path=destination/'sniper-id13.sav';path.write_bytes(b'offline test fixture')
            win='Z:'+str(lab).replace('/','\\')+'\\Temp\\SS2VR\\sniper-id13.sav'
            grant='Lab sniper preparation issued stage=grant owner=12\n'
            select='Lab sniper preparation issued stage=select owner=12\n'
            save='Lab sniper preparation issued stage=save owner=12\n'
            complete='Lab sniper preparation complete owner=12 nativeId=13 path='+win+'\n'
            log=grant+select+save+complete
            report=sniper_preparation_receipt(log,lab)
            self.assertEqual(report['files']['Temp/SS2VR/sniper-id13.sav']['sha256'],hashlib.sha256(path.read_bytes()).hexdigest())
            self.assertFalse(report['alignment_accepted']);self.assertFalse(report['save_reload_verified'])
            for bad in (select+grant+save+complete,log+complete,log.replace('stage=save owner=12','stage=save owner=13'),
                        log.replace(win,'Z:\\wrong\\sniper-id13.sav'),log+'Lab sniper preparation failed reason=owner\n'):
                with self.subTest(log=bad),self.assertRaises(ValueError):sniper_preparation_receipt(bad,lab)
            companion=destination/'sniper-id13.sav.preload';companion.write_bytes(b'private companion')
            self.assertEqual(len(sniper_preparation_receipt(log,lab)['files']),2)
            companion.unlink();companion.symlink_to(destination/'missing')
            with self.assertRaises(ValueError):sniper_preparation_receipt(log,lab)
            companion.unlink();path.write_bytes(b'')
            with self.assertRaises(ValueError):sniper_preparation_receipt(log,lab)

class IdlePreparationChecks(unittest.TestCase):
    def prepared(self,root):
        game=root/'game';prefix=root/'prefix'
        for relative in IDLE_FIXED_FILES:
            p=root/relative;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(b'private-fixture')
        users=prefix/'pfx/drive_c/users/steamuser';users.mkdir(parents=True)
        (users/'Documents').mkdir();(users/'My Documents').symlink_to('Documents')
        mappings=prefix/'pfx/dosdevices';mappings.mkdir()
        (mappings/'c:').symlink_to('../drive_c');(mappings/'z:').symlink_to('/')
        cfg={'idle_weapon_probe':True,'expected_product_source':'a'*64,'game_lab':str(game),'prefix':str(prefix)}
        record={'schema':1,'game_lab':str(game),'prefix':str(prefix),'source_fingerprint':'a'*64,
                'configuration_sha256':idle_configuration_digest(cfg),
                'user_links':{'prefix/pfx/drive_c/users/steamuser/My Documents':'Documents'},
                'files':{p:hashlib.sha256((root/p).read_bytes()).hexdigest() for p in IDLE_FIXED_FILES},
                'source_tools':{name:hashlib.sha256((Path(__file__).resolve().parents[1]/'tools'/name).read_bytes()).hexdigest() for name in IDLE_TOOLS}}
        path=root/'preparation.json'
        def seal():
            path.write_text(json.dumps(record));cfg['idle_preparation']={'path':str(path),'sha256':hashlib.sha256(path.read_bytes()).hexdigest()}
        seal()
        return cfg,record,seal,game,prefix

    def test_prepared_fixture_and_configuration_tampering(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);cfg,record,seal,game,prefix=self.prepared(root)
            validate_idle_preparation(cfg,root,game,prefix)
            validate_idle_preparation({**cfg,'compiled_product_contract':{},'verified_scene_providers':[]},root,game,prefix)
            for bad in ({**cfg,'timeout':180},{**cfg,'idle_weapon_probe':False},
                        {**cfg,'idle_native_id':13},
                        {k:v for k,v in cfg.items() if k!='idle_weapon_probe'},
                        {**cfg,'idle_weapon_probe':False,'native_dual_probe':'zap-initial-inventory'},
                        {**cfg,'idle_preparation':{}},
                        {**cfg,'idle_preparation':{**cfg['idle_preparation'],'sha256':'0'*64}}):
                with self.assertRaises(ValueError):validate_idle_preparation(bad,root,game,prefix)
            record['source_tools']['assess_idle_weapon.py']='0'*64;seal()
            with self.assertRaises(ValueError):validate_idle_preparation(cfg,root,game,prefix)

    def test_settings_inventory_and_profile_changes(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);cfg,record,seal,game,prefix=self.prepared(root)
            p=game/'Content/SeriousSam2/Config/new.cfg';p.parent.mkdir(parents=True);p.write_text('changed')
            with self.assertRaises(ValueError):validate_idle_preparation(cfg,root,game,prefix)
            p.unlink();p=game/'Content/PlayerProfiles/profile.dat';p.parent.mkdir();p.write_bytes(b'profile')
            with self.assertRaises(ValueError):validate_idle_preparation(cfg,root,game,prefix)
            p.unlink();(game/'Bin/SS2VR/SS2VR.ini').write_bytes(b'changed')
            with self.assertRaises(ValueError):validate_idle_preparation(cfg,root,game,prefix)

    def test_external_user_redirection_and_device_mapping(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);cfg,record,seal,game,prefix=self.prepared(root)
            redirect=prefix/'pfx/drive_c/users/steamuser/My Documents'
            redirect.unlink();redirect.symlink_to(root)
            with self.assertRaises(ValueError):validate_idle_preparation(cfg,root,game,prefix)
            redirect.unlink();redirect.symlink_to('./Documents')
            with self.assertRaises(ValueError):validate_idle_preparation(cfg,root,game,prefix)
            redirect.unlink()
            with self.assertRaises(ValueError):validate_idle_preparation(cfg,root,game,prefix)
            redirect.symlink_to('Documents')
            extra=redirect.parent/'Another Alias';extra.symlink_to('Documents')
            with self.assertRaises(ValueError):validate_idle_preparation(cfg,root,game,prefix)
            extra.unlink()
            mapping=prefix/'pfx/dosdevices/e:';mapping.symlink_to(root)
            with self.assertRaises(ValueError):validate_idle_preparation(cfg,root,game,prefix)
            mapping.unlink();record['files']['../escape']='a'*64;seal()
            with self.assertRaises(ValueError):validate_idle_preparation(cfg,root,game,prefix)


class ResourceReceiptChecks(unittest.TestCase):
    @staticmethod
    def receipts():
        return '\n'.join(f'Lab native grip resource key={key} bytes=100 positionRestored=0 sha256={key:064x}' for key in range(1,6))

    def test_complete_opens_and_repeated_identical_version(self):
        log=self.receipts()
        values=native_grip_resource_receipts(log+'\n'+log.splitlines()[0])
        self.assertEqual(len(values),5)
        self.assertEqual(values[0]['open_count'],2)
        self.assertEqual(values[-1]['resource'],'R_Hand.bmf')

    def test_partial_conflicting_saturated_or_invalid_receipts_fail(self):
        log=self.receipts()
        conflicts=log+'\n'+log.splitlines()[0].replace('bytes=100','bytes=101')
        for bad in ('',log.splitlines()[0],conflicts,
                    log+'\nLab native grip resource saturated key=1',
                    log+'\n'+log.splitlines()[0]+'\n'+log.splitlines()[0],
                    log.replace('positionRestored=0','positionRestored=101',1),
                    log.replace('bytes=100','bytes=33554433',1),
                    log.replace('key=5','key=6',1)):
            with self.subTest(bad=bad),self.assertRaises(ValueError):
                native_grip_resource_receipts(bad)

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
