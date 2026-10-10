#!/usr/bin/env python3
"""Assess private borrowed ride-clamp scalar records; no runtime or resource proof."""
import argparse
import json
from pathlib import Path
import re

ROOT=Path(__file__).resolve().parents[1]
PREFIX='Lab rideControl'
FIELDS={'schema','source','ordinal','input','generation','session','reference','player','ride','seat','brain','thread',
        'class','mode','executionAbilities','movementAbilities','parameterToken','renderableToken',
        'callbackCalls','callbackReturned','resourceAssociated','frameAssociated','steeringApplied'}
JOIN_FIELDS={'schema','source','invocation','receiver','class','handle','thread','renderableToken','instanceToken',
             'innerCalls','innerReturned','borrowedJoin','localRiderAssociated','operatedSeatAssociated',
             'resourceAssociated','frameAssociated','steeringApplied'}

def assess_join(text,expected_source):
    if not isinstance(expected_source,str) or not re.fullmatch('[a-f0-9]{64}',expected_source):
        raise ValueError('Expected exact source fingerprint')
    prefix='Lab rideModelJoin';records=[];previous=0
    for line in text.splitlines():
        if not line.startswith(prefix):continue
        if not line.startswith(prefix+' '):raise ValueError('Truncated reserved ride-model prefix')
        pairs=[]
        for token in line[len(prefix)+1:].split():
            if token.count('=')!=1:raise ValueError('Malformed ride-model field')
            pairs.append(token.split('='))
        fields=dict(pairs)
        if len(fields)!=len(pairs) or set(fields)!=JOIN_FIELDS or fields['source']!=expected_source:
            raise ValueError('Ride-model source/schema fields differ')
        row={}
        for key,value in fields.items():
            if key=='source':continue
            if not re.fullmatch('0|[1-9][0-9]*',value) or int(value)>(1<<32)-1:
                raise ValueError('Invalid unsigned ride-model field')
            row[key]=int(value)
        if row['schema']!=1 or not previous<row['invocation']<=64 or \
           row['class'] not in (0x2a8558,0x2b8420) or \
           (row['innerCalls'],row['innerReturned'],row['borrowedJoin'])!=(1,1,1):
            raise ValueError('Unsupported ride-model borrow identity')
        if any(not row[k] for k in ('receiver','thread','renderableToken','instanceToken')):
            raise ValueError('Missing native getter-result identity')
        if any(row[k] for k in ('localRiderAssociated','operatedSeatAssociated','resourceAssociated','frameAssociated','steeringApplied')):
            raise ValueError('Getter-time join cannot certify rider, resources, frames or physical steering')
        records.append(row);previous=row['invocation']
    return {'schema':1,'source_fingerprint':expected_source,'observations':records,
            'evidence_scope':'single-native-getter-entity-renderable-instance-chain',
            'getter_time_join_observed':bool(records),'local_rider_verified':False,
            'operated_seat_authority_verified':False,'installed_resource_association_verified':False,
            'evaluated_control_frame_verified':False,'physical_steering_verified':False,'runtime_executed_by_assessor':False}

