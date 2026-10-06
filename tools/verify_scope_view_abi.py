#!/usr/bin/env python3
"""Check the real compiled native capture callback's stack/return ABI."""
import hashlib
import json
from pathlib import Path
import subprocess
import capstone
import pefile
ROOT=Path(__file__).resolve().parents[1]
results=[]
for name in ('d3d9.dll','SS2VRServer.dll'):
    path=ROOT/'build-game'/name
    symbols=[]
    for line in subprocess.check_output(['i686-w64-mingw32-nm','-C',str(path)],text=True).splitlines():
        fields=line.split(maxsplit=2)
        if len(fields)==3 and fields[1] in ('t','T'):
            symbols.append((int(fields[0],16),fields[2]))
    address=[a for a,n in symbols if n=='ss2vr::game::captureNativeScope(void*)']
    if len(address)!=1: raise ValueError('Missing or ambiguous capture callback')
    address=address[0]; end=min(a for a,n in symbols if a>address)
    pe=pefile.PE(str(path))
    decoder=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
    code=list(decoder.disasm(pe.get_data(address-pe.OPTIONAL_HEADER.ImageBase,end-address),address))
    pairs=[(i.mnemonic,i.op_str) for i in code]
    if pairs[:5]!=[('push','ebp'),('mov','ebp, esp'),('push','esi'),('push','ebx'),('and','esp, 0xfffffff0')]:
        raise ValueError('Capture callback no longer realigns the incoming native stack')
    if ('mov','edx, dword ptr [ebp + 8]') not in pairs[:8]:
        raise ValueError('Native context argument is not read from the original stack frame')
    returns=[operand for mnemonic,operand in pairs if mnemonic=='ret']
    if not returns or any(returns): raise ValueError('Capture callback is not plain cdecl return')
    results.append({'product':name,'sha256':hashlib.sha256(path.read_bytes()).hexdigest(),
                    'context_argument':'original ebp+8','incoming_stack_realigned':True,'callee_pop_bytes':0})
print(json.dumps({'runtime_executed':False,'products':results,
                  'limits':['Exact supported compiler shape; native callback execution and failures not run']},indent=2))
