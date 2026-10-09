#!/usr/bin/env python3
"""Static submission-source ordering and x86 containment entries; no runtime."""
import argparse,hashlib,json,re,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]

def require(ok,message):
    if not ok:raise ValueError(message)

def region(text,start,end):
    body=text[text.index(start):text.index(end,text.index(start)+len(start))]
    return ''.join(re.sub(r'//[^\n]*|/\*.*?\*/','',body,flags=re.S).split())

def ordered(body,terms):
    require(all(body.count(t)==1 for t in terms),'Missing/duplicated submission lifetime marker')
    positions=[body.index(t) for t in terms]
    require(positions==sorted(positions),'Submission lifetime/forwarding order changed')

def source_checks(engine,gpu):
    wrapper=region(engine,'bool copyIdleSubmissionMetadata(','bool currentIdleRaster(')
    require('withNativeFinally(' in wrapper and 'currentIdleDraw(' in wrapper and
            'remote_render::copyIdleRaster(' in wrapper and 'trace->binding!=identity' in wrapper,
            'Submission wrapper lost owned containment')
    require('reject(' not in wrapper and 'copyBoundIdleRaster' not in wrapper and 'clipValid' not in wrapper,
            'Submission wrapper changes geometry policy')
    draw=region(gpu,'static HRESULT probeScopeDraw(','HRESULT scopeGpuDraw(')
    ordered(draw,['probe.submissionOwner=idleSubmissionOwner();',
                  'probe.submissionOwner->submissions.reserve(', 'd->AddRef();',
                  'copyIdleSubmissionMetadata(probe.submissionOwner,before)',
                  'result=forward(d,type,base,minimum,vertices,start,primitives);',
                  'copyIdleSubmissionMetadata(probe.submissionOwner,after)',
                  'finishIdleStreamProbe(d,result)'])
    cleanup=region(gpu,'static void cleanup(','void retireIdleSubmissionOwner(')
    ordered(cleanup,['release(probe.device);','probe.submissionOwner->submissions.finalize(',
                     'probe.submissionOwner=nullptr;','probe.busy=false;'])
    require('scopeGpuRoutingCurrent(device)' in cleanup,'Missing post-release numeric routing guard')
    outer=region(engine,'static void renderTrackedWeapon(','static void __fastcall weaponRender(')
    require(outer.count('retireIdleSubmissionOwner(')==2,'Missing normal/unwind outer retirement')
    ordered(outer,['retireIdleSubmissionOwner(&*idleStorage);','idleStorage->submissions.outerReturned=true;',
                   'emitIdleWeaponTrace(*idleStorage);','retireIdleSubmissionOwner(invocation.idle);'])
    return {'source_order_checked':True,'geometry_cap_changed':False,
            'limits':['Checks the bounded current source form, not general CFG dominance.']}

def bodies(path):
    assembly=subprocess.check_output(['objdump','-drC','-Mintel',str(path)],text=True)
    require('file format pe-i386' in assembly,'Expected x86 submission object')
    parts=re.split(r'^[0-9a-f]+ <(.+)>:\n',assembly,flags=re.M)
    return dict(zip(parts[1::2],parts[2::2]))

def verify(engine_obj,gpu_obj):
    source=source_checks((ROOT/'src/game/engine.cpp').read_text(),(ROOT/'src/game/scope_gpu.cpp').read_text())
    engine=bodies(engine_obj);gpu=bodies(gpu_obj)
    checked=[]
    for name in ('ss2vr::game::idleSubmissionOwner()',
                 'ss2vr::game::copyIdleSubmissionMetadata(ss2vr::IdleWeaponTrace const*, ss2vr::IdleSubmissionMetadata&)'):
        require(name in engine,'Missing compiled submission entry: '+name)
        require(engine[name].count('DISP32\tss2vrNativeFinally')==1,'Submission entry lost native finally')
        checked.append(name)
    require('ss2vr::game::retireIdleSubmissionOwner(ss2vr::IdleWeaponTrace*)' in gpu,'Missing compiled outer-borrow retirement')
    return {'runtime_executed':False,'source':source,'contained_entries':checked,
            'engine_object_sha256':hashlib.sha256(engine_obj.read_bytes()).hexdigest(),
            'gpu_object_sha256':hashlib.sha256(gpu_obj.read_bytes()).hexdigest(),
            'limits':['Compiled finally-callsite presence; not native execution, unwind recovery or GPU visibility.']}

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--engine',type=Path,required=True);p.add_argument('--gpu',type=Path,required=True)
    a=p.parse_args()
    try:print(json.dumps(verify(a.engine,a.gpu),indent=2))
    except (ValueError,OSError,subprocess.CalledProcessError) as e:p.exit(1,str(e)+'\n')
