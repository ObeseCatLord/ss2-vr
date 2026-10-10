"""Private offline collection wrapper controls; never launches a game."""
from pathlib import Path
import hashlib
import json
import sys
import tempfile
import unittest
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
sys.path.insert(0,str(Path(__file__).resolve().parent))
from collect_idle_evidence import collect,ROOT,palette_diagnostics
from idle_weapon_evidence_checks import VALID,SOURCE


class Checks(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory()
        self.root=Path(self.temp.name)
        self.log=self.root/'input.log';self.log.write_text(VALID)
        self.index=self.root/'index.json';self.index.write_text('{}')
        self.evaluator=self.root/'offline-executable';self.evaluator.write_bytes(b'not executed')
        self.digest=hashlib.sha256(self.evaluator.read_bytes()).hexdigest()
        self.output=self.root/'fresh-output'
    def tearDown(self):self.temp.cleanup()
    def run_collect(self,**changes):
        params=dict(log=self.log,candidates=self.index,private_root=self.root,output=self.output,
                    expected_source=SOURCE,native_id=1,evaluator=self.evaluator,evaluator_sha=self.digest)
        params.update(changes);return collect(**params)
    def test_pose_only_is_not_alignment_or_native_execution(self):
        with patch('subprocess.run',side_effect=AssertionError('Unexpected execution')):
            result=self.run_collect()
        self.assertEqual(result['qualified_unique_draws'],0)
        self.assertFalse(result['alignment_accepted']);self.assertFalse(result['positive_grasp_verified'])
        self.assertFalse(result['native_shader_or_world_execution_by_collector'])
        self.assertEqual((self.output/'input.log').read_bytes(),self.log.read_bytes())
        self.assertTrue((self.output/'input-receipt.json').is_file())
    def test_palette_pair_diagnostics_require_same_request_identity_and_copied_palette(self):
        import copy
        base={'request':1,'eye':0,'hand':1,'nativeId':2,'submission_index':3,'ordinal':4,
              'reference_kind':'id2-owned-cold-native-staged-palette',
              'candidates':[{'candidate':'gun','asset_sha256':'b'*64,'mesh_object':2,'lod':0,'channel_index':0}],
              'result':'unique-copied-channel-match','position_replay':{'position_replay_agrees_with_reference':True},
              'whole_trace_accepted':False,'api_geometry_coverage_complete':False,'gpu_execution':False,
              'positive_grasp_verified':False,'alignment_accepted':False,
              'binding':{'request':1,'input':2,'owner':3,'weapon':4,'model':5,'generation':6,'hand':1,'eye':0,'nativeId':2},
              'palette_identity':{'instance':10,'surface':11,'count':1,'root':[12,13,14],'render':[15,16,17],
                                  'world':[0]*12,'canonical':[[0]*12]},
              'native_reference':{'submission_index':3,'ordinal':4,'projection_sequence':1,'locals':[[0]*12]}}
        right=copy.deepcopy(base);right['eye']=right['binding']['eye']=1
        result=palette_diagnostics({'palette_draws':[base,right]},2)
        self.assertEqual(len(result['same_request_palette_reference_cohorts']),1)
        self.assertEqual(len(result['qualified_palette_reference_draws']),2)
        for field in ('palette_api_coverage_complete','palette_gpu_visibility_verified','palette_positive_grasp_verified','palette_alignment_accepted'):
            self.assertFalse(result[field])
        # Multiple material submissions form one cohort, not a draw bijection.
        second_left=copy.deepcopy(base);second_right=copy.deepcopy(right)
        for row in (second_left,second_right):
            row['submission_index']+=1;row['ordinal']+=1
            row['native_reference']['submission_index']+=1;row['native_reference']['ordinal']+=1
        multi=palette_diagnostics({'palette_draws':[base,second_left,right,second_right]},2)
        cohorts=multi['same_request_palette_reference_cohorts']
        self.assertEqual(len(cohorts),1);self.assertEqual(len(cohorts[0]['draws']),4)
        for section,key in [('binding','request'),('binding','input'),('binding','owner'),('binding','weapon'),
                ('binding','model'),('binding','generation'),('binding','hand'),('palette_identity','instance'),
                ('palette_identity','surface'),('palette_identity','count')]:
            bad=copy.deepcopy(right);bad[section][key]+=1
            if key in ('request','hand'):bad[key]=bad[section][key]
            self.assertFalse(palette_diagnostics({'palette_draws':[base,bad]},2)['same_request_palette_reference_cohorts'])
        for field in ('world','canonical','root','render'):
            bad=copy.deepcopy(right)
            if field=='canonical':bad['palette_identity'][field][0][0]+=1;bad['native_reference']['locals']=bad['palette_identity'][field]
            else:bad['palette_identity'][field][0]+=1
            self.assertFalse(palette_diagnostics({'palette_draws':[base,bad]},2)['same_request_palette_reference_cohorts'])
        for mode in ('unmatched','reference','cpu'):
            bad=copy.deepcopy(right)
            if mode=='unmatched':bad['result']='unmatched'
            elif mode=='reference':bad['reference_kind']='projection-association-unavailable'
            else:bad['position_replay']['position_replay_agrees_with_reference']=False
            self.assertFalse(palette_diagnostics({'palette_draws':[base,bad]},2)['same_request_palette_reference_cohorts'])
        for field in ('whole_trace_accepted','api_geometry_coverage_complete','gpu_execution','positive_grasp_verified','alignment_accepted'):
            bad=copy.deepcopy(base);bad[field]=True
            with self.assertRaises(ValueError):palette_diagnostics({'palette_draws':[bad]},2)
        with self.assertRaises(ValueError):palette_diagnostics({'palette_draws':[base]},13)
        bad=copy.deepcopy(base);bad['native_reference']['ordinal']=3
        with self.assertRaises(ValueError):palette_diagnostics({'palette_draws':[bad]},2)

    def test_wrong_weapon_and_fingerprint_do_not_create_output(self):
        for changes in ({'native_id':13},{'evaluator_sha':'0'*64},{'expected_source':'b'*64}):
            with self.subTest(changes=changes),self.assertRaises(ValueError):self.run_collect(**changes)
            self.assertFalse(self.output.exists())
    def test_collision_and_escaping_output(self):
        self.output.mkdir()
        with self.assertRaises(ValueError):self.run_collect()
        with self.assertRaises(ValueError):self.run_collect(output=self.root.parent/'outside-root')
    def test_input_symlink_is_rejected(self):
        alias=self.root/'alias.log';alias.symlink_to(self.log)
        with self.assertRaises(ValueError):self.run_collect(log=alias)
        self.assertFalse(self.output.exists())
    def test_source_ancestor_root_is_rejected_before_admission(self):
        with self.assertRaisesRegex(ValueError,'disjoint from source'):
            self.run_collect(private_root=ROOT.parent)
        self.assertFalse(self.output.exists())
    def test_replay_deadline_failure_preserves_copied_inputs(self):
        with patch('collect_idle_evidence.replay',side_effect=TimeoutError('Offline replay deadline exceeded')):
            with self.assertRaises(TimeoutError):self.run_collect()
        self.assertEqual(json.loads((self.output/'failure.json').read_text())['error_type'],'TimeoutError')
        self.assertTrue((self.output/'input-receipt.json').is_file())
        self.assertEqual((self.output/'input.log').read_bytes(),self.log.read_bytes())

    def test_failure_preserves_owned_receipt_and_snapshot(self):
        with patch('collect_idle_evidence.replay',side_effect=ValueError('failed replay')):
            with self.assertRaises(ValueError):self.run_collect()
        failure=json.loads((self.output/'failure.json').read_text())
        self.assertTrue(failure['preserve_output']);self.assertFalse(failure['game_launched'])
        self.assertEqual((self.output/'input.log').read_bytes(),self.log.read_bytes())
    def test_explicit_sniper_label(self):
        self.log.write_text(VALID.replace('schema=3','schema=4 copyLayout=1 nativeId=13'))
        result=self.run_collect(native_id=13)
        self.assertEqual(result['native_id'],13);self.assertEqual(result['qualified_unique_draws'],0)
    def test_replay_uses_fingerprinted_executable_snapshot(self):
        def inspect(evidence,index,candidate_root,evaluator,temporary,deadline=None):
            self.assertIsNotNone(deadline)
            self.evaluator.write_bytes(b'replaced after verification')
            self.assertNotEqual(evaluator,self.evaluator)
            self.assertEqual(evaluator.read_bytes(),b'not executed')
            self.assertEqual(evaluator.stat().st_mode & 0o777,0o700)
            self.assertTrue(evaluator.is_relative_to(temporary))
            return {'draws':[],'palette_draws':[]}
        with patch('collect_idle_evidence.replay',side_effect=inspect):
            self.run_collect()
        self.assertFalse(any(self.output.glob('owned-arithmetic-*')))
    def test_summary_qualification_and_retained_exclusion(self):
        base={'request':1,'eye':0,'hand':1,'nativeId':1,'geometry_index':0,
              'reference_kind':'uploaded-transform-corroboration','candidates':[{'candidate':'gun'}],
              'result':'unique-consumed-channel-match',
              'position_replay':{'position_replay_agrees_with_reference':True}}
        rows=[base,{**base,'geometry_index':1,'result':'ambiguous'},
              {**base,'geometry_index':2,'reference_kind':'uploaded-transform-uncorroborated'},
              {**base,'geometry_index':3,'position_replay':{'position_replay_agrees_with_reference':False}}]
        with patch('collect_idle_evidence.replay',return_value={'draws':rows,'retained_diagnostic_draws':[base],'palette_draws':[]}):
            result=self.run_collect()
        self.assertEqual(result['qualified_unique_draws'],1)
        self.assertEqual(result['qualified_draws'][0]['geometry_index'],0)
        self.assertFalse(result['alignment_accepted']);self.assertFalse(result['positive_grasp_verified'])
        with patch('collect_idle_evidence.replay',return_value={'draws':[],'retained_diagnostic_draws':[base],'palette_draws':[]}):
            retained=self.run_collect(output=self.root/'retained-only')
        self.assertEqual(retained['qualified_unique_draws'],0)
        self.assertEqual(retained['qualified_draws'],[])


if __name__=='__main__':unittest.main()
