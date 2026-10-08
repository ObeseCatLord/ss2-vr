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

def request_key(value):
    return tuple(value.get(k) for k in ('sequence','session','reference','tracking_generation'))

def dual_identity(value):
    return tuple(value.get(k) for k in ('session','reference','tracking_generation'))+tuple(value.get('primary_generations',[]))

def dual_controls_match(value,expected):
    trigger=value.get('trigger',[])
    return len(trigger)==2 and all(math.isfinite(v) and abs(v-w)<.01 for v,w in zip(trigger,expected))

class DualPhaseEvidence:
    """Reject loss after convergence; never filter contradictions into a pass."""
    def __init__(self,boundary,expected):
        self.boundary=boundary;self.expected=expected;self.observations=[]

    def observe(self,value):
        matching=value['input_sequence']>self.boundary and dual_controls_match(value,self.expected)
        if self.observations and not matching:
            raise RuntimeError('Controller phase contradicted after convergence')
        if matching:
            if self.observations and value['input_sequence']<self.observations[-1]['input_sequence']:
                raise RuntimeError('Controller input sequence regressed')
            self.observations.append(value)

    def require_progress(self):
        if not self.observations or self.observations[-1]['input_sequence']<=self.observations[0]['input_sequence']:
            raise RuntimeError('Controller phase lacks advancing matching input')

    def confirmed_boundary(self):
        self.require_progress()
        # The interval parser uses minimum < sequence. Include the first
        # confirmed high sample, and exclude preceding convergence packets.
        return self.observations[0]['input_sequence']-1

    def require_stopped(self):
        self.require_progress()
        # UI published before confirmed neutral input cannot prove cessation.
        anchor=self.observations[0]['input_tick_ms']
        fresh=[v for v in self.observations if v['ui_tick_ms']>anchor]
        if len({v['ui_tick_ms'] for v in fresh})<3 or len({tuple(v['fire_sequence']) for v in fresh})!=1:
            raise RuntimeError('Final neutral lacks stable counters across post-neutral UI updates')

    def require_quiet_after(self,input_tick,seconds=1):
        """Charged release may fire; require a later measured quiet UI window."""
        self.require_progress()
        fresh={v['ui_tick_ms']:v for v in self.observations if v['ui_tick_ms']>input_tick}
        if len(fresh)<3:raise RuntimeError('Release lacks post-completion UI updates')
        ordered=[fresh[k] for k in sorted(fresh)];last=ordered[-1]
        anchors=[i for i,v in enumerate(ordered) if last['ui_tick_ms']-v['ui_tick_ms']>=seconds*1000]
        quiet=ordered[max(anchors):] if anchors else []
        if len(quiet)<3 or len({tuple(v['fire_sequence']) for v in quiet})!=1:
            raise RuntimeError('Release lacks a bounded advancing quiet UI interval')

def require_dual_capture(metadata,phase,identity,after):
    """Bind copied native eyes to held controls and the frozen fixture identity."""
    first=phase.observations[0]
    sequence=metadata['input_sequence']
    if (not first['input_sequence']<=sequence<=after['input_sequence'] or
        metadata['input_tick_ms']<first['input_tick_ms'] or
        dual_identity(metadata)!=identity or dual_identity(after)!=identity or
        metadata.get('hand_valid')!=[1,1] or metadata.get('primary_active_mask')!=3 or
        not dual_controls_match(metadata,phase.expected) or not dual_controls_match(after,phase.expected)):
        raise RuntimeError('Native eye capture does not belong to confirmed controller phase')

def correlated_records(game,host):
    camera=[]
    pattern=r'Lab native camera eye=(\d) request=(\d+) session=(\d+) reference=(\d+) tracking=(\d+) head=([-\d.eE+,]+) camera=([-\d.eE+,]+)'
    for eye,sequence,session,reference,tracking,head,placement in re.findall(pattern,game):
        h=list(map(float,head.split(',')));c=list(map(float,placement.split(',')))
        if len(h)!=7 or len(c)!=7 or not all(math.isfinite(v) for v in h+c):
            raise ValueError('Malformed native camera observation')
        camera.append({'eye':int(eye),'sequence':int(sequence),'session':int(session),
            'reference':int(reference),'tracking_generation':int(tracking),
            'head':{'p':h[:3],'q':h[3:]},'camera':{'p':c[:3],'q':c[3:]}})
    def receipts(text,prefix):
        return {tuple(map(int,v[:4])) for v in re.findall(
            prefix+r' request=(\d+) session=(\d+) reference=(\d+) tracking=(\d+) presentation=(\d+)',text) if int(v[4])==1}
    return camera,receipts(game,'Lab native pair'),receipts(host,'Lab projection submitted')

