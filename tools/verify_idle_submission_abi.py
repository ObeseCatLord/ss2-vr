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
    wrapper=region(engine,'bool copyIdleSubmissionMetadata(','bool copyIdleSubmissionPalette(')
    require('withNativeFinally(' in wrapper and 'currentIdleDraw(' in wrapper and
            'remote_render::copyIdleRaster(' in wrapper and 'trace->binding!=identity' in wrapper,
            'Submission wrapper lost owned containment')
    require('reject(' not in wrapper and 'copyBoundIdleRaster' not in wrapper and 'clipValid' not in wrapper,
            'Submission wrapper changes geometry policy')
    palette=region(engine,'bool copyIdleSubmissionPalette(','bool currentIdleRaster(')
    require('withNativeFinally(' in palette and 'trace->nativeId!=2' in palette and
            palette.count('remote_render::copyIdlePalette(')==2 and 'before!=after' in palette and
            'copyBoundIdleRaster' not in palette and 'reject(' not in palette,
            'Palette wrapper lost its independent ID2-only native bookend')
    require(palette.count('remote_render::idlePaletteProjectionSequence(')==2 and
            'projection&&projection==finalProjection' in palette and
            'out.projectionSequence=projection;' in palette,
            'Palette receipt lost fresh raw-global bookends')
    require('diagnostic->wrapper!=99' in palette and 'diagnostic->wrapper=99;' in palette,
            'Palette unwind must remain unknown rather than a returned guard failure')
    require('if(!trace->config.configuration&&(trace->config.file||trace->config.resource!=-1))' in palette and
            'if(trace->config.configuration&&before.metadata.rootConfig!=trace->config)' in palette and
            'trace->config=' not in palette,
            'Diagnostic Event absence must be exact, retain present identity and never seed history')
    boundary=region(gpu,'static bool armPaletteBoundary(','static bool serializePaletteGeometry(')
    require('if(!paletteApiOwnerCurrent(d,owner,slot,api))returnfalse;' in boundary and
            'if(!receipt.kind){receipt={};receipt.phase=phase;}' in boundary and
            'if(!paletteApiOwnerCurrent(d,owner,slot,api)||copy.wrapper==99)return;' in boundary and
            'if(receipt.kind)return;' in boundary and
            'receipt=copy;receipt.phase=phase;receipt.kind=1;' in boundary,
            'Boundary receipt lost current-owner, unwind or first-failure protection')
    copy_boundary=region(gpu,'static bool copyPaletteBoundary(','static void recordPaletteApiFailure(')
    ordered(copy_boundary,['IdlePaletteBoundaryCopycopy;',
                      'constboolcopied=copyIdleSubmissionPalette(owner,out,&copy);',
                      'if(!copied)recordPaletteBoundaryFailure(d,owner,slot,api,phase,copy);'])
    draw=region(gpu,'static HRESULT probeScopeDraw(','HRESULT scopeGpuDraw(')
    ordered(draw,['probe.submissionOwner=idleSubmissionOwner();',
                  'probe.submissionOwner->submissions.reserve(', 'probe.submissionOwner->reservePaletteApi(', 'd->AddRef();',
                  'beginPaletteApi(d,owner,slot,api,idleAdmitted,paletteContentEligible)', 'copyIdleSubmissionMetadata(owner,before)',
                  'result=forward(d,type,base,minimum,vertices,start,primitives);',
                  'finishPaletteApi(d,owner,slot,api,paletteAfter)', 'copyIdleSubmissionMetadata(owner,after)',
                  'finishIdleStreamProbe(d,result)'])
    pre=region(gpu,'static bool beginPaletteApi(','static bool finishPaletteApi(')
    ordered(pre,['copyPaletteBoundary(d,owner,slot,api,2,before)','releaseBindings(probe.bindings[1]);',
                 'sampleIdleApi(d,probe.bindings[1],p.before,p.program,p.words,{},true,true)',
                 'copyPaletteBoundary(d,owner,slot,api,4,bookend)','before.projectionBookend(bookend);','owner->submissions.paletteBefore(slot,before,true);'])
    require('if(geometryAdmitted){copied=serializePaletteGeometry(p);' in pre and
            pre.index('nativeUiDeviceCurrent(d)')<pre.index('copyPaletteBoundary(d,owner,slot,api,2,before)'),
            'Companion disturbed certified geometry or queried device outside native/API bracket')
    guard=region(gpu,'static bool paletteApiOwnerCurrent(','static bool armPaletteBoundary(')
    calls=re.findall(r'([A-Za-z_][A-Za-z_0-9]*)\(',guard.split('{',1)[1])
    require(set(calls)<= {'paletteApiOwnerCurrent','if','scopeGpuRoutingCurrent','graphicsResourceGeneration','pending'},
            'Palette scalar owner guard gained an unreviewed callback')
    serialize=region(gpu,'static bool serializePaletteGeometry(','static bool collectPaletteContent(')
    require('releaseBindings(' not in serialize and 'sampleIdleApi(' not in serialize and '->' not in serialize,
            'Certified geometry serialization must retain references without foreign callbacks')
    # Narrow lexical barriers supplement portable lock-ledger checks, not CFG proof.
    require('constboolpaletteContentEligible=!admitted&&!probe.raster.pose.valid&&!idleCandidate;' in draw,
            'Content retry must exclude prior scope/legacy attempts')
    require('if(contentEligible&&before.count>1&&!probe.algorithm&&!probe.hash)' in pre and
            'collectPaletteContent(d,owner,slot,api,before);if(!scopeGpuForwardingAllowed()||' in pre,
            'Content attempt lost eligibility or inner lock barrier')
    require('beginPaletteApi(d,owner,slot,api,idleAdmitted,paletteContentEligible);if(!scopeGpuForwardingAllowed())return;' in draw,
            'Content sampling needs lock barrier before fallback/native forwarding')
    content=region(gpu,'static bool collectPaletteContent(IDirect3DDevice9 *d,','static bool sameInputs(')
    ordered(content,['releaseBindings(probe.bindings[0]);','boundInputs(d,b,draw,GeometryBufferPolicy::Idle,nullptr,2,&native.metadata.layout)',
                     'copySlice(i==1,ranges.slices[i],storage[i].first(ranges.slices[i].size))',
                     '!scopeGpuForwardingAllowed()||probe.algorithm||probe.hash||!hashIdleSlices(ranges)',
                     'p.content=b.values;p.contentHashes=probe.idle.hashes;'])
    require('paletteContentMatchesApi(probe.bindings[0],p.before)' in pre,
            'Content copy lacks explicit canonical API correspondence')
    post=region(gpu,'static bool finishPaletteApi(','static bool idleStreamOwnerCurrent(')
    ordered(post,['copyPaletteBoundary(d,owner,slot,api,5,after)',
                  'sampleIdleApi(d,probe.bindings[2],snapshot,{},unusedWords,{},true,false)',
                  'copyPaletteBoundary(d,owner,slot,api,7,bookend)','after.projectionBookend(bookend);','owner->submissions.paletteAfter(slot,after,true);'])
    for body in (pre,post):
        require(body.count('paletteApiOwnerCurrent(')>=3,'Missing foreign-call palette owner revalidation')
    require('!owner->paletteApiPayloads[api].matched' in draw and
            'idleAdmitted=false;' in draw,'Changed companion post-state must retire certified geometry')
    sampler=region(gpu,'static bool sampleIdleApi(','static bool sampleIdleStreams(')
    require('idleGeometryInputs(' in sampler and 'if(copyProgram)' in sampler and
            'program.size_bytes()' in sampler,'Actual-declaration/program copy contract changed')
    cleanup=region(gpu,'static void cleanup(','void retireIdleSubmissionOwner(')
    ordered(cleanup,['release(probe.device);','probe.submissionOwner->submissions.finalize(',
                     'probe.submissionOwner=nullptr;','probe.busy=false;'])
    require('scopeGpuRoutingCurrent(device)' in cleanup,'Missing post-release numeric routing guard')
    outer=region(engine,'static void renderTrackedWeapon(','static void __fastcall weaponRender(')
    require(outer.count('retireIdleSubmissionOwner(')==2,'Missing normal/unwind outer retirement')
    ordered(outer,['retireIdleSubmissionOwner(&*idleStorage);','idleStorage->submissions.outerReturned=true;',
                   'emitIdleWeaponTrace(*idleStorage);','retireIdleSubmissionOwner(invocation.idle);',
                   'submissions.completeOuter(', 'emitIdlePaletteCompanions(*idleStorage);',
                   'emitIdlePaletteBoundaries(*idleStorage);'])
    boundary_emitter=region(engine,'static void emitIdlePaletteBoundaries(','static void emitIdlePaletteCompanions(')
    require('!trace.admitted||trace.nativeId!=2||!trace.submissions.outerCompleted' in boundary_emitter and
            boundary_emitter.count('trace.paletteBoundaryPublishable(n)')==2 and 'diagnostic=1' in boundary_emitter,
            'Boundary output lost its own completed-outer/row guard')
    emitter=region(engine,'static void emitIdlePaletteCompanions(','static void emitIdleWeaponTrace(')
    require('trace.paletteApiPublishable(n)' in emitter and 'trace.paletteContentPublishable(n)' in emitter and 'trace.paletteProjectionSequence(n)' in emitter and 'alignment=0grasp=0' in emitter,
            'Joined palette output lost its completion gate or claim limits')
    return {'source_order_checked':True,'joined_palette_source_checked':True,'geometry_cap_changed':False,
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
                 'ss2vr::game::copyIdleSubmissionMetadata(ss2vr::IdleWeaponTrace const*, ss2vr::IdleSubmissionMetadata&)',
                 'ss2vr::game::copyIdleSubmissionPalette(ss2vr::IdleWeaponTrace const*, ss2vr::IdlePaletteCopy&, ss2vr::IdlePaletteBoundaryCopy*)'):
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
