#!/usr/bin/env python3
"""Read copied idle diagnostics offline; never launch or certify grip alignment."""
import argparse
from decimal import Decimal, InvalidOperation
import json
import math
from pathlib import Path
import re
import struct
from idle_stream_evidence import consume as consume_stream_probe, validate as validate_stream_probe, declaration_layout, OBSERVED, NO_UV56, OBSERVED_MULTI_UV, PASSIVE_FIVE_ROW78
from idle_submission_evidence import consume as consume_submission,validate as validate_submission
from idle_projection_evidence import consume as consume_projection_probe, validate as validate_projection_probe, validate_association

ROOT=Path(__file__).resolve().parents[1]

def draw_capacity(record):
    return {3:8,4:10}[record['schema']]

def fields(line):
    pairs=[part.split('=',1) for part in line.split()[3:]]
    if any(len(p)!=2 for p in pairs) or len({p[0] for p in pairs})!=len(pairs):
        raise ValueError('Malformed or duplicate idle fields')
    return dict(pairs)

def integer(value,low=0,high=(1<<64)-1):
    if not re.fullmatch(r'-?\d+',value):raise ValueError('Invalid integer')
    result=int(value)
    if not low<=result<=high:raise ValueError('Integer outside native range')
    return result

def floats(value,count):
    parts=value.split(',')
    if len(parts)!=count:raise ValueError('Incomplete matrix/vector')
    result=list(map(float,parts))
    if not all(math.isfinite(v) for v in result):raise ValueError('Non-finite idle pose')
    for token,v in zip(parts,result):
        try:
            converted=struct.unpack('<f',struct.pack('<f',v))[0]
            nonzero=not Decimal(token).is_zero()
        except (OverflowError,InvalidOperation) as e:raise ValueError('Pose outside binary32 range') from e
        # %.9g legitimately rounds near FLT_MAX and for subnormal values.
        if not math.isfinite(converted) or (v==0 and nonzero) or \
           (v!=0 and abs(converted-v)>abs(v)*5.1e-9):raise ValueError('Not an emitted binary32 pose value')
    return result

def hexwords(value,count):
    parts=value.split(',')
    if len(parts)!=count or any(not re.fullmatch('[a-f0-9]{8}',v) for v in parts):
        raise ValueError('Incomplete raw native value header')
    return [int(v,16) for v in parts]

def declaration_weights(rows):
    return declaration_layout(rows)[1]

