#!/usr/bin/env python3
"""Inspect actual linked x86 zoom entries/callback returns; execute no PE code."""
import hashlib
import json
import re
import subprocess
from pathlib import Path
import capstone
import pefile
def require(ok, message):
    if not ok:
        raise ValueError(message)

ROOT=Path(__file__).resolve().parents[1]
ENTRIES=[('zoomActivatePredicate',28,0),('zoomOwnerPredicate',28,1),('zoomFovPredicate',28,2),
         ('zoomInterpolatePredicate',28,3),('zoomSoundStartPredicate',24,4),('zoomSoundStopPredicate',28,5)]
CALLBACKS={'sniperStep':0,'baseWeaponStep':0,'sniperFire':4,'zoomActivated':0,'zoomDeactivated':0,
           'sniperPutDown':4,'sniperDeleted':0}

def inspect(path):
    native=pefile.PE(str(path),max_symbol_exports=65536)
    require(native.FILE_HEADER.Machine==0x14c, 'Required boundary check failed: native.FILE_HEADER.Machine == 332')
    base=native.OPTIONAL_HEADER.ImageBase
    rows=subprocess.check_output(['i686-w64-mingw32-nm',str(path)],text=True).splitlines()
    symbols={r.split()[2]:int(r.split()[0],16) for r in rows
             if len(r.split())==3 and re.fullmatch('[0-9a-fA-F]+',r.split()[0])}
    text_addresses=sorted({int(r.split()[0],16) for r in rows if re.search(' [tT] ',r)})
    decoder=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
    relocations={entry.rva for block in native.DIRECTORY_ENTRY_BASERELOC
                 for entry in block.entries if entry.type==3}
    entries=[]
    for name,slot,kind in ENTRIES:
        address=symbols['_'+name]
        instructions=[]
        for instruction in decoder.disasm(native.get_data(address-base,128),address):
            instructions.append(instruction)
            if instruction.mnemonic=='jmp':break
        actual=[(i.mnemonic,i.op_str) for i in instructions]
        expected=[('pushfd',''),('pushal',''),('cld',''),('mov','ebp, esp'),('and','esp, 0xfffffff0'),
                  ('sub','esp, 0x200'),('fxsave','[esp]'),('fninit',''),('fldcw','word ptr [esp]'),
                  ('sub','esp, 4'),('push',str(kind)),('push','dword ptr [ebp + 4]'),
                  ('push',f'dword ptr [ebp + 0x{slot:x}]'),('call',f'0x{symbols["_ss2vrZoomPredicate"]:x}'),
                  ('add','esp, 0x10'),('mov',f'dword ptr [ebp + 0x{slot:x}], eax'),
                  ('fxrstor','[esp]'),('mov','esp, ebp'),('popal',''),('popfd',''),
                  ('jmp',f'dword ptr [0x{symbols["_"+name+"_original"]:x}]')]
        require(actual==expected, name+' entry ABI changed')
        jump=instructions[-1]
        require(jump.address-base+2 in relocations, name+' trampoline cell lacks HIGHLOW relocation')
        entries.append({'name':name,'saved_result_offset':slot,'saved_subject_offset':4,'kind':kind,
                        'instructions':len(instructions),'trampoline_cell_relocated':True})
    callbacks=[]
    for name,pop in CALLBACKS.items():
        matches=[n for n in symbols if re.fullmatch(r'@_ZN5ss2vr4gameL\d+'+name+r'EPvS\d_.*@\d+',n)]
        require(len(matches)==1, (name,'callback definition missing/ambiguous'))
        address=symbols[matches[0]]
        end=next(a for a in text_addresses if a>address)
        returns=[i for i in decoder.disasm(native.get_data(address-base,end-address),address) if i.mnemonic=='ret']
        require(returns and all(i.op_str==str(pop) if pop else not i.op_str for i in returns), name+' return ABI changed')
        callbacks.append({'name':name,'return_stack_bytes':pop,'return_sites':len(returns)})
    return {'sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'entries':entries,'callbacks':callbacks}

report={'runtime_executed':False,'windows_code_executed':False,'hooks_installed':False,
        'method':'actual linked PE instructions, exact helper/cell targets, ASLR relocations and callback returns',
        'limits':['No native control flow, installed MinHook trampoline or callback lifetime executed',
                  'Callback argument/receiver provenance still requires source/compiled-call review'],
        'artifacts':{name:inspect(ROOT/'build-game'/name) for name in ('d3d9.dll','SS2VRServer.dll')}}
print(json.dumps(report,indent=2))
