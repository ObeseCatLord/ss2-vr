"""Strict copied API/content diagnostics; never GPU visibility or a grasp proof."""
from idle_stream_evidence import number,signed,words,declaration_layout,validate_buffer_content
BASE={'request','eye','hand'}

def values(raw,count,signed_indices=()):
    parts=raw.split(',')
    if len(parts)!=count:raise ValueError('Submission vector width mismatch')
    return [signed(v) if i in signed_indices else number(v) for i,v in enumerate(parts)]

def consume(record,kind,f):
    if kind not in ('submissionSummary','submissionRow','submissionMetadata'):return False
    if record is None or not BASE.issubset(f) or any(number(f[k],(1<<64)-1 if k=='request' else 0xffffffff)!=record[k] for k in BASE):
        raise ValueError('Foreign submission metadata')
    if kind=='submissionSummary':
        if set(f)!=BASE|{'attempts','count','overflow','outerReturned','limit'} or 'submissions' in record:
            raise ValueError('Submission summary schema/duplicate')
        p={k:number(f[k]) for k in ('attempts','count','overflow','outerReturned','limit')}
        if p['limit']!=64 or p['count']!=min(p['attempts'],64) or p['overflow']!=int(p['attempts']>64) or p['outerReturned']!=1:
            raise ValueError('Submission budget/outer-return mismatch')
        p.update(rows={},coverage='qualified-bookended-indexed-api-calls-only',gpu_visibility_verified=False,
                 vertex_content_verified=False,hand_resource_verified=False,alignment_accepted=False)
        record['submissions']=p;return True
    p=record.get('submissions')
    if p is None:raise ValueError('Submission data without summary')
    index=number(f.get('index',''),63)
    if index>=p['count']:raise ValueError('Submission row outside reserved attempts')
    if kind=='submissionRow':
        if set(f)!=BASE|{'index','ordinal','status','hr','flags','draw'} or index in p['rows']:
            raise ValueError('Submission row schema/duplicate')
        row={k:number(f[k],15 if k=='flags' else 10 if k=='status' else 0xffffffff) for k in ('ordinal','status','flags')}
        row.update(hresult=signed(f['hr']),draw=values(f['draw'],6,(1,)),metadata=None)
        if row['ordinal']!=index+1 or not row['status'] or (not row['flags']&4 and row['hresult']):
            raise ValueError('Submission ordinal/completion mismatch')
        if row['flags']&8 and row['flags']&3!=3:
            raise ValueError('Submission match without both samples')
        if row['flags']&2 and not row['flags']&4:
            raise ValueError('Post sample without original return')
        status=row['status'];flags=row['flags'];hr=row['hresult']
        if status==1 and (not flags&4 or hr<0 or flags&1) or \
           status==2 and (flags!=5 or hr<0) or status==3 and (flags!=7 or hr<0) or \
           status==9 and (flags&14 or hr):
            raise ValueError('Submission unknown/mismatch/no-forward predicates disagree')
        if row['status']==8 and (row['flags']!=15 or row['hresult']<0) or row['status']==4 and (not row['flags']&4 or row['hresult']>=0):
            raise ValueError('Submission status contradicts original result')
        p['rows'][index]=row;return True
    if set(f)!=BASE|{'index','keys','root','render','layout'}:
        raise ValueError('Submission metadata schema')
    row=p['rows'].get(index)
    if row is None or row['status']!=8 or row['metadata'] is not None:
        raise ValueError('Unqualified/duplicate submission payload')
    keys=values(f['keys'],9,(8,));layout=values(f['layout'],14,(0,1))
    if any(layout[i]>255 for i in (3,4,6,7,9,10,12,13)):
        raise ValueError('Submission channel byte width exceeded')
    multi=record.get('native_id_explicit') and record.get('nativeId')==2 and \
          record.get('schema')==4 and record.get('copyLayout')==1 and keys[8]==-1
    if any(not keys[i] for i in (0,1,2,4,5)) or keys[2]>=2048 or keys[3]>=8192 or not (multi or 0<=keys[8]<8192):
        raise ValueError('Submission native membership keys outside bounds')
    row['metadata']={'keys':keys,'root':values(f['root'],3,(2,)),
                     'render':values(f['render'],3,(2,)),'layout':layout}
    if multi:row['metadata']['bone_identity']='multiple-or-unresolved-diagnostic-only'
    return True