def complete_pairs_for_pose(game,host,head):
    camera,native,submitted=correlated_records(game,host)
    observed={}
    for entry in camera:
        if same_pose(entry['head'],head):observed.setdefault(request_key(entry),set()).add(entry['eye'])
    return {key for key in native & submitted if observed.get(key)=={0,1}}

def dual_topologies(game,identity,minimum,maximum):
    pattern=(r'Lab native primary topology input=(\d+) session=(\d+) reference=(\d+) tracking=(\d+) owner=(\d+) '
        r'weapons=(\d+),(\d+) receivers=(\d+),(\d+) hands=(\d+) combo=(-?\d+) dual=(-?\d+) flip=(-?\d+) '
        r'buttons=(-?\d+),(-?\d+) topology_compatible=(\d+)')
    matches=[]
    for row in re.findall(pattern,game):
        seq,session,reference,tracking,owner,left,right,lptr,rptr,hands,combo,dual,flip,lbutton,rbutton,compatible=map(int,row)
        if (minimum<seq<=maximum and (session,reference,tracking)==tuple(identity[:3]) and owner and
            hands==3 and left and right and left!=right and lptr and rptr and lptr!=rptr and combo and dual and
            {lbutton,rbutton}=={0,1} and compatible==1):
            matches.append({'input':seq,'owner':owner,'weapon':[left,right],'receiver':[lptr,rptr]})
    return matches

def dual_weapon_events(game,identity,minimum,maximum,topologies,event):
    pattern=(r'Lab native weapon event='+event+r' input=(\d+) session=(\d+) reference=(\d+) tracking=(\d+) owner=(\d+) '
        r'weaponHint=(\d+) receiver=(\d+) id=(\d+) hand=(\d+) stateBefore=(\d+) returned=(\d+) aborted=(\d+) result=(-?\d+)')
    pattern+=r' tick=(\d+) actions=(\d+),(\d+) trigger=([-\d.eE+]+),([-\d.eE+]+) completion=(\d+)'
    events=[]
    for row in re.findall(pattern,game):
        seq,session,reference,tracking,owner,hint,receiver,weapon_id,hand,state,returned,aborted,result,tick,lg,rg=map(int,row[:16])
        trigger=list(map(float,row[16:18]));completion=int(row[18])
        if (minimum<seq<=maximum and (session,reference,tracking,lg,rg)==tuple(identity) and weapon_id==1 and
            hand in (0,1) and returned==1 and not aborted and (result if event=='fire' else result==0) and
            completion>=tick and all(math.isfinite(v) and 0<=v<=1 for v in trigger) and
            any(t['owner']==owner and t['receiver'][hand]==receiver and t['weapon'][hand]==hint for t in topologies)):
            events.append({'input':seq,'tick':tick,'completion_tick':completion,'hand':hand,'trigger':trigger,'owner':owner,'weapon':hint,'receiver':receiver})
    return events

def successful_dual_fire(game,identity,minimum,maximum,topologies):
    return {v['hand'] for v in dual_weapon_events(game,identity,minimum,maximum,topologies,'fire')}

