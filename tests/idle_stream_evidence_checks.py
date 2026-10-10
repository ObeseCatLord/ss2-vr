import copy
from pathlib import Path
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from assess_idle_weapon import assess
from idle_stream_evidence import OBSERVED,NO_UV56,OBSERVED_MULTI_UV,PASSIVE_FIVE_ROW78,diagnostic_stream_numbers,declaration_layout

SOURCE='a'*64
BASE='request=100 eye=0 hand=1'

def fixture(declaration=OBSERVED):
    lines=[f'Lab idle draw schema=3 rawGripValid=1 draws=0 source={SOURCE} ipc=10 wire=7 request=100 input=90 owner=1 weapon=2 model=3 generation=4 hand=1 eye=0 stage=3 cfg=20 file=30 resource=1 contributors=1 matrices=4 historicalBytes=0 grasp=0',
           'Lab idle rejection '+BASE+' reason=32 preceding=2 checks=0 state=63 callbacks=63',
           'Lab idle inputFailure '+BASE+' step=20 index=0 hr=0 valid=15 caps=2 declaration=6 rangeChecks=6015']
    lines += ['Lab idle inputDeclaration '+BASE+' index='+str(i)+' values='+','.join(map(str,e)) for i,e in enumerate(OBSERVED)]
    lines += ['Lab idle inputBinding '+BASE+' vertex=70272,0,1,100,0 index=10308,0,1,101,0 draw=4,0,0,218,0,248 software=0']
    streams=[(1,0,12,1),(2,13948,4,1),(1,59328,8,1),(0,0,0,0)]
    for i,v in enumerate(streams):
        lines+=['Lab idle inputStream '+BASE+f' index={i} object={v[0]} offset={v[1]} stride={v[2]} frequency={v[3]}']
    lines+=['Lab idle inputSurface '+BASE+' vertices=218 triangles=248']
    for i,(offset,fmt) in enumerate(((0,133),(0,135),(48384,128),(49256,128))):
        lines+=['Lab idle inputChannel '+BASE+f' index={i} offset={offset} format={fmt} buffer=0']
    lines+=['Lab idle streamProbe '+BASE+' attempts=2 flags=511 words=2 invalidations=0 forwardResult=0']
    for phase in range(2):
        p=BASE+' phase='+str(phase)
        lines+=['Lab idle streamSnapshot '+p+' status=1 step=0 index=0 hr=0',
                'Lab idle streamBinding '+p+' caps=2 declaration=6 declarationObject=5 indexObject=4 shaderObject=6']
        for i,offset,stride in ((0,0,12),(7,48384,4),(8,49256,4)):
            lines+=['Lab idle streamInput '+p+f' index={i} object=1 offset={offset} stride={stride} frequency=1']
        lines += ['Lab idle streamDeclaration '+p+' index='+str(i)+' values='+','.join(map(str,e)) for i,e in enumerate(OBSERVED)]
        for i in range(2):lines+=['Lab idle streamConstant '+p+f' index={i} values=00000000,00000000,00000000,00000000']
    lines+=['Lab idle streamProgram '+BASE+' chunk=0 values=fffe0101,0000ffff']
    if declaration==NO_UV56:
        lines=[line.replace('declaration=6','declaration=5').replace('rangeChecks=6015','rangeChecks=8191')
               for line in lines if not (('inputDeclaration' in line or 'streamDeclaration' in line) and 'index=5 values=' in line)]
        for n,line in enumerate(lines):
            if 'inputDeclaration' in line or 'streamDeclaration' in line:
                index=int(line.split(' index=')[1].split()[0])
                lines[n]=line.split(' values=')[0]+' values='+','.join(map(str,NO_UV56[index]))
            elif 'inputStream' in line and 'index=1 ' in line:
                lines[n]=line.replace('object=2 offset=13948','object=1 offset=49256')
            elif 'inputStream' in line and 'index=3 ' in line:
                lines[n]=line.replace('object=0 offset=0 stride=0 frequency=0','object=1 offset=48384 stride=4 frequency=1')
            elif 'streamInput' in line:
                lines[n]=line.replace('index=7 object=1 offset=48384','index=5 object=1 offset=49256').replace('index=8 object=1 offset=49256','index=6 object=1 offset=48384')
    elif declaration==PASSIVE_FIVE_ROW78:
        reduced=[]
        for line in lines:
            line=line.replace('declaration=6','declaration=5').replace('rangeChecks=6015','rangeChecks=6143 layout=0')
            if 'inputDeclaration' in line or 'streamDeclaration' in line:
                index=int(line.split(' index=')[1].split()[0])
                if index==2:continue
                new_index=index-1 if index>2 else index
                line=line.split(' index=')[0]+' index='+str(new_index)+' values='+','.join(map(str,PASSIVE_FIVE_ROW78[new_index]))
            reduced.append(line)
        lines=reduced
    elif declaration==OBSERVED_MULTI_UV:
        expanded=[]
        for line in lines:
            line=line.replace('declaration=6','declaration=8')
            if 'inputDeclaration' in line or 'streamDeclaration' in line:
                index=int(line.split(' index=')[1].split()[0])
                prefix=line.split(' index=')[0]
                if index==3:
                    for extra in (3,4):
                        expanded.append(prefix+' index='+str(extra)+' values='+','.join(map(str,OBSERVED_MULTI_UV[extra])))
                new_index=index+2 if index>=3 else index
                line=prefix+' index='+str(new_index)+' values='+','.join(map(str,OBSERVED_MULTI_UV[new_index]))
            expanded.append(line)
        lines=expanded
    return '\n'.join(lines)

def palette_fixture(count=3,selected=2,attempts=3,declaration=NO_UV56,equal=True,content=None):
    lines=[f'Lab idle draw schema=4 nativeId=2 copyLayout=1 rawGripValid=0 draws=0 source={SOURCE} ipc=10 wire=7 request=100 input=90 owner=1 weapon=2 model=3 generation=4 hand=1 eye=0 stage=3 cfg=20 file=30 resource=1 contributors=1 matrices=4 historicalBytes=0 grasp=0',
           'Lab idle rejection '+BASE+' reason=28 preceding=2 checks=0 state=47 callbacks=63',
           'Lab idle submissionSummary '+BASE+f' attempts={attempts} count={attempts} overflow=0 outerReturned=1 limit=64']
    for i in range(attempts):
        lines+=['Lab idle submissionRow '+BASE+f' index={i} ordinal={i+1} status=8 hr=0 flags=15 draw=4,0,0,3,0,1',
                'Lab idle submissionMetadata '+BASE+f' index={i} keys=1000,{2000+i*32},1,{3+i},3000,4000,123,{1200 if count==1 else 0},{2 if count==1 else -1} root=20,30,1 render=50,60,2 layout=3,1,0,133,0,0,135,0,48,128,0,36,128,0']
    lines+=['Lab idle paletteSummary schema=1 '+BASE+f' source={SOURCE} payloads={min(attempts,10)} overflow={int(attempts>10)} outerCompleted=1 alignment=0 grasp=0']
    if selected is None:return '\n'.join(lines)
    head=BASE+f' index={selected} ordinal={selected+1}'
    lines+=['Lab idle paletteRow schema=1 request=100 input=90 generation=4 owner=1 weapon=2 model=3 eye=0 hand=1 '+
            f'index={selected} ordinal={selected+1} api={selected} modelAddress=1000 drawAddress={2000+selected*32} modelRecord=1 drawRecord={3+selected} instance=4000 surface=3000 root=20,30,1 render=50,60,2 evaluated=7000 matrices=8000 mapping=9000 palette=10000 first=4 count={count} mapCount=7 paletteCount=7 canonicalCount=12 modelCount=2 canonicalEqual={int(equal)} words=2 constants=4 declaration={len(declaration)} objects=20,21,22 cleanupCertified=1 outerCurrent=1']
    def data(kind,item,values,chunk=0):
        lines.append('Lab idle paletteData '+head+f' kind={kind} item={item} chunk={chunk} values='+','.join(f'{v:08x}' for v in values))
    matrix=[0x3f800000,0,0,0,0,0x3f800000,0,0,0,0,0x3f800000,0]
    data('world',0,matrix);data('draw',0,[4,0,0,3,0,1])
    for i in range(count):
        canonical=matrix.copy();canonical[3]=(0x40e00000,0x41600000,0x41a80000)[i]
        actual=canonical.copy()
        if not equal and i==0:actual[3]=0x3f800000
        data('mapping',i,[3+selected,2+i,1,1100+i*120,1200+i])
        data('canonical',i,canonical);data('palette',i,actual)
    for i,offset,stride in [(0,0,12),(1,36,4),(2,48,4)]:data('stream',i,[101,offset,stride,1])
    for i,e in enumerate(declaration):data('declaration',i,e)
    for i in range(4):data('constant',i,[0,0,0,0])
    data('program',0,[0xfffe0101,0xffff])
    if content is not None:
        lines=[line.replace('schema=1','schema=2') if 'paletteSummary' in line or 'paletteRow' in line else line for line in lines]
        lines=[line.replace('cleanupCertified=1',f'contentCopied={int(content)} cleanupCertified=1') if 'paletteRow' in line else line for line in lines]
        if content:
            data('contentSurface',0,[3,1,0,133,0,0,135,0,48,128,0,36,128,0])
            data('contentBuffers',0,[84,0,1,100,0,6,0,1,101,0])
            data('contentStreams',0,[101,0,12,1,101,36,4,1,101,48,4,1,101,60,8,1,21,0])
            for i in range(5):data('contentHash',i,[i+1]*8)
    return '\n'.join(lines)

