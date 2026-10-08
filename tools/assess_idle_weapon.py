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

def assess(text,expected_source):
    if not re.fullmatch('[a-f0-9]{64}',expected_source):raise ValueError('Expected compiled source required')
    records=[];current=None;seen=set()
    for line in text.splitlines():
        if not line.startswith('Lab idle'):continue
        if not line.startswith('Lab idle ') or len(line.split())<3:
            raise ValueError('Truncated reserved idle prefix')
        kind=line.split()[2];f=fields(line)
        if kind=='draw':
            names={'source','ipc','wire','request','input','owner','weapon','model','generation','hand','eye',
                   'stage','cfg','file','resource','contributors','matrices','historicalBytes','grasp'}
            if set(f)!=names or f['source']!=expected_source:raise ValueError('Idle build/schema mismatch')
            current={k:integer(v,-(1<<31),(1<<31)-1) if k=='resource' else
                     integer(v,0,(1<<64)-1 if k in ('request','input') else (1<<32)-1)
                     for k,v in f.items() if k!='source'}
            if current['ipc']!=10 or current['wire']!=7 or current['historicalBytes'] or current['grasp']:
                raise ValueError('Unsupported layout or provenance/grasp claim')
            if current['stage'] not in range(5) or not 0<=current['contributors']<=16 or not 0<=current['matrices']<=64:
                raise ValueError('Unbounded idle record')
            if current['hand'] not in (0,1) or current['eye'] not in (0,1):raise ValueError('Invalid hand/eye')
            key=(current['request'],current['eye'],current['hand'])
            if key in seen:raise ValueError('Ambiguous repeated native draw')
            seen.add(key)
            current.update({'animations':{},'pose':{},'stretch':None})
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
            if which not in ('world','nativePlacement','trackedPlacement','controller','canonical'):
                raise ValueError('Unknown matrix source')
            if not 0<=index<(current['matrices'] if which=='canonical' else 1):raise ValueError('Matrix outside bounded copy')
            key=which+':'+str(index)
            if key in current['pose']:raise ValueError('Duplicate pose matrix')
            current['pose'][key]=floats(f['values'],12)
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
        wanted.update('canonical:'+str(i) for i in range(r['matrices']))
        if not r['contributors'] or not r['matrices'] or set(r['pose'])!=wanted or r['stretch'] is None or \
           set(r['animations'])!=set(range(r['contributors'])):raise ValueError('Truncated idle copy emission')
        completed.append(r)
    return {'schema':1,'source_fingerprint':expected_source,'native_execution_by_assessor':False,
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
