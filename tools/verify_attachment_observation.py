#!/usr/bin/env python3
"""Compiled passive attachment ABI/value-copy gate. Never executes native code."""
import argparse
import hashlib
import json
import re
import subprocess
from pathlib import Path


def require(value,message):
    if not value:raise ValueError(message)


def verify(obj):
    asm=subprocess.check_output(['objdump','-drC','-Mintel',str(obj)],text=True)
    symbols=subprocess.check_output(['objdump','-tC',str(obj)],text=True)
    require('file format pe-i386' in asm,'Expected GNU x86 object')
    parts=re.split(r'^[0-9a-f]+ <(.+)>:\n',asm,flags=re.M)
    bodies=dict(zip(parts[1::2],parts[2::2]))
    body=bodies.get('ss2vr::game::observedAttachment(void*, unsigned int, ss2vr::Matrix34&)','')
    require(body,'Missing observer entry')
    found=re.findall(r'0x([0-9a-f]+) ss2vr::game::originalObservedAttachment$',symbols,re.M)
    require(len(found)==1,'Missing unique original trampoline')
    pointer=hex(int(found[0],16))
    code={}
    for line in body.splitlines():
        m=re.match(r'^\s*([0-9a-f]+):\t[^\t]+\t(.+)',line)
        if m:code[int(m[1],16)]=' '.join(m[2].split()).split(' <')[0]
    addresses=list(code);following=dict(zip(addresses,addresses[1:]))
    transfers={a for a,i in code.items() if i in ('call DWORD PTR ds:'+pointer,'jmp DWORD PTR ds:'+pointer)}
    require(len(transfers)==2,'Expected one recorded-call and one native-tail-forward site')
    pending=[(addresses[0],0)];seen=set();returns=0
    while pending:
        address,calls=pending.pop()
        if (address,calls) in seen:continue
        seen.add((address,calls));i=code[address]
        calls+=int(address in transfers);require(calls<=1,'Multiple native forwards on one path')
        if i=='ret' or (address in transfers and i.startswith('jmp ')):
            require(calls==1,'Observer returns without exactly one native forward');returns+=1;continue
        require(not i.startswith('ret '),'Observer changed native cdecl cleanup')
        branch=re.match(r'(j[a-z]+) ([0-9a-f]+)$',i)
        if branch:
            target=int(branch[2],16);require(target in code,'Observer branch escapes body')
            pending.append((target,calls))
            if branch[1]=='jmp':continue
        require(address in following,'Observer falls through body');pending.append((following[address],calls))
    require(returns,'No verified return')
    ins=list(code.values())
    require(not any(re.match(r'f[a-z]|call (?!DWORD PTR ds:'+re.escape(pointer)+r'$)',i) or
                    re.search(r'\b(?:xmm|ymm|zmm)[0-9]',i) for i in ins),'Observer adds callbacks or FP arithmetic')
    require('mov eax,DWORD PTR [esp+0x18]' in ins and 'mov edx,DWORD PTR [esp+0x1c]' in ins and
            'mov esi,DWORD PTR [esp+0x20]' in ins and 'mov DWORD PTR [esp],eax' in ins and
            'mov DWORD PTR [esp+0x4],edx' in ins and 'mov DWORD PTR [esp+0x8],esi' in ins,
            'Recorded path lost exact three native arguments')
    require('cmp eax,0x1' in ins and 'cmp DWORD PTR [ebx+0x8],0x1' in ins and
            'cmp BYTE PTR [ebx+0x41],0x0' in ins and 'cmp BYTE PTR [ebx+0x42],0x0' in ins,
            'Success/single/nonnested/nonaborted guards missing')
    call=next(a for a in transfers if code[a].startswith('call '))
    after=[i for a,i in code.items() if a>call]
    require(not any(re.match(r'(?:mov|lea|xor|add|sub|pop) eax\b',i) for i in after),
            'Observer overwrites original integer result')
    for offset in range(0,48,4):
        src='[esi]' if offset==0 else '[esi+'+hex(offset)+']'
        dst='[ebx+'+hex(16+offset)+']'
        require('mov edx,DWORD PTR '+src in ins and 'mov DWORD PTR '+dst+',edx' in ins,
                'Incomplete exact 48-byte native output copy')
    require(not any(re.match(r'mov (?:DWORD|BYTE) PTR \[esi(?:\+[^]]+)?\],',i) for i in ins),
            'Observer writes the native output')
    return {'object_sha256':hashlib.sha256(obj.read_bytes()).hexdigest(),
            'cdecl_arguments':3,'forward_once_each_reachable_path':True,'success_copy_bytes':48,
            'native_result_and_output_preserved':True,'runtime_executed':False}

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--object',required=True,type=Path)
    print(json.dumps(verify(parser.parse_args().object),indent=2))