def palette_projection_fixture():
    from idle_projection_evidence_checks import producer,P
    text=palette_fixture(content=True).replace('schema=2','schema=3')
    text=text.replace('contentCopied=1 ','contentCopied=1 projectionSequence=1 ').replace('constants=4 ','constants=5 ')
    for i in range(1,4):
        target=f'kind=constant item={i} chunk=0 values=00000000,00000000,00000000,00000000'
        text=text.replace(target,f'kind=constant item={i} chunk=0 values='+','.join(f'{v:08x}' for v in P[(i-1)*4:i*4]))
    text+='\nLab idle paletteData '+BASE+' index=2 ordinal=3 kind=constant item=4 chunk=0 values='+','.join(f'{v:08x}' for v in P[12:])
    text+='\n'+'\n'.join(line.replace('eye=1 hand=0','eye=0 hand=1').replace('drawBefore=2000 drawAfter=2000','drawBefore=2064 drawAfter=2064') for line in producer())
    return text

def boundary_fixture(attempts=1,excluded=(),**changes):
    text=palette_fixture(selected=None,attempts=attempts)
    # A normal original draw may lack any copied palette metadata.
    text='\n'.join(line for line in text.splitlines() if 'submissionMetadata' not in line)
    text=text.replace('status=8 hr=0 flags=15','status=1 hr=0 flags=4')
    values=dict(index=0,ordinal=1,api=0,phase=2,kind=1,wrapper=4,leg=1,reader=14,valid=7,
                root=4000,render=4100,evaluated=7000,linked=0,cacheOwner=0,canonicalCount=0,matrices=0,
                apiStatus=0,apiStep=0,apiIndex=0,apiResult=0,software=0)
    values.update(changes)
    identity=BASE+' input=90 generation=4 owner=1 weapon=2 model=3'
    indices=[n for n in range(min(attempts,10)) if n not in excluded]
    for n in excluded:
        text=text.replace(f'index={n} ordinal={n+1} status=1',f'index={n} ordinal={n+1} status=5')
    text+='\nLab idle paletteBoundarySummary schema=1 '+identity+f' source={SOURCE} count={len(indices)} diagnostic=1'
    for n in indices:
        row=values if n==0 else {k:0 for k in values}
        if n:row.update(index=n,ordinal=n+1,api=n)
        text+='\nLab idle paletteBoundary '+identity+' '+' '.join(f'{k}={v}' for k,v in row.items())
    return text