def validate_geometry(geometry,count,copy_layout=0):
    if set(geometry)!=set(range(count)):raise ValueError('Missing geometry headers')
    draw_records=set()
    for g in geometry.values():
        wanted={k+':0' for k in ('affine','clip','layout','buffers','draw','streams')}
        wanted.update('hash:'+str(i) for i in range(5))
        wanted.update('constant:'+str(i) for i in range(g['constants']))
        wanted.update('program:'+str(i) for i in range(0,g['words'],32))
        wanted.update('declaration:'+str(i) for i in range(g['declaration']))
        if 'factors' in g:wanted.update(k+':0' for k in ('factorModel','factorLocal','factorView','factorProjection'))
        if set(g['data'])!=wanted:raise ValueError('Truncated copied geometry emission')
        if not copy_layout and g['drawRecord'] in draw_records:raise ValueError('Repeated native draw identity')
        draw_records.add(g['drawRecord'])
        layout=g['data']['layout:0'];buffers=g['data']['buffers:0'];draw=g['data']['draw:0'];streams=g['data']['streams:0']
        v,t=layout[:2]
        if not 1<=v<=1490 or not 1<=t<=1332 or layout[3]!=0x85 or layout[6]!=0x87 or \
           layout[9]!=0x80 or layout[12]!=0x80 or layout[4]>=255 or layout[7]>=255 or \
           layout[10]!=layout[4] or layout[13]!=layout[4] or draw!=[4,0,0,v,layout[5]//2,t] or layout[5]%2:
            raise ValueError('Unsupported declared geometry layout')
        if buffers[1:5]!=[0,1,100,0] or buffers[6:10]!=[0,1,101,0]:raise ValueError('Unsupported buffer use')
        if not streams[0] or streams[4]!=streams[0] or streams[12]!=streams[0] or not streams[16] or streams[17] or \
           streams[1:4]!=[layout[2],12,1] or streams[5:8]!=[layout[11],4,1] or streams[14:16]!=[8,1] or \
           (streams[8] and (streams[8]!=streams[0] or streams[9:12]!=[layout[8],4,1])):
            raise ValueError('Unsupported stream identity/ranges')
        input_layout,weights=declaration_layout([g['data']['declaration:'+str(i)] for i in range(g['declaration'])])
        g['input_layout']=input_layout
        if input_layout in (2,3,4):g['auxiliary_channels']=['uv']
        if bool(streams[8])!=weights:raise ValueError('Declaration/bound weight disagreement')
        ranges=[(layout[2],v*12,buffers[0]),(layout[5],t*6,buffers[5]),
                (layout[8],v*4,buffers[0]),(layout[11],v*4,buffers[0]),(streams[13],v*8,buffers[0])]
        if any(off+size>capacity for off,size,capacity in ranges):raise ValueError('Geometry outside bound buffer')

def assess(text,expected_source):
    if not re.fullmatch('[a-f0-9]{64}',expected_source):raise ValueError('Expected compiled source required')
    records=[];current=None;seen=set();copy_layout=None;producer_schema=None;producer_native_id=None
    for line in text.splitlines():
        if not line.startswith('Lab idle'):continue
        if not line.startswith('Lab idle ') or len(line.split())<3:
            raise ValueError('Truncated reserved idle prefix')
        kind=line.split()[2];f=fields(line)
        if kind.startswith('submission') and consume_submission(current,kind,f):continue
        if kind=='draw':
            names={'rawGripValid','schema','draws','source','ipc','wire','request','input','owner','weapon','model','generation','hand','eye',
                   'stage','cfg','file','resource','contributors','matrices','historicalBytes','grasp'}
            if set(f) not in (names,names|{'copyLayout'},names|{'nativeId'},names|{'copyLayout','nativeId'}) or f['source']!=expected_source:raise ValueError('Idle build/schema mismatch')
            current={k:integer(v,-(1<<31),(1<<31)-1) if k=='resource' else
                     integer(v,0,(1<<64)-1 if k in ('request','input') else (1<<32)-1)
                     for k,v in f.items() if k!='source'}
            if 'copyLayout' in f and current['copyLayout']!=1:raise ValueError('Unknown copy ordinal layout')
            current['native_id_explicit']='nativeId' in f
            current.setdefault('nativeId',1) # Historical collector admitted only ID1.
            if current['nativeId'] not in (1,13) or (current['nativeId']==13 and current['schema']!=4):
                raise ValueError('Unsupported idle native weapon selection')
            if producer_native_id is not None and producer_native_id!=current['nativeId']:
                raise ValueError('Mixed native weapon selectors')
            producer_native_id=current['nativeId']
            current.setdefault('copyLayout',0)
            if copy_layout is not None and copy_layout!=current['copyLayout']:raise ValueError('Mixed producer copy layouts')
            copy_layout=current['copyLayout']
            if current['schema'] not in (3,4) or current['rawGripValid'] not in (0,1) or current['draws']>draw_capacity(current):raise ValueError('Geometry schema/budget mismatch')
            if current['schema']==4 and current['copyLayout']!=1:raise ValueError('Schema4 requires ordinal copies')
            if producer_schema is not None and producer_schema!=current['schema']:raise ValueError('Mixed producer schemas')
            producer_schema=current['schema']
            if current['ipc']!=10 or current['wire']!=7 or current['historicalBytes'] or current['grasp']:
                raise ValueError('Unsupported layout or provenance/grasp claim')
            if current['stage'] not in range(5) or not 0<=current['contributors']<=16 or not 0<=current['matrices']<=64:
                raise ValueError('Unbounded idle record')
            if current['hand'] not in (0,1) or current['eye'] not in (0,1):raise ValueError('Invalid hand/eye')
            key=(current['request'],current['eye'],current['hand'])
            if key in seen:raise ValueError('Ambiguous repeated native draw')
            seen.add(key)
            current.update({'animations':{},'pose':{},'stretch':None,'geometry':{}})
            records.append(current)
            if len(records)>32:raise ValueError('Idle collection budget exceeded')
            continue
        if kind=='rejection':
            if current is None or current['stage']!=3 or 'rejection' in current or set(f)!={'request','eye','hand','reason','preceding','checks','state','callbacks'}:
                raise ValueError('Rejection without one rejected native-copy record')
            if any(integer(f[k])!=current[k] for k in ('request','eye','hand')):
                raise ValueError('Interleaved/foreign rejection evidence')
            reason=integer(f['reason'],1,46);preceding=integer(f['preceding'],0,4)
            if current['copyLayout']==1 and reason==11:raise ValueError('Unreachable ordinal producer duplicate rejection')
            if preceding==3:raise ValueError('First rejection cannot follow an earlier rejected stage')
            current['rejection']={'reason':reason,'preceding':preceding,'checks':integer(f['checks'],0,32767),
                                  'state':integer(f['state'],0,63),'callbacks':integer(f['callbacks'],0,63)}
            continue
        if kind=='animationNameFailure':
            if current is None or current['stage']!=3 or current.get('rejection',{}).get('reason')!=19 or \
                    current['rejection']['preceding']!=1 or 'animation_name_failure' in current or \
                    set(f)!={'request','eye','hand','index','expected','header'}:
                raise ValueError('Animation name failure without one matching query rejection')
            if any(integer(f[k])!=current[k] for k in ('request','eye','hand')):
                raise ValueError('Interleaved/foreign animation name failure')
            index=integer(f['index'],0,15)
            expected=hexwords(f['expected'],1)[0];header=hexwords(f['header'],4)
            if not current['cfg'] or not current['file'] or not 0<current['contributors']<=16 or \
                    index>=current['contributors'] or expected==header[0] or \
                    current['rejection']['callbacks']&33!=33 or current['draws'] or current['matrices'] or \
                    not current['rejection']['state']&1 or current['rejection']['state']&40 or current['rejection']['checks']:
                raise ValueError('Unqualified animation name failure')
            current['animation_name_failure']={'index':index,'expected':expected,'header':header}
            continue
        if kind=='retainedCopies':
            if current is None or current['stage']!=3 or 'retained_copies' in current or \
                    set(f)!={'request','eye','hand','count','postOriginal','cleanupCertified','outerCurrent'}:
                raise ValueError('Retained companion without one rejected record')
            if any(integer(f[k])!=current[k] for k in ('request','eye','hand')):
                raise ValueError('Foreign retained companion')
            count=integer(f['count'],1,draw_capacity(current))
            if count!=current['draws'] or f['postOriginal']!='1' or f['cleanupCertified']!='0' or f['outerCurrent']!='0':
                raise ValueError('Retained companion claim/count mismatch')
            current['retained_copies']={'count':count,'geometry':{},'evidence_class':'historical-post-original-pre-cleanup-copies',
                'post_original_return':True,'cleanup_certified':False,'outer_current':False,'whole_trace_accepted':False,
                'alignment_accepted':False,'positive_grasp_verified':False}
            continue
        if kind.startswith('stream'):
            if current is None:raise ValueError('Stream probe without native record')
            consume_stream_probe(current,kind,f)
            continue
        if kind.startswith('projection'):
            if current is None:raise ValueError('Native projection probe without draw record')
            consume_projection_probe(current,kind,f,integer,hexwords)
            continue
        if kind.startswith('input'):
            if current is None or current['stage']!=3 or current.get('rejection',{}).get('reason')!=32:
                raise ValueError('Input failure without matching CollectInputs rejection')
            if any(integer(f[k])!=current[k] for k in ('request','eye','hand')):
                raise ValueError('Interleaved/foreign input failure')
            if kind=='inputFailure':
                expected_fields={'request','eye','hand','step','index','hr','valid','caps','declaration','rangeChecks'}
                if set(f) not in (expected_fields,expected_fields|{'layout'}) or 'input_failure' in current:
                    raise ValueError('Duplicate or malformed input failure')
                failure={k:integer(f[k],-(1<<31),(1<<31)-1) if k=='hr' else integer(f[k],0,(1<<32)-1)
                         for k in ('step','index','hr','valid','caps','declaration','rangeChecks')}
                if not 1<=failure['step']<=25 or failure['index']>8 or failure['valid']>15 or failure['rangeChecks']>=(1<<20) or \
                   ((failure['valid']&2) and (not failure['valid']&8 or not 1<=failure['declaration']<=65)) or \
                   ((failure['valid']&4) and failure['valid']!=15):
                    raise ValueError('Input failure validity/budget mismatch')
                step=failure['step']
                failure['layout']=integer(f['layout'],0,4) if 'layout' in f else 0
                failure['layout_explicit']='layout' in f
                if step<=7 and failure['layout']:raise ValueError('Input layout selected before declaration')
                expected_valid=0 if step==1 else 1 if step<=6 else 9 if step==7 else 11 if step<=19 else 15
                api_failures={1,3,4,6,8,10,12,14,16,21,24}
                identity_failures={11,18,19,23}
                if failure['valid']!=expected_valid or \
                   (step in range(8,12) and failure['index'] not in (0,2,5,3,6,7,8)) or \
                   (step not in range(8,12) and failure['index']) or \
                   (step in api_failures and failure['hr']>=0) or \
                   (step not in api_failures|identity_failures and failure['hr']<0) or \
                   (step==1 and failure['caps']) or \
                   (step==2 and 1<=failure['caps']<=256) or \
                   (step>=3 and not 1<=failure['caps']<=256) or \
                   (step<=6 and failure['declaration']) or \
                   (step==7 and 1<=failure['declaration']<=65) or \
                   (step>=8 and not 1<=failure['declaration']<=65) or \
                   (step<20 and failure['rangeChecks']) or \
                   (step==20 and failure['rangeChecks']==(1<<20)-1) or \
                   (step>20 and failure['rangeChecks']!=(1<<20)-1):
                    raise ValueError('Input failure contradicts producer progression')
                failure.update(declaration_rows={},binding=None,streams={},surface=None,channels={})
                current['input_failure']=failure
            else:
                failure=current.get('input_failure')
                if failure is None:raise ValueError('Input data before qualified failure')
                def numbers(value,count,signed_index=None):
                    parts=value.split(',')
                    if len(parts)!=count:raise ValueError('Truncated scalar input snapshot')
                    return [integer(v,-(1<<31),(1<<31)-1) if i==signed_index else integer(v,0,(1<<32)-1) for i,v in enumerate(parts)]
                if kind=='inputDeclaration':
                    if not failure['valid']&2 or set(f)!={'request','eye','hand','index','values'}:raise ValueError('Unqualified declaration snapshot')
                    index=integer(f['index'],0,failure['declaration']-1);values=numbers(f['values'],6)
                    if index in failure['declaration_rows'] or any(v>(65535 if i<2 else 255) for i,v in enumerate(values)):
                        raise ValueError('Duplicate/outside declaration field')
                    failure['declaration_rows'][index]=values
                elif kind in ('inputBinding','inputStream','inputSurface','inputChannel'):
                    if not failure['valid']&4:raise ValueError('Unqualified input binding snapshot')
                    if kind=='inputBinding':
                        if set(f)!={'request','eye','hand','vertex','index','draw','software'} or failure['binding'] is not None:
                            raise ValueError('Binding snapshot schema mismatch')
                        failure['binding']={'vertex':numbers(f['vertex'],5),'index':numbers(f['index'],5),
                                            'draw':numbers(f['draw'],6,1),'software':integer(f['software'],0,1)}
                    elif kind=='inputSurface':
                        if set(f)!={'request','eye','hand','vertices','triangles'} or failure['surface'] is not None:
                            raise ValueError('Surface snapshot schema mismatch')
                        failure['surface']={k:integer(f[k],-(1<<31),(1<<31)-1) for k in ('vertices','triangles')}
                    else:
                        names=('object','offset','stride','frequency') if kind=='inputStream' else ('offset','format','buffer')
                        if set(f)!={'request','eye','hand','index',*names}:raise ValueError('Input channel/stream schema mismatch')
                        index=integer(f['index'],0,3);target=failure['streams' if kind=='inputStream' else 'channels']
                        if index in target:raise ValueError('Repeated input channel/stream')
                        target[index]={k:integer(f[k],0,255 if k in ('format','buffer') else (1<<32)-1) for k in names}
                else:raise ValueError('Unknown input diagnostic')
            continue
        retained=kind in ('retainedGeometry','retainedGeometryData','retainedGeometryFactors','retainedGeometryProjection')
        if retained:
            if current is None or current['stage']!=3 or 'retained_copies' not in current:
                raise ValueError('Retained payload without rejected companion')
            kind={'retainedGeometry':'geometry','retainedGeometryData':'geometryData',
                  'retainedGeometryFactors':'geometryFactors','retainedGeometryProjection':'geometryProjection'}[kind]
        elif current is None or current['stage']!=4:
            raise ValueError('Data without complete native-copy record')
        geometry=current['retained_copies']['geometry'] if retained else current['geometry']
        if any(integer(f[k])!=current[k] for k in ('request','eye','hand')):
            raise ValueError('Interleaved/foreign idle data')
        if kind=='animation':
            if set(f)!={'request','eye','hand','index','raw','header'}:raise ValueError('Animation schema mismatch')
            index=integer(f['index'])
            if not 0<=index<current['contributors'] or index in current['animations']:
                raise ValueError('Duplicate or out-of-range contributor')
            current['animations'][index]={'raw':hexwords(f['raw'],8),'header':hexwords(f['header'],4)}
        elif kind=='matrix':
            if set(f)!={'request','eye','hand','kind','index','values'}:raise ValueError('Matrix schema mismatch')
            index=integer(f['index']);which=f['kind']
            if which not in ('world','nativePlacement','trackedPlacement','controller','rawAim','rawGrip','canonical'):
                raise ValueError('Unknown matrix source')
            if not 0<=index<(current['matrices'] if which=='canonical' else 1):raise ValueError('Matrix outside bounded copy')
            key=which+':'+str(index)
            if key in current['pose']:raise ValueError('Duplicate pose matrix')
            current['pose'][key]=floats(f['values'],12)
        elif kind=='geometry':
            names={'request','eye','hand','index','modelRecord','drawRecord','surface','instance','surfaceName',
                   'boneName','bone','cfg','file','resource','words','constants','declaration'}
            if set(f)!=names:raise ValueError('Geometry schema mismatch')
            index=integer(f['index'],0,draw_capacity(current)-1)
            if index>=current['draws'] or index in geometry:raise ValueError('Duplicate/outside geometry')
            g={k:integer(v,-(1<<31),(1<<31)-1) if k=='resource' else integer(v,0,(1<<32)-1)
               for k,v in f.items() if k not in ('request','eye','hand','index')}
            if not 2<=g['words']<=512 or not 1<=g['declaration']<=65 or not 1<=g['constants']<=256 or not g['surface'] or not g['instance'] or not g['cfg']:
                raise ValueError('Incomplete/unbounded rendered identity')
            g['data']={};geometry[index]=g
        elif kind=='geometryFactors':
            if set(f)!={'request','eye','hand','index','palette','bookends','postOriginal','cleanupCertified','outerCurrent'}:
                raise ValueError('Factor companion schema mismatch')
            index=integer(f['index'],0,draw_capacity(current)-1)
            if index not in geometry or 'factors' in geometry[index]:raise ValueError('Factor companion before header or duplicate')
            if (integer(f['bookends']),integer(f['postOriginal']),integer(f['cleanupCertified']),integer(f['outerCurrent']))!=(3,1,0,0):
                raise ValueError('Unsupported factor provenance claim')
            geometry[index]['factors']={'palette_index':integer(f['palette'],0,32767),'bookends':3,
                'post_original_return':True,'cleanup_certified':False,'outer_current':False,
                'diagnostic_only':True,'reference_replaced':False}
        elif kind=='geometryProjection':
            if set(f)!={'request','eye','hand','index','sequence','modelAddress','drawAddress',
                        'bookends','postOriginal','cleanupCertified','outerCurrent'}:
                raise ValueError('Projection association schema mismatch')
            index=integer(f['index'],0,draw_capacity(current)-1)
            if index not in geometry or 'projection_association' in geometry[index] or 'factors' not in geometry[index]:
                raise ValueError('Projection association before factors or duplicate')
            if (integer(f['bookends']),integer(f['postOriginal']),integer(f['cleanupCertified']),integer(f['outerCurrent']))!=(3,1,0,0):
                raise ValueError('Unsupported projection association provenance')
            geometry[index]['projection_association']={'sequence':integer(f['sequence'],1,8),
                'model_address':integer(f['modelAddress'],1,0xffffffff),'draw_address':integer(f['drawAddress'],1,0xffffffff),
                'bookends':3,'post_original_return':True}
        elif kind=='geometryData':
            if set(f)!={'request','eye','hand','index','kind','chunk','values'}:raise ValueError('Geometry data schema mismatch')
            index=integer(f['index'],0,draw_capacity(current)-1)
            if index not in geometry:raise ValueError('Geometry payload before header')
            g=geometry[index];which=f['kind'];chunk=integer(f['chunk'],0,511)
            factors={'factorModel':12,'factorLocal':12,'factorView':12,'factorProjection':16}
            if which in factors and 'factors' not in g:raise ValueError('Factor payload before qualified companion')
            counts={'declaration':6,'clip':16,'affine':12,'layout':14,'buffers':10,'draw':6,'streams':18,'hash':8,'constant':4,**factors}
            if which=='program':
                if chunk%32 or chunk>=g['words']:raise ValueError('Program chunk outside declared copy')
                count=min(32,g['words']-chunk)
            elif which in counts:
                if chunk>=(5 if which=='hash' else g['constants'] if which=='constant' else g['declaration'] if which=='declaration' else 1):
                    raise ValueError('Geometry chunk outside copy')
                count=counts[which]
            else:raise ValueError('Unknown geometry data')
            key=which+':'+str(chunk)
            if key in g['data']:raise ValueError('Duplicate geometry chunk')
            values=hexwords(f['values'],count)
            if which in ('affine','clip','constant') or which in factors:
                if any(not math.isfinite(struct.unpack('<f',struct.pack('<I',v))[0]) for v in values):
                    raise ValueError('Non-finite native geometry values')
            g['data'][key]=values
        elif kind=='stretch':
            if set(f)!={'request','eye','hand','values'} or current['stretch'] is not None:
                raise ValueError('Stretch schema mismatch/duplicate')
            current['stretch']=floats(f['values'],3)
        else:raise ValueError('Unknown idle diagnostic kind')
    if not records:raise ValueError('No idle observations')
    completed=[];rejected=[]
    for r in records:
        validate_submission(r)
        validate_projection_probe(r)
        failure=r.get('input_failure')
        if failure:
            if failure['valid']&2 and set(failure['declaration_rows'])!=set(range(failure['declaration'])):
                raise ValueError('Truncated input declaration diagnostic')
            rows=[failure['declaration_rows'][i] for i in range(failure['declaration'])] if failure['valid']&2 else []
            if failure['step'] in range(8,12):
                observed=failure['layout'] in (1,3)
                if failure['layout']==4 and failure['index'] not in (0,2,7,8) or \
                   failure['layout']!=4 and (failure['index']==2 or failure['index'] in (7,8) and not observed or
                                            observed and failure['index'] in (5,6)):
                    raise ValueError('Stream failure index does not match selected declaration family')
                if failure['index']==6 and not any(row[0]==6 and row[2]!=17 for row in rows):
                    raise ValueError('Optional stream failure without an active declaration input')
            if failure['valid']&2 and (failure['layout']==1 and rows!=OBSERVED or
                    failure['layout']==2 and rows!=NO_UV56 or
                    failure['layout']==3 and rows!=OBSERVED_MULTI_UV or
                    failure['layout']==4 and rows!=PASSIVE_FIVE_ROW78 or
                    failure['layout_explicit'] and (failure['layout']==1)!=(rows==OBSERVED)):
                raise ValueError('Input failure layout contradicts qualified declaration')
            if failure['valid']&4 and (failure['binding'] is None or failure['surface'] is None or
                set(failure['streams'])!=set(range(4)) or set(failure['channels'])!=set(range(4))):
                raise ValueError('Truncated input binding diagnostic')
        retained=r.get('retained_copies')
        if retained is not None:
            reason=r.get('rejection',{})
            input_history=reason.get('reason')==32 and failure is not None and failure['step']==20 and failure['valid']==15
            repeated_history=r['copyLayout']==0 and reason.get('reason')==11 and 1<=r['draws']<8 and failure is None and 'stream_probe' not in r
            capacity_history=r['copyLayout']==1 and reason.get('reason')==10 and r['draws']==draw_capacity(r) and retained['count']==draw_capacity(r) and \
                reason.get('checks')==32639 and failure is None and 'stream_probe' not in r
            if r['stage']!=3 or not (input_history or repeated_history or capacity_history) or reason.get('preceding')!=2 or \
                    reason.get('state')!=(47|r['rawGripValid']*16) or (not capacity_history and reason.get('checks')) or reason.get('callbacks')!=63 or \
                    not 1<=r['contributors']<=16 or not 1<=r['matrices']<=64 or \
                    any(not r[k] for k in ('request','input','owner','weapon','model','generation','cfg','file')):
                raise ValueError('Retained companion lacks original copied-state qualification')
            validate_geometry(retained['geometry'],retained['count'],r['copyLayout'])
            for g in retained['geometry'].values():validate_association(r,g)
        validate_stream_probe(r,retained_validated=retained is not None)
        if r['stage']!=4:rejected.append(r);continue
        if any(r[k]<=0 for k in ('request','input','owner','weapon','model','generation','cfg')):
            raise ValueError('Missing complete native-copy identity')
        wanted={k+':0' for k in ('world','nativePlacement','trackedPlacement','controller')}
        wanted.add('rawAim:0')
        if r['rawGripValid']:wanted.add('rawGrip:0')
        wanted.update('canonical:'+str(i) for i in range(r['matrices']))
        if not r['contributors'] or not r['matrices'] or set(r['pose'])!=wanted or r['stretch'] is None or \
           set(r['animations'])!=set(range(r['contributors'])):raise ValueError('Truncated idle copy emission')
        validate_geometry(r['geometry'],r['draws'],r['copyLayout'])
        for g in r['geometry'].values():validate_association(r,g)
        r['evidence_class']=('event-pose-and-position-draws-with-auxiliary-uv' if any(
            g['input_layout'] in (2,3,4) for g in r['geometry'].values()) else 'event-pose-and-consumed-draws') if r['draws'] else 'event-pose-only'
        r['geometry_observed']=bool(r['draws'])
        completed.append(r)
    return {'schema':producer_schema,'source_fingerprint':expected_source,'native_execution_by_assessor':False,
            'copied_event_pose_observations':completed,'rejected_or_missing_observations':rejected,
            'historical_loaded_bytes_verified':False,'positive_grasp_verified':False,'alignment_accepted':False,
            'remaining':['rendered geometry content association','interpreted winning blend/cache evidence','positive measured grasp reference']}

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--log',type=Path,required=True);p.add_argument('--expected-source',required=True)
    p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    try:
        if a.log.stat().st_size>16*1024*1024:raise ValueError('Use a bounded private run log')
        out=a.output.resolve()
        if out.is_relative_to(ROOT) or out.exists():raise ValueError('Choose a fresh private output outside source')
        result=assess(a.log.read_text(),a.expected_source)
        with out.open('x') as f:json.dump(result,f,indent=2);f.write('\n')
    except (OSError,ValueError,KeyError) as e:p.exit(1,str(e)+'\n')

if __name__=='__main__':main()
