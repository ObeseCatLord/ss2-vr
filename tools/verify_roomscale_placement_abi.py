#!/usr/bin/env python3
"""Static/compile-only checked-placement cancellation boundary; no activation."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import capstone
import pefile

PIN='da6efc9f72637eb3b6f48eadca2107be89b09c00618b6e72d5d3632938a7d851'
def require(ok,message):
    if not ok: raise ValueError(message)
def instructions(body):
    out=[]
    for line in body.splitlines():
        cols=line.split('\t')
        if len(cols)>=3 and re.fullmatch(r'\s*[0-9a-f]+:',cols[0]):
            value=' '.join(cols[-1].split())
            if re.match('[a-z]',value):out.append(value)
    return out

def verify(game,obj):
    raw=(game/'Bin/Engine.dll').read_bytes()
    require(hashlib.sha256(raw).hexdigest()==PIN,'Unsupported Engine image')
    pe=pefile.PE(data=raw,max_symbol_exports=100000);base=pe.OPTIONAL_HEADER.ImageBase
    require(base==0x10000000 and pe.FILE_HEADER.Machine==0x14c,'Unsupported Engine ABI')
    exports={s.name.decode():s.address for s in pe.DIRECTORY_ENTRY_EXPORT.symbols if s.name}
    for name,rva in {
        '?SetAbsPlacement@CMechanism@SeriousEngine@@QAEHABVQuatVect@2@K@Z':0x134010,
        '?SetAbsPlacement@CMechanismPart@SeriousEngine@@QAEXABVQuatVect@2@@Z':0x131870,
        '?SetAbsPlacement@CAspect@SeriousEngine@@QAEXABVQuatVect@2@@Z':0x575d0,
    }.items():require(exports.get(name)==rva,'Placement export changed')
    md=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32);md.detail=True
    def decode(rva,length):return list(md.disasm(pe.get_data(rva,length),base+rva))
    whole=decode(0x134010,0x1d30)
    points={i.address-base:(i.mnemonic,i.op_str) for i in whole}
    for rva,expected in {
        0x134013:('sub','esp, 0x364'),
        0x134019:('push','ebx'),0x13401a:('push','esi'),0x134020:('push','edi'),
        0x13403a:('mov','dword ptr [ebp - 0x1f4], ebx'),
        0x134d07:('lea','edx, [ebp - 0x18c]'),0x134d0d:('push','edx'),
        0x134d79:('mov','ecx, esi'),0x134d7b:('call','0x10131870'),
        0x134d80:('mov','eax, dword ptr [esi + 0x10]'),
        0x134d83:('test','eax, eax'),0x134d93:('call','dword ptr [edx + 0x2c]'),
        0x135c62:('pop','edi'),0x135c63:('pop','esi'),0x135c64:('xor','eax, eax'),
        0x135c66:('pop','ebx'),0x135c67:('mov','esp, ebp'),0x135c69:('pop','ebp'),
        0x135c6a:('ret','8'),
    }.items():require(points.get(rva)==expected,'Placement boundary changed: '+hex(rva))
    part=decode(0x131870,0x268)
    require([(i.address-base,i.op_str) for i in part if i.mnemonic=='call']==
            [(0x131ab9,'0x100575d0'),(0x131acf,'0x100575d0')],
            'Part setter gained another callback before/after aspect commit')
    for i in part:
        for op in i.operands:
            if op.type==capstone.x86.X86_OP_MEM and op.access&capstone.CS_AC_WRITE:
                require(op.mem.base in (capstone.x86.X86_REG_EBP,capstone.x86.X86_REG_ESP),
                        'Part setter gained a non-stack write before aspect interception')
    require(pe.get_data(0x131abe,6)==bytes.fromhex('8be55dc20400'),'Root aspect return changed')
    asm=subprocess.check_output(['objdump','-drC','-Mintel',str(obj)],text=True)
    chunks=re.split(r'^[0-9a-f]+ <(.+)>:\n',asm,flags=re.M)
    bodies=dict(zip(chunks[1::2],chunks[2::2]))
    for name,after in [('roomscaleBeforePart',0),('roomscaleAfterPart',1)]:
        code=instructions(bodies[name])
        require(code[:9]==['pushf','pusha','cld','mov ebp,esp','and esp,0xfffffff0',
                'sub esp,0x200','fxsave [esp]','fninit','fldcw WORD PTR [esp]'],
                'Placement gate native register/FP save changed')
        require(f'push 0x{after:x}' in code and 'push DWORD PTR [ebp+0x4]' in code,
                'Placement gate must capture saved ESI and its stage')
        require(sum(x.startswith('call ') for x in code)==1 and
                code.count('fxrstor [esp]')==2 and code.count('popa')==2 and code.count('popf')==2,
                'Placement continuations must preserve registers/flags/FP')
        require(not any(x.startswith('ret') for x in code),'Gate must not invent a native return')
    cancel=instructions(bodies['roomscaleCancelPlacement'])
    require(cancel[0]=='lea esp,[ebp-0x370]' and cancel[1].startswith('jmp DWORD PTR'),
            'Cancellation must retire pending arguments and use native ret8 cleanup')
    decision=bodies['ss2vrPlacementDecision']
    require('_tls_index' in decision and '.tls$' in decision,'Placement decision needs native TLS')
    require(not any(x.startswith('call ') for x in instructions(decision)),
            'Placement decision must remain a call-free leaf')
    for marker,cleanup in [('L12mechanismSet','0x8'),('L7partSet','0x4'),('L9aspectSet','0x4')]:
        found=[instructions(v) for k,v in bodies.items() if marker in k and k.startswith('@')]
        require(len(found)==1,'Missing compiled native setter wrapper')
        returns=[v for v in found[0] if v.startswith('ret')]
        require(returns and all(v=='ret '+cleanup for v in returns),'Native setter cleanup changed')
    require('ss2vrNativeFinally' in asm,'Missing native unwind containment')
    return {'engine_sha256':PIN,'object_sha256':hashlib.sha256(obj.read_bytes()).hexdigest(),
            'native_part_pre_aspect_writes_are_stack_only':True,'native_cancellation_stack':'EBP-0x370',
            'runtime_executed':False,'placement_hooks_activated':False,'body_movement_implemented':False}

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--game',required=True,type=Path);p.add_argument('--object',required=True,type=Path)
    a=p.parse_args();print(json.dumps(verify(a.game,a.object),indent=2))
