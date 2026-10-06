#!/usr/bin/env python3
"""Pinned native and compiled query-observer ABI; no Windows execution."""
import argparse
import hashlib
import json
import re
import subprocess
from pathlib import Path
import pefile

PIN='da6efc9f72637eb3b6f48eadca2107be89b09c00618b6e72d5d3632938a7d851'
def require(ok, message):
    if not ok: raise ValueError(message)

def verify(game, obj):
    raw=(game/'Bin/Engine.dll').read_bytes()
    require(hashlib.sha256(raw).hexdigest()==PIN,'Unsupported Engine image')
    pe=pefile.PE(data=raw,max_symbol_exports=100000)
    exports={s.name.decode():s.address for s in pe.DIRECTORY_ENTRY_EXPORT.symbols if s.name}
    for name,rva in {
        '?rayInit@SeriousEngine@@YAXXZ':0x1b35e0,
        '?cldCheckRay@SeriousEngine@@YAHXZ':0x29200,
        '?cldContinueRay@SeriousEngine@@YAHXZ':0x29290,
        '?mdlModelCheckRay@SeriousEngine@@YAHPAVCModelInstance@1@ABVMatrix34f@1@H@Z':0xda280,
    }.items():require(exports.get(name)==rva,'Native observer export changed: '+name)
    for rva in (0x1b36a4,0x29286,0xda522):
        require(pe.get_data(rva,1)==b'\xc3','Native cdecl return changed')
    require(pe.get_data(0x292cd,5)==bytes.fromhex('e92efeffff'),'Continue traversal tail changed')
    assembly=subprocess.check_output(['objdump','-drC','-Mintel',str(obj)],text=True)
    symbols=subprocess.check_output(['objdump','-tC',str(obj)],text=True)
    parts=re.split(r'^[0-9a-f]+ <(.+)>:\n',assembly,flags=re.M)
    bodies=dict(zip(parts[1::2],parts[2::2]))
    def code(body):
        result=[]
        for line in body.splitlines():
            cols=line.split('\t')
            if len(cols)>=3 and re.fullmatch(r'\s*[0-9a-f]+:',cols[0]):
                value=' '.join(cols[-1].split())
                if re.match('[a-z]',value): result.append(value)
        return result
    def body_for(name, suffix=None):
        found=[v for k,v in bodies.items() if name in k and
               ((k.endswith(suffix) and 'clone' not in k) if suffix else k.startswith('ss2vr::game::'+name))]
        require(len(found)==1,'Ambiguous production observer: '+name)
        return found[0]
    for name in ('observedCheckRay()','observedContinueRay()',
                 'observedModelCheckRay(void*, ss2vr::Matrix34 const&, int)','trackedRayInit()'):
        outer=body_for(name)
        require('ss2vrNativeFinally' in outer and '_tls_index' in outer,'Missing native finally/TLS extent')
        require([v for v in code(outer) if v.startswith('ret')]==['ret'],'Observer must remain cdecl')
    init=code(body_for('trackedRayInit()'))
    require(init[0]=='sub esp,0x3c' and 'mov eax,DWORD PTR [esp+0x3c]' in init[:5],
            'rayInit must capture native return address, not an argument or wrapper caller')
    model=code(body_for('observedModelCheckRay(void*, ss2vr::Matrix34 const&, int)'))
    require(model[0]=='sub esp,0x44','Model observer frame changed')
    for value in ('lea eax,[esp+0x48]','mov eax,DWORD PTR [esp+0x4c]','lea eax,[esp+0x50]'):
        require(value in model,'Model/placement/mode capture changed')
    for name,original in (('observedCheckRay()','originalCheckRay'),
                          ('observedContinueRay()','originalContinueRay'),
                          ('observedModelCheckRay(void*, ss2vr::Matrix34 const&, int)','originalModelCheckRay')):
        callback=code(body_for(name,'::Context::run(void*)'))
        require('and esp,0xfffffff0' in callback[:7],'Native callback must realign locally')
        found=re.findall(r'0x([0-9a-f]+) ss2vr::game::'+original+r'$',symbols,re.M)
        require(len(found)==1,'Missing native trampoline storage')
        call='call DWORD PTR ds:0x%x'%int(found[0],16)
        require(callback.count(call)==1,'Observer must call original once')
        i=callback.index(call)
        require(callback[i+1]=='mov DWORD PTR [ebx],eax','Native result must be copied unchanged')
        if original=='originalModelCheckRay':
            require(callback[i-9:i]==[
                'mov edx,DWORD PTR [eax+0xc]','mov ebx,DWORD PTR [eax]',
                'mov edx,DWORD PTR [edx]','mov DWORD PTR [esp+0x8],edx',
                'mov edx,DWORD PTR [eax+0x8]','mov DWORD PTR [esp+0x4],edx',
                'mov eax,DWORD PTR [eax+0x4]','mov eax,DWORD PTR [eax]',
                'mov DWORD PTR [esp],eax'],'Native model argument forwarding changed')
    return {'native_sha256':PIN,'object_sha256':hashlib.sha256(obj.read_bytes()).hexdigest(),
            'native_cdecl_wrappers':4,'runtime_executed':False,
            'same_thread_extent_observation_only':True,'worker_ownership_proven':False}

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--game',type=Path,required=True);p.add_argument('--object',type=Path,required=True)
    a=p.parse_args();print(json.dumps(verify(a.game,a.object),indent=2))
