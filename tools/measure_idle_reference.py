#!/usr/bin/env python3
"""Measure an indexed hand-surface annotation in copied native pose; never apply alignment."""
import argparse
import hashlib
import json
import math
from pathlib import Path
import struct
ROOT=Path(__file__).resolve().parents[1]

def dot(a,b):return sum(x*y for x,y in zip(a,b))
def sub(a,b):return [x-y for x,y in zip(a,b)]
def cross(a,b):return [a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]]
def unit(v):
    n=math.sqrt(dot(v,v))
    if not math.isfinite(n) or n<=1e-10:raise ValueError('Degenerate annotation axis')
    return [x/n for x in v]
def finite_matrix(m):
    if not isinstance(m,(list,tuple)) or len(m)!=12 or any(type(v) not in (int,float) or not math.isfinite(v) for v in m):
        raise ValueError('Expected exactly twelve finite matrix components')
    return m

def transform(m,p):return [dot(m[i*4:i*4+3],p)+m[i*4+3] for i in range(3)]
def frame(points):
    p,i,t=points;u=sub(i,p);v=sub(t,p)
    x=unit(u);v=sub(v,[a*dot(x,v) for a in x]);y=unit(v);z=cross(x,y)
    if not all(math.isfinite(a) for q in (p,x,y,z) for a in q):raise ValueError('Nonfinite annotation frame')
    return [a for r in range(3) for a in (x[r],y[r],z[r],p[r])]

def measure(replay,annotation):
    if replay.get('schema')!=1 or replay.get('alignment_accepted') is not False or annotation.get('schema')!=1:
        raise ValueError('Unsupported replay/annotation')
    if annotation['reference_kind']!='indexed-surface-convention' or annotation['semantic_status'] not in ('candidate','reviewed-convention'):
        raise ValueError('Annotation must state its bounded semantic convention')
    if set(annotation['landmarks'])!={'P','I','T'}:raise ValueError('Three explicit annotation landmarks required')
    results=[]
    for row in replay['draws']:
        if row['position_replay']['position_replay_agrees_with_reference'] is not True:continue
        identity=row['candidates'][0]
        if identity['asset_sha256']!=annotation['asset_sha256'] or identity['mesh_object']!=annotation['mesh_object'] or \
           identity['lod']!=annotation['lod'] or identity['channel_index']!=annotation['channel_index']:continue
        g=row['render_geometry'];xyz=g['positions'];indices=g['triangle_indices']
        if g['channel_sha256']!=annotation['channel_sha256']:raise ValueError('Annotation channel identity differs')
        position_bytes=struct.pack('<'+str(len(xyz)*3)+'f',*[a for p in xyz for a in p])
        index_bytes=struct.pack('<'+str(len(indices))+'H',*indices)
        if hashlib.sha256(position_bytes).hexdigest()!=annotation['channel_sha256']['positions'] or \
           hashlib.sha256(index_bytes).hexdigest()!=annotation['channel_sha256']['indices']:
            raise ValueError('Replay geometry changed since channel matching')
        points=[]
        for name in ('P','I','T'):
            landmark=annotation['landmarks'][name];triangle=landmark['triangle'];weights=landmark['barycentric']
            if type(triangle) is not int or not 0<=triangle<len(indices)//3 or len(weights)!=3 or \
               any(type(v) not in (int,float) or not math.isfinite(v) or not 0<=v<=1 for v in weights) or abs(sum(weights)-1)>1e-8:
                raise ValueError('Invalid triangle/barycentric annotation')
            ids=indices[triangle*3:triangle*3+3]
            if ids!=landmark['ordered_vertex_indices'] or any(not 0<=i<len(xyz) for i in ids):
                raise ValueError('Annotation lost original triangle index order')
            points.append([sum(xyz[i][axis]*w for i,w in zip(ids,weights)) for axis in range(3)])
        native=frame(points);affine=finite_matrix(g['affine']);world_points=[transform(affine,p) for p in points]
        determinant=dot(affine[:3],cross(affine[4:7],affine[8:11]))
        if not math.isfinite(determinant) or abs(determinant)<=1e-12:raise ValueError('Singular captured affine')
        world=frame(world_points)
        raw_grip=g['raw_grip'];comparison=None
        if raw_grip is not None:
            raw_grip=finite_matrix(raw_grip)
            target=[raw_grip[3],raw_grip[7],raw_grip[11]]
            delta=sub(world_points[0],target)
            distance=math.sqrt(dot(delta,delta))
            if not all(math.isfinite(v) for v in delta) or not math.isfinite(distance):raise ValueError('Nonfinite grip comparison')
            comparison={'origin_delta_native_world':delta,'origin_distance_native_world':distance,
                        'controller_origin_equivalence':False}
        results.append({'binding':row['binding'],'geometry_index':row['geometry_index'],'draw_record':row['draw_record'],'reference_kind':annotation['reference_kind'],
            'semantic_status':annotation['semantic_status'],'native_landmarks':points,'world_landmarks':world_points,
            'native_reference_frame':native,'world_reference_frame':world,'captured_affine_reflected':determinant<0,
            'uncalibrated_grip_comparison':comparison,'controller_calibration_verified':False})
    return {'schema':1,'source_fingerprint':replay['source_fingerprint'],'measured_references':results,
            'positive_grasp_verified':False,'alignment_accepted':False,'native_mutation':False,
            'limits':['surface annotation convention; not pressure/contact or controller grip origin',
                      'right authored landmarks do not certify left-hand correspondence',
                      'idle geometry does not certify first-use/copy melee release']}

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--replay',type=Path,required=True)
    p.add_argument('--annotation',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    try:
        if a.replay.stat().st_size>64*1024*1024 or a.annotation.stat().st_size>65536:raise ValueError('Input budget exceeded')
        out=a.output.resolve()
        if out.is_relative_to(ROOT) or out.exists():raise ValueError('Choose a fresh private output outside source')
        result=measure(json.loads(a.replay.read_text()),json.loads(a.annotation.read_text()))
        out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(result,indent=2)+'\n')
        print(json.dumps({'measured':len(result['measured_references']),'alignment_accepted':False}))
    except (ValueError,KeyError,TypeError,OSError,struct.error) as e:p.exit(1,str(e)+'\n')
if __name__=='__main__':main()