def first_person_depth_probe(game):
    """Assess the bounded audited root-partition probe, not arbitrary game draws.

    These initial request samples precede captures. State coherence cannot
    certify image occlusion; actual eye images still require visual inspection.
    """
    attempts={}
    for values in re.findall(r'Lab world draw attempts request=(\d+) eye=(-?\d+) aborted=(\d+) calls=(\d+) primitives=(\d+) readFailures=(\d+)',game):
        request,eye,aborted,calls,primitives,failures=map(int,values)
        key=(request,eye)
        if key in attempts:return {'observed':True,'coherent':False,'reason':'duplicate pass'}
        attempts[key]=(aborted,calls,primitives,failures)
    states={}
    pattern=r'Lab world draw state request=(\d+) eye=(-?\d+) z=(\d+) write=(\d+) alpha=(\d+) blend=(\d+) calls=(\d+) primitives=(\d+) fullRange=(\d+) worldRange=(\d+) otherRange=(\d+)'
    for values in re.findall(pattern,game):
        request,eye,z,write,alpha,blend,calls,primitives,full,world,other=map(int,values)
        states.setdefault((request,eye),[]).append((z,write,alpha,blend,calls,primitives,full,world,other))
    if not attempts or not states:return {'observed':False,'coherent':False,'reason':'no complete draw-range observations'}
    bad=[];complete=[];full_opaque=0
    for request in sorted({r for r,e in attempts}):
        valid=True
        for eye in (0,1,-1):
            key=(request,eye);entry=attempts.get(key);rows=states.get(key,[])
            if not entry or entry[0] or entry[3] or not rows or not entry[1]:valid=False;continue
            if (sum(v[4] for v in rows)!=entry[1] or sum(v[5] for v in rows)!=entry[2] or
                any(v[6]+v[7]+v[8]!=v[4] for v in rows) or
                len({v[:4] for v in rows})!=len(rows)):valid=False;continue
            opaque=[v for v in rows if v[0]==1 and v[1]==1 and v[3]==0]
            if not opaque:valid=False;continue
            wrong=sum(v[6] for v in opaque);full_opaque+=wrong
            if wrong:bad.append({'request':request,'eye':eye,'full_range_opaque_calls':wrong})
        if valid:complete.append(request)
    return {'observed':True,'coherent':bool(complete) and len(complete)==len({r for r,e in attempts}) and not bad,
        'complete_initial_requests':complete,'full_range_opaque_calls':full_opaque,'wrong_ranges':bad,
        'image_occlusion_acceptance':'required_visual_review'}

def assess(run):
    result=json.loads((run/'result.json').read_text())
    host=(run/'ss2vr_host.log').read_text(errors='replace') if (run/'ss2vr_host.log').exists() else ''
    game=(run/'SS2VR.log').read_text(errors='replace') if (run/'SS2VR.log').exists() else ''
    counts=[int(v) for v in re.findall(r'xr_world=(\d+)',host)]
    camera,native,submitted=correlated_records(game,host)
    report={'source_head':result.get('source_head'),'simulation':result.get('simulation'),
        'hardware_acceptance':False,'successful_projection_frames':max(counts,default=0),
        'poses':{},'visual_parallax_acceptance':'required_unreviewed','full_vr_acceptance':False,
        'run_result':result.get('result'),'run_error':result.get('error')}
    if result.get('validated_config',{}).get('depth_range_probe')=='native-first-person-root-partition':
        report['first_person_depth_probe']=first_person_depth_probe(game)
    base_path=run/'baseline-left.ppm';base=read_ppm(base_path) if base_path.exists() else None
    for path in sorted(run.glob('*-metadata.json')):
        name=path.name.removesuffix('-metadata.json');meta=json.loads(path.read_text())
        left=read_ppm(run/(name+'-left.ppm'));right=read_ppm(run/(name+'-right.ppm'))
        key=request_key(meta)
        traces=[v for v in camera if request_key(v)==key and same_pose(v['head'],meta['head'])]
        eyes={v['eye'] for v in traces}
        report['poses'][name]={'native_request':meta['sequence'],'head':meta['head'],
            'eye_separation_metres':math.dist(meta['eyes'][0]['p'],meta['eyes'][1]['p']),
            'different_eye_rgb_bytes':changed(left,right),
            'different_from_baseline_rgb_bytes':changed(base,left) if base else None,
            'actual_native_camera_observed_both_eyes':eyes=={0,1},'camera_observations':traces,
            'same_request_native_ui_complete':meta.get('presentation')==1 and meta.get('ui_requested')==1 and key in native,
            'same_request_projection_submitted':key in submitted,
            'fov':meta['fov']}
    required={'baseline','translate-x','translate-y','translate-z','yaw','pitch','roll'}
    report['complete_pose_capture_set']=required.issubset(report['poses'])
    baseline=report['poses'].get('baseline')
    report['distinct_neutral_complete_pairs']=len(complete_pairs_for_pose(game,host,baseline['head'])) if baseline else 0
    report['world_evidence_candidate']=(report['distinct_neutral_complete_pairs']>=30 and
        report['complete_pose_capture_set'] and all(v['different_eye_rgb_bytes']>0 and
            v['actual_native_camera_observed_both_eyes'] and v['same_request_native_ui_complete'] and
            v['same_request_projection_submitted'] for v in report['poses'].values()) and
        result.get('loaded_scene_confirmed') and result.get('shutdown_observed') and not result.get('cleanup_errors'))
    # Distinct/different pixels can arise from animation or UI. Human/native-geometry
    # inspection of near/far parallax and spatial stability is still required.
    return report

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('run',type=Path);a=p.parse_args()
    print(json.dumps(assess(a.run.resolve(strict=True)),indent=2))
