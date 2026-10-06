#!/usr/bin/env python3
"""Inspect the real compiled native sphere-query sequence, without execution."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess

def require(ok,message):
    if not ok:raise ValueError(message)
def verify(obj):
    assembly=subprocess.check_output(['objdump','-drC','-Mintel',str(obj)],text=True)
    require('file format pe-i386' in assembly,'Expected native x86 object')
    chunks=re.split(r'^[0-9a-f]+ <(.+)>:\n',assembly,flags=re.M)
    bodies=dict(zip(chunks[1::2],chunks[2::2]))
    found=[v for k,v in bodies.items() if k=='ss2vr::game::(anonymous namespace)::Query::run(void*)']
    require(len(found)==1,'Missing exact query callback')
    body=found[0];code=[]
    for line in body.splitlines():
        cols=line.split('\t')
        if len(cols)>=3 and re.fullmatch(r'\s*[0-9a-f]+:',cols[0]):
            value=' '.join(cols[-1].split())
            if re.match('[a-z]',value):code.append(value)
    calls=[int(i.split('0x')[1],16) for i in code if i.startswith('call DWORD PTR ds:0x')]
    require(calls==list(range(0,48,4))+[0],
            'Expected init, six scalar/filter setters, owner filters, check, copied hit and cleanup')
    # NativeQueries has twelve x86 pointers in declaration order. The compiled
    # callback forwards one stack argument for the setters, never an ECX this.
    expected={
        4:['lea edx,[esp+0x8]','mov DWORD PTR [esp],edx'],
        8:['fld DWORD PTR [edx+0x468]','fstp DWORD PTR [esp]'],
        12:['mov DWORD PTR [esp],0x0'],
        16:['fld DWORD PTR [eax+0xc]','fstp DWORD PTR [esp]'],
        20:['mov eax,DWORD PTR [eax+0x64]','mov DWORD PTR [esp],eax'],
        24:['mov eax,DWORD PTR [eax+0x64]','mov DWORD PTR [esp],eax'],
        28:['mov eax,DWORD PTR [eax+0x4]','mov DWORD PTR [esp],eax'],
        32:['mov eax,DWORD PTR [eax+0xc]','mov DWORD PTR [esp],eax'],
        36:['mov DWORD PTR [esp],0x0'],
    }
    for offset,prefix in expected.items():
        index=code.index('call DWORD PTR ds:0x%x'%offset)
        require(code[index-len(prefix):index]==prefix,'Native query argument changed: '+hex(offset))
    for instruction in ['mov edx,DWORD PTR [eax]','mov edx,DWORD PTR [eax+0x4]',
                        'mov edx,DWORD PTR [eax+0x8]',
                        'mov ecx,DWORD PTR [edx+0x45c]',
                        'mov ecx,DWORD PTR [edx+0x460]',
                        'mov edx,DWORD PTR [edx+0x464]']:
        require(instruction in code,'Six-float origin/direction copy changed')
    require(code.count('call DWORD PTR [ebx+0x8]')>=4,'Missing owner revalidation')
    require(body.count('roomscaleResourceScopeUsable')>=3,'Missing enclosing resource rejection')
    require(all(i=='ret' for i in code if i.startswith('ret')),'Query callback must be cdecl')
    outer=[v for k,v in bodies.items() if k.startswith('ss2vr::game::runRoomscaleSweepQueries(')]
    require(len(outer)==1 and 'roomscaleCollisionKernelsUsable' in outer[0] and
            'runRoomscaleModelQueryScope' in outer[0], 'Missing enabled-kernel/math scope gate')
    return {'object_sha256':hashlib.sha256(obj.read_bytes()).hexdigest(),
            'native_pointer_calls_per_sphere':13,'cdecl_setter_arguments_checked':True,
            'runtime_executed':False,'owner_integration_complete':False}
if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--object',type=Path,required=True)
    print(json.dumps(verify(parser.parse_args().object),indent=2))
