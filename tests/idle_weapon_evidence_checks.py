import sys
from pathlib import Path
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from assess_idle_weapon import assess
from match_idle_geometry import match

SOURCE='a'*64
DRAW=f'Lab idle draw schema=3 rawGripValid=1 draws=0 source={SOURCE} ipc=10 wire=7 request=100 input=90 owner=1 weapon=2 model=3 generation=4 hand=0 eye=1 stage=4 cfg=20 file=30 resource=5 contributors=1 matrices=1 historicalBytes=0 grasp=0'
ANIM='Lab idle animation request=100 eye=1 hand=0 index=0 raw=00000001,00000002,00000003,00000004,00000005,00000006,00000007,00001000 header=00000037,00000000,0000000a,3f800000'
MATRICES=[f'Lab idle matrix request=100 eye=1 hand=0 kind={kind} index=0 values=1,0,0,0,0,1,0,0,0,0,1,0' for kind in ['world','nativePlacement','trackedPlacement','controller','canonical','rawAim','rawGrip']]
STRETCH='Lab idle stretch request=100 eye=1 hand=0 values=-1,1,1'
VALID='\n'.join([DRAW,ANIM,*MATRICES,STRETCH])

def geometry_fixture():
    import struct
    def row(kind,chunk,values):
        return 'Lab idle geometryData request=100 eye=1 hand=0 index=0 kind='+kind+' chunk='+str(chunk)+' values='+','.join(f'{x:08x}' for x in values)
    header='Lab idle geometry request=100 eye=1 hand=0 index=0 modelRecord=1 drawRecord=0 surface=40 instance=50 surfaceName=60 boneName=70 bone=0 cfg=20 file=30 resource=5 words=2 constants=1 declaration=5'
    data=[row('affine',0,[0x3f800000,0,0,0,0,0x3f800000,0,0,0,0,0x3f800000,0]),
          row('clip',0,[0]*16),
          row('layout',0,[317,338,0,133,0,0,135,0,12680,128,0,13948,128,0]),
          row('buffers',0,[17752,0,1,100,0,2028,0,1,101,0]),
          row('draw',0,[4,0,0,317,0,338]),
          row('streams',0,[1,0,12,1,1,13948,4,1,1,12680,4,1,1,15216,8,1,2,0]),
          *[row('hash',i,[i+1]*8) for i in range(5)],
          *[row('declaration',i,e) for i,e in enumerate([[0,0,2,0,5,0],[5,0,8,0,5,5],[6,0,8,0,5,6],[3,0,1,0,5,3],[255,0,17,0,0,0]])],row('program',0,[0xfffe0101,0xffff]),row('constant',0,[0,0,0,0])]
    text=VALID.replace('draws=0','draws=1')+'\n'+header+'\n'+'\n'.join(data)
    return text,row,header,data

