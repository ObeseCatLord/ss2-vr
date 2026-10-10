"""Finite scalar receipt controls; no gameplay or native execution."""
from pathlib import Path
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from assess_ride_control import assess,assess_join,assess_render
SOURCE='a'*64
RECORD=f'Lab rideControl schema=1 source={SOURCE} ordinal=1 input=12 generation=2 session=3 reference=4 player=5 ride=6 seat=7 brain=8 thread=9 class=2786648 mode=2 executionAbilities=4 movementAbilities=41 parameterToken=0 renderableToken=0 callbackCalls=1 callbackReturned=1 resourceAssociated=0 frameAssociated=0 steeringApplied=0'
JOIN=f'Lab rideModelJoin schema=1 source={SOURCE} invocation=1 receiver=2 class=2786648 handle=3 thread=4 renderableToken=5 instanceToken=6 innerCalls=1 innerReturned=1 borrowedJoin=1 localRiderAssociated=0 operatedSeatAssociated=0 resourceAssociated=0 frameAssociated=0 steeringApplied=0'
def render_fixture(schema=2,eyes=(-1,),row=0,bank=1):
    lines=[];words=','.join(['00000000']*12)
    for eye in eyes:
        key=f'schema={schema} row={row} bank={bank} eye={eye}'
        source=f' source={SOURCE}' if schema==2 else ''
        extra=' seatBone=3 seatDefinition=1120' if schema==2 else ''
        lines.append('Lab ride render '+key+source+' player=1 brain=2 ride=3 seat=0 class=2a8558 renderableHandle=4 renderable=5 instance=6 cfg=7 file=8 resource=0 modelRecord=1 evaluated=9 matrices=10 mainBone=2 definition=1000'+extra+' resourceClaim=0 seatClaim=0 graspClaim=0 steeringClaim=0')
        lines.append('Lab ride render binding '+key+' skeleton=11 lod=12 definitions=1000 definitionCount=4 boneFirst=2 boneCount=2 canonicalCount=4 cacheRows=13 cacheRowCount=2')
        for kind in ('modelWorld','MainCanonical')+(('SeatCanonical',) if schema==2 else ()):
            lines.append('Lab ride render matrix '+key+f' kind={kind} words={words}')
    return '\n'.join(lines)