def consume_palette(record,kind,f,expected_source):
    """Read emitter-reported copied evidence, without widening replay admission."""
    if kind not in ('paletteSummary','paletteRow','paletteData'):return False
    if record is None or record.get('schema')!=4 or record.get('copyLayout')!=1 or \
            not record.get('native_id_explicit') or record.get('nativeId')!=2 or not record.get('request') or \
            not BASE.issubset(f) or any(number(f[k],(1<<64)-1 if k=='request' else 0xffffffff)!=record[k] for k in BASE):
        raise ValueError('Foreign or unsupported copied palette owner')
    submitted=record.get('submissions')
    if submitted is None:raise ValueError('Copied palette without submission inventory')
    if kind=='paletteSummary':
        if set(f)!=BASE|{'schema','source','payloads','overflow','outerCompleted','alignment','grasp'} or \
                f['schema'] not in ('1','2','3') or f['source']!=expected_source or 'palette_copies' in submitted:
            raise ValueError('Copied palette summary schema/source/duplicate')
        p={k:number(f[k],10 if k=='payloads' else 1) for k in ('payloads','overflow','outerCompleted','alignment','grasp')}
        if p['alignment'] or p['grasp'] or p['payloads']!=min(submitted['count'],10) or \
                p['overflow']!=int(submitted['count']>10):raise ValueError('Copied palette capacity/claim mismatch')
        p.update(schema=int(f['schema']),rows={},evidence_class='emitter-reported-native-api-copied-diagnostic',
                 source_provenance_authenticated=False,api_coverage_complete=False,
                 vertex_content_verified=False,shader_index_association_verified=False,
                 gpu_visibility_verified=False,positive_grasp_verified=False,alignment_accepted=False)
        submitted['palette_copies']=p;return True
    p=submitted.get('palette_copies')
    if p is None or not p['outerCompleted']:raise ValueError('Palette detail without reported outer completion')
    index=number(f.get('index',''),63);ordinal=number(f.get('ordinal',''))
    original=submitted['rows'].get(index)
    if index>=p['payloads'] or ordinal!=index+1 or original is None or original['status']!=8 or original['metadata'] is None:
        raise ValueError('Palette detail without its qualified original submission')
    if kind=='paletteRow':
        identity={'input','generation','owner','weapon','model'}
        scalars={'schema','index','ordinal','api','modelAddress','drawAddress','modelRecord','drawRecord','instance','surface',
                 'evaluated','matrices','mapping','palette','first','count','mapCount','paletteCount','canonicalCount','modelCount',
                 'canonicalEqual','words','constants','declaration','cleanupCertified','outerCurrent'}
        if p['schema']>=2:scalars.add('contentCopied')
        if p['schema']==3:scalars.add('projectionSequence')
        if set(f)!=BASE|identity|scalars|{'root','render','objects'} or f['schema']!=str(p['schema']) or index in p['rows']:
            raise ValueError('Palette row schema/duplicate')
        if any(number(f[k],(1<<64)-1 if k=='input' else 0xffffffff)!=record[k] or not record[k] for k in identity):
            raise ValueError('Foreign palette invocation identity')
        if f['cleanupCertified']!='1' or f['outerCurrent']!='1':raise ValueError('Unqualified palette completion report')
        r={k:number(f[k],1 if k=='contentCopied' else 0xffffffff) for k in scalars-{'schema'}}
        r.update(root=values(f['root'],3,(2,)),render=values(f['render'],3,(2,)),objects=values(f['objects'],3),data={})
        if r['api']!=index or not 1<=r['count']<=3 or not 1<=r['modelCount']<=2048 or \
                not 0<r['modelRecord']<r['modelCount'] or r['drawRecord']>=8192 or \
                not 1<=r['canonicalCount']<=8192 or r['mapCount']>32768 or r['paletteCount']>32768 or \
                any(r['first']+r['count']>r[k] for k in ('mapCount','paletteCount')) or \
                not 2<=r['words']<=512 or not 1<=r['constants']<=256 or not 1<=r['declaration']<=65 or r['canonicalEqual']>1 or \
                any(not r[k] for k in ('modelAddress','drawAddress','surface','instance','evaluated','matrices','mapping','palette')) or \
                any(not x for x in r['objects']) or not r['root'][0] or not r['render'][0]:
            raise ValueError('Palette row outside copied native/API bounds')
        m=original['metadata']
        if m['keys'][:6]!=[r[k] for k in ('modelAddress','drawAddress','modelRecord','drawRecord','surface','instance')] or \
                r['root']!=m['root'] or r['render']!=m['render'] or \
                r['root']!=[record[k] for k in ('cfg','file','resource')]:
            raise ValueError('Palette and original metadata identity disagreement')
        if (r['count']>1 and (m['keys'][8]!=-1 or m['keys'][7]!=0)) or (r['count']==1 and m['keys'][8]<0):
            raise ValueError('Palette count contradicts single-bone metadata')
        if p['rows'] and index<=max(p['rows']):raise ValueError('Palette details are not in original submission order')
        p['rows'][index]=r;return True
    if set(f)!=BASE|{'index','ordinal','kind','item','chunk','values'}:
        raise ValueError('Palette data schema')
    r=p['rows'].get(index)
    if r is None or ordinal!=r['ordinal']:raise ValueError('Palette data without its own header')
    item=number(f['item']);chunk=number(f['chunk']);channel=f['kind']
    widths={'world':(1,12),'draw':(1,6),'mapping':(r['count'],5),'canonical':(r['count'],12),
            'palette':(r['count'],12),'stream':(3,4),'declaration':(r['declaration'],6),'constant':(r['constants'],4)}
    if r.get('contentCopied'):
        widths.update(contentSurface=(1,14),contentBuffers=(1,10),contentStreams=(1,18),contentHash=(5,8))
    if channel=='program':
        if item or chunk%32 or chunk>=r['words']:raise ValueError('Invalid palette program chunk')
        count=min(32,r['words']-chunk)
    elif channel in widths:
        limit,count=widths[channel]
        if item>=limit or chunk:raise ValueError('Palette data outside copied row bounds')
    else:raise ValueError('Unknown palette data channel')
    key=f'{channel}:{item}:{chunk}'
    if key in r['data']:raise ValueError('Duplicate palette data')
    raw=words(f['values'],count)
    if channel in ('world','canonical','palette') and any(v&0x7f800000==0x7f800000 for v in raw):
        raise ValueError('Nonfinite copied native palette matrix')
    r['data'][key]=raw;return True

