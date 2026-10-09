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
    p.add_argument('--kind',choices=('control','model-join'),default='control')
    p.add_argument('--expected-source',required=True);p.add_argument('--output',type=Path,required=True)
    a=p.parse_args()
    try:
        if a.log.stat().st_size>16*1024*1024:raise ValueError('Log exceeds private evidence budget')
        out=a.output.resolve()
        if out.is_relative_to(ROOT) or out.exists() or a.output.is_symlink():raise ValueError('Choose fresh private output outside source')
        result=(assess_join if a.kind=='model-join' else assess)(a.log.read_text(),a.expected_source)
        with out.open('x') as f:f.write(json.dumps(result,indent=2)+'\n')
        print(json.dumps({'observations':len(result['observations']),'physical_steering_verified':False}))
    except (ValueError,OSError,TypeError) as e:p.exit(1,str(e)+'\n')