def assess_render(text,expected_source):
    """Copied render evidence only; addresses never authorize a later read."""
    if not isinstance(expected_source,str) or not re.fullmatch('[a-f0-9]{64}',expected_source):
        raise ValueError('Expected exact source fingerprint')
    base={'schema','row','bank','eye'}
    identity={'player','brain','ride','seat','class','renderableHandle','renderable','instance','cfg','file','resource',
              'modelRecord','evaluated','matrices','mainBone','definition','resourceClaim','seatClaim','graspClaim','steeringClaim'}
    binding={'skeleton','lod','definitions','definitionCount','boneFirst','boneCount','canonicalCount','cacheRows','cacheRowCount'}
    main_draw={'model','draw','surface','instance','name','bone','definition','cfg','file','resource','lod',
               'paletteFirst','paletteCount','localMainSlot','topology','base','minimum','vertices','start',
               'primitives','buffersClaim','positionProgramClaim','graspClaim','steeringClaim','originalSucceeded',
               'cleanupCurrent'}
    main_draw_layout={'vertices','triangles','positions','indices','weights','localIndices'}
    gpu_fields={'programWords','constantRows','declarationElements','inputLayout','positions','localIndices','weights','uv',
                'indexObject','declarationObject','shaderObject','vertexDesc','indexDesc','softwarePositions','hashes'}
    gpu_word_fields={'kind','offset','words'}
    gpu_profiles={
        (2741,2806):((133,0,3600),(135,0,2520),(128,0,124040),(128,0,135004)),
        (2464,2626):((133,0,3024),(135,0,1188),(128,0,166048),(128,0,175904)),
    }
    copies={};active=None
    def unsigned(raw,hexadecimal=False):
        if not re.fullmatch('[0-9a-f]+' if hexadecimal else '0|[1-9][0-9]*',raw):raise ValueError('Invalid ride-render integer')
        v=int(raw,16 if hexadecimal else 10)
        if v>0xffffffff:raise ValueError('Ride-render field exceeds native width')
        return v
    def signed(raw):
        if not re.fullmatch('-?(0|[1-9][0-9]*)',raw):raise ValueError('Invalid signed ride-render integer')
        value=int(raw)
        if not -(1<<31)<=value<=(1<<31)-1:raise ValueError('Ride-render signed field exceeds native width')
        return value
    def core_complete(r):
        wanted={'modelWorld','MainCanonical'}|({'SeatCanonical'} if r['schema']>=2 else set())
        return r['binding'] is not None and set(r['raw_matrices'])==wanted and \
               (r['schema']<3 or not r['attachmentMapped'] or
                (r['attachment_metadata'] is not None and r['raw_attachment_pose'] is not None))
    def complete(r):
        return core_complete(r) and (r['schema'] not in (4,5) or
            (len(r['main_draws'])==r['mainDrawCount'] and all(
                d['layout'] is not None and set(d['raw_matrices'])=={'modelWorld','actualPalette'} and
                (r['schema']==4 or not d['gpuCopied'] or d['gpu'] is not None and
                 all(len(d['gpu_words'][kind])==d['gpu'][kind+'Words'] for kind in ('program','constants','declaration')))
                for d in r['main_draws'])))
    def gpu_values(raw,count,name):
        values=raw.split(',')
        if len(values)!=count or any(not re.fullmatch('0|[1-9][0-9]*',value) for value in values):
            raise ValueError('Invalid ride-render GPU '+name)
        parsed=[int(value) for value in values]
        if any(value>0xffffffff for value in parsed):raise ValueError('Ride-render GPU field exceeds native width')
        return parsed
    def descriptor(raw,name):
        size,usage,pool,format_,fvf=gpu_values(raw,5,name)
        return {'size':size,'usage':usage,'pool':pool,'format':format_,'fvf':fvf}
    def gpu_complete(d):
        if not d['gpuCopied']:return d['gpu'] is None
        if d['gpu'] is None:return False
        return all(len(d['gpu_words'][kind])==d['gpu'][kind+'Words'] for kind in ('program','constants','declaration'))
    def declaration_words(words):
        return [((word & 0xffff),word >> 16,(words[2*i+1] >> 24) & 0xff,(words[2*i+1] >> 16) & 0xff,
                 (words[2*i+1] >> 8) & 0xff,words[2*i+1] & 0xff) for i,word in enumerate(words[::2])]
    def declaration_matches(elements,layout):
        exact={
            1:((0,0,2,0,5,0),(0,2,1,0,5,2),(0,3,1,0,5,3),(0,7,8,0,5,7),(0,8,8,0,5,8),(0,255,17,0,0,0)),
            2:((0,0,2,0,5,0),(0,1,2,0,5,1),(0,5,8,0,5,5),(0,6,8,0,5,6),(0,255,17,0,0,0)),
            3:((0,0,2,0,5,0),(0,2,1,0,5,2),(0,3,1,0,5,3),(0,4,1,0,5,4),(0,5,1,0,5,5),(0,7,8,0,5,7),(0,8,8,0,5,8),(0,255,17,0,0,0)),
            4:((0,0,2,0,5,0),(0,2,1,0,5,2),(0,7,8,0,5,7),(0,8,8,0,5,8),(0,255,17,0,0,0)),
        }
        if layout in exact:return tuple(elements)==exact[layout]
        wanted={0:2,3:1,5:8,6:8}
        seen=set()
        for index,(offset,stream,type_,method,usage,usage_index) in enumerate(elements):
            if stream==255:return index+1==len(elements) and len(seen)==4 and (offset,type_,method,usage,usage_index)==(0,17,0,0,0)
            if stream>15 or stream in (7,8) or usage==5 and usage_index in (7,8):return False
            if stream in wanted:
                if stream in seen or (offset,type_,method,usage,usage_index)!=(0,wanted[stream],0,5,stream):return False
                seen.add(stream)
            elif usage==5 and usage_index in wanted and stream!=usage_index:return False
        return False
    for line in text.splitlines():
        if not re.match(r'^Lab\s+ride\s+render',line):continue
        if not line.startswith('Lab ride render '):raise ValueError('Malformed reserved ride-render prefix')
        parts=line.split();kind='identity'
        if len(parts)>3 and parts[3] in ('binding','matrix','attachment','attachmentPose','mainDraw','mainDrawLayout','mainDrawMatrix','mainDrawGpu','mainDrawGpuWords'):kind=parts.pop(3)
        pairs=[p.split('=') for p in parts[3:]]
        if any(len(p)!=2 for p in pairs):raise ValueError('Malformed ride-render record')
        f=dict(pairs)
        if len(f)!=len(pairs) or not base.issubset(f):raise ValueError('Duplicate/missing ride-render fields')
        schema=unsigned(f['schema']);row=unsigned(f['row']);bank=unsigned(f['bank'])
        if schema not in (1,2,3,4,5):raise ValueError('Unknown ride-render schema operation')
        if f['eye'] not in ('-1','0','1') or row>=32 or not bank:
            raise ValueError('Unsupported ride-render owner/budget')
        eye=int(f['eye']);key=(row,bank,eye)
        attachment={'parameter','parameterFlags','seatData','attachment','childState','childArray','childCount',
                    'descriptor','parentName','childFlags','childRecordPresent','childRecord','childWorldAvailable','flatTree'}
        wanted=base|(identity if kind=='identity' else binding if kind=='binding' else
                     {'kind','words'} if kind=='matrix' else attachment if kind=='attachment' else
                     {'pose','scale'} if kind=='attachmentPose' else
                     {'ordinal'}|main_draw|({'gpuCopied'} if schema==5 and kind=='mainDraw' else set()) if kind=='mainDraw' else
                     {'ordinal'}|main_draw_layout if kind=='mainDrawLayout' else {'ordinal','kind','words'})
        if kind=='mainDrawGpu':wanted=base|{'ordinal'}|gpu_fields
        if kind=='mainDrawGpuWords':wanted=base|{'ordinal'}|gpu_word_fields
        if kind=='identity' and schema>=2:wanted|={'source','seatBone','seatDefinition'}
        if kind=='identity' and schema>=3:wanted|={'attachmentMapped'}
        if kind=='identity' and schema in (4,5):wanted|={'mainDrawCount','mainDrawOverflow'}
        if set(f)!=wanted:raise ValueError('Ride-render schema mismatch')
        if kind=='identity':
            if key in copies or (active is not None and (key<=active or not complete(copies[active]))):raise ValueError('Duplicate/out-of-order/interleaved ride-render owner')
            if schema>=2 and f['source']!=expected_source:raise ValueError('Ride-render source mismatch')
            r={k:unsigned(f[k],k=='class') for k in wanted-{'source','eye'}};r['eye']=eye
            if any(not r[k] for k in ('player','brain','ride','renderableHandle','renderable','instance','cfg','modelRecord','evaluated','matrices','definition')) or \
                    r['class'] not in (0x2a8558,0x2b8420) or \
                    any(r[k] for k in ('resourceClaim','seatClaim','graspClaim','steeringClaim')):
                raise ValueError('Ride-render identity/claim mismatch')
            if schema>=2 and (not r['seatDefinition'] or r['definition']==r['seatDefinition'] or r['mainBone']==r['seatBone']):
                raise ValueError('Ride-render Main/Seat definitions are not distinct')
            if schema>=3 and r['attachmentMapped'] not in (0,1):raise ValueError('Ride-render attachment mapping mismatch')
            if schema in (4,5) and (r['mainDrawCount']>8 or r['mainDrawOverflow'] not in (0,1) or
                              (r['mainDrawOverflow'] and r['mainDrawCount']!=8)):
                raise ValueError('Ride-render Main draw inventory mismatch')
            r.update(source_fingerprint=f.get('source'),source_matches_expected=schema>=2,binding=None,raw_matrices={},
                     attachment_metadata=None,raw_attachment_pose=None,main_draws=[])
            copies[key]=r;active=key
        else:
            r=copies.get(key)
            if r is None or key!=active or r['schema']!=schema:raise ValueError('Orphan/interleaved/mixed ride-render details')
            if kind=='binding':
                if r['binding'] is not None or r['raw_matrices']:raise ValueError('Duplicate/late ride-render binding')
                b={k:unsigned(f[k]) for k in binding}
                if any(not b[k] for k in binding) or b['definitionCount']>8192 or b['canonicalCount']>8192 or \
                        b['cacheRowCount']>2048 or r['modelRecord']>=b['cacheRowCount'] or \
                        b['boneFirst']+b['boneCount']>b['canonicalCount']:
                    raise ValueError('Ride-render cache bounds mismatch')
                for name in ('main','seat') if schema>=2 else ('main',):
                    bone=r[name+'Bone'];definition=r['definition' if name=='main' else 'seatDefinition']
                    if not b['boneFirst']<=bone<b['boneFirst']+b['boneCount'] or definition<b['definitions'] or \
                            (definition-b['definitions'])%0x78 or (definition-b['definitions'])//0x78>=b['definitionCount']:
                        raise ValueError('Ride-render bone does not belong to selected model/LOD')
                r['binding']=b
            elif kind=='matrix':
                if r['binding'] is None:raise ValueError('Ride-render matrices without binding')
                channel=f['kind'];inventory={'modelWorld','MainCanonical'}|({'SeatCanonical'} if schema>=2 else set())
                v=f['words'].split(',')
                if channel not in inventory or channel in r['raw_matrices'] or len(v)!=12 or \
                        any(not re.fullmatch('[0-9a-f]{8}',x) for x in v):raise ValueError('Invalid ride-render raw matrix inventory')
                r['raw_matrices'][channel]=[int(x,16) for x in v]
            elif kind=='attachment':
                inventory={'modelWorld','MainCanonical','SeatCanonical'}
                if schema<3 or not r['attachmentMapped'] or r['binding'] is None or set(r['raw_matrices'])!=inventory or \
                        r['attachment_metadata'] is not None:
                    raise ValueError('Invalid ride-render attachment inventory')
                a={k:unsigned(f[k]) for k in attachment}
                if any(not a[k] for k in ('parameter','seatData','childState','childArray','descriptor')) or \
                        a['parameterFlags']&1 or not 1<=a['childCount']<=32 or a['flatTree']!=1 or \
                        a['childWorldAvailable']!=0 or a['childRecordPresent'] not in (0,1) or \
                        (not a['childRecordPresent'] and a['childRecord']!=0) or \
                        (a['childRecordPresent'] and (not 1<=a['childRecord']<r['binding']['cacheRowCount'] or a['childRecord']==r['modelRecord'])):
                    raise ValueError('Ride-render attachment metadata mismatch')
                r['attachment_metadata']=a
            elif kind=='attachmentPose':
                if schema<3 or r['attachment_metadata'] is None or r['raw_attachment_pose'] is not None:
                    raise ValueError('Invalid ride-render attachment pose inventory')
                pose=f['pose'].split(',');scale=f['scale'].split(',')
                if len(pose)!=7 or len(scale)!=3 or any(not re.fullmatch('[0-9a-f]{8}',x) for x in pose+scale):
                    raise ValueError('Invalid ride-render attachment pose')
                r['raw_attachment_pose']={'pose':[int(x,16) for x in pose],'scale':[int(x,16) for x in scale]}
            elif kind=='mainDraw':
                if schema not in (4,5) or not core_complete(r) or len(r['main_draws'])>=r['mainDrawCount']:
                    raise ValueError('Invalid ride-render Main draw ordering')
                if r['main_draws'] and (r['main_draws'][-1]['layout'] is None or
                                        set(r['main_draws'][-1]['raw_matrices'])!={'modelWorld','actualPalette'} or
                                        (schema==5 and not gpu_complete(r['main_draws'][-1]))):
                    raise ValueError('Truncated ride-render Main draw before next ordinal')
                ordinal=unsigned(f['ordinal'])
                if ordinal!=len(r['main_draws']):raise ValueError('Ride-render Main draw ordinal mismatch')
                d={k:(signed(f[k]) if k=='base' else unsigned(f[k])) for k in main_draw}
                if d['model']!=r['modelRecord'] or d['instance']!=r['instance'] or d['bone']!=r['mainBone'] or \
                   d['definition']!=r['definition'] or d['cfg']!=r['cfg'] or d['file']!=r['file'] or \
                   d['resource']!=r['resource'] or d['lod']!=r['binding']['lod'] or not d['surface'] or \
                   not 1<=d['paletteCount']<=32 or d['localMainSlot']>=d['paletteCount'] or \
                   d['paletteFirst']+d['paletteCount']>32768 or \
                   any(d[k] for k in ('buffersClaim','positionProgramClaim','graspClaim','steeringClaim')) or \
                   (d['originalSucceeded'],d['cleanupCurrent'])!=(1,1):
                    raise ValueError('Ride-render Main draw identity/claim mismatch')
                if schema==5:
                    d['gpuCopied']=unsigned(f['gpuCopied'])
                    if d['gpuCopied'] not in (0,1):raise ValueError('Ride-render GPU copied flag mismatch')
                d.update(ordinal=ordinal,layout=None,raw_matrices={},gpuCopied=d.get('gpuCopied',0),gpu=None,
                         gpu_words={'program':[],'constants':[],'declaration':[]})
                r['main_draws'].append(d)
            elif kind=='mainDrawLayout':
                if schema not in (4,5) or not r['main_draws'] or not core_complete(r):raise ValueError('Orphan ride-render Main layout')
                ordinal=unsigned(f['ordinal']);d=r['main_draws'][-1]
                if ordinal!=d['ordinal'] or d['layout'] is not None or d['raw_matrices']:
                    raise ValueError('Ride-render Main layout ordering mismatch')
                layout={'vertices':signed(f['vertices']),'triangles':signed(f['triangles'])}
                for channel in ('positions','indices','weights','localIndices'):
                    values=f[channel].split(',')
                    if len(values)!=3 or any(not re.fullmatch('0|[1-9][0-9]*',v) for v in values):
                        raise ValueError('Invalid ride-render Main layout channel')
                    fmt,buffer,offset=(int(v) for v in values)
                    if fmt>255 or buffer>255 or offset>0xffffffff:raise ValueError('Ride-render Main layout bounds mismatch')
                    layout[channel]={'format':fmt,'buffer':buffer,'offset':offset}
                d['layout']=layout
            elif kind=='mainDrawMatrix':
                if schema not in (4,5) or not r['main_draws'] or not core_complete(r):raise ValueError('Orphan ride-render Main matrix')
                ordinal=unsigned(f['ordinal']);d=r['main_draws'][-1]
                words=f['words'].split(',')
                if ordinal!=d['ordinal'] or d['layout'] is None or len(words)!=12 or \
                   f['kind'] not in {'modelWorld','actualPalette'} or f['kind'] in d['raw_matrices'] or \
                   any(not re.fullmatch('[0-9a-f]{8}',word) for word in words):
                    raise ValueError('Invalid ride-render Main matrix inventory')
                parsed=[int(word,16) for word in words]
                if f['kind']=='modelWorld' and parsed!=r['raw_matrices']['modelWorld']:
                    raise ValueError('Ride-render Main world matrix differs from frame')
                d['raw_matrices'][f['kind']]=parsed
                if set(d['raw_matrices'])=={'modelWorld','actualPalette'}:continue
            elif kind=='mainDrawGpu':
                if schema!=5 or not r['main_draws'] or not core_complete(r):raise ValueError('Orphan ride-render GPU receipt')
                ordinal=unsigned(f['ordinal']);d=r['main_draws'][-1]
                if ordinal!=d['ordinal'] or not d['gpuCopied'] or d['gpu'] is not None or \
                   set(d['raw_matrices'])!={'modelWorld','actualPalette'}:
                    raise ValueError('Ride-render GPU receipt ordering mismatch')
                if sum(draw['gpuCopied'] for copy in copies.values() for draw in copy['main_draws'])>64:
                    raise ValueError('Ride-render GPU receipt budget exceeded')
                g={key:unsigned(f[key]) for key in ('programWords','constantRows','declarationElements','inputLayout',
                                                     'indexObject','declarationObject','shaderObject','softwarePositions')}
                if not 2<=g['programWords']<=4096 or not 1<=g['constantRows']<=256 or \
                   not 1<=g['declarationElements']<=65 or g['inputLayout']>4 or \
                   any(not g[key] for key in ('indexObject','declarationObject','shaderObject')) or g['softwarePositions']!=0:
                    raise ValueError('Ride-render GPU receipt metadata mismatch')
                for key in ('positions','localIndices','weights','uv'):
                    object_,offset,stride,frequency=gpu_values(f[key],4,key)
                    g[key]={'object':object_,'offset':offset,'stride':stride,'frequency':frequency}
                g['vertexDesc']=descriptor(f['vertexDesc'],'vertex descriptor')
                g['indexDesc']=descriptor(f['indexDesc'],'index descriptor')
                hashes=f['hashes'].split(',')
                if len(hashes)!=5 or any(not re.fullmatch('[a-f0-9]{64}',value) for value in hashes):
                    raise ValueError('Invalid ride-render GPU hashes')
                g['hashes']=hashes
                profile=gpu_profiles.get((d['layout']['vertices'],d['layout']['triangles']))
                if profile is None or tuple(tuple(d['layout'][key][part] for part in ('format','buffer','offset'))
                                                    for key in ('positions','indices','weights','localIndices'))!=profile:
                    raise ValueError('Unsupported ride-render GPU surface profile')
                if (d['topology'],d['base'],d['minimum'],d['vertices'],d['primitives']) != \
                   (4,0,0,d['layout']['vertices'],d['layout']['triangles']) or 2*d['start']!=d['layout']['indices']['offset']:
                    raise ValueError('Ride-render GPU draw topology mismatch')
                if (g['vertexDesc']['usage'],g['vertexDesc']['pool'],g['vertexDesc']['format'],g['vertexDesc']['fvf']) != (0,1,100,0) or \
                   (g['indexDesc']['usage'],g['indexDesc']['pool'],g['indexDesc']['format'],g['indexDesc']['fvf']) != (0,1,101,0):
                    raise ValueError('Ride-render GPU descriptor mismatch')
                streams=(g['positions'],g['localIndices'],g['weights'],g['uv'])
                if not streams[0]['object'] or any(stream['object']!=streams[0]['object'] for stream in streams) or \
                   (g['positions']['offset'],g['positions']['stride'],g['positions']['frequency']) != (d['layout']['positions']['offset'],12,1) or \
                   (g['localIndices']['offset'],g['localIndices']['stride'],g['localIndices']['frequency']) != (d['layout']['localIndices']['offset'],4,1) or \
                   (g['weights']['offset'],g['weights']['stride'],g['weights']['frequency']) != (d['layout']['weights']['offset'],4,1) or \
                   (g['uv']['stride'],g['uv']['frequency']) != (8,1):
                    raise ValueError('Ride-render GPU vertex stream mismatch')
                if any(stream['offset']+stream['stride']*d['layout']['vertices']>g['vertexDesc']['size'] for stream in streams) or \
                   2*(d['start']+3*d['primitives'])>g['indexDesc']['size']:
                    raise ValueError('Ride-render GPU buffer range mismatch')
                g['constantsWords']=4*g['constantRows'];g['declarationWords']=2*g['declarationElements'];g['next_kind']=0
                d['gpu']=g
            else:
                if schema!=5 or not r['main_draws'] or not core_complete(r):raise ValueError('Orphan ride-render GPU words')
                ordinal=unsigned(f['ordinal']);d=r['main_draws'][-1];kind=f['kind']
                kinds=('program','constants','declaration')
                if ordinal!=d['ordinal'] or d['gpu'] is None or kind not in d['gpu_words'] or \
                   d['gpu']['next_kind']>=len(kinds) or kind!=kinds[d['gpu']['next_kind']]:
                    raise ValueError('Ride-render GPU word ordering mismatch')
                words=f['words'].split(',')
                expected=d['gpu'][kind+'Words'];offset=unsigned(f['offset'])
                if not 1<=len(words)<=64 or any(not re.fullmatch('[0-9a-f]{8}',word) for word in words) or \
                   offset!=len(d['gpu_words'][kind]) or len(words)!=min(64,expected-offset) or offset>=expected:
                    raise ValueError('Ride-render GPU word chunk mismatch')
                d['gpu_words'][kind].extend(int(word,16) for word in words)
                if len(d['gpu_words'][kind])==expected:
                    if kind=='declaration' and not declaration_matches(declaration_words(d['gpu_words'][kind]),d['gpu']['inputLayout']):
                        raise ValueError('Ride-render GPU declaration mismatch')
                    d['gpu']['next_kind']+=1
    groups={}
    for key,r in copies.items():
        wanted={'modelWorld','MainCanonical'}|({'SeatCanonical'} if r['schema']>=2 else set())
        if not complete(r):raise ValueError('Truncated ride-render copy')
        for draw in r['main_draws']:
            if draw['gpu'] is not None:draw['gpu'].pop('next_kind')
        group=groups.setdefault(key[0],[]);group.append(r)
        r.update(seat_canonical_copied=r['schema']>=2,attachment_mapping_copied=r['schema']>=3 and bool(r['attachmentMapped']),
                 operated_seat_attachment_verified=False,
                 source_provenance_authenticated=False,authentication_verified=False,
                 simulation_time_freshness_verified=False,buffers_verified=False,position_program_verified=False,
                 grasp_verified=False,physical_steering_verified=False,overflow=bool(r.get('mainDrawOverflow',0)))
    shared={'schema','bank','player','brain','ride','seat','class','renderableHandle','renderable','instance','cfg','file','resource','source_fingerprint'}
    for group in groups.values():
        if {r['eye'] for r in group} not in ({-1},{0,1}) or len(group) not in (1,2):raise ValueError('Incomplete/mixed ride-render eye group')
        if any(r['schema']!=group[0]['schema'] for r in group[1:]) or \
                any(any(r[k]!=group[0][k] for k in shared) for r in group[1:]) or \
                (group[0]['schema']>=3 and any(r['attachmentMapped']!=group[0]['attachmentMapped'] for r in group[1:])):
            raise ValueError('Crossed ride-render stereo owners')
        if len(group)==2 and group[0]['schema']>=3 and group[0]['attachmentMapped']:
            a,b=group
            if any(a['attachment_metadata'][k]!=b['attachment_metadata'][k] for k in attachment-{'childRecordPresent','childRecord'}) or \
                    a['raw_attachment_pose']!=b['raw_attachment_pose']:
                raise ValueError('Crossed ride-render stereo attachment')
    rows=list(copies.values());observed=expected_source if rows and all(r['schema']>=2 for r in rows) else None
    return {'schema':1,'observed_source_fingerprint':observed,'source_matches_expected':observed is not None,
            'observations':rows,'evidence_scope':'emitter-reported-completed-native-render-copies',
            'canonical_matrix_role':'native-draw-palette-source',
            'bone_placement_matrix_copied':False,'draw_palette_mapping_verified':False,
            'draw_mapping_copies_present':any(r['main_draws'] for r in rows),
            'declared_draw_inventory_complete':bool(rows) and all(r['schema'] in (4,5) for r in rows) and not any(r['overflow'] for r in rows),
            'gpu_receipts_present':any(d['gpu'] is not None for r in rows for d in r['main_draws']),
            'seat_frame_copies_present':any(r['seat_canonical_copied'] for r in rows),
            'source_provenance_authenticated':False,'operated_seat_authority_verified':False,
            'installed_resource_association_verified':False,'evaluated_control_frame_verified':False,
            'authentication_verified':False,'simulation_time_freshness_verified':False,'buffers_verified':False,
            'position_program_verified':False,'grasp_verified':False,'physical_steering_verified':False,
            'runtime_executed_by_assessor':False}

