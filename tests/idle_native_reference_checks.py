import copy
import hashlib
from fractions import Fraction
from pathlib import Path
import struct
import sys
import unittest
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from idle_native_reference import first_material_matrices, native_reference, uploaded_transform_reference, uploaded_api_association, round24, UnsupportedNativeArithmetic, UnsupportedUploadedTransform, UPLOADED_DECLARATION
from idle_projection_evidence_checks import fixture, M, P, SOURCE
from assess_idle_weapon import assess

def words(values):return list(struct.unpack('<'+str(len(values))+'I',struct.pack('<'+str(len(values))+'f',*values)))

def uploaded_fixture():
    # Authored synthetic shader, not proprietary captured bytecode. The test
    # replaces the hash pin only in memory to exercise qualification behavior.
    program=[0xfffe0101,31,0x80000005,0x900f0000,
             81,0xa00f00ff,0x3f800000,0,0,0,
             21,0x80070000,0x90e40000,0xa0e40015,
             1,0x80080000,0xa00000ff,
             20,0xc00f0000,0x80e40000,0xa0e40001]
    program += [5,0x800f0003,0xa0e40064,0xa0e40065]*73
    program += [1,0x800f0003,0xa0e40064]*2+[0xffff]
    if len(program)!=320:raise RuntimeError('Synthetic program length')
    digest=hashlib.sha256(struct.pack('<320I',*program)).hexdigest()
    lines=[line for line in fixture().splitlines() if not (
        line.split()[2].startswith('projection') or 'geometryProjection ' in line or
        any('kind='+kind+' ' in line for kind in ('program','constant','declaration')))]
    lines=[line.replace('schema=3 ','schema=4 copyLayout=1 ').replace('words=2 constants=5 declaration=5',
        'words=320 constants=256 declaration=8') for line in lines]
    def data(kind,chunk,values):
        return 'Lab idle geometryData request=100 eye=1 hand=0 index=0 kind='+kind+' chunk='+str(chunk)+' values='+','.join(f'{v:08x}' for v in values)
    for i in range(0,320,32):lines.append(data('program',i,program[i:i+32]))
    for i in range(256):
        value=P[(i-1)*4:i*4] if 1<=i<=4 else M[(i-21)*4:(i-20)*4] if 21<=i<=23 else [0]*4
        lines.append(data('constant',i,value))
    for i,decl in enumerate(UPLOADED_DECLARATION):lines.append(data('declaration',i,decl))
    base='request=100 eye=1 hand=0'
    lines += ['Lab idle submissionSummary '+base+' attempts=1 count=1 overflow=0 outerReturned=1 limit=64',
              'Lab idle submissionRow '+base+' index=0 ordinal=1 status=8 hr=0 flags=15 draw=4,0,0,317,0,338',
              'Lab idle submissionMetadata '+base+' index=0 keys=1000,2000,1,0,40,50,60,70,0 root=20,30,5 render=20,30,5 layout=317,338,0,133,0,0,135,0,12680,128,0,13948,128,0']
    log='\n'.join(lines)
    record=assess(log,SOURCE)['copied_event_pose_observations'][0]
    channels={'weights':bytes((255,0,0,0))*317,'local_indices':bytes(4*317),
              'positions':bytes(12*317),'uv':bytes(8*317),'indices':bytes(6*338)}
    return record,record['geometry'][0],channels,digest,log