def validate_palette(record):
    submitted=record.get('submissions',{});p=submitted.get('palette_copies')
    if p is None:return
    for index,r in p['rows'].items():
        d=r['data'];wanted={'world:0:0','draw:0:0'}
        for kind,count in [('mapping',r['count']),('canonical',r['count']),('palette',r['count']),
                           ('stream',3),('declaration',r['declaration']),('constant',r['constants'])]:
            wanted.update(f'{kind}:{i}:0' for i in range(count))
        wanted.update(f'program:0:{i}' for i in range(0,r['words'],32))
        if r.get('contentCopied'):
            wanted.update(f'{kind}:0:0' for kind in ('contentSurface','contentBuffers','contentStreams'))
            wanted.update(f'contentHash:{i}:0' for i in range(5))
        if set(d)!=wanted:raise ValueError('Truncated copied palette/native/API arrays')
        original=submitted['rows'][index];m=original['metadata']
        if d['draw:0:0']!=[v&0xffffffff for v in original['draw']]:raise ValueError('Foreign original draw in palette data')
        equal=True
        for i in range(r['count']):
            draw,bone,owner,definition,name=d[f'mapping:{i}:0']
            if draw!=r['drawRecord'] or bone>=r['canonicalCount'] or owner!=r['modelRecord'] or not definition:
                raise ValueError('Palette mapping violates copied native ownership')
            if r['count']==1 and [bone,name]!=[m['keys'][8],m['keys'][7]]:
                raise ValueError('Single palette bone/name disagrees with original metadata')
            equal &= d[f'canonical:{i}:0']==d[f'palette:{i}:0']
        if r['canonicalEqual']!=int(equal):raise ValueError('Canonical equality report contradicts copied rows')
        layout,weighted=declaration_layout([d[f'declaration:{i}:0'] for i in range(r['declaration'])])
        if not weighted:raise ValueError('Palette API copy without selected weighted declaration')
        if any(not d[f'stream:{i}:0'][0] for i in range(3)):raise ValueError('Null retained palette stream identity')
        if r.get('contentCopied'):
            surface=d['contentSurface:0:0'];streams=d['contentStreams:0:0']
            if surface!=m['layout'] or streams[16]!=r['objects'][1] or \
                    any(streams[i*4:i*4+4]!=d[f'stream:{i}:0'] for i in range(3)):
                raise ValueError('Copied content differs from native surface or API bindings')
            validate_buffer_content(surface,d['contentBuffers:0:0'],d['draw:0:0'],streams,
                [d[f'declaration:{i}:0'] for i in range(r['declaration'])],2)
            r.update(content_snapshot_copied=True,content_evidence_class='pre-original-bound-buffer-copies',
                     in_place_content_immutability_verified=False)
            if layout in (2,3,4):r['auxiliary_channels']=['uv']
        sequence=r.get('projectionSequence',0)
        if sequence:
            probe=record.get('projection_probe',{})
            pair=probe.get('pairs',{}).get(sequence)
            if probe.get('blocked') or probe.get('pending') or pair is None or pair['source']!=1 or \
                    [pair['modelAfter'],pair['drawAfter']]!=[r['modelAddress'],r['drawAddress']] or \
                    pair['data'].get('1:model')!=d['world:0:0'] or r['constants']<5 or \
                    pair['data'].get('1:cachedMVP')!=sum((d[f'constant:{i}:0'] for i in range(1,5)),[]):
                raise ValueError('Palette receipt without its own current native projection')
            r['projection_association']={'sequence':sequence,'submission_index':index,'ordinal':r['ordinal'],
                'model_address':r['modelAddress'],'draw_address':r['drawAddress'],
                'evidence_class':'emitter-reported-submission-native-api-bookends',
                'source_provenance_authenticated':False,'reference_replaced':False,
                'gpu_visibility_verified':False,'positive_grasp_verified':False,'alignment_accepted':False}
        r.update(input_layout=layout,stream_numbers=[0,7,8] if layout in (1,3,4) else [0,5,6],
                 completion_reported=True,shader_index_association_verified=False,
                 vertex_content_verified=False,alignment_accepted=False,positive_grasp_verified=False)

def validate(record):
    p=record.get('submissions')
    if p is None:return
    if set(p['rows'])!=set(range(p['count'])):raise ValueError('Truncated submission rows')
    for row in p['rows'].values():
        if (row['status']==8)!=(row['metadata'] is not None):raise ValueError('Missing/unqualified submission payload')
    validate_palette(record)