def assess(text,expected_source):
    if not isinstance(expected_source,str) or not re.fullmatch('[a-f0-9]{64}',expected_source):
        raise ValueError('Expected exact source fingerprint')
    records=[]
    for line in text.splitlines():
        if not line.startswith(PREFIX):continue
        if not line.startswith(PREFIX+' '):raise ValueError('Truncated reserved ride-control prefix')
        tokens=line[len(PREFIX)+1:].split()
        pairs=[]
        for token in tokens:
            if token.count('=')!=1:raise ValueError('Malformed ride-control field')
            pairs.append(token.split('='))
        fields=dict(pairs)
        if len(fields)!=len(pairs) or set(fields)!=FIELDS or fields['source']!=expected_source:
            raise ValueError('Ride-control source/schema fields differ')
        row={}
        for key,value in fields.items():
            if key=='source':continue
            maximum=(1<<64)-1 if key=='input' else (1<<32)-1
            if not re.fullmatch('0|[1-9][0-9]*',value) or int(value)>maximum:
                raise ValueError('Invalid unsigned ride-control field')
            row[key]=int(value)
        if row['schema']!=1 or row['ordinal']!=len(records)+1 or row['ordinal']>64 or \
           row['class'] not in (0x2a8558,0x2b8420) or row['callbackCalls']!=1 or row['callbackReturned']!=1:
            raise ValueError('Unsupported ride-control callback identity')
        if any(not row[k] for k in ('input','generation','session','reference','player','ride','brain','thread')):
            raise ValueError('Missing ride-control observation identity')
        if any(row[k]!=0 for k in ('resourceAssociated','frameAssociated','steeringApplied')):
            raise ValueError('Scalar-only observation cannot certify resources, frames or physical steering')
        records.append(row)
    return {'schema':1,'source_fingerprint':expected_source,'observations':records,
            'evidence_scope':'borrowed-callback-scalars-before-downstream-ClientAction',
            'mode_observed_at_clamp':bool(records),'input_application_verified':False,
            'operated_seat_authority_verified':False,'installed_resource_association_verified':False,
            'evaluated_control_frame_verified':False,'physical_steering_verified':False,'runtime_executed_by_assessor':False}

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--log',type=Path,required=True)
    p.add_argument('--kind',choices=('control','model-join','render'),default='control')
    p.add_argument('--expected-source',required=True);p.add_argument('--output',type=Path,required=True)
    a=p.parse_args()
    try:
        limit=5*1024*1024 if a.kind=='render' else 16*1024*1024
        if a.log.stat().st_size>limit:raise ValueError('Log exceeds private evidence budget')
        out=a.output.resolve()
        if out.is_relative_to(ROOT) or out.exists() or a.output.is_symlink():raise ValueError('Choose fresh private output outside source')
        result={'control':assess,'model-join':assess_join,'render':assess_render}[a.kind](a.log.read_text(),a.expected_source)
        with out.open('x') as f:f.write(json.dumps(result,indent=2)+'\n')
        print(json.dumps({'observations':len(result['observations']),'physical_steering_verified':False}))
    except (ValueError,OSError,TypeError) as e:p.exit(1,str(e)+'\n')
