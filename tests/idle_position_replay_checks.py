import hashlib
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from replay_idle_geometry import replay
from assess_idle_weapon import assess
from measure_idle_reference import measure

EVALUATOR=Path(sys.argv.pop(1)).resolve()
IDENTITY=[1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.,0.,0.,0.,0.,1.]
PROGRAM=[0xfffe0101,31,0x80000005,0x900f0000,20,0xc00f0000,0x90e40000,0xa0e40000,0xffff]

def fixture(program=PROGRAM,constants=None,points=None,clip=IDENTITY,weights=1,layout=None,local=None):
    constants=constants or [IDENTITY[i:i+4] for i in range(0,16,4)]
    points=points or [(1.,2.,3.,.25,.75)]
    schema=3 if local is not None else 1 if layout is None else 2
    data=b'SS2VIRP1'+struct.pack('<5I',schema,len(program),len(constants),len(points),weights)
    if schema>=2:data+=struct.pack('<I',0 if layout is None else layout)
    data+=struct.pack('<16f',*clip)
    if local is not None:data+=struct.pack('<12f',*local)
    data+=struct.pack('<'+str(len(program))+'I',*program)
    data+=struct.pack('<'+str(len(constants)*4)+'f',*[v for row in constants for v in row])
    return data+b''.join(struct.pack('<5f',*p)+bytes([255,0,0,0,0,0,0,0]) for p in points)