class Checks(unittest.TestCase):
    def test_paired_render_source_mutations_decline(self):
        from verify_remote_render_unwind import verify_ride_frame_source
        text=(Path(__file__).resolve().parents[1]/'src/game/remote_render.cpp').read_text()
        self.assertTrue(verify_ride_frame_source(text)['paired_main_seat_source_checked'])
        for old,new in [('seats!=1','seats>1'),('frame.mainBone==frame.seatBone','false'),
                        ('frame.matrices+frame.seatBone*48','frame.matrices+(frame.seatBone-frame.boneFirst)*48'),
                        ('before!=after','false'),('emit("SeatCanonical",frame.seat);',''),
                        ('seatClaim=0','seatClaim=1'),('stringId(&seatName,"Seat");',''),
                        ('schema=2 source=%.*s','schema=2 source=unknown')]:
            with self.subTest(old=old):
                bad=text.replace(old,new);self.assertNotEqual(bad,text)
                with self.assertRaises(ValueError):verify_ride_frame_source(bad)

    def test_render_reserved_prefix_is_exact(self):
        text=render_fixture()
        for kind in ('schema','binding','matrix'):
            old='Lab ride render '+kind
            for prefix in ('Lab ride rendering ','Lab ride renderX ','Lab  ride render ','Lab ride\trender '):
                bad=text.replace(old,prefix+kind,1);self.assertNotEqual(bad,text)
                with self.assertRaises(ValueError):assess_render(bad,SOURCE)
        for line in ('Lab ride render','Lab ride rendering','Lab ride renderX'):
            with self.assertRaises(ValueError):assess_render(line,SOURCE)

    def test_render_copy_history_and_source_are_distinct(self):
        for schema in (1,2):
            for eyes in ((-1,),(0,1)):
                r=assess_render(render_fixture(schema,eyes),SOURCE)
                self.assertEqual(r['source_matches_expected'],schema==2)
                self.assertEqual(r['observed_source_fingerprint'],SOURCE if schema==2 else None)
                self.assertEqual(r['seat_frame_copies_present'],schema==2)
                for key in ('source_provenance_authenticated','evaluated_control_frame_verified',
                            'operated_seat_authority_verified','physical_steering_verified'):
                    self.assertFalse(r[key])
        self.assertFalse(assess_render('ordinary output',SOURCE)['source_matches_expected'])
    def test_render_eye_cache_evidence_is_independent(self):
        text=render_fixture(eyes=(0,1));at=text.index('Lab ride render schema=2 row=0 bank=1 eye=1')
        # Each eye has a separately owned cache and model index; do not borrow eye0.
        text=text[:at]+text[at:].replace('modelRecord=1','modelRecord=2').replace('cacheRowCount=2','cacheRowCount=3').replace('evaluated=9','evaluated=90').replace('matrices=10','matrices=100').replace('cacheRows=13','cacheRows=130')
        self.assertEqual(len(assess_render(text,SOURCE)['observations']),2)
        raw=render_fixture().replace('kind=SeatCanonical words=00000000','kind=SeatCanonical words=7fc00001')
        self.assertEqual(assess_render(raw,SOURCE)['observations'][0]['raw_matrices']['SeatCanonical'][0],0x7fc00001)
    def test_render_complete_groups_owner_bounds_and_claim_mutations(self):
        text=render_fixture()
        changes=(('source='+SOURCE,'source='+'b'*64),('schema=2','schema=3'),('row=0','row=32'),
                 ('bank=1','bank=0'),('class=2a8558','class=2a4648'),('player=1','player=0'),
                 ('mainBone=2','mainBone=4'),('seatBone=3','seatBone=2'),('seatDefinition=1120','seatDefinition=1121'),
                 ('seatDefinition=1120','seatDefinition=1480'),('boneCount=2','boneCount=3'),
                 ('modelRecord=1','modelRecord=2'),('definitionCount=4','definitionCount=8193'),
                 ('resourceClaim=0','resourceClaim=1'),('seatClaim=0','seatClaim=1'),
                 ('graspClaim=0','graspClaim=1'),('steeringClaim=0','steeringClaim=1'))
        for before,after in changes:
            with self.subTest(field=before),self.assertRaises(ValueError):assess_render(text.replace(before,after),SOURCE)
        for line in text.splitlines():
            with self.assertRaises(ValueError):assess_render('\n'.join(l for l in text.splitlines() if l!=line),SOURCE)
            with self.assertRaises(ValueError):assess_render(text+'\n'+line,SOURCE)
        for eyes in ((0,),(1,),(-1,0,1)):
            with self.assertRaises(ValueError):assess_render(render_fixture(eyes=eyes),SOURCE)
        for before,after in [('ride=3','ride=4'),('cfg=7','cfg=8'),('seat=0','seat=1'),('bank=1','bank=2')]:
            paired=render_fixture(eyes=(0,1));at=paired.index('Lab ride render schema=2 row=0 bank=1 eye=1')
            with self.assertRaises(ValueError):assess_render(paired[:at]+paired[at:].replace(before,after),SOURCE)
        with self.assertRaises(ValueError):assess_render(text.replace('binding schema=2','binding schema=1'),SOURCE)
        with self.assertRaises(ValueError):assess_render(text.replace('words=00000000','words=0'),SOURCE)
        all_rows='\n'.join(render_fixture(row=i,bank=i+1) for i in range(32))
        self.assertEqual(len(assess_render(all_rows,SOURCE)['observations']),32)
        with self.assertRaises(ValueError):assess_render(all_rows+'\n'+render_fixture(row=32,bank=33),SOURCE)
        mixed=render_fixture(schema=1)+ '\n'+render_fixture(schema=2,row=1,bank=2)
        r=assess_render(mixed,SOURCE);self.assertFalse(r['source_matches_expected']);self.assertIsNone(r['observed_source_fingerprint'])

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
