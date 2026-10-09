#!/usr/bin/env python3
"""Match copied ID1 channels to a private candidate index; no game launch."""
import argparse
import json
from pathlib import Path
import re
import struct
from idle_stream_evidence import declaration_layout

ROOT=Path(__file__).resolve().parents[1]
CHANNELS=('positions','indices','weights','local_indices','uv')

def match_draws(observations,candidates,retained=False):
    rows=[]
    for observation in observations:
        for index,g in (observation['retained_copies']['geometry'] if retained else observation['geometry']).items():
            d=g['data'];layout=d['layout:0'];buffers=d['buffers:0'];streams=d['streams:0']
            input_layout,_=declaration_layout([d['declaration:'+str(i)] for i in range(g['declaration'])])
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
                for channel_index,c in enumerate(asset['candidate_channels']):
                    if c.get('single_body_influence') is not True:continue
                    if c['vertices']==layout[0] and c['triangles']==layout[1] and \
                       c['whole_vertex_buffer_bytes']==buffers[0] and c['whole_index_buffer_bytes']==buffers[5] and \
                       c['channel_ranges']==ranges and c['channel_sha256']==hashes:
                        found.append({'candidate':name,'asset_sha256':asset['asset_sha256'],
                                      'mesh_object':c['mesh_object'],'lod':c['lod'],'channel_index':channel_index})
            rows.append({'request':observation['request'],'eye':observation['eye'],'hand':observation['hand'],
                'geometry_index':int(index),'draw_record':g['drawRecord'],
                'result':('unique-position-and-auxiliary-channel-match' if input_layout==2 else
                          'unique-consumed-channel-match') if len(found)==1 else 'unmatched' if not found else 'ambiguous',
                'candidates':found})
            if input_layout==2:rows[-1].update(input_layout=2,auxiliary_channels=['uv'])
    if retained:
        for row in rows:row.update(evidence_class='historical-post-original-pre-cleanup-copies',cleanup_certified=False,
            outer_current=False,whole_trace_accepted=False,alignment_accepted=False)
    return rows

def match(evidence,candidates):
    if evidence.get('schema')!=3 or evidence.get('alignment_accepted') is not False:
        raise ValueError('Expected strict schema3 copied collector evidence')
    rows=match_draws(evidence['copied_event_pose_observations'],candidates)
    diagnostic=match_draws([o for o in evidence['rejected_or_missing_observations'] if 'retained_copies' in o],candidates,True)
    missing=[{'request':o['request'],'eye':o['eye'],'hand':o['hand'],'stage':o['stage']}
             for o in evidence['rejected_or_missing_observations']]
    missing.extend({'request':o['request'],'eye':o['eye'],'hand':o['hand'],'stage':o['stage']}
                   for o in evidence['copied_event_pose_observations'] if not o['geometry'])
    return {'schema':1,'source_fingerprint':evidence['source_fingerprint'],'matches':rows,'retained_diagnostic_matches':diagnostic,
        'observations_without_geometry':missing,'copied_geometry_coverage_complete':bool(rows) and not missing,
        'consumed_channels_all_uniquely_matched':bool(rows) and not missing and all(r['result']=='unique-consumed-channel-match' for r in rows),
        'copied_channels_all_uniquely_matched':bool(rows) and not missing and all(r['result'] in (
            'unique-consumed-channel-match','unique-position-and-auxiliary-channel-match') for r in rows),
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
