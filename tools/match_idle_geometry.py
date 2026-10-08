#!/usr/bin/env python3
"""Match copied consumed ID1 channels to a private candidate index; no game launch."""
import argparse
import json
from pathlib import Path
import re
import struct

ROOT=Path(__file__).resolve().parents[1]
CHANNELS=('positions','indices','weights','local_indices','uv')

def match(evidence,candidates):
    if evidence.get('schema')!=2 or evidence.get('alignment_accepted') is not False:
        raise ValueError('Expected strict schema2 copied collector evidence')
    rows=[]
    for observation in evidence['copied_event_pose_observations']:
        for index,g in observation['geometry'].items():
            d=g['data'];layout=d['layout:0'];buffers=d['buffers:0'];streams=d['streams:0']
            hashes={name:struct.pack('<8I',*d['hash:'+str(i)]).hex() for i,name in enumerate(CHANNELS)}
            ranges={name:{'offset':off,'size':size,'format':fmt,'buffer':buf} for name,off,size,fmt,buf in [
                ('positions',layout[2],layout[0]*12,layout[3],layout[4]),
                ('indices',layout[5],layout[1]*6,layout[6],layout[7]),
                ('weights',layout[8],layout[0]*4,layout[9],layout[10]),
                ('local_indices',layout[11],layout[0]*4,layout[12],layout[13]),
                ('uv',streams[13],layout[0]*8,132,layout[4])]}
            found=[]
            for name,asset in candidates.items():
                if not re.fullmatch('[a-f0-9]{64}',asset['asset_sha256']):raise ValueError('Missing candidate asset fingerprint')
                for c in asset['candidate_channels']:
                    if c.get('single_body_influence') is not True:continue
                    if c['vertices']==layout[0] and c['triangles']==layout[1] and \
                       c['whole_vertex_buffer_bytes']==buffers[0] and c['whole_index_buffer_bytes']==buffers[5] and \
                       c['channel_ranges']==ranges and c['channel_sha256']==hashes:
                        found.append({'candidate':name,'asset_sha256':asset['asset_sha256'],
                                      'mesh_object':c['mesh_object'],'lod':c['lod']})
            rows.append({'request':observation['request'],'eye':observation['eye'],'hand':observation['hand'],
                'geometry_index':int(index),'draw_record':g['drawRecord'],
                'result':'unique-consumed-channel-match' if len(found)==1 else 'unmatched' if not found else 'ambiguous',
                'candidates':found})
    return {'schema':1,'source_fingerprint':evidence['source_fingerprint'],'matches':rows,
        'consumed_channels_all_uniquely_matched':bool(rows) and all(r['result']=='unique-consumed-channel-match' for r in rows),
        'historical_loaded_bytes_verified':False,'shader_replay_verified':False,
        'positive_grasp_verified':False,'alignment_accepted':False}

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--evidence',type=Path,required=True);p.add_argument('--candidates',type=Path,required=True)
    p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    try:
        if any(f.stat().st_size>16*1024*1024 for f in (a.evidence,a.candidates)):raise ValueError('Input budget exceeded')
        out=a.output.resolve()
        if out.is_relative_to(ROOT) or out.exists():raise ValueError('Choose fresh private output outside source')
        result=match(json.loads(a.evidence.read_text()),json.loads(a.candidates.read_text()))
        out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(result,indent=2)+'\n')
        print(json.dumps({'draws':len(result['matches']),'alignment_accepted':False}))
    except (ValueError,KeyError,OSError,TypeError,struct.error) as e:p.exit(1,str(e)+'\n')
if __name__=='__main__':main()
