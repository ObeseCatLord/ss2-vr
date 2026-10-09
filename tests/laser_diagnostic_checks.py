"""Offline consumer controls; no game, logger or native getters are executed."""
from pathlib import Path
import sys
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from assess_laser_diagnostic import assess

BASE = ('Lab laser freeze schema=1 order=1 request=10 input=20 generation=3 owner=4 weapon=5 '
        'hand=0 eye=0 valid=1 sampleValid=1 sampleRequest=10 sampleInput=20 sampleGeneration=3 '
        'sampleOwner=4 sampleWeapon=5 now=1000 tick=950 kinds=0,0 handValid=1 wheel=0 '
        'selecting=0 finite=1,1,1 end=0,0,-5 drift2=0 alignment=1')


class Checks(unittest.TestCase):
    def test_rejection_controls(self):
        for old, new, reason in [
            ('now=1000', 'now=1100', 'sample-age'),
            ('tick=950', 'tick=1001', 'sample-age'),
            ('sampleOwner=4', 'sampleOwner=8', 'sampleOwner-mismatch'),
            ('sampleInput=20', 'sampleInput=19', 'sampleInput-mismatch'),
            ('sampleRequest=10', 'sampleRequest=9', 'sampleRequest-mismatch'),
            ('wheel=0', 'wheel=1', 'wheel-or-selection'),
            ('drift2=0', 'drift2=0.01', 'body-drift'),
            ('sampleValid=1', 'sampleValid=0', 'sample-invalid'),
            ('end=0,0,-5', 'end=nan,0,-5', 'end-nonfinite')]:
            with self.subTest(reason=reason):
                row = BASE.replace('valid=1 ', 'valid=0 ').replace(old, new)
                result = assess([row])
                self.assertEqual(result['freeze_outcomes'], {reason: 1})
                self.assertFalse(result['freeze_predicate_disagreements'])

    def test_positive_and_disagreement(self):
        self.assertEqual(assess([BASE])['freeze_outcomes'], {'eligible': 1})
        self.assertTrue(assess([BASE.replace('tick=950', 'tick=1001')])['freeze_predicate_disagreements'])

    def test_schema_and_missing_evidence(self):
        with self.assertRaises(ValueError):
            assess([BASE.replace('schema=1', 'schema=2')])
        self.assertFalse(assess(['unrelated log line'])['diagnostic_records_present'])

    def test_raw_binding_bits_and_drop_limits(self):
        header = ('Lab laser binding schema=1 order=1 owner=4 weapon=5 model=6 instance=7 '
                  'selector=0 hand=0 cfg=8 file=9 resource=1 stretch=00000000,3f800000,3f800000 ')
        result = assess([header + 'kind=cache', header.replace('00000000', '80000000') + 'kind=current',
                         'Lab laser budget schema=1 order=2 stage=3 dropped=5,0,0,7,0,0',
                         'Lab laser budget schema=1 order=3 stage=3 dropped=4,0,0,8,0,0',
                         'Lab laser saturation schema=1 stage=5 cap=1024 dropped=1'])
        self.assertEqual(result['binding_differences'][0]['changed_fields'], ['stretch'])
        self.assertEqual(result['binding_differences'][0]['difference_mask'], 1 << 9)
        self.assertEqual(result['observed_dropped_lower_bounds'], [5, 0, 0, 8, 0, 1])
        self.assertFalse(result['visible_lasers_verified'])

    def test_vehicle_is_not_guessed(self):
        result = assess([BASE.replace('kinds=0,0', 'kinds=1,1')])
        self.assertEqual(result['freeze_outcomes'], {'vehicle-or-kind-change-unqualified': 1})


if __name__ == '__main__':
    unittest.main()
