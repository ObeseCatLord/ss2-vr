#!/usr/bin/env python3
"""Inspect compiled DIP argument/caller forwarding; never execute Windows code."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import re

def require(ok, message):
    if not ok:
        raise ValueError(message)

ROOT = Path(__file__).resolve().parents[1]
GFX_SHA256 = '88749b79be36f0c0dccb623c4f1712685b5af603b451e25ed3c5029bedcdf3ed'


def verify_input_stream_routing(source: str) -> dict:
    """Narrow source gate tied to the actual getter loop, not synthetic receipts.

    This verifies the current bounded source form; compiled-product provenance
    and native forwarding have separate gates. It does not execute COM getters.
    """
    start=source.index('static bool boundInputs(')
    end=source.index('static bool sampleIdleStreams(',start)
    body=re.sub(r'//[^\n]*|/\*.*?\*/','',source[start:end],flags=re.S)
    body=''.join(body.split())
    selectors=[
        'constboolobserved78=(idle||ride)&&observed78Declaration(std::span(b.elements).first(b.count));',
        'constboolmultiUV78=(idle||ride)&&observedMultiUV78Declaration(std::span(b.elements).first(b.count));',
        'constboolnoUV78=(idle||ride)&&noUV78Declaration(std::span(b.elements).first(b.count));',
        'constbooluses78=observed78||multiUV78||noUV78;',
        'constUINTstreamNumbers[]{0,uses78?7u:5u,noUV78?2u:3u,uses78?8u:6u};',
        'for(unsignedi=0;i<(weights?4u:3u);++i){auto&stream=*streams[i];',
        'result=d->GetStreamSource(streamNumbers[i],&b.vertex[i],&stream.offset,&stream.stride);',
        'result=d->GetStreamSourceFreq(streamNumbers[i],&stream.frequency);',
    ]
    require(all(body.count(term)==1 for term in selectors),'Bound input stream routing differs from the reviewed getter path')
    positions=[body.index(term) for term in selectors]
    require(positions==sorted(positions),'Bound input selectors/getters are out of order')
    return {'source_checked':True,'no_uv78_streams':[0,7,2,8],
            'observed78_and_multi_uv78_streams':[0,7,3,8],
            'limits':['Source routing only; no COM getter or native content executed.']}



def verify_ride_gpu_source(source: str, render: str) -> dict:
    """Finite current-source ordering; not a general CFG or callback proof."""
    def compact(s): return ''.join(re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S).split())
    def body(a,b):
        start=source.index(a);return compact(source[start:source.index(b,start+len(a))])
    def ordered(s, terms):
        cursor=0
        for term in terms: cursor=s.index(term,cursor)+len(term)
    begin=body('static bool beginRideGpu(', 'static bool finishRideGpu(')
    ordered(begin,['boundInputs(d,probe.bindings[0],draw,GeometryBufferPolicy::Ride',
                   '!boundProgram(false,true)', '!rideGpuBookend(d)',
                   '!copySlice(i==1,', '!hashGeometrySlices(ranges,g.hashes)',
                   'boundInputs(d,probe.bindings[1],draw,GeometryBufferPolicy::Ride',
                   '!sameInputs(b,probe.bindings[1])', '!rideGpuBookend(d))', 'g.inputs=b.values'])
    require('ranges.slices[i].size>storage[i].size()' in begin and
            'if(!rideGpuOwnerCurrent(d))returnfalse;' in begin,
            'Ride copies require bounded destinations and post-unlock owner checks')
    finish=body('static bool finishRideGpu(', 'static bool collectIdleGeometry(')
    require('returnrideGpuOwnerCurrent(d)&&boundInputs(d,probe.bindings[2],probe.rideBefore.api,GeometryBufferPolicy::Ride,nullptr,-1,&probe.rideBefore.layout)&&sameInputs(probe.bindings[0],probe.bindings[2]);' in finish,
            'Ride post-original inputs are not compared')
    work=body('static HRESULT probeScopeDraw(', 'HRESULT scopeGpuDraw(')
    require('!probe.submissionOwner&&!probe.idleTrace&&!probe.raster.pose.valid&&remote_render::claimRideGpuAttempt(probe.rideBefore)' in work,
            'Ride GPU collection must not borrow another collector owner')
    ordered(work,['remote_render::claimRideGpuAttempt(probe.rideBefore)', 'probe.rideGpuAttempt=beginRideGpu(d);'])
    tail=work[work.index('result=forward(d,type,base,minimum,vertices,start,primitives);'):]
    ordered(tail,['result=forward(', 'probe.rideOriginalReturned=true;', 'probe.rideGpuMatched=finishRideGpu(d);',
                  'copyCurrentRideMainDraw(probe.rideAfter)', 'probe.rideMatched=probe.rideBefore==probe.rideAfter;',
                  'cleanup(aborted);', 'constautocompletedRide=probe.rideAfter;',
                  'conststd::optional<RideDrawGpuCopy>completedGpu=', 'probe.rideReady=false;',
                  'recordRideMainDraw(completedRide,completedGpu?&*completedGpu:nullptr)'])
    require('std::optional<RideDrawGpuCopy>{probe.rideGpu}' in tail,
            'GPU publication must snapshot a value rather than reusable TLS')
    cleanup=body('static void cleanup(', 'void retireIdleSubmissionOwner(')
    ordered(cleanup,['releaseBindings(b);', 'release(probe.device);',
                     'constboolrideOwnerCurrent=', 'constboolrideCameraCurrent=',
                     'probe.rideReady=rideOwnerCurrent',
                     'probe.rideGpu.copied=probe.rideReady&&probe.rideGpuMatched&&rideCameraCurrent;'])
    require('constboolrideCameraCurrent=!probe.rideCameraCaptured||(copyExecutedRideCamera(probe.rideAfter.identity.player,probe.generation,probe.rideAfter.eye,cameraAfterRelease)&&cameraAfterRelease==probe.rideCamera);' in cleanup,
            'Captured root camera must remain current after every Release')
    require('if(profile!=RideHandleProfile::Unknown&&copyRideHandles(slices,profile,probe.rideBefore.paletteCount,g.handles)&&probe.rideCameraCaptured)' in begin and
            'g.handlePositionAgrees=rideHandlesPosition(g.handles,' in begin,
            'Handle admission requires exact content, copied geometry and executed camera')
    require('!aborted&&!retired&&!probe.rideReentered&&!probe.split' in cleanup and
            'probe.generation==graphicsResourceGeneration()&&scopeGpuForwardingAllowed()&&scopeGpuMappingObservationCurrent(device)' in cleanup,
            'GPU readiness must retain final release/reentry/generation/routing checks')
    inputs=body('static bool boundInputs(', 'static bool sampleIdleApi(')
    ordered(inputs,['ride?rideBufferRanges(', 'if(ride)returntrue;', 'GetVertexShaderConstantF(8,'])
    budget=compact(render[render.index('bool claimRideGpuAttempt('):render.index('void recordRideMainDraw(')])
    require('returnchargeRideGpuAttempt(rideGpuAttempts[index],rideGpuSessionAttempts,rideObservationRows.load(std::memory_order_relaxed)>=32);' in budget,
            'GPU work must charge existing-bank and session budgets before work')
    return {'source_checked':True,'limits':['Finite lexical ordering only; no GPU/COM/native callbacks executed; not general CFG/lifetime proof.']}

def symbols(path: Path) -> dict:
    result = {}
    output = subprocess.run(['i686-w64-mingw32-nm','-C',str(path)],check=True,capture_output=True,text=True)
    for line in output.stdout.splitlines():
        fields = line.split(maxsplit=2)
        if len(fields) == 3 and fields[1] in ('t','T'):
            result[fields[2]] = int(fields[0],16)
    return result


def verify(game: Path) -> dict:
    import capstone
    import pefile
    source=(ROOT/'src/game/scope_gpu.cpp').read_text()
    routing=verify_input_stream_routing(source)
    ride=verify_ride_gpu_source(source,(ROOT/'src/game/remote_render.cpp').read_text())
    native = game/'Bin/GfxD3D.dll'
    if hashlib.sha256(native.read_bytes()).hexdigest() != GFX_SHA256:
        raise ValueError('Native DIP caller fingerprint changed')
    decoder = capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
    gfx = pefile.PE(str(native),max_symbol_exports=65536)
    call = list(decoder.disasm(gfx.get_data(0xa00b,6),gfx.OPTIONAL_HEADER.ImageBase+0xa00b))
    require(len(call)==1 and call[0].mnemonic=='call' and call[0].op_str=='dword ptr [ecx + 0x148]', "Required boundary check failed: len(call) == 1 and call[0].mnemonic == 'call' and (call[0].op_str == 'dword ptr [ecx + 0x148]')")
    require(call[0].address+call[0].size==gfx.OPTIONAL_HEADER.ImageBase+0xa011, 'Required boundary check failed: call[0].address + call[0].size == gfx.OPTIONAL_HEADER.ImageBase + 40977')
    products = {}
    for filename in ('d3d9.dll','SS2VRServer.dll'):
        path = ROOT/'build-game'/filename
        table = symbols(path)
        hook = next(value for name,value in table.items()
                    if name.startswith('ss2vr::game::drawIndexed(') and name.endswith('@28'))
        target = next(value for name,value in table.items() if name.startswith('ss2vr::game::scopeGpuDraw('))
        pe = pefile.PE(str(path),max_symbol_exports=65536)
        require(pe.FILE_HEADER.Machine==0x14c, 'Required boundary check failed: pe.FILE_HEADER.Machine == 332')
        code = list(decoder.disasm(pe.get_data(hook-pe.OPTIONAL_HEADER.ImageBase,96),hook))
        returned = next(i for i,insn in enumerate(code) if insn.mnemonic=='ret')
        code = code[:returned+1]
        pairs = [(i.mnemonic,i.op_str) for i in code]
        # 36-byte outgoing cdecl area: seven original arguments, actual native
        # return address, bridge forwarding callback. Incoming WINAPI retires28.
        require(pairs[0]==('sub','esp, 0x24') and pairs[-2:]==[('add','esp, 0x24'),('ret','0x1c')], "Required boundary check failed: pairs[0] == ('sub', 'esp, 0x24') and pairs[-2:] == [('add', 'esp, 0x24'), ('ret', '0x1c')]")
        require(pairs[1]==('mov','eax, dword ptr [esp + 0x24]'), "Required boundary check failed: pairs[1] == ('mov', 'eax, dword ptr [esp + 0x24]')")
        require(pairs[3]==('mov','dword ptr [esp + 0x1c], eax'), "Required boundary check failed: pairs[3] == ('mov', 'dword ptr [esp + 0x1c], eax')")
        require(pairs[2][0]=='mov' and pairs[2][1].startswith('dword ptr [esp + 0x20], 0x'), "Required boundary check failed: pairs[2][0] == 'mov' and pairs[2][1].startswith('dword ptr [esp + 0x20], 0x')")
        require(pairs[4:18]==[
            ('mov','eax, dword ptr [esp + 0x40]'),('mov','dword ptr [esp + 0x18], eax'),
            ('mov','eax, dword ptr [esp + 0x3c]'),('mov','dword ptr [esp + 0x14], eax'),
            ('mov','eax, dword ptr [esp + 0x38]'),('mov','dword ptr [esp + 0x10], eax'),
            ('mov','eax, dword ptr [esp + 0x34]'),('mov','dword ptr [esp + 0xc], eax'),
            ('mov','eax, dword ptr [esp + 0x30]'),('mov','dword ptr [esp + 8], eax'),
            ('mov','eax, dword ptr [esp + 0x2c]'),('mov','dword ptr [esp + 4], eax'),
            ('mov','eax, dword ptr [esp + 0x28]'),('mov','dword ptr [esp], eax')], "Required boundary check failed: pairs[4:18] == [('mov', 'eax, dword ptr [esp + 0x40]'), ('mov', 'dword ptr [esp + 0x18], eax'), ('mov', 'eax, dword ptr [esp + 0x3c]'), ('mov', 'dword ptr [esp ")
        require(pairs[18]==('call',hex(target)), "Required boundary check failed: pairs[18] == ('call', hex(target))")
        products[filename]={'sha256':hashlib.sha256(path.read_bytes()).hexdigest(),
                            'dip_winapi_stack_retirement':28,'original_arguments_preserved':True,
                            'actual_native_caller_forwarded':True,'scope_gpu_cdecl_arguments':9}
    return {'runtime_executed':False,'windows_code_executed':False,
            'input_stream_routing':routing,'ride_gpu_source':ride,
            'native_gfx_sha256':GFX_SHA256,'native_dip_return_rva':'0xa011','products':products,
            'limits':['Static compiled shape only; no COM invocation or exception recovery executed.']}


if __name__ == '__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game',type=Path,required=True)
    args=parser.parse_args()
    print(json.dumps(verify(args.game),indent=2))
