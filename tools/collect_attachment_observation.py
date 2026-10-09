#!/usr/bin/env python3
"""Decode the private existing-call sniper receipt; no animation/aim acceptance."""
import argparse
import json
import math
from pathlib import Path
import re
import struct


def parse(text):
    headers=[];matrices=[]
    namespace='Lab sniper attachment'
    for line in text.splitlines():
        if namespace not in line:continue
        record=line.split(namespace,1)[1]
        if namespace in record:raise ValueError('Multiple attachment records on one line')
        if record.startswith(' schema=1 '):rows=headers;tail=record[len(' schema=1 '):]
        elif record.startswith(' matrix schema=1 '):rows=matrices;tail=record[len(' matrix schema=1 '):]
        else:raise ValueError('Unsupported or truncated attachment record')
        items=tail.split();pairs=[item.split('=',1) for item in items]
        if not pairs or any(len(p)!=2 for p in pairs) or len({p[0] for p in pairs})!=len(pairs):
            raise ValueError('Malformed/duplicate receipt fields')
        rows.append(dict(pairs))
    if len(headers)!=1 or len(matrices)!=2:raise ValueError('Need exactly one complete existing-call receipt')
    row=headers[0]
    integer=('input','generation','owner','weapon','hand','nativeId','model','instance','ident','calls','result',
             'calibrationSerial','drawRequest','drawInput','drawEye','cacheAge','cfg','file','resource')
    if set(row)!=set(integer)|{'nativeReturn'}:raise ValueError('Unexpected attachment receipt schema')
    result={key:int(row[key]) for key in integer}
    if any(result[k]<=0 for k in ('input','generation','owner','weapon','model','instance','calibrationSerial',
                                 'drawRequest','drawInput','cfg','file')) or result['resource']<0 or \
       result['nativeId']!=13 or result['hand']!=1 or result['calls']!=1 or result['result']!=1 or \
       result['drawEye'] not in (0,1) or not 0<=result['cacheAge']<=100 or not 0<=result['ident']<=0xffffffff:
        raise ValueError('Receipt does not describe an admitted neutral right-ID13 observation')
    pose=[float(v) for v in row['nativeReturn'].split(',')]
    if len(pose)!=7 or not all(math.isfinite(v) for v in pose):raise ValueError('Invalid copied original pose')
    result['nativeReturn']=pose;copied={}
    for matrix in matrices:
        if set(matrix)!={'calibrationSerial','kind','words'} or int(matrix['calibrationSerial'])!=result['calibrationSerial']:
            raise ValueError('Matrix receipt is not associated with this calibration publication')
        kind=matrix['kind']
        if kind not in ('nativeAttachment','calibrationNativeModel') or kind in copied:raise ValueError('Duplicate/unknown matrix')
        words=matrix['words'].split(',')
        if len(words)!=12 or not all(re.fullmatch('[0-9a-fA-F]{8}',word) for word in words):
            raise ValueError('Matrix must contain exactly 48 copied bytes')
        values=struct.unpack('<12f',struct.pack('<12I',*(int(word,16) for word in words)))
        if not all(math.isfinite(v) for v in values):raise ValueError('Nonfinite native matrix')
        copied[kind]={'words':words,'values':values}
    return {'schema':1,'receipt':result,'matrices':copied,'same_original_call_provenance':True,
            'draw_animation_equivalence_verified':False,'visual_muzzle_alignment_verified':False,
            'native_firing_verified':False,'bound_changed':False}

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--log',type=Path,required=True)
    p.add_argument('--private-root',type=Path,required=True);p.add_argument('--output',type=Path,required=True)
    a=p.parse_args();root=a.private_root.resolve(strict=True);source=a.log.resolve(strict=True)
    output=a.output.resolve()
    if not source.is_relative_to(root) or not output.is_relative_to(root):raise ValueError('Keep native observations private')
    report=parse(source.read_text(errors='strict'))
    with output.open('x') as f:json.dump(report,f,indent=2);f.write('\n')
    print('Existing-call receipt decoded. Draw/visual/firing acceptance remains unverified.')