class Checks(unittest.TestCase):
    def test_sparse_api_association_and_mvp_fence(self):
        record,g,channels,digest,_=uploaded_fixture()
        own=record['submissions']['rows'][0];extra=copy.deepcopy(own)
        extra['metadata']['keys'][3]=9;extra['draw'][3]=1
        own['ordinal']=2
        record['submissions'].update(attempts=2,count=2,rows={0:extra,1:own})
        association=uploaded_api_association(record,g,0)
        self.assertEqual(association['api_index'],1);self.assertEqual(association['api_ordinal'],2)
        self.assertFalse(association['api_geometry_coverage_complete'])
        with patch('idle_native_reference.UPLOADED_PROGRAM_HASH',digest):
            self.assertEqual(uploaded_transform_reference(record,g,channels,0)['api_association'],association)
            bad=copy.deepcopy(record);bad['geometry'][0]['data']['constant:2'][2]^=1
            with self.assertRaisesRegex(UnsupportedUploadedTransform,'mvp-word-disagreement') as caught:
                uploaded_transform_reference(bad,bad['geometry'][0],channels,0)
            self.assertEqual(caught.exception.api_association,association)
        for case in ('missing','duplicate','foreign-root','unqualified','incomplete-copy','overflow','wrong-ordinal','not-returned'):
            r=copy.deepcopy(record)
            if case=='missing':del r['submissions']['rows'][1]
            elif case=='duplicate':
                r['submissions']['rows'][0]=copy.deepcopy(r['submissions']['rows'][1]);r['submissions']['rows'][0]['ordinal']=1
            elif case=='foreign-root':r['submissions']['rows'][0]['metadata']['root'][0]+=1
            elif case=='unqualified':r['submissions']['rows'][0]['flags']=7
            elif case=='incomplete-copy':r['geometry']={}
            elif case=='overflow':r['submissions']['overflow']=1
            elif case=='wrong-ordinal':r['submissions']['rows'][1]['ordinal']=1
            else:r['submissions']['outerReturned']=0
            with self.subTest(case=case),self.assertRaises(UnsupportedUploadedTransform):uploaded_api_association(r,g,0)
        json_record=__import__('json').loads(__import__('json').dumps(record))
        self.assertEqual(uploaded_api_association(json_record,json_record['geometry']['0'],0),association)

    def test_api_mapping_preserves_repeated_ordinals_and_rejects_sparse_reuse_or_reordering(self):
        r,g,_,_,_=uploaded_fixture();own=r['submissions']['rows'][0]
        r['geometry'][1]=copy.deepcopy(g);r['draws']=2
        r['submissions'].update(attempts=2,count=2)
        r['submissions']['rows'][1]=copy.deepcopy(own);r['submissions']['rows'][1]['ordinal']=2
        for index in (0,1):
            a=uploaded_api_association(r,r['geometry'][index],index)
            self.assertEqual(a['api_index'],index);self.assertTrue(a['api_geometry_coverage_complete'])
        extra=copy.deepcopy(own);extra['metadata']['keys'][3]=10;extra['ordinal']=3
        r['submissions'].update(attempts=3,count=3);r['submissions']['rows'][2]=extra
        # Two copies sharing a sole row cannot be reused in the sparse path.
        r['submissions']['rows'][1]['metadata']['keys'][3]=11
        with self.assertRaisesRegex(UnsupportedUploadedTransform,'ordered-injective'):
            uploaded_api_association(r,g,0)
        r['geometry'][1]['drawRecord']=11
        self.assertEqual(uploaded_api_association(r,r['geometry'][1],1)['api_index'],1)
        r['geometry'][0],r['geometry'][1]=r['geometry'][1],r['geometry'][0]
        with self.assertRaisesRegex(UnsupportedUploadedTransform,'ordered-injective'):
            uploaded_api_association(r,r['geometry'][0],0)

    def test_sparse_replay_does_not_promote_whole_api_coverage(self):
        from replay_idle_geometry import replay
        evidence={'source_fingerprint':SOURCE,'copied_event_pose_observations':[{'draws':1,'submissions':{'attempts':2}}],
                  'rejected_or_missing_observations':[]}
        matching={'matches':[],'retained_diagnostic_matches':[],
                  'observations_without_geometry':[],'copied_geometry_coverage_complete':True}
        # Unknown program can return the historical legacy verdict without an
        # association. Whole coverage must still reject surplus native draws.
        row={'reference_kind':'legacy-collapsed-matrix',
             'position_replay':{'position_replay_agrees_with_reference':True}}
        with patch('replay_idle_geometry.match',return_value=matching), \
             patch('replay_idle_geometry.replay_draws',side_effect=[[row],[]]):
            result=replay(evidence,{},Path('.'),Path('.'),Path('.'))
        self.assertTrue(result['all_copied_consumed_positions_agree_with_native_reference'])
        for field in ('all_consumed_positions_agree_with_native_reference','api_geometry_coverage_complete',
                      'gpu_execution','native_execution','positive_grasp_verified','alignment_accepted'):
            self.assertFalse(result[field])

    def test_uploaded_copy_is_independent_and_bounded(self):
        record,g,channels,digest,log=uploaded_fixture()
        with patch('idle_native_reference.UPLOADED_PROGRAM_HASH',digest):
            ref=uploaded_transform_reference(record,g,channels,0)
            self.assertEqual(ref['kind'],'uploaded-transform-corroboration')
            self.assertEqual(ref['matrix'],P);self.assertEqual(ref['local'],M)
            for key in ('uploads_used_as_inputs','cache_outputs_used_as_inputs','native_producer_verified',
                        'pc_rc_observed','positive_grasp_verified','alignment_accepted'):
                self.assertFalse(ref[key])
            old=copy.deepcopy(record);old['schema']=3
            self.assertIsNone(uploaded_transform_reference(old,g,channels,0))
            other=copy.deepcopy(g);other['data']['program:0'][1]^=1
            self.assertIsNone(uploaded_transform_reference(record,other,channels,0))
            # A foreign-eye line cannot be attached to this owned record.
            with self.assertRaises(ValueError):assess(log.replace(
                'geometryData request=100 eye=1 hand=0 index=0 kind=constant chunk=1 ',
                'geometryData request=100 eye=0 hand=0 index=0 kind=constant chunk=1 '),SOURCE)

    def test_uploaded_disagreement_cannot_use_passing_legacy(self):
        record,g,channels,digest,_=uploaded_fixture()
        with patch('idle_native_reference.UPLOADED_PROGRAM_HASH',digest):
            for what in ('R','R-zero','L','L-zero','decl','bookends','api','owner','cross-copy','cross-eye'):
                r=copy.deepcopy(record);v=r['geometry'][0]
                if what=='R':v['data']['constant:1'][0]^=1
                elif what=='R-zero':v['data']['constant:1'][1]=0x80000000
                elif what=='L':v['data']['constant:21'][0]^=1
                elif what=='L-zero':v['data']['constant:21'][1]=0x80000000
                elif what=='decl':v['data']['declaration:2'][2]=1
                elif what=='bookends':v['factors']['bookends']=1
                elif what=='api':r['submissions']['rows'][0]['status']=6
                elif what=='owner':r['stage']=3
                elif what=='cross-copy':v['instance']+=1
                else:v['data']['factorProjection:0'][0]=0x40000000
                with self.subTest(what=what),self.assertRaises(UnsupportedUploadedTransform):
                    uploaded_transform_reference(r,v,channels,0)
            for name in ('weights','local_indices'):
                bad=dict(channels);value=bytearray(bad[name]);value[0]^=1;bad[name]=bytes(value)
                with self.assertRaises(UnsupportedUploadedTransform):uploaded_transform_reference(record,g,bad,0)
            missing=dict(channels);missing['weights']=missing['weights'][:-1]
            with self.assertRaises(ValueError):uploaded_transform_reference(record,g,missing,0)
            bad=copy.deepcopy(g);bad['data']['factorLocal:0'][0]=0x7fc00000
            with self.assertRaises(ValueError):uploaded_transform_reference(record,bad,channels,0)
            huge=copy.deepcopy(g);huge['data']['factorModel:0'][0]=words([3e38])[0]
            huge['data']['factorProjection:0'][0]=0x40000000
            with self.assertRaises(UnsupportedUploadedTransform):
                uploaded_transform_reference(record,huge,channels,0)

    def test_qualified_uploaded_verdict_and_mixed_draws(self):
        from replay_idle_geometry import replay_draws
        record,g,channels,digest,_=uploaded_fixture()
        other=copy.deepcopy(record);other['request']=101
        other['geometry'][0]['data']['program:0'][1]^=1
        candidates={'synthetic.mesh':{'asset_sha256':'0'*64,'candidate_channels':[
            {'channel_sha256':{k:hashlib.sha256(v).hexdigest() for k,v in channels.items()}}]}}
        rows=[{'request':r['request'],'eye':r['eye'],'hand':r['hand'],'geometry_index':0,
               'result':'unique-consumed-channel-match','candidates':[{'candidate':'synthetic.mesh','channel_index':0}]}
              for r in (record,other)]
        def evaluator(*args):
            reference=args[-1] if isinstance(args[-1],dict) else None
            return {'position_replay_agrees_with_reference':reference is None,'reason':'intentional-test-verdict'}
        with patch('idle_native_reference.UPLOADED_PROGRAM_HASH',digest), \
             patch('replay_idle_geometry.channel_bytes',return_value=channels), \
             patch('replay_idle_geometry.evaluate_geometry',side_effect=evaluator):
            out=replay_draws(rows,[record,other],candidates,Path('.'),Path('.'),Path('.'))
            self.assertFalse(out[0]['position_replay']['position_replay_agrees_with_reference'])
            self.assertTrue(out[0]['legacy_position_replay']['position_replay_agrees_with_reference'])
            self.assertEqual(out[0]['reference_kind'],'uploaded-transform-corroboration')
            self.assertNotIn('render_geometry',out[0])
            self.assertTrue(out[1]['position_replay']['position_replay_agrees_with_reference'])
            self.assertEqual(out[1]['reference_kind'],'legacy-collapsed-matrix')
            bad=copy.deepcopy(record);bad['geometry'][0]['data']['constant:1'][0]^=1
            failed=replay_draws(rows[:1],[bad],candidates,Path('.'),Path('.'),Path('.'))[0]
            self.assertEqual(failed['reference_kind'],'uploaded-transform-uncorroborated')
            self.assertFalse(failed['position_replay']['position_replay_agrees_with_reference'])
            self.assertNotIn('render_geometry',failed)
            bad=copy.deepcopy(record)
            bad['geometry'][0]['data']['factorModel:0'][0]=words([3e38])[0]
            bad['geometry'][0]['data']['factorProjection:0'][0]=0x40000000
            overflow=replay_draws(rows[:1],[bad],candidates,Path('.'),Path('.'),Path('.'))[0]
            self.assertEqual(overflow['reference_kind'],'uploaded-transform-uncorroborated')
            self.assertEqual(overflow['position_replay']['reason'],'unsupported-matrix-store')
            self.assertEqual(overflow['api_association']['api_index'],0)
            self.assertEqual(overflow['api_association']['api_ordinal'],1)

    def test_measurement_retains_uploaded_and_world_provenance(self):
        from measure_idle_reference import measure
        xyz=[[0.,0.,0.],[1.,0.,0.],[0.,1.,0.]]
        indices=[0,1,2]
        hashes={'positions':hashlib.sha256(struct.pack('<9f',*[v for p in xyz for v in p])).hexdigest(),
                'indices':hashlib.sha256(struct.pack('<3H',*indices)).hexdigest()}
        identity={'asset_sha256':'0'*64,'mesh_object':1,'lod':0,'channel_index':0}
        geometry={'positions':xyz,'triangle_indices':indices,'channel_sha256':hashes,
                  'affine':[float(v) for v in (1,0,0,0,0,1,0,0,0,0,1,0)],'raw_grip':None,
                  'reference_kind':'uploaded-transform-corroboration',
                  'world_positions_independently_verified':False,'world_position_reference':'collapsed-affine-unverified'}
        row={'binding':{},'geometry_index':0,'draw_record':0,'candidates':[identity],
             'reference_kind':'uploaded-transform-corroboration',
             'position_replay':{'position_replay_agrees_with_reference':True},'render_geometry':geometry}
        replay={'schema':1,'alignment_accepted':False,'source_fingerprint':SOURCE,'draws':[row]}
        annotation={'schema':1,'reference_kind':'indexed-surface-convention','semantic_status':'candidate',
                    **identity,'channel_sha256':hashes,'landmarks':{
                      key:{'triangle':0,'ordered_vertex_indices':indices,'barycentric':weights}
                      for key,weights in zip(('P','I','T'),([1,0,0],[0,1,0],[0,0,1]))}}
        out=measure(replay,annotation)['measured_references'][0]
        self.assertEqual(out['reference_kind'],'indexed-surface-convention')
        self.assertEqual(out['position_reference_kind'],'uploaded-transform-corroboration')
        self.assertFalse(out['world_positions_independently_verified'])
        self.assertEqual(out['world_position_reference'],'collapsed-affine-unverified')
        for what in ('missing','contradiction','missing-geometry-kind','different-geometry-kind','missing-row-kind','different-row-kind'):
            bad=copy.deepcopy(replay);g=bad['draws'][0]['render_geometry']
            if what=='missing':g.pop('world_position_reference')
            elif what=='contradiction':g['world_positions_independently_verified']=True
            elif what=='missing-geometry-kind':g.pop('reference_kind')
            elif what=='different-geometry-kind':g['reference_kind']='legacy-collapsed-matrix'
            elif what=='missing-row-kind':bad['draws'][0].pop('reference_kind')
            else:bad['draws'][0]['reference_kind']='legacy-collapsed-matrix'
            with self.assertRaises(ValueError):measure(bad,annotation)

    def test_identity_and_independent_model_translation(self):
        self.assertEqual(first_material_matrices(M,M,P),(P,P))
        model=M.copy();model[3]=0x41200000;model[7]=0xc1200000
        q,r=first_material_matrices(model,M,P)
        self.assertEqual(q,P)
        self.assertEqual(r[:12],model);self.assertEqual(r[12:],P[12:])

    def test_native_add_order_and_precision_cancellation(self):
        view=words([2**24,2**24,0,0,1,1,0,0,-2**24,-2**24,1,0])
        projection=words([1,1,1,0,0,1,0,0,1,1,1,0,0,0,0,1])
        q,_=first_material_matrices(M,view,projection)
        self.assertEqual(q[0],0) # (1+large)+(-large), PC24 stores the tie.
        self.assertEqual(q[9],0x3f800000) # (1+(-large))+large.
        model=words([2**24,0,0,0,1,1,0,0,-2**24,0,1,0])
        _,r=first_material_matrices(model,M,projection)
        self.assertEqual(r[0],0x3f800000) # (-large+large)+1.
        self.assertEqual(round24(Fraction(1)+Fraction(1,2**24)),Fraction(1))
        self.assertEqual(round24(Fraction(1)+Fraction(3,2**24)),Fraction(1)+Fraction(1,2**22))

    def test_float_stores_and_invalid_inputs(self):
        projection=P.copy();projection[0]=1
        view=M.copy();view[0]=0x3f000000
        q,r=first_material_matrices(M,view,projection)
        self.assertEqual((q[0],r[0]),(0,0)) # Binary32 store of half smallest subnormal.
        for bad in (M[:-1],M+[0],[True,*M[1:]],[0x7f800000,*M[1:]],[0x7fc00000,*M[1:]]):
            with self.assertRaises(ValueError):first_material_matrices(bad,M,P)
        huge=words([3e38,0,0,0,0,1,0,0,0,0,1,0])
        projection=P.copy();projection[0]=0x40000000
        with self.assertRaises(ValueError):first_material_matrices(M,huge,projection)

    def test_only_observed_cold_associations_are_supported(self):
        for retained in (False,True):
            evidence=assess(fixture(retained),SOURCE)
            record=evidence['rejected_or_missing_observations' if retained else 'copied_event_pose_observations'][0]
            g=(record['retained_copies']['geometry'] if retained else record['geometry'])[0]
            reference=native_reference(record,g)
            self.assertEqual(reference['matrix'],P);self.assertEqual(reference['local'],M)
            self.assertFalse(reference['cache_outputs_used_as_inputs']);self.assertFalse(reference['alignment_accepted'])
            for flags in (2,4,6):
                other=assess(fixture(retained,flags),SOURCE)
                r=other['rejected_or_missing_observations' if retained else 'copied_event_pose_observations'][0]
                geometry=(r['retained_copies']['geometry'] if retained else r['geometry'])[0]
                self.assertIsNone(native_reference(r,geometry))
            no_association=copy.deepcopy(g);no_association.pop('projection_association')
            self.assertIsNone(native_reference(record,no_association))
            for control in (383,639,895,1151,2175,3199):
                r=copy.deepcopy(record);r['projection_probe']['pairs'][1]['controlBefore']=control
                r['projection_probe']['pairs'][1]['controlAfter']=control
                self.assertIsNone(native_reference(r,g))
            for flags in (1,8,9):
                r=copy.deepcopy(record);p=r['projection_probe']['pairs'][1]
                p['flagsBefore']=flags;p['flagsAfter']=flags|6
                self.assertIsNone(native_reference(r,g))

    def test_outputs_cannot_seed_the_independent_reference(self):
        record=assess(fixture(),SOURCE)['copied_event_pose_observations'][0]
        g=record['geometry'][0]
        r=copy.deepcopy(record);p=r['projection_probe']['pairs'][1]
        p['data']['1:cachedVP'][0]+=1
        with self.assertRaises(UnsupportedNativeArithmetic):native_reference(r,r['geometry'][0])
        r=copy.deepcopy(record);p=r['projection_probe']['pairs'][1]
        p['data']['0:projection'][0]+=1;p['data']['1:projection'][0]+=1
        r['geometry'][0]['data']['factorProjection:0'][0]+=1
        with self.assertRaises(UnsupportedNativeArithmetic):native_reference(r,r['geometry'][0])
        r=copy.deepcopy(record);p=r['projection_probe']['pairs'][1]
        p['data']['1:cachedVP'][1]=0x80000000 # Signed-zero disagreement rejects.
        with self.assertRaises(UnsupportedNativeArithmetic):native_reference(r,r['geometry'][0])
        r=copy.deepcopy(record);r['geometry'][0]['data']['factorLocal:0'][0]=0x7fc00000
        with self.assertRaises(ValueError):native_reference(r,r['geometry'][0])
        self.assertEqual(native_reference(record,g)['matrix'],P)

if __name__=='__main__':unittest.main()
