"""Finite scalar receipt controls; no gameplay or native execution."""
from pathlib import Path
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from assess_ride_control import assess,assess_join
SOURCE='a'*64
RECORD=f'Lab rideControl schema=1 source={SOURCE} ordinal=1 input=12 generation=2 session=3 reference=4 player=5 ride=6 seat=7 brain=8 thread=9 class=2786648 mode=2 executionAbilities=4 movementAbilities=41 parameterToken=0 renderableToken=0 callbackCalls=1 callbackReturned=1 resourceAssociated=0 frameAssociated=0 steeringApplied=0'
JOIN=f'Lab rideModelJoin schema=1 source={SOURCE} invocation=1 receiver=2 class=2786648 handle=3 thread=4 renderableToken=5 instanceToken=6 innerCalls=1 innerReturned=1 borrowedJoin=1 localRiderAssociated=0 operatedSeatAssociated=0 resourceAssociated=0 frameAssociated=0 steeringApplied=0'
class Checks(unittest.TestCase):
    def test_scalar_scope_and_nullable_association(self):
        result=assess(RECORD,SOURCE)
        self.assertEqual(len(result['observations']),1)
        self.assertEqual(result['observations'][0]['class'],0x2a8558)
        self.assertEqual(result['observations'][0]['movementAbilities'],41)
        self.assertEqual(result['observations'][0]['executionAbilities'],4)
        for key in ('input_application_verified','operated_seat_authority_verified',
                    'installed_resource_association_verified','evaluated_control_frame_verified',
                    'physical_steering_verified','runtime_executed_by_assessor'):
            self.assertFalse(result[key])
        self.assertFalse(assess('ordinary game output',SOURCE)['mode_observed_at_clamp'])
    def test_rejections_cannot_turn_into_association(self):
        changes=(('source='+SOURCE,'source='+'b'*64),('class=2786648','class=2784928'),
            ('schema=1','schema=2'),('callbackCalls=1','callbackCalls=2'),('callbackReturned=1','callbackReturned=0'),
            ('ordinal=1','ordinal=2'),('input=12','input=0'),('generation=2','generation=0'),
            ('resourceAssociated=0','resourceAssociated=1'),('frameAssociated=0','frameAssociated=1'),
            ('steeringApplied=0','steeringApplied=1'),('movementAbilities=41','movementAbilities=-1'),
            ('executionAbilities=4','executionAbilities=4294967296'))
        for before,after in changes:
            with self.subTest(field=before),self.assertRaises(ValueError):assess(RECORD.replace(before,after),SOURCE)
        for bad in (RECORD+' class=2786648',RECORD.replace(' movementAbilities=41',''),RECORD+' unknown=0',
                    'Lab rideControl', 'Lab rideControl schema=1',RECORD+'\n'+RECORD):
            with self.subTest(record=bad),self.assertRaises(ValueError):assess(bad,SOURCE)
    def test_bounded_order_allows_new_source_incarnations(self):
        rows=[RECORD.replace('ordinal=1','ordinal='+str(i)) for i in range(1,65)]
        self.assertEqual(len(assess('\n'.join(rows),SOURCE)['observations']),64)
        with self.assertRaises(ValueError):assess('\n'.join(rows+[RECORD.replace('ordinal=1','ordinal=65')]),SOURCE)
        changed=RECORD.replace('ordinal=1','ordinal=2').replace('session=3','session=10').replace('input=12','input=1')
        self.assertEqual(len(assess(RECORD+'\n'+changed,SOURCE)['observations']),2)
    def test_getter_join_stays_separate_from_render_and_rider(self):
        result=assess_join(JOIN,SOURCE)
        self.assertTrue(result['getter_time_join_observed'])
        for key in ('local_rider_verified','operated_seat_authority_verified',
                    'installed_resource_association_verified','evaluated_control_frame_verified',
                    'physical_steering_verified','runtime_executed_by_assessor'):
            self.assertFalse(result[key])
        self.assertFalse(assess_join(RECORD,SOURCE)['getter_time_join_observed'])
        self.assertFalse(assess(JOIN,SOURCE)['mode_observed_at_clamp'])
        # Failed attempts consume invocation numbers; successful rows can have gaps.
        self.assertEqual(len(assess_join(JOIN+'\n'+JOIN.replace('invocation=1','invocation=64'),SOURCE)['observations']),2)
        for before,after in (('source='+SOURCE,'source='+'b'*64),('invocation=1','invocation=65'),
                ('class=2786648','class=0'),('innerCalls=1','innerCalls=2'),('innerReturned=1','innerReturned=0'),
                ('borrowedJoin=1','borrowedJoin=0'),('receiver=2','receiver=0'),('renderableToken=5','renderableToken=0'),
                ('instanceToken=6','instanceToken=0'),('localRiderAssociated=0','localRiderAssociated=1'),
                ('operatedSeatAssociated=0','operatedSeatAssociated=1'),('resourceAssociated=0','resourceAssociated=1'),
                ('frameAssociated=0','frameAssociated=1'),('steeringApplied=0','steeringApplied=1')):
            with self.subTest(join_field=before),self.assertRaises(ValueError):
                assess_join(JOIN.replace(before,after),SOURCE)
        for bad in ('Lab rideModelJoin',JOIN+' unknown=0',JOIN+' innerCalls=1',JOIN.replace(' handle=3',''),JOIN+'\n'+JOIN):
            with self.subTest(join_record=bad),self.assertRaises(ValueError):assess_join(bad,SOURCE)
if __name__=='__main__':unittest.main()
