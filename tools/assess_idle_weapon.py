#!/usr/bin/env python3
"""Read copied idle diagnostics offline; never launch or certify grip alignment."""
import argparse
from decimal import Decimal, InvalidOperation
import json
import math
from pathlib import Path
import re
import struct

ROOT=Path(__file__).resolve().parents[1]

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
    seen=set();active=set()
    for i,e in enumerate(rows):
        stream,offset,kind,method,usage,index=e
        if stream>65535 or offset>65535 or any(x>255 for x in e[2:]):raise ValueError('Declaration field exceeds native width')
        if stream==255:
            if i!=len(rows)-1 or e!=[255,0,17,0,0,0]:raise ValueError('Invalid declaration end')
            break
        if stream>15:raise ValueError('Unsupported declaration stream')
        if usage==5 and index in (0,3,5,6) and stream!=index:raise ValueError('Aliased declaration semantic')
        if stream not in (0,3,5,6):continue
        if stream in seen:raise ValueError('Duplicate declaration input')
        seen.add(stream)
        if kind==17:
            if stream!=6:raise ValueError('Missing required declaration input')
            continue
        if offset or method or usage!=5 or index!=stream or kind!={0:2,3:1,5:8,6:8}[stream]:
            raise ValueError('Unsupported declaration input type')
        active.add(stream)
    else:raise ValueError('Missing declaration end')
    if not {0,3,5}.issubset(active):raise ValueError('Missing required declaration stream')
    return 6 in active

def assess(text,expected_source):
    if not re.fullmatch('[a-f0-9]{64}',expected_source):raise ValueError('Expected compiled source required')
    records=[];current=None;seen=set()
    for line in text.splitlines():
        if not line.startswith('Lab idle'):continue
        if not line.startswith('Lab idle ') or len(line.split())<3:
            raise ValueError('Truncated reserved idle prefix')
        kind=line.split()[2];f=fields(line)
        if kind=='draw':
            names={'rawGripValid','schema','draws','source','ipc','wire','request','input','owner','weapon','model','generation','hand','eye',
                   'stage','cfg','file','resource','contributors','matrices','historicalBytes','grasp'}
            if set(f)!=names or f['source']!=expected_source:raise ValueError('Idle build/schema mismatch')
            current={k:integer(v,-(1<<31),(1<<31)-1) if k=='resource' else
                     integer(v,0,(1<<64)-1 if k in ('request','input') else (1<<32)-1)
                     for k,v in f.items() if k!='source'}
            if current['schema']!=3 or current['rawGripValid'] not in (0,1) or current['draws']>8:raise ValueError('Geometry schema/budget mismatch')
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
        if current is None or current['stage']!=4:raise ValueError('Data without complete native-copy record')
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
            index=integer(f['index'],0,7)
            if index>=current['draws'] or index in current['geometry']:raise ValueError('Duplicate/outside geometry')
            g={k:integer(v,-(1<<31),(1<<31)-1) if k=='resource' else integer(v,0,(1<<32)-1)
               for k,v in f.items() if k not in ('request','eye','hand','index')}
            if not 2<=g['words']<=512 or not 1<=g['declaration']<=65 or not 1<=g['constants']<=256 or not g['surface'] or not g['instance'] or not g['cfg']:
                raise ValueError('Incomplete/unbounded rendered identity')
            g['data']={};current['geometry'][index]=g
        elif kind=='geometryData':
            if set(f)!={'request','eye','hand','index','kind','chunk','values'}:raise ValueError('Geometry data schema mismatch')
            index=integer(f['index'],0,7)
            if index not in current['geometry']:raise ValueError('Geometry payload before header')
            g=current['geometry'][index];which=f['kind'];chunk=integer(f['chunk'],0,511)
            counts={'declaration':6,'clip':16,'affine':12,'layout':14,'buffers':10,'draw':6,'streams':18,'hash':8,'constant':4}
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
            if which in ('affine','clip','constant'):
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
        if r['stage']!=4:rejected.append(r);continue
        if any(r[k]<=0 for k in ('request','input','owner','weapon','model','generation','cfg')):
            raise ValueError('Missing complete native-copy identity')
        wanted={k+':0' for k in ('world','nativePlacement','trackedPlacement','controller')}
        wanted.add('rawAim:0')
        if r['rawGripValid']:wanted.add('rawGrip:0')
        wanted.update('canonical:'+str(i) for i in range(r['matrices']))
        if not r['contributors'] or not r['matrices'] or set(r['pose'])!=wanted or r['stretch'] is None or \
           set(r['animations'])!=set(range(r['contributors'])):raise ValueError('Truncated idle copy emission')
        if set(r['geometry'])!=set(range(r['draws'])):raise ValueError('Missing geometry headers')
        draw_records=set()
        for g in r['geometry'].values():
            wanted={k+':0' for k in ('affine','clip','layout','buffers','draw','streams')}
            wanted.update('hash:'+str(i) for i in range(5))
            wanted.update('constant:'+str(i) for i in range(g['constants']))
            wanted.update('program:'+str(i) for i in range(0,g['words'],32))
            wanted.update('declaration:'+str(i) for i in range(g['declaration']))
            if set(g['data'])!=wanted:raise ValueError('Truncated consumed geometry emission')
            if g['drawRecord'] in draw_records:raise ValueError('Repeated native draw identity')
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
            weights=declaration_weights([g['data']['declaration:'+str(i)] for i in range(g['declaration'])])
            if bool(streams[8])!=weights:raise ValueError('Declaration/bound weight disagreement')
            ranges=[(layout[2],v*12,buffers[0]),(layout[5],t*6,buffers[5]),
                    (layout[8],v*4,buffers[0]),(layout[11],v*4,buffers[0]),(streams[13],v*8,buffers[0])]
            if any(off+size>capacity for off,size,capacity in ranges):raise ValueError('Geometry outside bound buffer')
        r['evidence_class']='event-pose-and-consumed-draws' if r['draws'] else 'event-pose-only'
        r['geometry_observed']=bool(r['draws'])
        completed.append(r)
    return {'schema':3,'source_fingerprint':expected_source,'native_execution_by_assessor':False,
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