class Checks(unittest.TestCase):
    def test_input_failure_indices_follow_explicit_or_historical_layout(self):
        from idle_stream_evidence import OBSERVED
        head=DRAW.replace('stage=4','stage=3')+'\nLab idle rejection request=100 eye=1 hand=0 reason=32 preceding=2 checks=0 state=63 callbacks=63\n'
        failure='Lab idle inputFailure request=100 eye=1 hand=0 step=8 index=7 hr=-1 valid=11 caps=256 declaration=6 rangeChecks=0 layout=1'
        rows='\n'.join('Lab idle inputDeclaration request=100 eye=1 hand=0 index='+str(i)+' values='+','.join(map(str,e)) for i,e in enumerate(OBSERVED))
        text=head+failure+'\n'+rows
        for index in (0,3,7,8):
            self.assertFalse(assess(text.replace('index=7 hr=','index='+str(index)+' hr='),SOURCE)['copied_event_pose_observations'])
        for bad in (text.replace('index=7 hr=','index=5 hr='),text.replace('index=7 hr=','index=6 hr='),
                    text.replace(' layout=1',''),text.replace('layout=1','layout=0'),
                    text.replace('layout=1','layout=2'),text.replace('values=7,0,8,0,5,7','values=7,0,8,0,5,5')):
            with self.subTest(bad=bad),self.assertRaises(ValueError):assess(bad,SOURCE)
        historical=text.replace(' layout=1','').replace('index=7 hr=','index=5 hr=')
        self.assertEqual(assess(historical,SOURCE)['rejected_or_missing_observations'][0]['input_failure']['layout'],0)
    def test_rejected_animation_name_snapshot_is_qualified_and_never_a_winner(self):
        rejected=DRAW.replace('stage=4','stage=3').replace('contributors=1','contributors=2').replace('matrices=1','matrices=0')
        reason='Lab idle rejection request=100 eye=1 hand=0 reason=19 preceding=1 checks=0 state=23 callbacks=49'
        name='Lab idle animationNameFailure request=100 eye=1 hand=0 index=1 expected=00000077 header=00000037,00000000,0000000a,3f800000'
        text='\n'.join([rejected,reason,name])
        result=assess(text,SOURCE)
        self.assertFalse(result['copied_event_pose_observations'])
        self.assertEqual(result['rejected_or_missing_observations'][0]['animation_name_failure'],
                         {'index':1,'expected':119,'header':[55,0,10,0x3f800000]})
        self.assertFalse(result['positive_grasp_verified']);self.assertFalse(result['alignment_accepted'])
        for bad in (text+'\n'+name,name,text.replace('reason=19','reason=18'),
                    text.replace('preceding=1','preceding=2'),text.replace('stage=3','stage=4'),
                    text.replace('callbacks=49','callbacks=17'),text.replace('cfg=20','cfg=0'),
                    text.replace('matrices=0','matrices=1'),text.replace('draws=0','draws=1'),
                    text.replace('state=23','state=22'),text.replace('state=23','state=31'),
                    text.replace('state=23','state=55'),text.replace('checks=0','checks=1'),
                    text.replace('file=30','file=0'),text.replace('contributors=2','contributors=1'),
                    text.replace('index=1 expected=','index=16 expected='),
                    text.replace('expected=00000077','expected=00000037'),
                    text.replace('expected=00000077','expected=77'),text.replace('3f800000','3F800000'),
                    text.replace('hand=0 index=1 expected=','hand=1 index=1 expected='),
                    text.replace('header=00000037,00000000,0000000a,3f800000','header=00000037'),
                    text+' extra=1'):
            with self.subTest(bad=bad),self.assertRaises(ValueError):assess(bad,SOURCE)
    def test_input_substeps_do_not_certify_bindings_and_partial_or_unqualified_payloads_reject(self):
        rejected=DRAW.replace('stage=4','stage=3')
        reason='Lab idle rejection request=100 eye=1 hand=0 reason=32 preceding=2 checks=0 state=63 callbacks=63'
        failure='Lab idle inputFailure request=100 eye=1 hand=0 step=7 index=0 hr=0 valid=9 caps=256 declaration=0 rangeChecks=0'
        text=rejected+'\n'+reason+'\n'+failure
        r=assess(text,SOURCE)
        self.assertFalse(r['copied_event_pose_observations'])
        self.assertEqual(r['rejected_or_missing_observations'][0]['input_failure']['step'],7)
        row='Lab idle inputDeclaration request=100 eye=1 hand=0 index=0 values=255,0,17,0,0,0'
        qualified=text.replace('step=7','step=8').replace('hr=0','hr=-1').replace('valid=9','valid=11').replace('declaration=0','declaration=1')+'\n'+row
        self.assertEqual(len(assess(qualified,SOURCE)['rejected_or_missing_observations'][0]['input_failure']['declaration_rows']),1)
        for bad in (text.replace('valid=9','valid=11').replace('declaration=0','declaration=1')+'\n'+row,
                    text+'\n'+row,qualified+'\n'+row,qualified.replace('\n'+row,''),
                    text.replace('valid=9','valid=4'),text.replace('step=7','step=26'),
                    text.replace('rangeChecks=0','rangeChecks=1048576'),text.replace('reason=32','reason=31'),
                    qualified.replace('valid=11','valid=15'),qualified.replace('values=255','values=65536'),qualified.replace('index=0 hr=','index=1 hr='),
                    qualified.replace('index=0 hr=','index=6 hr='),qualified.replace('caps=256','caps=0'),
                    qualified.replace('hr=-1','hr=0'),qualified.replace('rangeChecks=0','rangeChecks=1')):
            with self.subTest(bad=bad),self.assertRaises(ValueError):assess(bad,SOURCE)
    def test_first_rejection_diagnostics_never_promote_geometry_and_require_one_matching_record(self):
        rejected=DRAW.replace('stage=4','stage=3')
        reason='Lab idle rejection request=100 eye=1 hand=0 reason=28 preceding=2 checks=1 state=7 callbacks=31'
        r=assess(rejected+'\n'+reason,SOURCE)
        self.assertFalse(r['copied_event_pose_observations'])
        self.assertEqual(r['rejected_or_missing_observations'][0]['rejection']['reason'],28)
        for text in (reason,rejected+'\n'+reason+'\n'+reason,DRAW+'\n'+reason,
                     rejected+'\n'+reason.replace('request=100','request=101'),
                     rejected+'\n'+reason.replace('preceding=2','preceding=3'),
                     rejected+'\n'+reason.replace('reason=28','reason=47'),
                     rejected+'\n'+reason.replace('callbacks=31','callbacks=64')):
            with self.subTest(text=text),self.assertRaises(ValueError):assess(text,SOURCE)
    def test_complete_copy_is_not_alignment(self):
        r=assess(VALID,SOURCE)
        self.assertEqual(len(r['copied_event_pose_observations']),1)
        self.assertFalse(r['alignment_accepted']);self.assertFalse(r['positive_grasp_verified'])
        self.assertFalse(r['historical_loaded_bytes_verified'])
        self.assertEqual(r['copied_event_pose_observations'][0]['evidence_class'],'event-pose-only')
        self.assertFalse(r['copied_event_pose_observations'][0]['geometry_observed'])
        self.assertEqual(r['copied_event_pose_observations'][0]['stretch'],[-1,1,1])
    def test_consumed_geometry_copy(self):
        import struct
        text,row,header,data=geometry_fixture()
        result=assess(text,SOURCE)
        self.assertFalse(result['alignment_accepted'])
        self.assertEqual(result['copied_event_pose_observations'][0]['geometry'][0]['words'],2)
        from idle_stream_evidence import OBSERVED
        observed=text.replace('declaration=5','declaration=6')
        old_rows=[row('declaration',i,e) for i,e in enumerate([[0,0,2,0,5,0],[5,0,8,0,5,5],[6,0,8,0,5,6],[3,0,1,0,5,3],[255,0,17,0,0,0]])]
        for r in old_rows:observed=observed.replace(r+'\n','')
        observed+='\n'+'\n'.join(row('declaration',i,e) for i,e in enumerate(OBSERVED))
        self.assertEqual(assess(observed,SOURCE)['copied_event_pose_observations'][0]['geometry'][0]['input_layout'],1)
        mixed=observed.replace('values=00000007,00000000,00000008,00000000,00000005,00000007',
                               'values=00000005,00000000,00000008,00000000,00000005,00000005')
        with self.assertRaises(ValueError):assess(mixed,SOURCE)
        channels=('positions','indices','weights','local_indices','uv')
        candidate={'hand':{'asset_sha256':'b'*64,'candidate_channels':[{
            'single_body_influence':True,'mesh_object':1,'lod':0,'vertices':317,'triangles':338,
            'whole_vertex_buffer_bytes':17752,'whole_index_buffer_bytes':2028,
            'channel_ranges':dict(zip(channels,[
                {'offset':0,'size':3804,'format':133,'buffer':0},
                {'offset':0,'size':2028,'format':135,'buffer':0},
                {'offset':12680,'size':1268,'format':128,'buffer':0},
                {'offset':13948,'size':1268,'format':128,'buffer':0},
                {'offset':15216,'size':2536,'format':132,'buffer':0}])),
            'channel_sha256':{name:struct.pack('<8I',*([i+1]*8)).hex() for i,name in enumerate(channels)}}]}}
        self.assertTrue(match(result,candidate)['consumed_channels_all_uniquely_matched'])
        self.assertFalse(match(result,candidate)['alignment_accepted'])
        import copy
        for channel in channels:
            bad=copy.deepcopy(candidate);bad['hand']['candidate_channels'][0]['channel_sha256'][channel]='0'*64
            self.assertFalse(match(result,bad)['consumed_channels_all_uniquely_matched'])
        ambiguous=copy.deepcopy(candidate);ambiguous['other']=ambiguous['hand']
        self.assertEqual(match(result,ambiguous)['matches'][0]['result'],'ambiguous')
        for bad in [text.replace(data[-1],''),text+'\n'+data[0],text.replace('words=2','words=513'),
                    text.replace('bone=0','bone=-1'),text.replace('values=00000000,00000000,00000000,00000000','values=7fc00000,00000000,00000000,00000000'),
                    text.replace(data[3],row('buffers',0,[17752,8,1,100,0,2028,0,1,101,0])),
                    text.replace(data[5],row('streams',0,[1,0,12,1,3,13948,4,1,1,12680,4,1,1,15216,8,1,2,0]))]:
            with self.assertRaises(ValueError):assess(bad,SOURCE)
    def retained_fixture(self,declaration=None):
        from idle_stream_evidence_checks import fixture
        text=(fixture(declaration) if declaration is not None else fixture()).split('Lab idle streamProbe')[0].rstrip().replace('draws=0','draws=1')
        _,_,header,data=geometry_fixture()
        # The stored earlier draw belongs to the same selected binding, not the
        # subsequent failed declaration. Reuse the normal geometry payload shape.
        payload='\n'.join([header,*data]).replace('eye=1 hand=0','eye=0 hand=1')
        payload=payload.replace('Lab idle geometryData','Lab idle retainedGeometryData').replace('Lab idle geometry ','Lab idle retainedGeometry ')
        companion='Lab idle retainedCopies request=100 eye=0 hand=1 count=1 postOriginal=1 cleanupCertified=0 outerCurrent=0'
        return text+'\n'+companion+'\n'+payload
    def test_retained_copy_is_rejected_history_not_completed(self):
        text=self.retained_fixture();r=assess(text,SOURCE)
        self.assertFalse(r['copied_event_pose_observations'])
        retained=r['rejected_or_missing_observations'][0]['retained_copies']
        self.assertEqual(len(retained['geometry']),1)
        self.assertTrue(retained['post_original_return'])
        for key in ('cleanup_certified','outer_current','whole_trace_accepted','alignment_accepted','positive_grasp_verified'):
            self.assertIs(retained[key],False)
        matching=match(r,{})
        self.assertFalse(matching['matches']);self.assertEqual(len(matching['retained_diagnostic_matches']),1)
        self.assertFalse(matching['copied_geometry_coverage_complete'])
        self.assertFalse(matching['consumed_channels_all_uniquely_matched'])
    def test_retained_companion_requires_complete_qualified_payload(self):
        text=self.retained_fixture();lines=text.splitlines()
        for line in lines:
            if 'Lab idle retained' not in line:continue
            with self.subTest(removed=line),self.assertRaises(ValueError):assess('\n'.join(x for x in lines if x!=line),SOURCE)
            with self.subTest(duplicate=line),self.assertRaises(ValueError):assess(text+'\n'+line,SOURCE)
        for bad in (text.replace('count=1 postOriginal','count=2 postOriginal'),text.replace('postOriginal=1','postOriginal=0'),
                    text.replace('cleanupCertified=0','cleanupCertified=1'),text.replace('outerCurrent=0','outerCurrent=1'),
                    text.replace('state=63','state=55'),text.replace('preceding=2','preceding=1'),
                    text.replace('step=20 index=0','step=19 index=0'),text.replace('retainedCopies request=100','retainedCopies request=101'),
                    text.replace('Lab idle retainedGeometry ','Lab idle geometry '),text.replace('Lab idle retainedGeometryData','Lab idle geometryData'),
                    text.replace('matrices=4','matrices=0'),text.replace('contributors=1','contributors=0')):
            with self.subTest(bad=bad),self.assertRaises(ValueError):assess(bad,SOURCE)
    def test_retained_callback_provenance_and_passive_coexistence(self):
        from idle_stream_evidence_checks import fixture
        retained=self.retained_fixture()
        for flags in (0,31,47,55,59,61,62):
            with self.subTest(callbacks=flags),self.assertRaises(ValueError):assess(retained.replace('callbacks=63','callbacks='+str(flags)),SOURCE)
        passive=fixture().replace('draws=0','draws=1')
        both=passive+'\n'+retained[retained.index('Lab idle retainedCopies'):]
        result=assess(both,SOURCE)
        self.assertFalse(result['copied_event_pose_observations'])
        r=result['rejected_or_missing_observations'][0]
        self.assertEqual(r['stream_probe']['flags'],511)
        self.assertEqual(len(r['retained_copies']['geometry']),1)
        # The passive zero-draw rule is lifted only after full companion validation.
        with self.assertRaises(ValueError):assess(passive,SOURCE)
        with self.assertRaises(ValueError):assess(both.replace('count=1 postOriginal','count=2 postOriginal'),SOURCE)
        with self.assertRaises(ValueError):assess(both.replace('outerCurrent=0','outerCurrent=1'),SOURCE)
        with self.assertRaises(ValueError):assess(both[:both.index('Lab idle retainedGeometryData')],SOURCE)
        matching=match(result,{})
        self.assertFalse(matching['copied_geometry_coverage_complete'])
        self.assertFalse(matching['matches'])
    def test_no_uv_retained_and_passive_coexist_without_promotion(self):
        from idle_stream_evidence import NO_UV56
        from idle_stream_evidence_checks import fixture
        retained=self.retained_fixture(NO_UV56)
        both=fixture(NO_UV56).replace('draws=0','draws=1')+'\n'+retained[retained.index('Lab idle retainedCopies'):]
        evidence=assess(both,SOURCE);r=evidence['rejected_or_missing_observations'][0]
        self.assertEqual(set(r['stream_probe']['snapshots'][0]['streams']),{0,5,6})
        self.assertEqual(len(r['retained_copies']['geometry']),1)
        self.assertFalse(evidence['copied_event_pose_observations'])
        self.assertFalse(match(evidence,{})['copied_geometry_coverage_complete'])
    def test_agreeing_diagnostic_replay_cannot_promote_readiness(self):
        import struct,tempfile
        from unittest.mock import patch
        from replay_idle_geometry import replay
        evidence=assess(self.retained_fixture(),SOURCE)
        row={'request':100,'eye':0,'hand':1,'geometry_index':0,'draw_record':0,
             'result':'unique-consumed-channel-match','candidates':[{'candidate':'hand','asset_sha256':'b'*64,'mesh_object':1,'lod':0,'channel_index':0}]}
        candidate={'hand':{'asset_sha256':'b'*64,'candidate_channels':[{'channel_sha256':{}}]}}
        copied={'positions':struct.pack('<3f',0,0,0)*317,'indices':b'\0\0'*1014}
        matching={'matches':[],'retained_diagnostic_matches':[row],'observations_without_geometry':[{}],'copied_geometry_coverage_complete':False}
        with tempfile.TemporaryDirectory() as d,patch('replay_idle_geometry.match',return_value=matching), \
                patch('replay_idle_geometry.channel_bytes',return_value=copied), \
                patch('replay_idle_geometry.evaluate_geometry',return_value={'position_replay_agrees_with_reference':True}):
            result=replay(evidence,candidate,Path(d),Path(d)/'unused',Path(d))
        self.assertEqual(len(result['retained_diagnostic_draws']),1)
        self.assertTrue(result['retained_diagnostic_draws'][0]['position_replay']['position_replay_agrees_with_reference'])
        self.assertIsNone(result['retained_diagnostic_draws'][0]['render_geometry']['raw_grip'])
        self.assertFalse(result['draws']);self.assertFalse(result['copied_geometry_coverage_complete'])
        self.assertFalse(result['all_consumed_positions_agree_with_native_reference'])
        self.assertFalse(result['alignment_accepted'])
    def test_uncalibrated_reference_availability(self):
        grip_row=next(row for row in MATRICES if 'kind=rawGrip ' in row)
        text=VALID.replace('rawGripValid=1','rawGripValid=0').replace(grip_row,'')
        result=assess(text,SOURCE)
        self.assertNotIn('rawGrip:0',result['copied_event_pose_observations'][0]['pose'])
        with self.assertRaises(ValueError):assess(VALID.replace('rawGripValid=1','rawGripValid=0'),SOURCE)
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
                  VALID.replace(STRETCH,''),VALID.replace(ANIM,''),VALID.replace(MATRICES[-1],''),VALID.replace(MATRICES[-2],''),VALID.replace('rawGripValid=1','rawGripValid=2'),
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
