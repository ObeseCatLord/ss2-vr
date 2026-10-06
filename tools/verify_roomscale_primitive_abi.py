#!/usr/bin/env python3
"""Static pinned-native and compile-only hidden-output primitive ABI checks.

Never executes native/Windows code or emits proprietary disassembly. This does
not certify native TOI arithmetic, shared-query ownership or body movement.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import struct
import subprocess

import capstone
import pefile
from verify_roomscale_query_abi import functions, instructions

ROOT=Path(__file__).resolve().parents[1]
PIN='7a1bd56b9bfa3edfbb23f4d3c96490e40a7c0b031b85e797e1af91b2ba3cf207'
EXPORT=b'?mthIntersectThickRayPrimitive@SeriousEngine@@YA?AVBox1f@1@ABVRay3f@1@ABVCPrimitiveDesc@1@M@Z'

def require(condition,message):
    if not condition: raise ValueError(message)

def run(*args):
    return subprocess.check_output(args,text=True)

def verify(game,output):
    raw=(game/'Bin/Core.dll').read_bytes()
    require(hashlib.sha256(raw).hexdigest()==PIN,'Core fingerprint mismatch')
    pe=pefile.PE(data=raw)
    require(pe.FILE_HEADER.Machine==0x14c and pe.OPTIONAL_HEADER.ImageBase==0x10000000,'Native architecture changed')
    matching=[x for x in pe.DIRECTORY_ENTRY_EXPORT.symbols if x.name==EXPORT]
    require(len(matching)==1 and matching[0].address==0x19a40 and not matching[0].forwarder,'Primitive export changed')
    branches=[x-0x10000000 for x in struct.unpack('<5I',pe.get_data(0x19bac,20))]
    require(branches==[0x19a5c,0x19aed,0x19ab8,0x19a83,0x19b62],'Descriptor branch table changed')
    for rva,value in [(0x809e0,.5),(0x8198c,-.5),(0xb38a4,3.0000000054977558e38),(0xb38a8,-3.0000000054977558e38)]:
        require(struct.unpack('<f',pe.get_data(rva,4))[0]==value,'Native scalar changed')
    md=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
    decoded={}
    for start,size in [(0x19a40,0x16c),(0x191c0,0x120),(0x192e0,0xaf),(0x19710,0x60)]:
        decoded.update({i.address-0x10000000:(i.mnemonic,i.op_str) for i in md.disasm(pe.get_data(start,size),0x10000000+start)})
    expected={
        0x19a46:('mov','eax, dword ptr [ebp + 0x10]'),
        0x19a49:('mov','ecx, dword ptr [eax]'),
        0x19a5c:('fld','dword ptr [eax + 4]'),
        0x19a5f:('mov','eax, dword ptr [ebp + 0xc]'),
        0x19a68:('mov','esi, dword ptr [ebp + 8]'),
        0x19a6c:('fadd','dword ptr [ebp + 0x14]'),
        0x19a7c:('mov','eax, esi'),0x19a82:('ret',''),
        0x191c9:('mov','ecx, dword ptr [eax + 0x14]'),
        0x191cc:('fld','dword ptr [eax + 0xc]'),
        0x191d8:('mov','edx, dword ptr [eax]'),
        0x191dc:('mov','eax, dword ptr [eax + 8]'),
        0x192e6:('mov','eax, dword ptr [ecx + 0x10]'),
        0x19304:('mov','ecx, dword ptr [ecx + 4]'),
        0x19753:('fld','dword ptr [ebp + 0x14]'),
        0x1975e:('lea','edx, [ebp - 0x24]'),
        0x19761:('fsub','dword ptr [ebp + 0x10]'),
    }
    for address,expected_instruction in expected.items():
        require(decoded.get(address)==expected_instruction,f'Native ABI/axis mismatch at {address:x}')
    output.mkdir(parents=True,exist_ok=True)
    for name,source,optimization in [('fixture','tests/native_roomscale_primitive_abi.cpp','-O0'),
                                     ('production','src/game/roomscale_query.cpp','-O2')]:
        obj=output/(name+'.o')
        subprocess.run(['i686-w64-mingw32-g++','-std=c++20',optimization,'-fno-omit-frame-pointer',
                        '-Wall','-Wextra','-Werror','-I'+str(ROOT/'src'),'-c',str(ROOT/source),'-o',str(obj)],check=True)
        parsed=functions(run('i686-w64-mingw32-objdump','-dr','-Mintel',str(obj)))
        if name=='fixture':
            for symbol,indirect in [('_primitiveAbiCall',True),('_primitiveAbiEntry',False)]:
                body=parsed[symbol];code=instructions(body)
                first=12 if indirect else 8;reg='edx' if indirect else 'eax'
                prefix=['push ebp','mov ebp,esp','sub esp,0x10']
                if indirect:prefix+=['mov eax,DWORD PTR [ebp+0x8]']
                prefix += [f'fld DWORD PTR [ebp+0x{first+12:x}]','fstp DWORD PTR [esp+0xc]']
                for index in (2,1,0):
                    destination='[esp]' if not index else f'[esp+0x{index*4:x}]'
                    prefix += [f'mov {reg},DWORD PTR [ebp+0x{first+index*4:x}]',f'mov DWORD PTR {destination},{reg}']
                require(code[:len(prefix)]==prefix,'Four outgoing slots changed')
                require(code[-2:]==['leave','ret'],'Fixture caller cleanup/return changed')
                if indirect:require(code[len(prefix)]=='call eax','Indirect original ABI changed')
                else:require('DISP32\t_ss2vrRoomscalePrimitiveQuery' in body,'Detour relocation changed')
        else:
            body=parsed['_ss2vrRoomscalePrimitiveQuery'];code=instructions(body)
            require('and esp,0xfffffff0' in code and '__tls_index' in body and 'secrel32\t.tls$' in body,'Native entry alignment/TLS changed')
            calls=[i for i,c in enumerate(code) if c=='call eax']
            require(len(calls)==2,'Expected scoped and unscoped original calls')
            for i,reg in zip(calls,('ecx','edx')):
                require(code[i-7:i]==['fld DWORD PTR [ebp+0x14]','fstp DWORD PTR [esp+0xc]',
                    f'mov {reg},DWORD PTR [ebp+0x10]',f'mov DWORD PTR [esp+0x8],{reg}',
                    f'mov {reg},DWORD PTR [ebp+0xc]','mov DWORD PTR [esp],ebx',
                    f'mov DWORD PTR [esp+0x4],{reg}'],'Production hidden-output argument forwarding changed')
            require('mov ebx,DWORD PTR [ebp+0x8]' in code and 'cmp ebx,eax' in code,'Native output pointer ownership check changed')
            require(not any(c.startswith('ret ') for c in code),'Cdecl callee must not pop arguments')
    return {'runtime_executed':False,'core_sha256':PIN,'descriptor_bytes':16,'output_bytes':8,
            'cdecl_slots':4,'vertical_axis':'Y','admitted_types':[0,1,2,3],'type_4_admitted':False,
            'fixture_and_production_abi':'pass','native_toi_conservatism_proved':False}

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game',type=Path,required=True)
    parser.add_argument('--output-dir',type=Path,required=True)
    args=parser.parse_args()
    print(json.dumps(verify(args.game.resolve(),args.output_dir.resolve()),indent=2))
