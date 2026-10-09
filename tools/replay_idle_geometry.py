#!/usr/bin/env python3
"""Replay private matched copied ID1 input offline, without launching any game."""
import argparse
import hashlib
import json
import math
from pathlib import Path
import struct
import subprocess
import tempfile
from assess_idle_weapon import assess, declaration_layout
from match_idle_geometry import match,CHANNELS

ROOT=Path(__file__).resolve().parents[1]

def private_path(root,relative):
    path=(root/relative).resolve()
    if not path.is_relative_to(root.resolve()) or path.is_relative_to(ROOT):
        raise ValueError('Candidate files must remain inside the private candidate directory')
    return path

def channel_bytes(root,asset,row):
    asset_path=private_path(root,asset)
    if asset_path.stat().st_size>16*1024*1024:raise ValueError('Private candidate asset byte budget exceeded')
    raw=asset_path.read_bytes()
    if hashlib.sha256(raw).hexdigest()!=row['asset_sha256']:raise ValueError('Candidate asset bytes changed')
    channels={}
    for name in CHANNELS:
        r=row['channel_ranges'][name];size=r['size']
        if not 0<size<=1490*12:raise ValueError('Candidate channel byte budget exceeded')
        path=private_path(root,row['channel_files'][name])
        if path.stat().st_size!=size:raise ValueError('Private channel length mismatch')
        data=path.read_bytes()
        if hashlib.sha256(data).hexdigest()!=row['channel_sha256'][name]:raise ValueError('Private channel content mismatch')
        channels[name]=data
    return channels

def evaluate_geometry(g,channels,evaluator,temporary):
    d=g['data'];vertices=d['layout:0'][0]
    program=[w for i in range(0,g['words'],32) for w in d['program:'+str(i)]]
    constants=[w for i in range(g['constants']) for w in d['constant:'+str(i)]]
    data=bytearray(b'SS2VIRP1')
    layout,weights=declaration_layout([d['declaration:'+str(i)] for i in range(g['declaration'])])
    if weights!=bool(d['streams:0'][8]):raise ValueError('Replay input family/binding mismatch')
    data+=struct.pack('<5I',2 if layout else 1,len(program),g['constants'],vertices,int(weights))
    if layout:data+=struct.pack('<I',layout)
    data+=struct.pack('<16I',*d['clip:0'])+struct.pack('<'+str(len(program))+'I',*program)
    data+=struct.pack('<'+str(len(constants))+'I',*constants)
    for i in range(vertices):
        data+=channels['positions'][i*12:i*12+12]+channels['uv'][i*8:i*8+8]
        data+=channels['weights'][i*4:i*4+4]+channels['local_indices'][i*4:i*4+4]
    if len(data)>65536:raise ValueError('Evaluator input budget exceeded')
    path=temporary/'copied-input.bin';path.write_bytes(data)
    p=subprocess.run([str(evaluator),str(path)],capture_output=True,text=True,timeout=10)
    if p.returncode or len(p.stdout)>2048:raise ValueError('Offline evaluator rejected input: '+p.stderr.strip()[:160])
    result=json.loads(p.stdout)
    if result.get('schema')!=1 or result.get('gpu_execution') is not False or result.get('alignment_accepted') is not False:
        raise ValueError('Unexpected offline evaluator output')
    return result

def replay(evidence,candidates,candidate_root,evaluator,temporary):
    matching=match(evidence,candidates)
    observations={(r['request'],r['eye'],r['hand']):r for r in evidence['copied_event_pose_observations']}
    results=[]
    for row in matching['matches']:
        result=dict(row)
        if row['result']!='unique-consumed-channel-match':
            result['position_replay']={'position_replay_agrees_with_reference':False,'reason':row['result']}
            results.append(result);continue
        identity=row['candidates'][0];asset=candidates[identity['candidate']]
        c=dict(asset['candidate_channels'][identity['channel_index']]);c['asset_sha256']=asset['asset_sha256']
        channels=channel_bytes(candidate_root,identity['candidate'],c)
        o=observations[(row['request'],row['eye'],row['hand'])]
        g=o['geometry'].get(str(row['geometry_index']),o['geometry'].get(row['geometry_index']))
        result['binding']={k:o[k] for k in ('request','input','owner','weapon','model','generation','eye','hand')}
        result['position_replay']=evaluate_geometry(g,channels,evaluator,temporary)
        if result['position_replay']['position_replay_agrees_with_reference']:
            affine=struct.unpack('<12f',struct.pack('<12I',*g['data']['affine:0']))
            xyz=list(struct.iter_unpack('<3f',channels['positions']))
            indices=[t[0] for t in struct.iter_unpack('<H',channels['indices'])]
            if any(i>=len(xyz) for i in indices):raise ValueError('Candidate index exceeds actual matched vertex channel')
            world=[[sum(float(affine[r*4+j])*p[j] for j in range(3))+affine[r*4+3] for r in range(3)] for p in xyz]
            if any(not math.isfinite(v) for p in world for v in p):raise ValueError('Nonfinite replay geometry')
            result['render_geometry']={'positions':xyz,'world_positions':world,'triangle_indices':indices,
                'affine':affine,'controller':o['pose']['controller:0'],'raw_aim':o['pose']['rawAim:0'],
                'raw_grip':o['pose'].get('rawGrip:0'),'channel_sha256':c['channel_sha256'],'surface_name':g['surfaceName'],'bone_name':g['boneName'],
                'render_instance':g['instance'],'render_cfg':g['cfg'],'render_resource':g['resource']}
        results.append(result)
    return {'schema':1,'source_fingerprint':evidence['source_fingerprint'],'draws':results,
        'observations_without_geometry':matching['observations_without_geometry'],
        'copied_geometry_coverage_complete':matching['copied_geometry_coverage_complete'],
        'all_consumed_positions_agree_with_native_reference':matching['copied_geometry_coverage_complete'] and all(
            r['position_replay']['position_replay_agrees_with_reference'] for r in results),
        'gpu_execution':False,'native_execution':False,'historical_loaded_bytes_verified':False,
        'positive_grasp_verified':False,'alignment_accepted':False}

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--log',type=Path,required=True);p.add_argument('--expected-source',required=True)
    p.add_argument('--candidates',type=Path,required=True);p.add_argument('--candidate-root',type=Path,required=True)
    p.add_argument('--evaluator',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    try:
        if any(f.stat().st_size>16*1024*1024 for f in (a.log,a.candidates)):raise ValueError('Input budget exceeded')
        out=a.output.resolve()
        if out.is_relative_to(ROOT) or out.exists():raise ValueError('Choose fresh private output outside source')
        out.parent.mkdir(parents=True,exist_ok=True)
        evidence=assess(a.log.read_text(),a.expected_source);candidates=json.loads(a.candidates.read_text())
        with tempfile.TemporaryDirectory(prefix='idle-offline-',dir=out.parent) as tmp:
            result=replay(evidence,candidates,a.candidate_root.resolve(),a.evaluator.resolve(),Path(tmp))
        out.write_text(json.dumps(result,indent=2)+'\n')
        print(json.dumps({'draws':len(result['draws']),'all_positions_agree':result['all_consumed_positions_agree_with_native_reference'],
                          'alignment_accepted':False}))
    except (ValueError,KeyError,TypeError,OSError,struct.error,subprocess.SubprocessError) as e:p.exit(1,str(e)+'\n')
if __name__=='__main__':main()