class Checks(unittest.TestCase):
    def test_boundary_unlinked_cache_is_unknown_and_not_a_reference(self):
        r=assess(boundary_fixture(),SOURCE)['rejected_or_missing_observations'][0]['submissions']
        b=r['palette_boundaries'];self.assertEqual(b['rows'][0]['reader'],14)
        self.assertEqual(b['rows'][0]['valid'],7)
        for key in ('reference_qualified','alignment_accepted','positive_grasp_verified','source_provenance_authenticated'):
            self.assertFalse(b[key])
        self.assertEqual(r['palette_copies']['rows'],{})
        with self.assertRaises(ValueError):assess(boundary_fixture(cacheOwner=4000),SOURCE)

    def test_boundary_each_copy_leg_and_api_failure(self):
        for phase in (2,4,5,7):
            for wrapper,leg in ((4,1),(8,2)):
                b=assess(boundary_fixture(phase=phase,wrapper=wrapper,leg=leg),SOURCE)
                self.assertEqual(b['rejected_or_missing_observations'][0]['submissions']['palette_boundaries']['rows'][0]['leg'],leg)
        for phase in (3,6):
            text=boundary_fixture(phase=phase,wrapper=0,leg=0,reader=0,valid=16,
                                  root=0,render=0,evaluated=0,apiStatus=2,apiStep=9,apiIndex=7,apiResult=-1)
            self.assertEqual(assess(text,SOURCE)['rejected_or_missing_observations'][0]['submissions']['palette_boundaries']['rows'][0]['apiResult'],-1)
        text=boundary_fixture(kind=0,phase=5,wrapper=0,leg=0,reader=0,valid=0,root=0,render=0,evaluated=0)
        self.assertEqual(assess(text,SOURCE)['rejected_or_missing_observations'][0]['submissions']['palette_boundaries']['rows'][0]['kind'],0)

    def test_boundary_join_capacity_completion_and_skipped_reads(self):
        text=boundary_fixture();lines=text.splitlines()
        for old,new in [('count=1 diagnostic=1','count=2 diagnostic=1'),
                        ('count=1 diagnostic=1','count=11 diagnostic=1'),
                        ('outerCompleted=1','outerCompleted=0'),('status=1 hr=0 flags=4','status=4 hr=0 flags=4'),
                        ('status=1 hr=0 flags=4','status=1 hr=-1 flags=4'),
                        ('status=1 hr=0 flags=4','status=1 hr=0 flags=0'),
                        (f'source={SOURCE} count=1','source='+'c'*64+' count=1')]:
            with self.assertRaises(ValueError,msg=old):assess(text.replace(old,new),SOURCE)
        for index in (-1,-2):
            with self.assertRaises(ValueError):assess(text+'\n'+lines[index],SOURCE)
        with self.assertRaises(ValueError):assess('\n'.join(lines[:-1]),SOURCE)
        bads=[dict(ordinal=2),dict(api=1),dict(wrapper=99),dict(phase=10),dict(leg=2),
              dict(reader=9),dict(valid=15),dict(cacheOwner=4000),dict(matrices=8000),
              dict(apiResult=-1),dict(software=-1,reader=3,valid=33,render=0,evaluated=0),
              dict(kind=0),dict(phase=0)]
        # Software guard failure is valid only for a copied value other than -1.
        for changes in bads:
            with self.assertRaises(ValueError,msg=str(changes)):assess(boundary_fixture(**changes),SOURCE)
        for old,new in [('input=90','input=91'),('generation=4','generation=5'),('owner=1','owner=9'),
                        ('model=3','model=8'),('hand=1','hand=0'),('eye=0','eye=1')]:
            with self.assertRaises(ValueError,msg=old):assess(text.replace(lines[-1],lines[-1].replace(old,new)),SOURCE)

    def test_boundary_failure_freeze_unwind_and_normal_outer_are_source_bound(self):
        from verify_idle_submission_abi import source_checks
        root=Path(__file__).resolve().parents[1]
        engine=(root/'src/game/engine.cpp').read_text();gpu=(root/'src/game/scope_gpu.cpp').read_text()
        for old,new in [('copy.wrapper==99','copy.wrapper==0'),('if(receipt.kind)return;',''),
                        ('if(!receipt.kind) {','if(true) {'),
                        ('if(!paletteApiOwnerCurrent(d,owner,slot,api) || copy.wrapper==99)return;',
                         'if(copy.wrapper==99)return;')]:
            bad=gpu.replace(old,new);self.assertNotEqual(gpu,bad)
            with self.assertRaises(ValueError):source_checks(engine,bad)
        for old,new in [('diagnostic->wrapper!=99','true'),('diagnostic->wrapper=99;','diagnostic->wrapper=1;'),
                        ('emitIdlePaletteBoundaries(*idleStorage);',''),('trace.paletteBoundaryPublishable(n)','true')]:
            bad=engine.replace(old,new);self.assertNotEqual(engine,bad)
            with self.assertRaises(ValueError):source_checks(bad,gpu)

    def test_boundary_api_rejects_impossible_completion_and_phase(self):
        def api(**changes):
            fields=dict(phase=6,wrapper=0,leg=0,reader=0,valid=16,root=0,render=0,evaluated=0,
                        apiStatus=2,apiStep=9,apiIndex=7,apiResult=-1)
            fields.update(changes);return boundary_fixture(**fields)
        for changes in (dict(apiStatus=1),dict(apiStatus=3),dict(apiStep=0),dict(apiStep=4294967295),
                        dict(apiStep=19,apiIndex=0),dict(apiStep=20,apiIndex=0),dict(apiStep=21,apiIndex=0),
                        dict(apiIndex=1),dict(apiStep=13),dict(apiResult=0),
                        dict(valid=0,apiStatus=0,apiStep=0,apiIndex=0,apiResult=0),
                        dict(apiStatus=0,apiStep=0,apiIndex=0,apiResult=0)):
            with self.assertRaises(ValueError,msg=str(changes)):assess(api(**changes),SOURCE)
        # Serialization can decline before sampling, yielding a zero Missing snapshot.
        self.assertEqual(assess(api(phase=3,apiStatus=0,apiStep=0,apiIndex=0,apiResult=0),SOURCE)
                         ['rejected_or_missing_observations'][0]['submissions']['palette_boundaries']['rows'][0]['apiStatus'],0)
        for phase in (3,6):
            for step in range(1,23):
                if phase==6 and step in (19,20,21):continue
                index=5 if step in (9,10,11,12) else 0
                result=0 if step in (2,5,7,10,14,17,20,22) else -1
                assess(api(phase=phase,apiStep=step,apiIndex=index,apiResult=result),SOURCE)

    def test_boundary_inventory_exact_at_capacity_and_excluded_rows(self):
        for count in (0,1,10,11):
            text=boundary_fixture(attempts=count)
            boundary=assess(text,SOURCE)['rejected_or_missing_observations'][0]['submissions']['palette_boundaries']
            self.assertEqual(set(boundary['rows']),set(range(min(count,10))))
            if count:
                lines=text.splitlines();missing='\n'.join(lines[:-1])
                missing=missing.replace(f'count={min(count,10)} diagnostic=1',f'count={min(count,10)-1} diagnostic=1')
                with self.assertRaises(ValueError):assess(missing,SOURCE)
        text=boundary_fixture(attempts=3,excluded=(1,))
        self.assertEqual(set(assess(text,SOURCE)['rejected_or_missing_observations'][0]['submissions']['palette_boundaries']['rows']),{0,2})
        with self.assertRaises(ValueError):assess(text.replace('count=2 diagnostic=1','count=0 diagnostic=1').rsplit('\n',2)[0],SOURCE)

    def test_palette_schema3_attaches_only_its_owned_submission_producer(self):
        text=palette_projection_fixture()
        r=assess(text,SOURCE)['rejected_or_missing_observations'][0]
        a=r['submissions']['palette_copies']['rows'][2]['projection_association']
        self.assertEqual((a['sequence'],a['submission_index'],a['ordinal']),(1,2,3))
        self.assertFalse(a['source_provenance_authenticated']);self.assertFalse(a['alignment_accepted'])
        for old,new in [('projectionSequence=1','projectionSequence=2'),
                        ('source=1 complete=1','source=2 complete=1'),
                        ('drawBefore=2064 drawAfter=2064','drawBefore=2032 drawAfter=2032'),
                        ('modelBefore=1000 modelAfter=1000','modelBefore=1001 modelAfter=1001'),
                        ('invalidations=0 blocked=0','invalidations=1 blocked=1'),
                        ('kind=constant item=4 chunk=0 values=00000000','kind=constant item=4 chunk=0 values=3f800000')]:
            with self.subTest(old=old),self.assertRaises(ValueError):assess(text.replace(old,new),SOURCE)

    def test_palette_schema3_missing_receipt_stays_diagnostic(self):
        text=palette_fixture(content=True).replace('schema=2','schema=3').replace('contentCopied=1 ','contentCopied=1 projectionSequence=0 ')
        r=assess(text,SOURCE)['rejected_or_missing_observations'][0]
        row=r['submissions']['palette_copies']['rows'][2]
        self.assertNotIn('projection_association',row)
        self.assertFalse(row['alignment_accepted'])
        for value in ('1','9','4294967296'):
            with self.assertRaises(ValueError):assess(text.replace('projectionSequence=0','projectionSequence='+value),SOURCE)
        with self.assertRaises(ValueError):assess(text.replace(' projectionSequence=0',''),SOURCE)


    def test_palette_content_snapshot_uses_shared_bounds_without_pose_admission(self):
        for declaration in (NO_UV56,OBSERVED,OBSERVED_MULTI_UV,PASSIVE_FIVE_ROW78):
            for present in (False,True):
                with self.subTest(declaration=declaration,present=present):
                    result=assess(palette_fixture(declaration=declaration,content=present),SOURCE)
                    row=result['rejected_or_missing_observations'][0]['submissions']['palette_copies']['rows'][2]
                    self.assertEqual(row.get('content_snapshot_copied',False),present)
                    self.assertFalse(row['vertex_content_verified']);self.assertFalse(row['alignment_accepted'])
                    self.assertFalse(result['copied_event_pose_observations'])
    def test_palette_content_rejects_missing_crossed_unbounded_or_unclaimed_channels(self):
        text=palette_fixture(content=True)
        for old,new in [('contentCopied=1','contentCopied=0'),('contentCopied=1','contentCopied=2'),
                        ('contentCopied=1 ',' '),('schema=2','schema=3'),
                        ('kind=contentSurface item=0','kind=contentSurface item=1'),
                        ('kind=contentHash item=4','kind=contentHash item=5'),
                        ('kind=contentBuffers item=0 chunk=0 values=00000054','kind=contentBuffers item=0 chunk=0 values=00000053'),
                        ('kind=contentStreams item=0 chunk=0 values=00000065','kind=contentStreams item=0 chunk=0 values=00000066'),
                        ('00000015,00000000','00000016,00000000')]:
            with self.subTest(old=old):
                bad=text.replace(old,new);self.assertNotEqual(bad,text)
                with self.assertRaises(ValueError):assess(bad,SOURCE)
        for line in text.splitlines():
            if 'kind=content' in line:
                with self.assertRaises(ValueError):assess(text.replace(line+'\n','').removesuffix(line),SOURCE)
                with self.assertRaises(ValueError):assess(text+'\n'+line,SOURCE)
    def test_palette_content_match_keeps_actual_copy_evidence_separate(self):
        from match_idle_geometry import match,CHANNELS
        import struct
        evidence=assess(palette_fixture(content=True),SOURCE)
        ranges={name:{'offset':off,'size':size,'format':fmt,'buffer':0} for name,off,size,fmt in
                [('positions',0,36,133),('indices',0,6,135),('weights',48,12,128),
                 ('local_indices',36,12,128),('uv',60,24,132)]}
        channel={'vertices':3,'triangles':1,'whole_vertex_buffer_bytes':84,'whole_index_buffer_bytes':6,
                 'single_body_influence':False,'rigid_palette_id2':True,'mesh_object':1,'lod':0,
                 'channel_ranges':ranges,'channel_sha256':{name:struct.pack('<8I',*([i+1]*8)).hex() for i,name in enumerate(CHANNELS)}}
        candidates={'asset':{'asset_sha256':'b'*64,'candidate_native_id':2,'candidate_channels':[channel]}}
        result=match(evidence,candidates);r=result['palette_content_matches'][0]
        self.assertEqual(r['result'],'unique-copied-channel-match');self.assertEqual(r['auxiliary_channels'],['uv'])
        self.assertEqual(result['matches'],[]);self.assertFalse(result['copied_channels_all_uniquely_matched'])
        self.assertFalse(r['shader_replay_verified']);self.assertFalse(result['alignment_accepted'])
        for key in ('whole_vertex_buffer_bytes','whole_index_buffer_bytes','vertices','triangles'):
            bad=copy.deepcopy(candidates);bad['asset']['candidate_channels'][0][key]+=1
            self.assertEqual(match(evidence,bad)['palette_content_matches'][0]['result'],'unmatched')
        for name in CHANNELS:
            for key in ('channel_ranges','channel_sha256'):
                bad=copy.deepcopy(candidates);c=bad['asset']['candidate_channels'][0]
                if key=='channel_ranges':c[key][name]['offset']+=1
                else:c[key][name]='c'*64
                self.assertEqual(match(evidence,bad)['palette_content_matches'][0]['result'],'unmatched')
        duplicate=copy.deepcopy(candidates);duplicate['second']=copy.deepcopy(duplicate['asset'])
        self.assertEqual(match(evidence,duplicate)['palette_content_matches'][0]['result'],'ambiguous')

    def test_joined_palette_reader_retains_owned_copies_and_gaps_without_alignment(self):
        for count in (1,2,3):
            for declaration in (NO_UV56,OBSERVED,OBSERVED_MULTI_UV,PASSIVE_FIVE_ROW78):
                r=assess(palette_fixture(count=count,declaration=declaration),SOURCE)
                self.assertFalse(r['copied_event_pose_observations'])
                p=r['rejected_or_missing_observations'][0]['submissions']['palette_copies']
                self.assertEqual(set(p['rows']),{2});row=p['rows'][2]
                self.assertTrue(row['completion_reported']);self.assertEqual(row['count'],count)
                self.assertEqual(row['stream_numbers'],[0,5,6] if declaration==NO_UV56 else [0,7,8])
                for name in ('source_provenance_authenticated','api_coverage_complete','vertex_content_verified',
                             'shader_index_association_verified','gpu_visibility_verified','positive_grasp_verified','alignment_accepted'):
                    self.assertFalse(p[name])
        crossed=assess(palette_fixture(equal=False),SOURCE)['rejected_or_missing_observations'][0]['submissions']['palette_copies']
        self.assertEqual(crossed['rows'][2]['canonicalEqual'],0)
        summary=assess(palette_fixture(selected=None),SOURCE)['rejected_or_missing_observations'][0]['submissions']['palette_copies']
        self.assertFalse(summary['rows']);self.assertFalse(summary['api_coverage_complete'])
        overflow=assess(palette_fixture(attempts=11,selected=9),SOURCE)['rejected_or_missing_observations'][0]['submissions']['palette_copies']
        self.assertTrue(overflow['overflow']);self.assertEqual(set(overflow['rows']),{9})
        # Passive API evidence permits unknown input behavior and unused NaNs;
        # these raw words must not be mistaken for position replay admission.
        text=palette_fixture().replace('kind=constant item=0 chunk=0 values=00000000','kind=constant item=0 chunk=0 values=7fc00001')
        text=text.replace('kind=stream item=0 chunk=0 values=00000065,00000000,0000000c,00000001',
                          'kind=stream item=0 chunk=0 values=00000065,00000000,00000000,40000001')
        r=assess(text,SOURCE)['rejected_or_missing_observations'][0]['submissions']['palette_copies']['rows'][2]
        self.assertEqual(r['data']['constant:0:0'][0],0x7fc00001);self.assertFalse(r['shader_index_association_verified'])
        negative=palette_fixture().replace('draw=4,0,0,3,0,1','draw=4,-1,0,3,0,1')
        negative=negative.replace('kind=draw item=0 chunk=0 values=00000004,00000000',
                                  'kind=draw item=0 chunk=0 values=00000004,ffffffff')
        r=assess(negative,SOURCE)['rejected_or_missing_observations'][0]['submissions']['palette_copies']['rows'][2]
        self.assertEqual(r['data']['draw:0:0'][1],0xffffffff)

    def test_joined_palette_reader_rejects_truncated_foreign_contradictory_data(self):
        text=palette_fixture()
        replacements=[('api=2','api=1'),('count=3 mapCount=7','count=4 mapCount=7'),
                      ('first=4 count=3','first=5 count=3'),('canonicalCount=12','canonicalCount=3'),
                      ('canonicalEqual=1','canonicalEqual=0'),('objects=20,21,22','objects=20,0,22'),
                      ('cleanupCertified=1 outerCurrent=1','cleanupCertified=0 outerCurrent=1'),
                      ('payloads=3','payloads=2'),('outerCompleted=1','outerCompleted=0'),
                      ('kind=mapping item=0 chunk=0 values=00000005,00000002,00000001',
                       'kind=mapping item=0 chunk=0 values=00000005,00000002,00000000'),
                      ('kind=world item=0 chunk=0 values=3f800000','kind=world item=0 chunk=0 values=7fc00001'),
                      ('kind=program item=0 chunk=0','kind=program item=0 chunk=1'),
                      ('kind=palette item=2 chunk=0','kind=palette item=3 chunk=0'),
                      ('kind=declaration item=0 chunk=0 values=00000000','kind=declaration item=0 chunk=0 values=00000007'),
                      ('modelAddress=1000','modelAddress=1001'),('input=90 generation=4 owner=1','input=91 generation=4 owner=1')]
        replacements.append(('kind=draw item=0 chunk=0 values=00000004,00000000,00000000,00000003,00000000,00000001',
                             'kind=draw item=0 chunk=0 values=00000004,00000000,00000000,00000003,00000001,00000001'))
        replacements.extend([('request=100','request=0'),('123,0,-1','123,999,-1')])
        for old,new in replacements:
            bad=text.replace(old,new);self.assertTrue(bad!=text,old)
            with self.assertRaises(ValueError,msg=old):assess(bad,SOURCE)
        for native in (1,13):
            with self.assertRaises(ValueError):assess(text.replace('nativeId=2',f'nativeId={native}'),SOURCE)
        lines=text.splitlines()
        for i,line in enumerate(lines):
            if line.startswith('Lab idle paletteData'):
                with self.assertRaises(ValueError,msg=line):assess('\n'.join(lines[:i]+lines[i+1:]),SOURCE)
                with self.assertRaises(ValueError,msg=line):assess(text+'\n'+line,SOURCE)
        row=next(x for x in lines if x.startswith('Lab idle paletteRow'))
        with self.assertRaises(ValueError):assess(text+'\n'+row,SOURCE)
        with self.assertRaises(ValueError):assess(text.replace('alignment=0 grasp=0','alignment=1 grasp=0'),SOURCE)
    def test_submission_native_borrow_cleanup_order_is_source_bound(self):
        from verify_idle_submission_abi import source_checks
        root=Path(__file__).resolve().parents[1]
        engine=(root/'src/game/engine.cpp').read_text();gpu=(root/'src/game/scope_gpu.cpp').read_text()
        self.assertTrue(source_checks(engine,gpu)['source_order_checked'])
        for bad in (gpu.replace('probe.submissionOwner=idleSubmissionOwner();','probe.submissionOwner=nullptr;'),
                    gpu.replace('copyIdleSubmissionMetadata(owner,after)','copyBoundIdleRaster(after)'),
                    gpu.replace('probe.submissionOwner=nullptr;probe.submissionSlot=IdleSubmissionTrace::NoSlot;',
                                'probe.submissionSlot=IdleSubmissionTrace::NoSlot;'),
                    gpu.replace('scopeGpuRoutingCurrent(device)','true')):
            self.assertTrue(bad!=gpu,'Mutation must change the actual source form')
            with self.assertRaises(ValueError):source_checks(engine,bad)
        for bad in (engine.replace('retireIdleSubmissionOwner(invocation.idle);',''),
                    engine.replace('out=IdleSubmissionMetadata::copy(raster);copied=true;',
                                   'trace->reject();out=IdleSubmissionMetadata::copy(raster);copied=true;')):
            self.assertTrue(bad!=engine,'Mutation must change the actual source form')
            with self.assertRaises(ValueError):source_checks(bad,gpu)

    def test_joined_palette_source_mutations_reject(self):
        from verify_idle_submission_abi import source_checks
        root=Path(__file__).resolve().parents[1]
        engine=(root/'src/game/engine.cpp').read_text();gpu=(root/'src/game/scope_gpu.cpp').read_text()
        for old,new in [('probe.submissionOwner->reservePaletteApi(', 'reserveDifferentPayload('),
                        ('sampleIdleApi(d,probe.bindings[1],p.before,p.program,p.words,{},true,true)', 'sampleWrongInputs()'),
                        ('copyPaletteBoundary(d,owner,slot,api,4,bookend)', 'copyWrongPalette(bookend)'),
                        ('before.projectionBookend(bookend);',''),('after.projectionBookend(bookend);',''),
                        ('paletteApiOwnerCurrent(d,owner,slot,api)', 'true'),
                        ('api>=IdleWeaponTrace::MaxPaletteApiPayloads ||',
                         'api>=IdleWeaponTrace::MaxPaletteApiPayloads || !nativeUiDeviceCurrent(d) ||'),
                        ('if(geometryAdmitted) {', 'if(false) {'),
                        ('!owner->paletteApiPayloads[api].matched', 'false'),
                        ('program.size_bytes()', 'UINT32_MAX'),
                        ('const bool paletteContentEligible=!admitted && !probe.raster.pose.valid && !idleCandidate;',
                         'const bool paletteContentEligible=true;'),
                        ('if(!scopeGpuForwardingAllowed())return;', 'if(false)return;'),
                        ('if(!scopeGpuForwardingAllowed() || !paletteApiOwnerCurrent(d,owner,slot,api))return false;',
                         'if(!paletteApiOwnerCurrent(d,owner,slot,api))return false;'),
                        ('probe.algorithm || probe.hash || !hashIdleSlices(ranges)', '!hashIdleSlices(ranges)'),
                        ('paletteContentMatchesApi(probe.bindings[0],p.before)', 'true')]:
            bad=gpu.replace(old,new);self.assertTrue(bad!=gpu,'GPU mutation did not change source: '+old)
            with self.assertRaises(ValueError):source_checks(engine,bad)
        for old,new in [('trace->nativeId!=2', 'trace->nativeId!=13'),
                        ('submissions.completeOuter(', 'submissions.ignoreOuter('),
                        ('trace.paletteApiPublishable(n)', 'true'),
                        ('trace.paletteContentPublishable(n)', 'true'),
                        ('trace.paletteProjectionSequence(n)','p.projectionSequence'),
                        ('alignment=0 grasp=0', 'alignment=1 grasp=1')]:
            bad=engine.replace(old,new);self.assertTrue(bad!=engine,'Engine mutation did not change source: '+old)
            with self.assertRaises(ValueError):source_checks(bad,gpu)

    def test_submission_after_geometry_rejection_is_not_geometry_or_hand_acceptance(self):
        text=fixture();base=BASE
        summary='Lab idle submissionSummary '+base+' attempts=1 count=1 overflow=0 outerReturned=1 limit=64'
        row='Lab idle submissionRow '+base+' index=0 ordinal=1 status=8 hr=0 flags=15 draw=4,0,0,132,4782,108'
        metadata='Lab idle submissionMetadata '+base+' index=0 keys=100,200,1,3,300,400,500,600,0 root=700,800,1 render=700,800,1 layout=132,108,14448,133,0,9564,135,0,58016,128,0,58544,128,0'
        result=assess(text+'\n'+summary+'\n'+row+'\n'+metadata,SOURCE)
        record=result['rejected_or_missing_observations'][0];self.assertEqual(record['rejection']['reason'],32)
        self.assertFalse(result['copied_event_pose_observations']);self.assertFalse(record['geometry'])
        s=record['submissions'];self.assertEqual(s['rows'][0]['metadata']['layout'][0],132)
        for key in ('gpu_visibility_verified','vertex_content_verified','hand_resource_verified','alignment_accepted'):self.assertFalse(s[key])
        for bad in (text+'\n'+summary+'\n'+row,text+'\n'+row+'\n'+metadata,
                    text+'\n'+summary+'\n'+row.replace('status=8','status=6')+'\n'+metadata,
                    text+'\n'+summary+'\n'+row.replace('flags=15','flags=7')+'\n'+metadata,
                    text+'\n'+summary+'\n'+row.replace('hr=0','hr=-1')+'\n'+metadata,
                    text+'\n'+summary+'\n'+row+'\n'+metadata+'\n'+metadata):
            with self.assertRaises(ValueError):assess(bad,SOURCE)

    def test_submission_attempt_budget_unknown_rows_and_repeated_native_keys(self):
        head=fixture()+'\nLab idle submissionSummary '+BASE+' attempts=65 count=64 overflow=1 outerReturned=1 limit=64'
        rows=['Lab idle submissionRow '+BASE+' index='+str(i)+' ordinal='+str(i+1)+' status=1 hr=0 flags=4 draw=4,0,0,132,4782,108' for i in range(64)]
        text=head+'\n'+'\n'.join(rows);r=assess(text,SOURCE)['rejected_or_missing_observations'][0]['submissions']
        self.assertEqual(len(r['rows']),64);self.assertTrue(r['overflow'])
        for bad in (text.replace('count=64','count=65'),text.replace('overflow=1','overflow=0'),
                    text.replace('outerReturned=1','outerReturned=0'),text.replace(rows[-1],''),
                    text.replace('index=63 ordinal=64','index=63 ordinal=63')):
            with self.assertRaises(ValueError):assess(bad,SOURCE)

    def test_submission_signed_counts_byte_width_and_status_predicates(self):
        summary='Lab idle submissionSummary '+BASE+' attempts=1 count=1 overflow=0 outerReturned=1 limit=64'
        row='Lab idle submissionRow '+BASE+' index=0 ordinal=1 status=8 hr=0 flags=15 draw=4,0,0,132,4782,108'
        payload='Lab idle submissionMetadata '+BASE+' index=0 keys=100,200,1,3,300,400,500,600,0 root=700,800,1 render=700,800,1 layout=-1,-2147483648,14448,133,0,9564,135,0,58016,128,0,58544,128,0'
        prefix=fixture()+'\n'+summary+'\n';text=prefix+row+'\n'+payload
        observed=assess(text,SOURCE)['rejected_or_missing_observations'][0]['submissions']
        self.assertEqual(observed['rows'][0]['metadata']['layout'][:2],[-1,-2147483648])
        for wrong in (payload.replace('14448,133,0','14448,256,0'),payload.replace('14448,133,0','14448,133,256')):
            with self.assertRaises(ValueError):assess(prefix+row+'\n'+wrong,SOURCE)
        for status in (1,2,3,9):
            with self.assertRaises(ValueError):assess(prefix+row.replace('status=8','status='+str(status)),SOURCE)
        with self.assertRaises(ValueError):assess(prefix+row.replace('status=8','status=7').replace('flags=15','flags=2'),SOURCE)
        # Late Release reentry/retirement/abort may invalidate already matching
        # bookends: retain their flags while withholding every metadata payload.
        for status in (5,6,7):
            r=assess(prefix+row.replace('status=8','status='+str(status)),SOURCE)['rejected_or_missing_observations'][0]['submissions']
            self.assertEqual(r['rows'][0]['flags'],15);self.assertIsNone(r['rows'][0]['metadata'])
    def test_ride_gpu_actual_source_order_mutations_decline(self):
        from verify_scope_gpu_abi import verify_ride_gpu_source
        root=Path(__file__).resolve().parents[1]
        source=(root/'src/game/scope_gpu.cpp').read_text();render=(root/'src/game/remote_render.cpp').read_text()
        self.assertTrue(verify_ride_gpu_source(source,render)['source_checked'])
        for old,new in [('!probe.submissionOwner &&','true &&'),
                        ('!sameInputs(b,probe.bindings[1])','false'),
                        ('!copySlice(i==1,','!copyWrongSlice(i==1,'),
                        ('!probe.rideReentered && !probe.split','!probe.split'),
                        ('probe.rideGpuMatched=finishRideGpu(d);','probe.rideGpuMatched=true;'),
                        ('&& rideCameraCurrent;',';'),
                        ('&&\n         cameraAfterRelease==probe.rideCamera','||\n         cameraAfterRelease==probe.rideCamera'),
                        ('profile!=RideHandleProfile::Unknown && copyRideHandles','true && copyRideHandles'),
                        ('std::optional<RideDrawGpuCopy>{probe.rideGpu}','std::nullopt'),
                        ('if(ride)return true;','if(false)return true;')]:
            bad=source.replace(old,new);self.assertTrue(bad!=source,old)
            with self.subTest(mutation=old),self.assertRaises((ValueError,IndexError)):
                verify_ride_gpu_source(bad,render)
        bad=render.replace('rideObservationRows.load(std::memory_order_relaxed)>=32','false')
        with self.assertRaises(ValueError):verify_ride_gpu_source(source,bad)
        bad=render.replace('return chargeRideGpuAttempt(','return true || chargeRideGpuAttempt(')
        with self.assertRaises(ValueError):verify_ride_gpu_source(source,bad)
        old='GeometryBufferPolicy::Ride,nullptr,-1,&probe.rideBefore.layout) &&\n        sameInputs(probe.bindings[0],probe.bindings[2]);'
        bad=source.replace(old,old.replace(' &&',' ||'));self.assertTrue(bad!=source)
        with self.assertRaises(ValueError):verify_ride_gpu_source(bad,render)


    def test_live_input_getter_routing_cannot_revert_or_leave_selector_unused(self):
        from verify_scope_gpu_abi import verify_input_stream_routing
        source=(Path(__file__).resolve().parents[1]/'src/game/scope_gpu.cpp').read_text()
        checked=verify_input_stream_routing(source)
        self.assertEqual(checked['no_uv78_streams'],[0,7,2,8])
        self.assertEqual(checked['observed78_and_multi_uv78_streams'],[0,7,3,8])
        for bad in (
                source.replace('noUV78?2u:3u','3'),
                source.replace('noUV78?2u:3u','noUV78?3u:2u'),
                source.replace('noUV78?2u:3u','noUV78?2u:2u'),
                source.replace('const bool noUV78=(idle || ride) &&','const bool noUV78=true || (idle || ride) &&'),
                source.replace('GetStreamSource(streamNumbers[i],','GetStreamSource(3,'),
                source.replace('noUV78?2u:3u','3 /* noUV78?2u:3u */')):
            with self.subTest(mutation=bad[bad.index('const UINT streamNumbers'):][:100]),self.assertRaises(ValueError):
                verify_input_stream_routing(bad)

    def test_five_row78_passive_with_five_retained_copies_never_promotes(self):
        from idle_weapon_evidence_checks import geometry_fixture
        _,_,header,data=geometry_fixture()
        passive=fixture(PASSIVE_FIVE_ROW78).replace('schema=3 ','schema=3 copyLayout=1 ').replace('draws=0','draws=5')
        payload=[line.replace('eye=1 hand=0','eye=0 hand=1').replace('Lab idle geometry','Lab idle retainedGeometry')
                 for line in [header,*data]]
        companion='Lab idle retainedCopies '+BASE+' count=5 postOriginal=1 cleanupCertified=0 outerCurrent=0'
        copied=[line.replace('index=0 ','index='+str(i)+' ') for i in range(5) for line in payload]
        text='\n'.join([passive,companion,*copied]);result=assess(text,SOURCE)
        self.assertFalse(result['copied_event_pose_observations'])
        record=result['rejected_or_missing_observations'][0]
        self.assertEqual(record['stage'],3)
        self.assertEqual(record['rejection']['reason'],32)
        self.assertEqual(set(record['retained_copies']['geometry']),set(range(5)))
        for field in ('geometry_admitted','roles_inferred','alignment_accepted'):
            self.assertIs(record['stream_probe'][field],False)
        for field in ('cleanup_certified','outer_current','whole_trace_accepted','positive_grasp_verified','alignment_accepted'):
            self.assertIs(record['retained_copies'][field],False)
        for bad in (passive,text.replace(companion,''),text.replace('count=5 postOriginal','count=4 postOriginal'),
                    text.replace(copied[-1],''),text.replace('cleanupCertified=0','cleanupCertified=1'),
                    text.replace('outerCurrent=0','outerCurrent=1')):
            with self.subTest(case=bad[-80:]),self.assertRaises(ValueError):assess(bad,SOURCE)

    def test_five_row78_is_exact_and_passive_receipt_stays_diagnostic(self):
        text=fixture(PASSIVE_FIVE_ROW78);result=assess(text,SOURCE)
        record=result['rejected_or_missing_observations'][0]
        self.assertEqual(diagnostic_stream_numbers(record['input_failure']),(0,7,8))
        self.assertEqual(set(record['stream_probe']['snapshots'][0]['streams']),{0,7,8})
        self.assertFalse(result['copied_event_pose_observations'])
        for field in ('geometry_admitted','roles_inferred','alignment_accepted'):
            self.assertIs(record['stream_probe'][field],False)
        self.assertEqual(declaration_layout(PASSIVE_FIVE_ROW78),(4,True))
        for index in range(5):
            for field in range(6):
                rows=copy.deepcopy(PASSIVE_FIVE_ROW78);rows[index][field]+=1
                with self.subTest(index=index,field=field),self.assertRaises(ValueError):
                    diagnostic_stream_numbers({'declaration':5,'declaration_rows':dict(enumerate(rows))})
        for count in (4,6,66):
            with self.assertRaises(ValueError):diagnostic_stream_numbers({'declaration':count,'declaration_rows':dict(enumerate(PASSIVE_FIVE_ROW78))})
        for foreign in (2,3,5,6):
            with self.assertRaises(ValueError):assess(text.replace('phase=0 index=7 object','phase=0 index='+str(foreign)+' object'),SOURCE)

    def test_five_row78_keeps_original_selection_and_failure_bounds(self):
        text=fixture(PASSIVE_FIVE_ROW78)
        head=text[:text.index('Lab idle streamSnapshot')].replace('attempts=2 flags=511 words=2','attempts=1 flags=284 words=0')
        partial='Lab idle streamSnapshot '+BASE+' phase=0 status=2 step=9 index=7 hr=-1'
        self.assertFalse(assess(head+partial,SOURCE)['copied_event_pose_observations'])
        for foreign in (2,3,5,6):
            with self.assertRaises(ValueError):assess(head+partial.replace('index=7','index='+str(foreign)),SOURCE)
        changed=[line for line in text.replace('flags=511','flags=447').splitlines() if not ('streamDeclaration '+BASE+' phase=1' in line)]
        changed+=['Lab idle streamDeclaration '+BASE+' phase=1 index='+str(i)+' values='+','.join(map(str,e)) for i,e in enumerate(NO_UV56)]
        changed='\n'.join(changed)
        record=assess(changed,SOURCE)['rejected_or_missing_observations'][0]
        self.assertEqual(set(record['stream_probe']['snapshots'][1]['streams']),{0,7,8})
        self.assertFalse(record['stream_probe']['flags']&64)
        with self.assertRaises(ValueError):assess(changed.replace('flags=447','flags=511'),SOURCE)
        with self.assertRaises(ValueError):assess(changed.replace('phase=1 index=7 object','phase=1 index=5 object'),SOURCE)
    def test_multi_uv_passive_receipt_stays_diagnostic_with_exact_copy_family(self):
        text=fixture(OBSERVED_MULTI_UV).replace('rangeChecks=6015','rangeChecks=6143 layout=0');result=assess(text,SOURCE)
        record=result['rejected_or_missing_observations'][0]
        self.assertEqual(diagnostic_stream_numbers(record['input_failure']),(0,7,8))
        self.assertEqual(set(record['stream_probe']['snapshots'][0]['streams']),{0,7,8})
        self.assertEqual(record['stream_probe']['flags'],511)
        self.assertFalse(result['copied_event_pose_observations'])
        for field in ('geometry_admitted','roles_inferred','alignment_accepted'):
            self.assertIs(record['stream_probe'][field],False)
        self.assertEqual(declaration_layout(OBSERVED_MULTI_UV),(3,True))
        new=text.replace('layout=0','layout=3')
        self.assertEqual(assess(new,SOURCE)['rejected_or_missing_observations'][0]['input_failure']['layout'],3)
        with self.assertRaises(ValueError):assess(new.replace('layout=3','layout=4'),SOURCE)
        for index in range(8):
            for field in range(6):
                rows=copy.deepcopy(OBSERVED_MULTI_UV);rows[index][field]+=1
                with self.subTest(index=index,field=field),self.assertRaises(ValueError):
                    diagnostic_stream_numbers({'declaration':8,'declaration_rows':dict(enumerate(rows))})
        for count in (7,9):
            with self.assertRaises(ValueError):diagnostic_stream_numbers({'declaration':count,'declaration_rows':dict(enumerate(OBSERVED_MULTI_UV))})
        for foreign in (4,5):
            with self.assertRaises(ValueError):assess(text.replace('streamInput '+BASE+' phase=0 index=7','streamInput '+BASE+' phase=0 index='+str(foreign)),SOURCE)
    def test_multi_uv_resampling_cannot_reselect_or_inherit_equality(self):
        text=fixture(OBSERVED_MULTI_UV).replace('flags=511','flags=447')
        rows=[line for line in text.splitlines() if not ('streamDeclaration '+BASE+' phase=1' in line)]
        rows=[line.replace('phase=1 caps=2 declaration=8','phase=1 caps=2 declaration=5') for line in rows]
        rows+=['Lab idle streamDeclaration '+BASE+' phase=1 index='+str(i)+' values='+','.join(map(str,e)) for i,e in enumerate(NO_UV56)]
        changed='\n'.join(rows)
        r=assess(changed,SOURCE)['rejected_or_missing_observations'][0]
        self.assertEqual(set(r['stream_probe']['snapshots'][1]['streams']),{0,7,8})
        self.assertFalse(r['stream_probe']['flags']&64)
        with self.assertRaises(ValueError):assess(changed.replace('phase=1 index=7 object','phase=1 index=5 object'),SOURCE)
        with self.assertRaises(ValueError):assess(changed.replace('flags=447','flags=511'),SOURCE)
    def test_exact_no_uv_selection_and_original_binding_agreement(self):
        text=fixture(NO_UV56);result=assess(text,SOURCE)
        record=result['rejected_or_missing_observations'][0]
        self.assertEqual(diagnostic_stream_numbers(record['input_failure']),(0,5,6))
        self.assertEqual(set(record['stream_probe']['snapshots'][0]['streams']),{0,5,6})
        self.assertFalse(result['copied_event_pose_observations'])
        for field in ('geometry_admitted','roles_inferred','alignment_accepted'):
            self.assertIs(record['stream_probe'][field],False)
        for bad in (text.replace('inputStream '+BASE+' index=1 object=1 offset=49256','inputStream '+BASE+' index=1 object=1 offset=49260'),
                    text.replace('inputStream '+BASE+' index=3 object=1 offset=48384','inputStream '+BASE+' index=3 object=2 offset=48384')):
            with self.assertRaises(ValueError):assess(bad,SOURCE)
        for rows in ([],NO_UV56[:-1],OBSERVED+NO_UV56):
            with self.assertRaises(ValueError):diagnostic_stream_numbers({'declaration':len(rows),'declaration_rows':dict(enumerate(rows))})
        for index in range(5):
            for field in range(6):
                rows=copy.deepcopy(NO_UV56);rows[index][field]+=1
                with self.subTest(index=index,field=field),self.assertRaises(ValueError):
                    diagnostic_stream_numbers({'declaration':5,'declaration_rows':dict(enumerate(rows))})
    def test_no_uv_partial_and_cross_family_indices(self):
        for declaration,valid,foreign in ((NO_UV56,5,7),(OBSERVED,7,5)):
            text=fixture(declaration);head=text[:text.index('Lab idle streamSnapshot')]
            head=head.replace('attempts=2 flags=511 words=2','attempts=1 flags=284 words=0')
            row='Lab idle streamSnapshot '+BASE+f' phase=0 status=2 step=9 index={valid} hr=-1'
            self.assertFalse(assess(head+row,SOURCE)['copied_event_pose_observations'])
            with self.assertRaises(ValueError):assess(head+row.replace('index='+str(valid),'index='+str(foreign)),SOURCE)
        text=fixture(NO_UV56)
        for foreign in (7,8):
            with self.assertRaises(ValueError):assess(text.replace('streamInput '+BASE+' phase=0 index=5','streamInput '+BASE+' phase=0 index='+str(foreign)),SOURCE)
    def test_changed_after_declaration_does_not_reselect_numbers(self):
        text=fixture(NO_UV56).replace('flags=511','flags=447')
        rows=[line for line in text.splitlines() if not ('streamDeclaration '+BASE+' phase=1' in line)]
        rows=[line.replace('phase=1 caps=2 declaration=5','phase=1 caps=2 declaration=6') for line in rows]
        rows+=['Lab idle streamDeclaration '+BASE+' phase=1 index='+str(i)+' values='+','.join(map(str,e)) for i,e in enumerate(OBSERVED)]
        changed='\n'.join(rows)
        r=assess(changed,SOURCE)['rejected_or_missing_observations'][0]
        self.assertEqual(set(r['stream_probe']['snapshots'][1]['streams']),{0,5,6})
        self.assertFalse(r['stream_probe']['flags']&64)
        with self.assertRaises(ValueError):assess(changed.replace('phase=1 index=5 object','phase=1 index=7 object'),SOURCE)
        with self.assertRaises(ValueError):assess(changed.replace('flags=447','flags=511'),SOURCE)
    def test_complete_diagnostic_never_promotes_or_infers_roles(self):
        r=assess(fixture(),SOURCE)
        self.assertFalse(r['copied_event_pose_observations'])
        p=r['rejected_or_missing_observations'][0]['stream_probe']
        self.assertEqual(p['flags'],511)
        self.assertEqual(p['snapshots'][0]['streams'][7]['offset'],48384)
        for k in ('geometry_admitted','roles_inferred','alignment_accepted'):self.assertIs(p[k],False)
    def test_partial_snapshot_cannot_expose_outputs(self):
        text=fixture();start=text.index('Lab idle streamSnapshot')
        head=text[:start].replace('attempts=2 flags=511 words=2','attempts=1 flags=284 words=0')
        partial='Lab idle streamSnapshot '+BASE+' phase=0 status=2 step=9 index=7 hr=-1'
        self.assertEqual(assess(head+partial,SOURCE)['rejected_or_missing_observations'][0]['stream_probe']['snapshots'][0]['status'],2)
        bad=partial+'\nLab idle streamBinding '+BASE+' phase=0 caps=2 declaration=6 declarationObject=5 indexObject=4 shaderObject=6'
        with self.assertRaises(ValueError):assess(head+bad,SOURCE)
        with self.assertRaises(ValueError):assess(head+partial.replace('hr=-1','hr=0'),SOURCE)
    def test_interrupted_call_has_no_qualified_hresult(self):
        text=fixture();head=text[:text.index('Lab idle streamSnapshot')].replace('attempts=2 flags=511 words=2 invalidations=0','attempts=1 flags=0 words=0 invalidations=2')
        row='Lab idle streamSnapshot '+BASE+' phase=0 status=3 step=16 index=0 hr=0'
        self.assertFalse(assess(head+row,SOURCE)['copied_event_pose_observations'])
        with self.assertRaises(ValueError):assess(head+row.replace('hr=0','hr=-1'),SOURCE)
    def test_foreign_duplicate_unknown_and_truncated_receipts_reject(self):
        text=fixture();rows=text.splitlines()
        for row in rows:
            if 'Lab idle stream' not in row:continue
            with self.subTest(row=row),self.assertRaises(ValueError):assess(text+'\n'+row,SOURCE)
            with self.subTest(missing=row),self.assertRaises(ValueError):assess('\n'.join(x for x in rows if x!=row),SOURCE)
        for bad in (text.replace('streamProbe request=100','streamProbe request=101'),text.replace('streamProbe','streamUnknown'),
                    text.replace('chunk=0','chunk=1'),text.replace('words=2','words=513'),
                    text.replace('declarationObject=5','declarationObject=0'),text.replace('streamInput '+BASE+' phase=0 index=7','streamInput '+BASE+' phase=0 index=9'),
                    text.replace('values=fffe0101,0000ffff','values=fffe0101'),text.replace('caps=2 declaration=6 declarationObject','caps=257 declaration=6 declarationObject')):
            with self.subTest(bad=bad),self.assertRaises(ValueError):assess(bad,SOURCE)
    def test_changed_inputs_cannot_claim_same(self):
        text=fixture()
        for a,b in [('phase=1 caps=2 declaration=6 declarationObject=5','phase=1 caps=2 declaration=6 declarationObject=9'),
                    ('phase=1 caps=2 declaration=6 declarationObject=5 indexObject=4 shaderObject=6','phase=1 caps=2 declaration=6 declarationObject=5 indexObject=4 shaderObject=9'),
                    ('phase=1 index=7 object=1 offset=48384','phase=1 index=7 object=1 offset=49256'),
                    ('phase=1 index=8 object=1','phase=1 index=8 object=2'),
                    ('phase=1 index=0 values=00000000','phase=1 index=0 values=3f800000')]:
            with self.subTest(a=a),self.assertRaises(ValueError):assess(text.replace(a,b),SOURCE)
            self.assertFalse(assess(text.replace(a,b).replace('flags=511','flags=447'),SOURCE)['copied_event_pose_observations'])
    def test_original_identity_and_declaration_required(self):
        text=fixture()
        for bad in (text.replace('inputDeclaration '+BASE+' index=3 values=7','inputDeclaration '+BASE+' index=3 values=6'),
                    text.replace('valid=15','valid=11'),text.replace('step=20 index=0 hr=0 valid','step=19 index=0 hr=0 valid'),
                    text.replace('preceding=2','preceding=1'),text.replace('state=63','state=55'),
                    text.replace('matrices=4','matrices=0'),text.replace('streamInput '+BASE+' phase=0 index=0 object=1','streamInput '+BASE+' phase=0 index=0 object=2')):
            with self.subTest(bad=bad),self.assertRaises(ValueError):assess(bad,SOURCE)
    def test_abort_reentry_retirement_cleanup_are_inconclusive(self):
        for mask in (1,2,4,8,16,31):
            text=fixture().replace('invalidations=0','invalidations='+str(mask))
            with self.assertRaises(ValueError):assess(text,SOURCE)
            self.assertFalse(assess(text.replace('flags=511','flags=255'),SOURCE)['copied_event_pose_observations'])
    def test_forward_return_and_flag_dependencies(self):
        text=fixture()
        for flags in (1,2,4,8,16,32,64,128,510,509,507,503,495,479):
            with self.subTest(flags=flags),self.assertRaises(ValueError):assess(text.replace('flags=511','flags='+str(flags)),SOURCE)
        with self.assertRaises(ValueError):assess(text.replace('forwardResult=0','forwardResult=-1'),SOURCE)
    def test_unused_nonfinite_constant_bits_are_diagnostic_only(self):
        r=assess(fixture().replace('values=00000000,00000000,00000000,00000000','values=7fc00000,7f800000,00000000,00000000'),SOURCE)
        self.assertFalse(r['copied_event_pose_observations'])
    def test_attempted_phase_requires_prerequisites_even_without_copied_payload(self):
        text=fixture();start=text.index('Lab idle streamSnapshot '+BASE+' phase=1')
        program=text[text.index('Lab idle streamProgram'):]
        head=text[:start]
        failed='Lab idle streamSnapshot '+BASE+' phase=1 status=2 step=9 index=7 hr=-1\n'
        valid=head.replace('flags=511','flags=287')+failed+program
        self.assertFalse(assess(valid,SOURCE)['copied_event_pose_observations'])
        for bad in (valid.replace('flags=287','flags=285'),
                    valid.replace('flags=287','flags=271').replace('forwardResult=0','forwardResult=-1'),
                    valid.replace('status=2 step=9 index=7 hr=-1','status=3 step=9 index=7 hr=0')):
            with self.subTest(bad=bad),self.assertRaises(ValueError):assess(bad,SOURCE)
        interrupted=valid.replace('flags=287','flags=31').replace('invalidations=0','invalidations=2').replace('status=2 step=9 index=7 hr=-1','status=3 step=9 index=7 hr=0')
        self.assertFalse(assess(interrupted,SOURCE)['copied_event_pose_observations'])
    def test_exact_original_palette_history_and_bidirectional_equality(self):
        text=fixture()
        for bad in (text.replace('contributors=1','contributors=0'),text.replace('state=63','state=8'),
                    text.replace('callbacks=63','callbacks=0'),text.replace('owner=1','owner=0'),
                    text.replace('rawGripValid=1','rawGripValid=0'),text.replace('flags=511','flags=447')):
            with self.subTest(bad=bad),self.assertRaises(ValueError):assess(bad,SOURCE)
        legitimate=text.replace('rawGripValid=1','rawGripValid=0').replace('state=63','state=47')
        self.assertFalse(assess(legitimate,SOURCE)['copied_event_pose_observations'])

if __name__=='__main__':unittest.main()
