#!/usr/bin/env python3
"""Static/compiled controller integration checks; never run Windows code."""
import argparse
import configparser
import hashlib
import json
from pathlib import Path
import re
import subprocess
from verify_roomscale_owner_boundary import verify as verify_owner

def require(ok,message):
    if not ok:raise ValueError(message)
def verify(game,obj,root):
    owner=verify_owner(game)
    assembly=subprocess.check_output(['objdump','-drC','-Mintel',str(obj)],text=True)
    symbols=subprocess.check_output(['objdump','-tC',str(obj)],text=True)
    require('file format pe-i386' in assembly,'Expected x86 controller object')
    chunks=re.split(r'^[0-9a-f]+ <(.+)>:\n',assembly,flags=re.M)
    bodies=dict(zip(chunks[1::2],chunks[2::2]))
    def select(fragment,suffix=None):
        found=[v for k,v in bodies.items() if fragment in k and 'clone' not in k and
               (k.endswith(suffix) if suffix else k.startswith('ss2vr::game::'+fragment))]
        require(len(found)==1,'Missing/ambiguous controller boundary: '+fragment)
        return found[0]
    def code(text):
        out=[]
        for line in text.splitlines():
            cols=line.split('\t')
            if len(cols)>=3 and re.fullmatch(r'\s*[0-9a-f]+:',cols[0]):
                value=' '.join(cols[-1].split())
                if re.match('[a-z]',value):out.append(value)
        return out
    cleanup=select('runPostSimulationRoomscale(', '::Context::finish(void*, int)')
    instructions=code(cleanup)
    require(not any(re.match(r'f[a-z]',i) or re.search(r'\b(?:xmm|ymm|zmm)[0-9]',i) for i in instructions),
            'Native controller cleanup must not consume the caller FP stack/registers')
    require(any(i.startswith('lock cmpxchg8b') for i in instructions),
            'Cleanup revision access must use integer atomic operations')
    imports=re.findall(r'DISP32\s+([^\n]+)',cleanup)
    require(set(imports)<= {'AcquireSRWLockExclusive@4','ReleaseSRWLockExclusive@4','memcmp'},
            'Cleanup gained an unreviewed call')
    require(sum(i.startswith('call ') for i in instructions)==len(imports),
            'Cleanup gained an unreviewed direct/indirect native call')
    require('_tls_index' in cleanup and '.tls$' in cleanup,'Missing explicit TLS retirement')
    callback=select('checkRoomscaleTarget(')
    require('runRoomscaleMathFrame' in callback and
            sum(i.startswith('call ') for i in code(callback))==1,
            'Root-target check must copy data and enter the isolated math frame')
    require(not any(re.match(r'f[a-z]',i) for i in code(callback)),
            'Do not do FP work before the math frame')
    require([i for i in code(callback) if i.startswith('ret')]==['ret'],
            'Root target callback must remain cdecl')
    math=select('checkRoomscaleTargetMath(')
    if 'runRoomscaleSweepQueries' not in math:
        part='ss2vr::game::checkRoomscaleTargetMath(void*) [clone .part.0]'
        require(part in bodies and part in math and 'roomscaleOwnerCurrent' in math,
                'Unexpected outlined math callback')
        require(any(i.startswith('jmp ') and part in i for i in code(math)),
                'Entry must tail-delegate to the inspected math body')
        math=bodies[part]
    require(math.index('runRoomscaleSweepQueries')<math.index('AcquireSRWLockExclusive'),
            'Do not begin a rig mutation before every sphere query completes')
    require('lock cmpxchg8b' in math,'Missing atomic rig-transition claim')
    simulation=select('simulationStep(void*, void*)','::Context::run(void*)')
    original=re.findall(r'0x([0-9a-f]+) ss2vr::game::originalSimulationStep$',symbols,re.M)
    require(len(original)==1,'Missing original simulation trampoline')
    native_call='call   DWORD PTR ds:0x%x'%int(original[0],16)
    require(simulation.index(native_call)<simulation.index('runPostSimulationRoomscale')<
            simulation.index('multiplayer::completeTick'),
            'Controller must follow native physics and precede tick retirement')
    run=select('runPostSimulationRoomscale(')
    require('ss2vrNativeFinally' in run,'Controller attempt needs native-finally cleanup')
    body=select('runPostSimulationRoomscale(','::Context::run(void*)')
    require('runCheckedRoomscalePlacement' in body and 'runRoomscaleMathFrame' in body,
            'Controller is not connected to the production movement/math adapters')
    configuration=configparser.ConfigParser();configuration.read(root/'config/SS2VR.ini')
    require(configuration.getint('Roomscale','Enabled')==0,'Development roomscale default unexpectedly enabled')
    source=(root/'src/game/engine.cpp').read_text()
    for guard in ['roomscalePhaseCurrent(c,false)','roomscaleOwnerCurrent(&c)',
                  'nativePresentationIdleForBodyMove()', 'roomscaleSnapshotMatches(c.snapshot,current)',
                  'c.reads.seal()', 'c.reads.clear()', 'roomscaleResourceGatesUsable()',
                  'armRoomscalePlacementAfterEnable()', 'roomscaleCollisionKernelsUsable()']:
        require(guard in source,'Missing explicit integration guard: '+guard)
    require('||!singlePlayer())return;' in source,'Multiplayer must remain gated until settlement is paired')
    return {'object_sha256':hashlib.sha256(obj.read_bytes()).hexdigest(),
            'native_owner_evidence':owner,'cleanup_fp_free':True,'post_simulation_wiring':True,
            'development_default_enabled':False,'multiplayer_roomscale_enabled':False,
            'runtime_executed':False}
if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game',type=Path,required=True)
    parser.add_argument('--object',type=Path,required=True)
    parser.add_argument('--root',type=Path,default=Path(__file__).resolve().parents[1])
    a=parser.parse_args();print(json.dumps(verify(a.game,a.object,a.root),indent=2))
