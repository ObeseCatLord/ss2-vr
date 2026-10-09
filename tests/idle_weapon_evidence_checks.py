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
    def test_native_weapon_selection_is_explicit(self):
        result=assess(VALID,SOURCE)
        self.assertEqual(result['copied_event_pose_observations'][0]['nativeId'],1)
        self.assertFalse(result['copied_event_pose_observations'][0]['native_id_explicit'])
        sniper=VALID.replace('schema=3','schema=4 copyLayout=1 nativeId=13')
        result=assess(sniper,SOURCE)
        self.assertEqual(result['copied_event_pose_observations'][0]['nativeId'],13)
        self.assertTrue(result['copied_event_pose_observations'][0]['native_id_explicit'])
        for bad in (sniper.replace('nativeId=13','nativeId=2'),sniper.replace('nativeId=13','nativeId=-1'),
                    VALID.replace('schema=3','schema=3 nativeId=13')):
            with self.assertRaises(ValueError):assess(bad,SOURCE)

    def test_native_selector_consistency(self):
        base=VALID.replace('schema=3','schema=4 copyLayout=1')
        implicit=base
        explicit=base.replace('copyLayout=1','copyLayout=1 nativeId=1')
        sniper=base.replace('copyLayout=1','copyLayout=1 nativeId=13')
        def combine(a,b):return a+'\n'+b.replace('request=100','request=101')
        for a,b in [(explicit,sniper),(sniper,explicit),(implicit,sniper)]:
            with self.assertRaisesRegex(ValueError,'Mixed native weapon'):
                assess(combine(a,b),SOURCE)
        self.assertEqual(len(assess(combine(implicit,explicit),SOURCE)['copied_event_pose_observations']),2)
        self.assertEqual(len(assess(combine(sniper,sniper),SOURCE)['copied_event_pose_observations']),2)

    @staticmethod
    def ten_copy_fixture(retained=False):
        text,_,_,_=geometry_fixture();lines=text.splitlines()
        head=lines[0].replace('schema=3 ','schema=4 copyLayout=1 ').replace('draws=1','draws=10')
        copies=[line for line in lines if line.startswith('Lab idle geometry')]
        if retained:
            body=[head.replace('stage=4','stage=3'),
                  'Lab idle rejection request=100 eye=1 hand=0 reason=10 preceding=2 checks=32639 state=63 callbacks=63',
                  'Lab idle retainedCopies request=100 eye=1 hand=0 count=10 postOriginal=1 cleanupCertified=0 outerCurrent=0']
            copies=[line.replace('Lab idle geometry','Lab idle retainedGeometry') for line in copies]
        else:body=[head,*[line for line in lines[1:] if not line.startswith('Lab idle geometry')]]
        for i in range(10):body.extend(line.replace('index=0 ','index='+str(i)+' ').replace('instance=50','instance='+str(50+i)) for line in copies)
        return '\n'.join(body)
    def test_schema4_ten_distinct_copies_match_and_preserve_old_budget(self):
        for retained in (False,True):
            text=self.ten_copy_fixture(retained);e=assess(text,SOURCE)
            self.assertEqual(e['schema'],4)
            r=(e['rejected_or_missing_observations'] if retained else e['copied_event_pose_observations'])[0]
            g=r['retained_copies']['geometry'] if retained else r['geometry']
            self.assertEqual(list(g),list(range(10)))
            self.assertEqual([x['instance'] for x in g.values()],list(range(50,60)))
            self.assertEqual([x['drawRecord'] for x in g.values()],[0]*10)
            matched=match(e,{})
            self.assertEqual(len(matched['retained_diagnostic_matches'] if retained else matched['matches']),10)
            self.assertFalse(e['alignment_accepted']);self.assertFalse(e['positive_grasp_verified'])
            lines=text.splitlines()
            for bad in (text.replace('schema=4','schema=3'),text.replace('copyLayout=1 ','') ,
                        text.replace('copyLayout=1','copyLayout=0'),text.replace('index=9 ','index=10 '),
                        text+'\n'+next(x for x in lines if 'index=9 ' in x),
                        '\n'.join(x for x in lines if 'index=8 ' not in x),
                        text+'\n'+DRAW.replace('schema=3 ','schema=3 copyLayout=1 ').replace('request=100','request=101').replace('stage=4','stage=3')):
                with self.subTest(retained=retained,bad=bad[:120]),self.assertRaises(ValueError):assess(bad,SOURCE)
            mixed=text+'\n'+DRAW.replace('schema=3 ','schema=3 copyLayout=1 ').replace('request=100','request=101').replace('stage=4','stage=3')
            with self.assertRaisesRegex(ValueError,'Mixed producer schemas'):assess(mixed,SOURCE)
            if retained:
                eight='\n'.join(line for line in lines if 'index=8 ' not in line and 'index=9 ' not in line)
                eight=eight.replace('draws=10','draws=8').replace('count=10','count=8')
                with self.assertRaisesRegex(ValueError,'Retained companion lacks original copied-state qualification'):assess(eight,SOURCE)
                for bad in (text.replace('draws=10','draws=8').replace('count=10','count=8'),
                            text.replace('checks=32639','checks=32767'),text.replace('count=10','count=11')):
                    with self.assertRaises(ValueError):assess(bad,SOURCE)
        self.assertEqual(assess(VALID,SOURCE)['schema'],3)

    def test_exact_capacity_history_preserves_eight_owned_copies_only(self):
        text,_,_,_=geometry_fixture()
        lines=text.splitlines();base='request=100 eye=1 hand=0'
        header=lines[0].replace('stage=4','stage=3').replace('draws=1','draws=8').replace('schema=3 ','schema=3 copyLayout=1 ')
        copies=[line.replace('Lab idle geometry','Lab idle retainedGeometry')
                for line in lines if line.startswith('Lab idle geometry')]
        body=[header,'Lab idle rejection '+base+' reason=10 preceding=2 checks=32639 state=63 callbacks=63',
              'Lab idle retainedCopies '+base+' count=8 postOriginal=1 cleanupCertified=0 outerCurrent=0']
        for i in range(8):body += [line.replace('index=0 ','index='+str(i)+' ') for line in copies]
        log='\n'.join(body);result=assess(log,SOURCE)
        self.assertEqual(result['copied_event_pose_observations'],[])
        r=result['rejected_or_missing_observations'][0]
        self.assertEqual(len(r['retained_copies']['geometry']),8)
        self.assertFalse(r['retained_copies']['whole_trace_accepted'])
        self.assertFalse(result['alignment_accepted']);self.assertFalse(result['positive_grasp_verified'])
        for bad in (log.replace('draws=8','draws=7').replace('count=8','count=7'),
                    log.replace('draws=8','draws=9').replace('count=8','count=9'),
                    log.replace('preceding=2','preceding=1'),log.replace('callbacks=63','callbacks=47'),
                    log.replace('state=63','state=47'),log.replace('checks=32639','checks=32767'),
                    log.replace('copyLayout=1','copyLayout=0'),log.replace('reason=10','reason=12'),
                    '\n'.join(v for v in body if 'index=7 ' not in v),
                    log+'\nLab idle inputFailure '+base+' step=1 index=0 hr=-1 valid=0 caps=0 declaration=0 rangeChecks=0',
                    log+'\nLab idle streamProbe '+base+' attempts=0 flags=0 words=0 invalidations=0 forwardResult=0'):
            with self.subTest(bad=bad[:150]),self.assertRaises(ValueError):assess(bad,SOURCE)
        for bit in range(15):
            if bit==7:continue
            with self.assertRaises(ValueError):assess(log.replace('checks=32639','checks='+str(32639&~(1<<bit))),SOURCE)
    def test_input_failure_indices_follow_explicit_or_historical_layout(self):
        from idle_stream_evidence import OBSERVED,OBSERVED_MULTI_UV,PASSIVE_FIVE_ROW78
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
        for layout,declaration in ((1,OBSERVED),(3,OBSERVED_MULTI_UV),(4,PASSIVE_FIVE_ROW78)):
            rows='\n'.join('Lab idle inputDeclaration request=100 eye=1 hand=0 index='+str(i)+' values='+','.join(map(str,e))
                           for i,e in enumerate(declaration))
            for step in range(8,12):
                hrs=(0,) if step==9 else (-1,0) if step==11 else (-1,)
                for hr in hrs:
                    failed='Lab idle inputFailure request=100 eye=1 hand=0 step='+str(step)+' index=7 hr='+str(hr)+\
                        ' valid=11 caps=256 declaration='+str(len(declaration))+' rangeChecks=0 layout='+str(layout)
                    text=head+failed+'\n'+rows
                    for index in ((0,2,7,8) if layout==4 else (0,3,7,8)):
                        qualified=assess(text.replace('index=7 hr=','index='+str(index)+' hr='),SOURCE)
                        self.assertFalse(qualified['copied_event_pose_observations'])
                    for index in ((3,5,6) if layout==4 else (2,5,6)):
                        with self.assertRaises(ValueError):assess(text.replace('index=7 hr=','index='+str(index)+' hr='),SOURCE)
            old=text.replace(' layout='+str(layout),'').replace('index=7 hr=','index=5 hr=') if layout in (3,4) else historical
            self.assertEqual(assess(old,SOURCE)['rejected_or_missing_observations'][0]['input_failure']['layout'],0)
    def test_no_uv_failure_header_and_historical_passive_compatibility(self):
        from idle_stream_evidence import NO_UV56
        from idle_stream_evidence_checks import fixture
        head=DRAW.replace('stage=4','stage=3')+'\nLab idle rejection request=100 eye=1 hand=0 reason=32 preceding=2 checks=0 state=63 callbacks=63\n'
        failure='Lab idle inputFailure request=100 eye=1 hand=0 step=8 index=5 hr=-1 valid=11 caps=256 declaration=5 rangeChecks=0 layout=2'
        rows='\n'.join('Lab idle inputDeclaration request=100 eye=1 hand=0 index='+str(i)+' values='+','.join(map(str,row))
                       for i,row in enumerate(NO_UV56))
        text=head+failure+'\n'+rows
        for index in (0,3,5,6):
            for suffix in (' layout=2',' layout=0',''):
                result=assess(text.replace('index=5 hr=','index='+str(index)+' hr=').replace(' layout=2',suffix),SOURCE)
                self.assertFalse(result['copied_event_pose_observations'])
                self.assertEqual(result['rejected_or_missing_observations'][0]['input_failure']['layout'],2 if suffix.endswith('2') else 0)
        for bad in (text.replace('index=5 hr=','index=7 hr='),text.replace('index=5 hr=','index=8 hr='),
                    text.replace('layout=2','layout=1'),text.replace('values=1,0,2,0,5,1','values=1,0,2,0,5,2')):
            with self.subTest(bad=bad),self.assertRaises(ValueError):assess(bad,SOURCE)
        passive=fixture(NO_UV56)
        for suffix in (' layout=0',' layout=2'):
            selected='\n'.join(line+suffix if line.startswith('Lab idle inputFailure ') else line for line in passive.splitlines())
            result=assess(selected,SOURCE)
            self.assertFalse(result['copied_event_pose_observations'])
            self.assertEqual(set(result['rejected_or_missing_observations'][0]['stream_probe']['snapshots'][0]['streams']),{0,5,6})
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
        from idle_stream_evidence import NO_UV56
        noUV=text
        for r in old_rows:noUV=noUV.replace(r+'\n','')
        noUV+='\n'+'\n'.join(row('declaration',i,e) for i,e in enumerate(NO_UV56))
        auxiliary=assess(noUV,SOURCE)
        self.assertEqual(auxiliary['copied_event_pose_observations'][0]['geometry'][0]['input_layout'],2)
        self.assertEqual(auxiliary['copied_event_pose_observations'][0]['geometry'][0]['auxiliary_channels'],['uv'])
        matched=match(auxiliary,candidate)
        self.assertEqual(matched['matches'][0]['result'],'unique-position-and-auxiliary-channel-match')
        self.assertEqual(matched['matches'][0]['auxiliary_channels'],['uv'])
        self.assertTrue(matched['copied_channels_all_uniquely_matched'])
        self.assertFalse(matched['consumed_channels_all_uniquely_matched'])
        from idle_stream_evidence import OBSERVED_MULTI_UV
        multi=text.replace('declaration=5','declaration=8')
        for r in old_rows:multi=multi.replace(r+'\n','')
        multi+='\n'+'\n'.join(row('declaration',i,e) for i,e in enumerate(OBSERVED_MULTI_UV))
        actual_multi=assess(multi,SOURCE)
        self.assertEqual(actual_multi['copied_event_pose_observations'][0]['geometry'][0]['input_layout'],3)
        multi_match=match(actual_multi,candidate)
        self.assertEqual(multi_match['matches'][0]['result'],'unique-position-and-auxiliary-channel-match')
        self.assertEqual(multi_match['matches'][0]['input_layout'],3)
        self.assertFalse(multi_match['consumed_channels_all_uniquely_matched'])
        from idle_stream_evidence import PASSIVE_FIVE_ROW78
        five=text
        for r in old_rows:five=five.replace(r+'\n','')
        five+='\n'+'\n'.join(row('declaration',i,e) for i,e in enumerate(PASSIVE_FIVE_ROW78))
        five_evidence=assess(five,SOURCE)
        self.assertEqual(five_evidence['copied_event_pose_observations'][0]['geometry'][0]['input_layout'],4)
        five_match=match(five_evidence,candidate)
        self.assertEqual(five_match['matches'][0]['input_layout'],4)
        self.assertEqual(five_match['matches'][0]['auxiliary_channels'],['uv'])
        self.assertEqual(five_match['matches'][0]['result'],'unique-position-and-auxiliary-channel-match')
        self.assertFalse(five_match['consumed_channels_all_uniquely_matched'])
        import copy
        changed_candidate=copy.deepcopy(candidate)
        changed_candidate['hand']['candidate_channels'][0]['channel_sha256']['uv']='0'*64
        self.assertEqual(match(five_evidence,changed_candidate)['matches'][0]['result'],'unmatched')
        # A stale auxiliary range may still fit the buffer. Even matching other
        # channels and apparent hashes cannot rescue the wrong authored range.
        streams=[1,0,12,1,1,13948,4,1,1,12680,4,1,1,15216,8,1,2,0]
        stale=streams.copy();stale[13]-=8
        wrong_range=assess(five.replace(row('streams',0,streams),row('streams',0,stale)),SOURCE)
        self.assertEqual(match(wrong_range,candidate)['matches'][0]['result'],'unmatched')
        for row_index in range(8):
            for field in range(6):
                rows=[r.copy() for r in OBSERVED_MULTI_UV];rows[row_index][field]+=1
                bad=multi
                for i,element in enumerate(OBSERVED_MULTI_UV):bad=bad.replace(row('declaration',i,element),row('declaration',i,rows[i]))
                with self.assertRaises(ValueError):assess(bad,SOURCE)
        # Positive five-hash evidence must have exactly the same readiness with
        # the optional factors present. Retained copies remain outside coverage.
        self.assertEqual(match(assess(self.factor_fixture(),SOURCE),candidate),match(result,candidate))
        retained=assess(self.factor_fixture(True),SOURCE)
        retained_match=match(retained,candidate)
        self.assertEqual(match(assess(self.repeated_fixture(True),SOURCE),candidate),retained_match)
        self.assertEqual(retained_match['retained_diagnostic_matches'][0]['result'],'unique-consumed-channel-match')
        self.assertFalse(retained_match['copied_geometry_coverage_complete'])
        import copy
        for channel in channels:
            bad=copy.deepcopy(candidate);bad['hand']['candidate_channels'][0]['channel_sha256'][channel]='0'*64
            self.assertFalse(match(result,bad)['consumed_channels_all_uniquely_matched'])
            self.assertFalse(match(auxiliary,bad)['copied_channels_all_uniquely_matched'])
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
    def repeated_fixture(self,factors=False):
        text=self.factor_fixture(True) if factors else self.retained_fixture()
        # Duplicate rejection has no CollectInputs/passive companion. Keep only
        # the independently stored earlier draw and its optional factor payload.
        return '\n'.join(line.replace('reason=32 ','reason=11 ') for line in text.splitlines()
                         if not line.startswith('Lab idle input'))
    def test_repeated_native_pass_preserves_stored_history_only(self):
        for factors in (False,True):
            text=self.repeated_fixture(factors);evidence=assess(text,SOURCE)
            self.assertFalse(evidence['copied_event_pose_observations'])
            r=evidence['rejected_or_missing_observations'][0]
            self.assertEqual(r['rejection']['reason'],11);self.assertNotIn('input_failure',r)
            self.assertEqual(len(r['retained_copies']['geometry']),1)
            self.assertEqual('factors' in r['retained_copies']['geometry'][0],factors)
            self.assertFalse(match(evidence,{})['copied_geometry_coverage_complete'])
            lines=text.splitlines()
            for line in (line for line in lines if 'Lab idle retained' in line):
                with self.assertRaises(ValueError):assess('\n'.join(x for x in lines if x!=line),SOURCE)
                with self.assertRaises(ValueError):assess(text+'\n'+line,SOURCE)
            for bad in (text.replace('draws=1','draws=8').replace('count=1 postOriginal','count=8 postOriginal'),
                        text.replace('callbacks=63','callbacks=47'),text.replace('state=63','state=47'),
                        text.replace('checks=0','checks=1'),text.replace('preceding=2','preceding=1'),
                        text.replace('model=3','model=0'),text.replace('file=30','file=0')):
                with self.assertRaises(ValueError):assess(bad,SOURCE)
            from idle_stream_evidence_checks import fixture
            fabricated=fixture().splitlines()
            for line in fabricated:
                if line.startswith(('Lab idle inputFailure ','Lab idle streamProbe ')):
                    with self.assertRaises(ValueError):assess(text+'\n'+line,SOURCE)
        # Seven stored copies are reachable; eight are not. Repeated native
        # records among those earlier copies still violate the unchanged grammar.
        text=self.repeated_fixture();payload=[line for line in text.splitlines() if line.startswith(('Lab idle retainedGeometry ','Lab idle retainedGeometryData '))]
        seven=text.replace('draws=1','draws=7').replace('count=1 postOriginal','count=7 postOriginal')
        for index in range(1,7):seven+='\n'+'\n'.join(line.replace('index=0 ','index='+str(index)+' ').replace('drawRecord=0','drawRecord='+str(index)) for line in payload)
        r=assess(seven,SOURCE)['rejected_or_missing_observations'][0]
        self.assertEqual(len(r['retained_copies']['geometry']),7)
        with self.assertRaises(ValueError):assess(seven.replace('drawRecord=6','drawRecord=0'),SOURCE)
        extra='\n'.join(line.replace('index=0 ','index=7 ').replace('drawRecord=0','drawRecord=7') for line in payload)
        eight=seven.replace('draws=7','draws=8').replace('count=7 postOriginal','count=8 postOriginal')+'\n'+extra
        with self.assertRaises(ValueError):assess(eight,SOURCE)
        old=self.retained_fixture().replace('draws=1','draws=8').replace('count=1 postOriginal','count=8 postOriginal')
        for index in range(1,8):old+='\n'+'\n'.join(line.replace('index=0 ','index='+str(index)+' ').replace('drawRecord=0','drawRecord='+str(index)) for line in payload)
        previous=assess(old,SOURCE)['rejected_or_missing_observations'][0]
        self.assertEqual(previous['rejection']['reason'],32)
        self.assertEqual(len(previous['retained_copies']['geometry']),8)
    def test_ordinal_layout_preserves_each_copy_and_legacy_uniqueness(self):
        text,_,_,_=geometry_fixture()
        payload=[line for line in text.splitlines() if line.startswith('Lab idle geometry')]
        second='\n'.join(line.replace('index=0 ','index=1 ').replace('instance=50','instance=51')
                         for line in payload)
        legacy=text.replace('draws=1','draws=2')+'\n'+second
        with self.assertRaises(ValueError):assess(legacy,SOURCE)
        new=legacy.replace('schema=3 ','schema=3 copyLayout=1 ')
        result=assess(new,SOURCE)['copied_event_pose_observations'][0]
        self.assertEqual(result['copyLayout'],1);self.assertEqual(set(result['geometry']),{0,1})
        self.assertEqual([g['drawRecord'] for g in result['geometry'].values()],[0,0])
        self.assertEqual([g['instance'] for g in result['geometry'].values()],[50,51])
        for bad in (new.replace('copyLayout=1','copyLayout=0'),new.replace('copyLayout=1','copyLayout=2'),
                    new.replace('index=1 ','index=2 '),new.replace('index=1 ','index=8 '),new+'\n'+second,
                    new+'\n'+DRAW.replace('request=100','request=101').replace('stage=4','stage=3')):
            with self.assertRaises(ValueError):assess(bad,SOURCE)
        with self.assertRaises(ValueError):assess(self.repeated_fixture().replace('schema=3 ','schema=3 copyLayout=1 '),SOURCE)
        eight=text.replace('draws=1','draws=8').replace('schema=3 ','schema=3 copyLayout=1 ')
        for index in range(1,8):eight+='\n'+'\n'.join(line.replace('index=0 ','index='+str(index)+' ') for line in payload)
        self.assertEqual(len(assess(eight,SOURCE)['copied_event_pose_observations'][0]['geometry']),8)
        with self.assertRaises(ValueError):assess(eight.replace('draws=8','draws=9'),SOURCE)
        # New reason32 histories may repeat native records without accepting the
        # rejected trace. Legacy reason11 remains readable only without layout1.
        history=self.retained_fixture().replace('schema=3 ','schema=3 copyLayout=1 ').replace('draws=1','draws=2').replace('count=1 postOriginal','count=2 postOriginal')
        old=[line for line in history.splitlines() if line.startswith(('Lab idle retainedGeometry ','Lab idle retainedGeometryData '))]
        history+='\n'+'\n'.join(line.replace('index=0 ','index=1 ') for line in old)
        r=assess(history,SOURCE)
        self.assertFalse(r['copied_event_pose_observations'])
        self.assertEqual(len(r['rejected_or_missing_observations'][0]['retained_copies']['geometry']),2)
    @staticmethod
    def factor_words():
        import struct
        values={name:list(struct.unpack('<'+str(size)+'I',struct.pack('<'+str(size)+'f',
                *[float(i*100+j+1) for j in range(size)])))
                for i,(name,size) in enumerate([('factorModel',12),('factorLocal',12),('factorView',12),('factorProjection',16)])}
        values['factorModel'][1]=0x80000000;values['factorLocal'][2]=1;values['factorView'][3]=0x80000001
        return values
    def factor_fixture(self,retained=False):
        text,row,_,_=geometry_fixture()
        header='Lab idle geometryFactors request=100 eye=1 hand=0 index=0 palette=17 bookends=3 postOriginal=1 cleanupCertified=0 outerCurrent=0'
        payload='\n'.join(row(kind,0,values) for kind,values in self.factor_words().items())
        if retained:
            text=self.retained_fixture()
            header=header.replace('geometryFactors','retainedGeometryFactors').replace('eye=1 hand=0','eye=0 hand=1')
            payload=payload.replace('geometryData','retainedGeometryData').replace('eye=1 hand=0','eye=0 hand=1')
        return text+'\n'+header+'\n'+payload
    def test_optional_factors_are_bounded_history_not_acceptance(self):
        for retained in (False,True):
            text=self.factor_fixture(retained);evidence=assess(text,SOURCE)
            observation=(evidence['rejected_or_missing_observations'] if retained else evidence['copied_event_pose_observations'])[0]
            geometry=(observation['retained_copies']['geometry'] if retained else observation['geometry'])[0]
            self.assertEqual(geometry['factors']['palette_index'],17)
            for name,values in self.factor_words().items():self.assertEqual(geometry['data'][name+':0'],values)
            self.assertTrue(geometry['factors']['post_original_return']);self.assertTrue(geometry['factors']['diagnostic_only'])
            for flag in ('cleanup_certified','outer_current','reference_replaced'):
                self.assertFalse(geometry['factors'][flag])
            self.assertFalse(evidence['positive_grasp_verified']);self.assertFalse(evidence['alignment_accepted'])
            lines=text.splitlines()
            factor_lines=[line for line in lines if 'GeometryFactors ' in line or 'geometryFactors ' in line or 'kind=factor' in line]
            for line in factor_lines:
                with self.subTest(retained=retained,line=line),self.assertRaises(ValueError):
                    assess('\n'.join(x for x in lines if x!=line),SOURCE)
                with self.assertRaises(ValueError):assess(text+'\n'+line,SOURCE)
            for bad in (text.replace('palette=17','palette=32768'),text.replace('bookends=3','bookends=1'),
                        text.replace('postOriginal=1 cleanupCertified=0 outerCurrent=0','postOriginal=0 cleanupCertified=0 outerCurrent=0'),
                        text.replace('cleanupCertified=0','cleanupCertified=1'),text.replace('outerCurrent=0','outerCurrent=1'),
                        text.replace('kind=factorLocal chunk=0','kind=factorLocal chunk=1'),
                        text.replace('kind=factorModel chunk=0 values=3f800000','kind=factorModel chunk=0 values=7fc00000'),
                        text.replace('index=0 palette=17','index=1 palette=17')):
                with self.subTest(bad=bad),self.assertRaises(ValueError):assess(bad,SOURCE)
            without='\n'.join(line for line in lines if line not in factor_lines)
            original=assess(without,SOURCE)
            self.assertEqual(len(original['copied_event_pose_observations']),len(evidence['copied_event_pose_observations']))
            self.assertEqual(len(original['rejected_or_missing_observations']),len(evidence['rejected_or_missing_observations']))
            self.assertEqual(match(original,{})['copied_geometry_coverage_complete'],match(evidence,{})['copied_geometry_coverage_complete'])
    def test_factor_identity_ordering_widths_and_multiple_draws(self):
        for retained in (False,True):
            text=self.factor_fixture(retained);lines=text.splitlines()
            header=next(line for line in lines if 'palette=17 bookends=3' in line)
            payload=next(line for line in lines if 'kind=factorModel ' in line)
            geometry_header=next(line for line in lines if 'modelRecord=1 drawRecord=0' in line)
            for bad in (text.replace(header,header.replace('request=100','request=101')),
                        text.replace(header,header.replace('eye=0' if retained else 'eye=1','eye=1' if retained else 'eye=0')),
                        text.replace(payload,payload.replace('hand=1' if retained else 'hand=0','hand=0' if retained else 'hand=1')),
                        text.replace(header,'').replace(geometry_header,header+'\n'+geometry_header),
                        text.replace(payload,'').replace(header,payload+'\n'+header),text.replace('palette=17','palette=-1')):
                with self.subTest(retained=retained),self.assertRaises(ValueError):assess(bad,SOURCE)
            for line in (line for line in lines if 'kind=factor' in line):
                prefix,values=line.split(' values=');parts=values.split(',')
                for replaced in (parts[:-1],parts+['00000000'],['7f800000',*parts[1:]]):
                    with self.assertRaises(ValueError):assess(text.replace(line,prefix+' values='+','.join(replaced)),SOURCE)
            boundary=assess(text.replace('palette=17','palette=32767'),SOURCE)
            observation=(boundary['rejected_or_missing_observations'] if retained else boundary['copied_event_pose_observations'])[0]
            g=(observation['retained_copies']['geometry'] if retained else observation['geometry'])[0]
            self.assertEqual(g['factors']['palette_index'],32767)
        text=self.factor_fixture()
        geometry_lines=[line for line in text.splitlines() if line.startswith('Lab idle geometry')]
        second='\n'.join(line.replace('index=0 ','index=1 ').replace('drawRecord=0','drawRecord=1').replace('palette=17','palette=32767')
                         for line in geometry_lines)
        second=second.replace('kind=factorModel chunk=0 values=3f800000','kind=factorModel chunk=0 values=40000000')
        two=text.replace('draws=1','draws=2')+'\n'+second
        other_eye=two.replace('eye=1','eye=0').replace('kind=factorLocal chunk=0 values=42ca0000',
                                                     'kind=factorLocal chunk=0 values=4479c000')
        both=two+'\n'+other_eye
        result=assess(both,SOURCE)
        self.assertEqual(len(result['copied_event_pose_observations']),2)
        for o in result['copied_event_pose_observations']:
            self.assertEqual(o['request'],100)
            self.assertEqual(set(o['geometry']),{0,1})
            for index,g in o['geometry'].items():
                self.assertEqual(g['factors']['palette_index'],17 if index==0 else 32767)
                expected=self.factor_words()
                if index==1:expected['factorModel'][0]=0x40000000
                if o['eye']==0:expected['factorLocal'][0]=0x4479c000
                for name,values in expected.items():self.assertEqual(g['data'][name+':0'],values)
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
