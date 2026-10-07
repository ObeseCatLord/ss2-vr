#!/usr/bin/env python3
"""Assess private native-gameplay evidence without calling counters a VR pass."""
import argparse
import json
import math
from pathlib import Path
import re

def read_ppm(path):
    with path.open('rb') as f:
        if f.readline()!=b'P6\n':raise ValueError('Expected observer P6 image')
        width,height=map(int,f.readline().split())
        if f.readline()!=b'255\n':raise ValueError('Expected byte RGB')
        pixels=f.read()
    if len(pixels)!=width*height*3:raise ValueError('Incomplete native image')
    return width,height,pixels

def changed(a,b):
    if a[:2]!=b[:2]:raise ValueError('Native capture dimensions changed')
    return sum(x!=y for x,y in zip(a[2],b[2]))

def same_pose(a,b):
    return max(abs(x-y) for x,y in zip(a['p'],b['p']))<.001 and abs(sum(x*y for x,y in zip(a['q'],b['q'])))>.99999

def assess(run):
    result=json.loads((run/'result.json').read_text())
    host=(run/'ss2vr_host.log').read_text(errors='replace') if (run/'ss2vr_host.log').exists() else ''
    game=(run/'SS2VR.log').read_text(errors='replace') if (run/'SS2VR.log').exists() else ''
    counts=[int(v) for v in re.findall(r'xr_world=(\d+)',host)]
    camera=[]
    pattern=r'Lab native camera eye=(\d) request=(\d+) head=([-\d.eE+,]+) camera=([-\d.eE+,]+)'
    for eye,sequence,head,placement in re.findall(pattern,game):
        h=list(map(float,head.split(',')));c=list(map(float,placement.split(',')))
        if len(h)!=7 or len(c)!=7:raise ValueError('Malformed native camera observation')
        camera.append({'eye':int(eye),'sequence':int(sequence),'head':{'p':h[:3],'q':h[3:]},'camera':{'p':c[:3],'q':c[3:]}})
    report={'source_head':result.get('source_head'),'simulation':result.get('simulation'),
        'hardware_acceptance':False,'successful_projection_frames':max(counts,default=0),
        'poses':{},'visual_parallax_acceptance':'required_unreviewed','full_vr_acceptance':False,
        'run_result':result.get('result'),'run_error':result.get('error')}
    base_path=run/'baseline-left.ppm';base=read_ppm(base_path) if base_path.exists() else None
    for path in sorted(run.glob('*-metadata.json')):
        name=path.name.removesuffix('-metadata.json');meta=json.loads(path.read_text())
        left=read_ppm(run/(name+'-left.ppm'));right=read_ppm(run/(name+'-right.ppm'))
        traces=[v for v in camera if same_pose(v['head'],meta['head'])]
        eyes={v['eye'] for v in traces}
        report['poses'][name]={'native_request':meta['sequence'],'head':meta['head'],
            'eye_separation_metres':math.dist(meta['eyes'][0]['p'],meta['eyes'][1]['p']),
            'different_eye_rgb_bytes':changed(left,right),
            'different_from_baseline_rgb_bytes':changed(base,left) if base else None,
            'actual_native_camera_observed_both_eyes':eyes=={0,1},'camera_observations':traces,
            'fov':meta['fov']}
    required={'baseline','translate-x','translate-y','translate-z','yaw','pitch','roll'}
    report['complete_pose_capture_set']=required.issubset(report['poses'])
    report['world_evidence_candidate']=(report['successful_projection_frames']>0 and
        report['complete_pose_capture_set'] and all(v['different_eye_rgb_bytes']>0 and
            v['actual_native_camera_observed_both_eyes'] for v in report['poses'].values()))
    # Distinct/different pixels can arise from animation or UI. Human/native-geometry
    # inspection of near/far parallax and spatial stability is still required.
    return report

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('run',type=Path);a=p.parse_args()
    print(json.dumps(assess(a.run.resolve(strict=True)),indent=2))
