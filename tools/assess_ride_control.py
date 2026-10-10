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
    copies={};active=None
    def unsigned(raw,hexadecimal=False):
        if not re.fullmatch('[0-9a-f]+' if hexadecimal else '0|[1-9][0-9]*',raw):raise ValueError('Invalid ride-render integer')
        v=int(raw,16 if hexadecimal else 10)
        if v>0xffffffff:raise ValueError('Ride-render field exceeds native width')
        return v
    for line in text.splitlines():
        if not re.match(r'^Lab\s+ride\s+render',line):continue
        if not line.startswith('Lab ride render '):raise ValueError('Malformed reserved ride-render prefix')
        parts=line.split();kind='identity'
        if len(parts)>3 and parts[3] in ('binding','matrix'):kind=parts.pop(3)
        pairs=[p.split('=') for p in parts[3:]]
        if any(len(p)!=2 for p in pairs):raise ValueError('Malformed ride-render record')
        f=dict(pairs)
        if len(f)!=len(pairs) or not base.issubset(f):raise ValueError('Duplicate/missing ride-render fields')
        schema=unsigned(f['schema']);row=unsigned(f['row']);bank=unsigned(f['bank'])
        if f['eye'] not in ('-1','0','1') or schema not in (1,2) or row>=32 or not bank:
            raise ValueError('Unsupported ride-render owner/budget')
        eye=int(f['eye']);key=(row,bank,eye)
        wanted=base|(identity if kind=='identity' else binding if kind=='binding' else {'kind','words'})
        if kind=='identity' and schema==2:wanted|={'source','seatBone','seatDefinition'}
        if set(f)!=wanted:raise ValueError('Ride-render schema mismatch')
        if kind=='identity':
            if key in copies or (active is not None and key<=active):raise ValueError('Duplicate/out-of-order ride-render owner')
            if schema==2 and f['source']!=expected_source:raise ValueError('Ride-render source mismatch')
            r={k:unsigned(f[k],k=='class') for k in wanted-{'source','eye'}};r['eye']=eye
            if any(not r[k] for k in ('player','brain','ride','renderableHandle','renderable','instance','cfg','modelRecord','evaluated','matrices','definition')) or \
                    r['class'] not in (0x2a8558,0x2b8420) or \
                    any(r[k] for k in ('resourceClaim','seatClaim','graspClaim','steeringClaim')):
                raise ValueError('Ride-render identity/claim mismatch')
            if schema==2 and (not r['seatDefinition'] or r['definition']==r['seatDefinition'] or r['mainBone']==r['seatBone']):
                raise ValueError('Ride-render Main/Seat definitions are not distinct')
            r.update(source_fingerprint=f.get('source'),source_matches_expected=schema==2,binding=None,raw_matrices={})
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
                for name in ('main','seat') if schema==2 else ('main',):
                    bone=r[name+'Bone'];definition=r['definition' if name=='main' else 'seatDefinition']
                    if not b['boneFirst']<=bone<b['boneFirst']+b['boneCount'] or definition<b['definitions'] or \
                            (definition-b['definitions'])%0x78 or (definition-b['definitions'])//0x78>=b['definitionCount']:
                        raise ValueError('Ride-render bone does not belong to selected model/LOD')
                r['binding']=b
            else:
                if r['binding'] is None:raise ValueError('Ride-render matrices without binding')
                channel=f['kind'];inventory={'modelWorld','MainCanonical'}|({'SeatCanonical'} if schema==2 else set())
                v=f['words'].split(',')
                if channel not in inventory or channel in r['raw_matrices'] or len(v)!=12 or \
                        any(not re.fullmatch('[0-9a-f]{8}',x) for x in v):raise ValueError('Invalid ride-render raw matrix inventory')
                r['raw_matrices'][channel]=[int(x,16) for x in v]
    groups={}
    for key,r in copies.items():
        wanted={'modelWorld','MainCanonical'}|({'SeatCanonical'} if r['schema']==2 else set())
        if r['binding'] is None or set(r['raw_matrices'])!=wanted:raise ValueError('Truncated ride-render copy')
        group=groups.setdefault(key[0],[]);group.append(r)
        r.update(seat_canonical_copied=r['schema']==2,operated_seat_attachment_verified=False,
                 source_provenance_authenticated=False,simulation_time_freshness_verified=False,physical_steering_verified=False)
    shared={'schema','bank','player','brain','ride','seat','class','renderableHandle','renderable','instance','cfg','file','resource','source_fingerprint'}
    for group in groups.values():
        if {r['eye'] for r in group} not in ({-1},{0,1}) or len(group) not in (1,2):raise ValueError('Incomplete/mixed ride-render eye group')
        if any(any(r[k]!=group[0][k] for k in shared) for r in group[1:]):raise ValueError('Crossed ride-render stereo owners')
    rows=list(copies.values());observed=expected_source if rows and all(r['schema']==2 for r in rows) else None
    return {'schema':1,'observed_source_fingerprint':observed,'source_matches_expected':observed is not None,
            'observations':rows,'evidence_scope':'emitter-reported-completed-native-render-copies',
            'seat_frame_copies_present':any(r['seat_canonical_copied'] for r in rows),
            'source_provenance_authenticated':False,'operated_seat_authority_verified':False,
            'installed_resource_association_verified':False,'evaluated_control_frame_verified':False,
            'simulation_time_freshness_verified':False,'physical_steering_verified':False,'runtime_executed_by_assessor':False}

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
        if a.log.stat().st_size>16*1024*1024:raise ValueError('Log exceeds private evidence budget')
        out=a.output.resolve()
        if out.is_relative_to(ROOT) or out.exists() or a.output.is_symlink():raise ValueError('Choose fresh private output outside source')
        result={'control':assess,'model-join':assess_join,'render':assess_render}[a.kind](a.log.read_text(),a.expected_source)
        with out.open('x') as f:f.write(json.dumps(result,indent=2)+'\n')
        print(json.dumps({'observations':len(result['observations']),'physical_steering_verified':False}))
    except (ValueError,OSError,TypeError) as e:p.exit(1,str(e)+'\n')
