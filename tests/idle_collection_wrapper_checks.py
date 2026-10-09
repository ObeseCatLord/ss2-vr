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
from collect_idle_evidence import collect,ROOT
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
        def inspect(evidence,index,candidate_root,evaluator,temporary):
            self.evaluator.write_bytes(b'replaced after verification')
            self.assertNotEqual(evaluator,self.evaluator)
            self.assertEqual(evaluator.read_bytes(),b'not executed')
            self.assertEqual(evaluator.stat().st_mode & 0o777,0o700)
            self.assertTrue(evaluator.is_relative_to(temporary))
            return {'draws':[]}
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
        with patch('collect_idle_evidence.replay',return_value={'draws':rows,'retained_diagnostic_draws':[base]}):
            result=self.run_collect()
        self.assertEqual(result['qualified_unique_draws'],1)
        self.assertEqual(result['qualified_draws'][0]['geometry_index'],0)
        self.assertFalse(result['alignment_accepted']);self.assertFalse(result['positive_grasp_verified'])
        with patch('collect_idle_evidence.replay',return_value={'draws':[],'retained_diagnostic_draws':[base]}):
            retained=self.run_collect(output=self.root/'retained-only')
        self.assertEqual(retained['qualified_unique_draws'],0)
        self.assertEqual(retained['qualified_draws'],[])


if __name__=='__main__':unittest.main()