class Checks(unittest.TestCase):
    def test_staged_local_reference_preserves_old_schemas(self):
        local=[1.,0.,0.,100.,0.,1.,0.,-10.,0.,0.,1.,2.]
        program=[0xfffe0101,31,0x80000005,0x900f0000,
                 9,0x80010000,0x90e40000,0xa0e40004,
                 9,0x80020000,0x90e40000,0xa0e40005,
                 9,0x80040000,0x90e40000,0xa0e40006,
                 1,0x80080000,0xa0ff0007,
                 20,0xc00f0000,0x80e40000,0xa0e40000,0xffff]
        constants=[IDENTITY[i:i+4] for i in range(0,16,4)]+[local[i:i+4] for i in range(0,12,4)]+[[0,0,0,1]]
        for layout in (0,1,2):
            staged=fixture(program=program,constants=constants,local=local,layout=layout)
            result=json.loads(self.run_fixture(staged).stdout)
            self.assertTrue(result['position_replay_agrees_with_reference']);self.assertFalse(result['alignment_accepted'])
            bad=local.copy();bad[3]+=1
            self.assertEqual(json.loads(self.run_fixture(fixture(program=program,constants=constants,local=bad,layout=layout)).stdout)['reason'],'projection-mismatch')
            for broken in (staged[:-1],staged+b'x',fixture(local=local,weights=0),fixture(local=local,layout=3)):
                self.assertNotEqual(self.run_fixture(broken).returncode,0)
        old=json.loads(self.run_fixture(fixture(program=program,constants=constants)).stdout)
        self.assertEqual(old['reason'],'projection-mismatch')
        bad=local.copy();bad[0]=float('nan')
        self.assertNotEqual(self.run_fixture(fixture(local=bad)).returncode,0)
        self.assertTrue(json.loads(self.run_fixture(fixture()).stdout)['position_replay_agrees_with_reference'])
        # Intermediate binary32 rounding is observable: the mathematically
        # collapsed local dot is 1, but the staged native-style dot is 0.
        cancel=[float(2**24),1.,float(-2**24),0.,0.,1.,0.,0.,0.,0.,1.,0.]
        constants=[IDENTITY[i:i+4] for i in range(0,16,4)]+[cancel[i:i+4] for i in range(0,12,4)]+[[0,0,0,1]]
        self.assertTrue(json.loads(self.run_fixture(fixture(program=program,constants=constants,local=cancel,
            points=[(1.,1.,1.,0.,0.)])).stdout)['position_replay_agrees_with_reference'])
        self.assertEqual(json.loads(self.run_fixture(fixture(program=program,constants=constants,
            points=[(1.,1.,1.,0.,0.)])).stdout)['reason'],'projection-mismatch')

    def test_observed_input_layout_and_schema_compatibility(self):
        p=[0xfffe0101,31,0x80000005,0x900f0000,31,0x80070005,0x900f0007,31,0x80080005,0x900f0008,
           1,0x800f0000,0x90e40000,4,0x800f0000,0x90e40007,0x90e40008,0x80e40000,
           20,0xc00f0000,0x80e40000,0xa0e40000,0xffff]
        old=json.loads(self.run_fixture(fixture(program=p)).stdout)
        self.assertEqual(old['reason'],'unknown-position-dependency')
        new=json.loads(self.run_fixture(fixture(program=p,layout=1)).stdout)
        self.assertTrue(new['position_replay_agrees_with_reference']);self.assertFalse(new['gpu_execution'])
        self.assertTrue(json.loads(self.run_fixture(fixture(layout=0)).stdout)['position_replay_agrees_with_reference'])
        for data in (fixture(program=p,layout=3),fixture(program=p,layout=1,weights=0)):
            self.assertNotEqual(self.run_fixture(data).returncode,0)
        data=bytearray(fixture(program=p,layout=1));data[-4]=1
        self.assertEqual(json.loads(self.run_fixture(data).stdout)['reason'],'unsupported-influence')
    def test_no_uv_leaves_undeclared_inputs_unknown(self):
        self.assertTrue(json.loads(self.run_fixture(fixture(layout=2)).stdout)['position_replay_agrees_with_reference'])
        for reg in range(1,5):
            p=[0xfffe0101,31,0x80000005,0x900f0000,31,0x80000005|(reg<<16),0x900f0000|reg,
               20,0xc00f0000,0x90e40000|reg,0xa0e40000,0xffff]
            self.assertEqual(json.loads(self.run_fixture(fixture(program=p,layout=2)).stdout)['reason'],
                             'unknown-position-dependency')
        self.assertNotEqual(self.run_fixture(fixture(layout=2,weights=0)).returncode,0)
        for byte,value in ((-8,127),(-4,1)):
            data=bytearray(fixture(layout=2));data[byte]=value
            self.assertEqual(json.loads(self.run_fixture(data).stdout)['reason'],'unsupported-influence')
        # Shader-local DEF and relative palette addressing remain exact for 5/6;
        # auxiliary UV is irrelevant to position and cannot fill unknown v3.
        p=[0xfffe0101,31,0x80000005,0x900f0000,31,0x80050005,0x900f0005,31,0x80060005,0x900f0006,
           81,0xa00f00ff,0x3f800000,0x3f000000,0,0x443f40a4,
           5,0x80010000,0x90000005,0xa0ff00ff,1,0xb0010000,0x80000000,
           20,0xc00f0000,0x90e40000,0xa0e42000,0xffff]
        result=json.loads(self.run_fixture(fixture(program=p,layout=2,points=[(1.,2.,3.,123.,-456.)])).stdout)
        self.assertTrue(result['position_replay_agrees_with_reference'])
    def run_fixture(self,data):
        with tempfile.TemporaryDirectory() as tmp:
            p=Path(tmp)/'input.bin';p.write_bytes(data)
            return subprocess.run([str(EVALUATOR),str(p)],capture_output=True,text=True,timeout=5)
    def test_known_position_and_scope_independence(self):
        r=self.run_fixture(fixture())
        self.assertEqual(r.returncode,0,r.stderr)
        result=json.loads(r.stdout);self.assertTrue(result['position_replay_agrees_with_reference'])
        self.assertFalse(result['gpu_execution']);self.assertFalse(result['alignment_accepted'])
        changed=IDENTITY.copy();changed[3]=.01
        result=json.loads(self.run_fixture(fixture(clip=changed)).stdout)
        self.assertEqual(result['reason'],'projection-mismatch')
    def test_unknown_dependency_and_local_defs(self):
        unknown=[0xfffe0101,31,0x80010005,0x900f0001,31,0x80000005,0x900f0000,
                 20,0xc00f0000,0x90e40001,0xa0e40000,0xffff]
        self.assertEqual(json.loads(self.run_fixture(fixture(program=unknown)).stdout)['reason'],'unknown-position-dependency')
        defined=PROGRAM[:-1]+[81,0xa00f0000,0,0,0,0,0xffff]
        self.assertEqual(json.loads(self.run_fixture(fixture(program=defined)).stdout)['reason'],'projection-mismatch')
    def test_malformed_and_influence(self):
        original=fixture()
        for bad in [original[:-1],original+b'x',b'x'+original[1:],
                    original[:8]+struct.pack('<I',2)+original[12:],
                    original[:12]+struct.pack('<I',513)+original[16:]]:
            self.assertNotEqual(self.run_fixture(bad).returncode,0)
        unsupported=bytearray(original);unsupported[-8]=127
        self.assertEqual(json.loads(self.run_fixture(unsupported).stdout)['reason'],'unsupported-influence')
        for end in range(1,len(PROGRAM)):
            r=self.run_fixture(fixture(program=PROGRAM[:end]))
            if r.returncode==0:self.assertFalse(json.loads(r.stdout)['position_replay_agrees_with_reference'])
        incomplete=[0xfffe0101,31,0x80000005,0x900f0000,1,0xc0010000,0x90000000,0xffff]
        self.assertEqual(json.loads(self.run_fixture(fixture(program=incomplete)).stdout)['reason'],'unknown-position-dependency')
        p=PROGRAM.copy();p[0]=0xfffe0200
        self.assertEqual(json.loads(self.run_fixture(fixture(program=p)).stdout)['reason'],'unsupported-program')
    def test_all_vertices_not_just_first(self):
        points=[(0.,0.,0.,0.,0.),(1.,2.,3.,.25,.75)]
        result=json.loads(self.run_fixture(fixture(points=points)).stdout)
        self.assertTrue(result['position_replay_agrees_with_reference']);self.assertEqual(result['vertex'],2)
        collapsed=[[0.,0.,0.,0.],[0.,0.,0.,0.],[0.,0.,0.,0.],[0.,0.,0.,1.]]
        result=json.loads(self.run_fixture(fixture(points=points,constants=collapsed)).stdout)
        self.assertFalse(result['position_replay_agrees_with_reference']);self.assertEqual(result['vertex'],1)
    def test_full_private_log_channel_roundtrip(self):
        with tempfile.TemporaryDirectory() as tmp:
            root=Path(tmp);asset=b'owned-synthetic-test-candidate';(root/'test.mesh').write_bytes(asset)
            channels={'positions':struct.pack('<9f',0.,0.,0.,1.,0.,0.,0.,1.,0.),
                      'indices':struct.pack('<3H',0,1,2),'weights':bytes([255,0,0,0])*3,
                      'local_indices':bytes(12),'uv':struct.pack('<6f',0.,0.,1.,0.,0.,1.)}
            ranges={'positions':(0,36,133),'indices':(0,6,135),'weights':(36,12,128),
                    'local_indices':(48,12,128),'uv':(60,24,132)}
            files={}
            for name,data in channels.items():files[name]=name+'.bin';(root/files[name]).write_bytes(data)
            index={'test.mesh':{'asset_sha256':hashlib.sha256(asset).hexdigest(),'candidate_channels':[{
                'mesh_object':1,'lod':0,'vertices':3,'triangles':1,'whole_vertex_buffer_bytes':84,'whole_index_buffer_bytes':6,
                'single_body_influence':True,'channel_files':files,
                'channel_ranges':{n:{'offset':v[0],'size':v[1],'format':v[2],'buffer':0} for n,v in ranges.items()},
                'channel_sha256':{n:hashlib.sha256(v).hexdigest() for n,v in channels.items()}}]}}
            source='a'*64
            lines=[f'Lab idle draw schema=3 rawGripValid=1 draws=1 source={source} ipc=10 wire=7 request=1 input=1 owner=1 weapon=2 model=3 generation=4 hand=0 eye=0 stage=4 cfg=20 file=30 resource=5 contributors=1 matrices=1 historicalBytes=0 grasp=0',
                   'Lab idle animation request=1 eye=0 hand=0 index=0 raw='+','.join(['00000000']*8)+' header='+','.join(['00000001']*4)]
            for kind in ('world','nativePlacement','trackedPlacement','controller','canonical','rawAim','rawGrip'):
                lines.append(f'Lab idle matrix request=1 eye=0 hand=0 kind={kind} index=0 values=1,0,0,0,0,1,0,0,0,0,1,0')
            lines+=['Lab idle stretch request=1 eye=0 hand=0 values=1,1,1',
                    'Lab idle geometry request=1 eye=0 hand=0 index=0 modelRecord=1 drawRecord=0 surface=40 instance=50 surfaceName=60 boneName=70 bone=0 cfg=20 file=30 resource=5 words=9 constants=4 declaration=5']
            def row(kind,chunk,values):lines.append('Lab idle geometryData request=1 eye=0 hand=0 index=0 kind='+kind+' chunk='+str(chunk)+' values='+','.join(f'{x:08x}' for x in values))
            affine=[0.,0.,1.,10.,0.,1.,0.,20.,-1.,0.,0.,30.]
            clip=[0.,0.,2.,20.,0.,3.,0.,60.,-4.,0.,0.,120.,0.,0.,0.,1.]
            row('affine',0,struct.unpack('<12I',struct.pack('<12f',*affine)))
            row('clip',0,struct.unpack('<16I',struct.pack('<16f',*clip)))
            row('layout',0,[3,1,0,133,0,0,135,0,36,128,0,48,128,0]);row('buffers',0,[84,0,1,100,0,6,0,1,101,0])
            row('draw',0,[4,0,0,3,0,1]);row('streams',0,[1,0,12,1,1,48,4,1,1,36,4,1,1,60,8,1,2,0])
            for i,name in enumerate(('positions','indices','weights','local_indices','uv')):
                row('hash',i,struct.unpack('<8I',hashlib.sha256(channels[name]).digest()))
            for i,e in enumerate([[0,0,2,0,5,0],[5,0,8,0,5,5],[6,0,8,0,5,6],[3,0,1,0,5,3],[255,0,17,0,0,0]]):row('declaration',i,e)
            row('program',0,PROGRAM)
            for i in range(4):row('constant',i,struct.unpack('<4I',struct.pack('<4f',*clip[i*4:i*4+4])))
            evidence=assess('\n'.join(lines),source)
            result=replay(evidence,index,root,EVALUATOR,root)
            self.assertTrue(result['all_consumed_positions_agree_with_native_reference']);self.assertFalse(result['alignment_accepted'])
            self.assertEqual(result['draws'][0]['render_geometry']['triangle_indices'],[0,1,2])
            self.assertEqual(result['draws'][0]['render_geometry']['world_positions'],[[10.,20.,30.],[10.,20.,29.],[10.,21.,30.]])
            from idle_weapon_evidence_checks import Checks as EvidenceChecks
            factor_lines=['Lab idle geometryFactors request=1 eye=0 hand=0 index=0 palette=17 bookends=3 postOriginal=1 cleanupCertified=0 outerCurrent=0']
            for name,values in EvidenceChecks.factor_words().items():
                factor_lines.append('Lab idle geometryData request=1 eye=0 hand=0 index=0 kind='+name+' chunk=0 values='+','.join(f'{x:08x}' for x in values))
            diagnostic=replay(assess('\n'.join([*lines,*factor_lines]),source),index,root,EVALUATOR,root)
            self.assertEqual(diagnostic,result)
            stored=[line.replace('Lab idle geometry','Lab idle retainedGeometry') for line in [*lines,*factor_lines]
                    if line.startswith('Lab idle geometry')]
            repeated=[lines[0].replace('stage=4','stage=3'),
                      'Lab idle rejection request=1 eye=0 hand=0 reason=11 preceding=2 checks=0 state=63 callbacks=63',
                      'Lab idle retainedCopies request=1 eye=0 hand=0 count=1 postOriginal=1 cleanupCertified=0 outerCurrent=0',*stored]
            historical=replay(assess('\n'.join(repeated),source),index,root,EVALUATOR,root)
            self.assertEqual(historical['draws'],[]);self.assertEqual(len(historical['retained_diagnostic_draws']),1)
            self.assertTrue(historical['retained_diagnostic_draws'][0]['position_replay']['position_replay_agrees_with_reference'])
            self.assertFalse(historical['all_consumed_positions_agree_with_native_reference'])
            self.assertFalse(historical['copied_geometry_coverage_complete']);self.assertFalse(historical['alignment_accepted'])
            annotation={'schema':1,'reference_kind':'indexed-surface-convention','semantic_status':'reviewed-convention',
                'asset_sha256':index['test.mesh']['asset_sha256'],'mesh_object':1,'lod':0,'channel_index':0,
                'channel_sha256':index['test.mesh']['candidate_channels'][0]['channel_sha256'],
                'landmarks':{name:{'triangle':0,'ordered_vertex_indices':[0,1,2],'barycentric':w}
                             for name,w in zip(('P','I','T'),([1,0,0],[0,1,0],[0,0,1]))}}
            measured=measure(result,annotation);ref=measured['measured_references'][0]
            self.assertEqual(ref['world_landmarks'],[[10.,20.,30.],[10.,20.,29.],[10.,21.,30.]])
            self.assertEqual(ref['world_reference_frame'],[0.,0.,1.,10.,0.,1.,0.,20.,-1.,0.,0.,30.])
            self.assertFalse(measured['positive_grasp_verified']);self.assertFalse(measured['alignment_accepted'])
            import copy
            reflected=copy.deepcopy(result)
            reflected['draws'][0]['render_geometry']['affine']=[-1.,0.,0.,10.,0.,1.,0.,20.,0.,0.,1.,30.]
            ref=measure(reflected,annotation)['measured_references'][0]
            self.assertTrue(ref['captured_affine_reflected'])
            self.assertEqual(ref['world_reference_frame'],[-1.,0.,0.,10.,0.,1.,0.,20.,0.,0.,-1.,30.])
            for bad_grip in ([0]*11,[0]*3+[float('nan')]+[0]*8,[0]*7+[float('inf')]+[0]*4,
                             [0]*3+[1e300]+[0]*8,[True]*12):
                broken=copy.deepcopy(result);broken['draws'][0]['render_geometry']['raw_grip']=bad_grip
                with self.assertRaises(ValueError):measure(broken,annotation)
            for bad in ('index','barycentric','degenerate','hash'):
                broken=copy.deepcopy(annotation)
                if bad=='index':broken['landmarks']['P']['ordered_vertex_indices']=[2,1,0]
                elif bad=='barycentric':broken['landmarks']['P']['barycentric']=[-1,1,1]
                elif bad=='degenerate':broken['landmarks']['I']=broken['landmarks']['P']
                else:broken['channel_sha256']['positions']='0'*64
                with self.assertRaises(ValueError):measure(result,broken)
            failed_log=lines+[lines[0].replace('request=1','request=2').replace('stage=4','stage=3')]
            incomplete=replay(assess('\n'.join(failed_log),source),index,root,EVALUATOR,root)
            self.assertFalse(incomplete['all_consumed_positions_agree_with_native_reference'])
            self.assertFalse(incomplete['copied_geometry_coverage_complete'])
            self.assertEqual(incomplete['observations_without_geometry'][0]['request'],2)
            self.assertTrue(incomplete['draws'][0]['position_replay']['position_replay_agrees_with_reference'])
            changed_log='\n'.join(lines).replace('kind=constant chunk=0 values=00000000,00000000,40000000,41a00000',
                'kind=constant chunk=0 values=00000000,00000000,40000000,41a80000')
            mismatch=replay(assess(changed_log,source),index,root,EVALUATOR,root)
            self.assertFalse(mismatch['all_consumed_positions_agree_with_native_reference'])
            self.assertNotIn('render_geometry',mismatch['draws'][0])
            # Same native record, independently qualified copies: different
            # assets/programs/constants/affines/factors, both eyes in one request.
            second_asset=b'other-owned-synthetic-test-candidate';(root/'test2.mesh').write_bytes(second_asset)
            second_channels=dict(channels)
            second_channels['positions']=struct.pack('<9f',.25,0.,0.,1.25,0.,0.,.25,1.,0.)
            (root/'positions2.bin').write_bytes(second_channels['positions'])
            second_candidate=copy.deepcopy(index['test.mesh'])
            second_candidate['asset_sha256']=hashlib.sha256(second_asset).hexdigest()
            second_candidate['candidate_channels'][0]['mesh_object']=2
            second_candidate['candidate_channels'][0]['channel_files']['positions']='positions2.bin'
            second_candidate['candidate_channels'][0]['channel_sha256']['positions']=hashlib.sha256(second_channels['positions']).hexdigest()
            pair_index={**index,'test2.mesh':second_candidate}
            second_clip=clip.copy();second_clip[3]+=4
            second_affine=affine.copy();second_affine[3]+=2
            second_program=[0xfffe0101,31,0x80000005,0x900f0000,1,0x800f0000,0x90e40000,
                            20,0xc00f0000,0x80e40000,0xa0e40000,0xffff]
            def second_row(kind,chunk,values):
                return 'Lab idle geometryData request=1 eye=0 hand=0 index=1 kind='+kind+' chunk='+str(chunk)+' values='+','.join(f'{x:08x}' for x in values)
            copied=[]
            for line in lines:
                if not line.startswith('Lab idle geometry'):continue
                line=line.replace('index=0 ','index=1 ').replace('words=9','words=12').replace('instance=50','instance=51').replace('modelRecord=1','modelRecord=2')
                if 'kind=affine ' in line:line=second_row('affine',0,struct.unpack('<12I',struct.pack('<12f',*second_affine)))
                elif 'kind=clip ' in line:line=second_row('clip',0,struct.unpack('<16I',struct.pack('<16f',*second_clip)))
                elif 'kind=program ' in line:line=second_row('program',0,second_program)
                elif 'kind=hash chunk=0 ' in line:line=second_row('hash',0,struct.unpack('<8I',hashlib.sha256(second_channels['positions']).digest()))
                elif 'kind=constant ' in line:
                    chunk=int(line.split(' chunk=')[1].split()[0])
                    line=second_row('constant',chunk,struct.unpack('<4I',struct.pack('<4f',*second_clip[chunk*4:chunk*4+4])))
                copied.append(line)
            second_factors=[factor_lines[0].replace('index=0 ','index=1 ').replace('palette=17','palette=18')]
            for name,values in EvidenceChecks.factor_words().items():second_factors.append(second_row(name,0,[v+1 for v in values]))
            pair_log='\n'.join([*lines,*factor_lines,*copied,*second_factors]).replace('draws=1','draws=2').replace('schema=3 ','schema=3 copyLayout=1 ')
            both=pair_log+'\n'+pair_log.replace('eye=0','eye=1')
            pair_evidence=assess(both,source)
            pair=replay(pair_evidence,pair_index,root,EVALUATOR,root)
            self.assertEqual([(r['eye'],r['geometry_index'],r['draw_record']) for r in pair['draws']],[(0,0,0),(0,1,0),(1,0,0),(1,1,0)])
            self.assertTrue(pair['all_consumed_positions_agree_with_native_reference'])
            self.assertEqual([r['candidates'][0]['candidate'] for r in pair['draws']],['test.mesh','test2.mesh']*2)
            self.assertNotEqual(pair['draws'][0]['render_geometry']['positions'],pair['draws'][1]['render_geometry']['positions'])
            self.assertNotEqual(pair['draws'][0]['render_geometry']['affine'],pair['draws'][1]['render_geometry']['affine'])
            for o in pair_evidence['copied_event_pose_observations']:
                for name,values in EvidenceChecks.factor_words().items():
                    self.assertEqual(o['geometry'][0]['data'][name+':0'],values)
                    self.assertEqual(o['geometry'][1]['data'][name+':0'],[v+1 for v in values])
            first_measure=measure(pair,annotation)
            self.assertEqual([r['geometry_index'] for r in first_measure['measured_references']],[0,0])
            other_annotation=copy.deepcopy(annotation)
            other_annotation.update(asset_sha256=second_candidate['asset_sha256'],mesh_object=2,
                                    channel_sha256=second_candidate['candidate_channels'][0]['channel_sha256'])
            self.assertEqual([r['geometry_index'] for r in measure(pair,other_annotation)['measured_references']],[1,1])
            bad_index=copy.deepcopy(pair_index);bad_index['test2.mesh']['candidate_channels'][0]['channel_sha256']['positions']='0'*64
            unmatched=replay(pair_evidence,bad_index,root,EVALUATOR,root)
            self.assertFalse(unmatched['all_consumed_positions_agree_with_native_reference'])
            self.assertEqual(unmatched['draws'][1]['position_replay']['reason'],'unmatched')
            without='\n'.join(line for line in both.splitlines() if 'geometryFactors ' not in line and 'kind=factor' not in line)
            self.assertEqual(replay(assess(without,source),pair_index,root,EVALUATOR,root),pair)
            original_second=second_row('program',0,second_program)
            unknown_second=[0xfffe0101,31,0x80000005,0x900f0000,31,0x80010005,0x900f0001,
                            20,0xc00f0000,0x90e40001,0xa0e40000,0xffff]
            unsupported_second=second_program.copy();unsupported_second[0]=0xfffe0200
            for shader,reason in ((unknown_second,'unknown-position-dependency'),(unsupported_second,'unsupported-program')):
                bad_shader=both.replace(original_second,second_row('program',0,shader))
                # The replacement applies only to eye0. Eye1 and the first copy
                # retain their independent valid payload/results.
                failed=replay(assess(bad_shader,source),pair_index,root,EVALUATOR,root)
                self.assertTrue(failed['draws'][0]['position_replay']['position_replay_agrees_with_reference'])
                self.assertEqual(failed['draws'][1]['position_replay']['reason'],reason)
                self.assertNotIn('render_geometry',failed['draws'][1])
                self.assertTrue(failed['draws'][2]['position_replay']['position_replay_agrees_with_reference'])
                self.assertTrue(failed['draws'][3]['position_replay']['position_replay_agrees_with_reference'])
                self.assertFalse(failed['all_consumed_positions_agree_with_native_reference'])
            from idle_stream_evidence import NO_UV56
            noUV=[line for line in lines if 'kind=declaration ' not in line]
            for i,e in enumerate(NO_UV56):
                noUV.append('Lab idle geometryData request=1 eye=0 hand=0 index=0 kind=declaration chunk='+str(i)+
                            ' values='+','.join(f'{x:08x}' for x in e))
            auxiliary=replay(assess('\n'.join(noUV),source),index,root,EVALUATOR,root)
            self.assertTrue(auxiliary['all_consumed_positions_agree_with_native_reference'])
            self.assertEqual(auxiliary['draws'][0]['auxiliary_channels'],['uv'])
            self.assertEqual(auxiliary['draws'][0]['result'],'unique-position-and-auxiliary-channel-match')
            self.assertFalse(auxiliary['positive_grasp_verified']);self.assertFalse(auxiliary['alignment_accepted'])
            # A supported cold native reference is selected by evidence, not
            # by whichever replay passes. All data here is synthetic.
            def raw_line(kind,chunk,values,index=0):
                return 'Lab idle geometryData request=1 eye=0 hand=0 index='+str(index)+' kind='+kind+' chunk='+str(chunk)+\
                    ' values='+','.join(f'{x:08x}' for x in values)
            word=lambda values:list(struct.unpack('<'+str(len(values))+'I',struct.pack('<'+str(len(values))+'f',*values)))
            native_program=PROGRAM.copy();native_program[-2]=0xa0e40001
            wrong_legacy=clip.copy();wrong_legacy[3]+=1
            native=[]
            for line in lines:
                if 'kind=constant ' in line:continue
                if 'Lab idle geometry request=' in line:line=line.replace('constants=4','constants=5')
                elif 'kind=program ' in line:line=raw_line('program',0,native_program)
                elif 'kind=clip ' in line:line=raw_line('clip',0,word(wrong_legacy))
                native.append(line)
            native += [raw_line('constant',0,[0]*4),*[raw_line('constant',i,word(clip[(i-1)*4:i*4])) for i in range(1,5)]]
            base='request=1 eye=0 hand=0'
            native += ['Lab idle projectionSummary '+base+' configured=1 count=1 invalidations=0 blocked=0 pending=0',
                       'Lab idle projectionPair '+base+' sequence=1 source=1 complete=1 controlBefore=127 controlAfter=127 flagsBefore=0 flagsAfter=6 modelBefore=1000 modelAfter=1000 drawBefore=2000 drawAfter=2000 cleanupCertified=0 outerCurrent=0']
            projection=[2.,0.,0.,0.,0.,3.,0.,0.,0.,0.,4.,0.,0.,0.,0.,1.]
            identity34=IDENTITY[:12]
            for phase in (0,1):
                for kind,values in [('model',affine),('view',identity34),('projection',projection),
                                    ('cachedVP',projection if phase else [0.]*16),('cachedMVP',clip if phase else [0.]*16)]:
                    native.append('Lab idle projectionData '+base+' sequence=1 phase='+str(phase)+' kind='+kind+
                                  ' values='+','.join(f'{x:08x}' for x in word(values)))
            native.append('Lab idle geometryFactors '+base+' index=0 palette=0 bookends=3 postOriginal=1 cleanupCertified=0 outerCurrent=0')
            for kind,values in [('factorModel',affine),('factorLocal',identity34),('factorView',identity34),('factorProjection',projection)]:
                native.append(raw_line(kind,0,word(values)))
            native.append('Lab idle geometryProjection '+base+' index=0 sequence=1 modelAddress=1000 drawAddress=2000 bookends=3 postOriginal=1 cleanupCertified=0 outerCurrent=0')
            native_log='\n'.join(native)
            corrected=replay(assess(native_log,source),index,root,EVALUATOR,root)
            first=corrected['draws'][0]
            self.assertTrue(first['position_replay']['position_replay_agrees_with_reference'])
            self.assertFalse(first['legacy_position_replay']['position_replay_agrees_with_reference'])
            self.assertEqual(first['reference_kind'],'cold-first-material-pc24-nearest-staged-local')
            self.assertFalse(first['render_geometry']['cleanup_certified'])
            self.assertFalse(first['render_geometry']['positive_grasp_verified'])
            # Legacy succeeds against the shader's c0 matrix, while the
            # qualified native reference fails. Never rescue it with legacy.
            shader_c0=native_log.replace(raw_line('program',0,native_program),raw_line('program',0,PROGRAM))
            legacy_c0=[0.]*4+clip[:12]
            shader_c0=shader_c0.replace(raw_line('clip',0,word(wrong_legacy)),raw_line('clip',0,word(legacy_c0)))
            failed=replay(assess(shader_c0,source),index,root,EVALUATOR,root)['draws'][0]
            self.assertTrue(failed['legacy_position_replay']['position_replay_agrees_with_reference'])
            self.assertFalse(failed['position_replay']['position_replay_agrees_with_reference'])
            self.assertNotIn('render_geometry',failed)
            missing='\n'.join(v for v in native if 'geometryProjection ' not in v)
            missing_result=replay(assess(missing,source),index,root,EVALUATOR,root)['draws'][0]
            self.assertEqual(missing_result['reference_kind'],'legacy-collapsed-matrix')
            self.assertNotIn('legacy_position_replay',missing_result)
            # Signed-zero arithmetic is unsupported, not structural corruption:
            # preserve legacy and unrelated rows but suppress this geometry.
            vp='phase=1 kind=cachedVP values=40000000,00000000'
            unknown=native_log.replace(vp,'phase=1 kind=cachedVP values=40000000,80000000')
            unknown=unknown.replace(raw_line('clip',0,word(wrong_legacy)),raw_line('clip',0,word(clip)))
            row_unknown=replay(assess(unknown,source),index,root,EVALUATOR,root)['draws'][0]
            self.assertTrue(row_unknown['legacy_position_replay']['position_replay_agrees_with_reference'])
            self.assertFalse(row_unknown['position_replay']['position_replay_agrees_with_reference'])
            self.assertEqual(row_unknown['reference_kind'],'native-cold-arithmetic-unknown')
            self.assertNotIn('render_geometry',row_unknown)
            with self.assertRaises(ValueError):replay(assess(native_log.replace('modelAddress=1000','modelAddress=1001'),source),index,root,EVALUATOR,root)
            second=[v.replace('index=0 ','index=1 ') for v in native if v.startswith('Lab idle geometry') and
                    'geometryProjection ' not in v and 'geometryFactors ' not in v and 'kind=factor' not in v]
            second=[v.replace(raw_line('clip',0,word(wrong_legacy),1),raw_line('clip',0,word(clip),1)) for v in second]
            mixed_log='\n'.join([*native,*second]).replace('draws=1','draws=2').replace('schema=3 ','schema=3 copyLayout=1 ')
            mixed=replay(assess(mixed_log,source),index,root,EVALUATOR,root)
            self.assertEqual([r['reference_kind'] for r in mixed['draws']],['cold-first-material-pc24-nearest-staged-local','legacy-collapsed-matrix'])
            self.assertTrue(all(r['position_replay']['position_replay_agrees_with_reference'] for r in mixed['draws']))
            partial=replay(assess(mixed_log.replace(vp,'phase=1 kind=cachedVP values=40000000,80000000'),source),index,root,EVALUATOR,root)
            self.assertFalse(partial['draws'][0]['position_replay']['position_replay_agrees_with_reference'])
            self.assertTrue(partial['draws'][1]['position_replay']['position_replay_agrees_with_reference'])
            stored=[v.replace('Lab idle geometry','Lab idle retainedGeometry') for v in [*native,*second] if v.startswith('Lab idle geometry')]
            from idle_stream_evidence_checks import fixture as input_failure_fixture
            failure=[v.replace('request=100','request=1').replace('hand=1','hand=0')
                     for v in input_failure_fixture().splitlines() if v.startswith('Lab idle input')]
            retained=[native[0].replace('draws=1','draws=2').replace('stage=4','stage=3').replace('schema=3 ','schema=3 copyLayout=1 '),
                      'Lab idle rejection '+base+' reason=32 preceding=2 checks=0 state=63 callbacks=63',
                      *failure,
                      'Lab idle retainedCopies '+base+' count=2 postOriginal=1 cleanupCertified=0 outerCurrent=0',
                      *[v for v in native if v.startswith('Lab idle projection')],*stored]
            history=replay(assess('\n'.join(retained),source),index,root,EVALUATOR,root)
            self.assertTrue(history['retained_diagnostic_draws'][0]['position_replay']['position_replay_agrees_with_reference'])
            self.assertFalse(history['all_consumed_positions_agree_with_native_reference'])
            self.assertFalse(history['copied_geometry_coverage_complete']);self.assertFalse(history['alignment_accepted'])
            (root/'positions.bin').write_bytes(bytes(36))
            with self.assertRaises(ValueError):replay(evidence,index,root,EVALUATOR,root)
            with self.assertRaises(ValueError):replay(assess('\n'.join(noUV),source),index,root,EVALUATOR,root)

if __name__=='__main__':unittest.main()
