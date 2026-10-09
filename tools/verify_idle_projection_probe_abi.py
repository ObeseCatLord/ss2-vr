#!/usr/bin/env python3
"""Pinned native matrix bookends and compiled integer-only observer; no execution."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import capstone
import pefile

PINS={'Engine.dll':'da6efc9f72637eb3b6f48eadca2107be89b09c00618b6e72d5d3632938a7d851',
      'Shaders.dll':'de7652d61f5af2ec8b218b902d09820b3ef76c1d796f23d66a31ea35374fd642'}

def require(ok,message):
    if not ok:raise ValueError(message)

def decode(raw,address):
    decoder=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32);decoder.detail=True
    return list(decoder.disasm(raw,address))

def plain_integer(i,allow_sample=False):
    m=i.mnemonic
    require((m=='fnstcw' and allow_sample) or not m.startswith('f'),'FP instruction in observer window')
    require(not re.search(r'\b(?:xmm|ymm|zmm|mm|st)\d?\b',i.op_str),'FP/SIMD register in observer window')
    require(m not in ('call','int','int3','syscall','sysenter','ud2','xbegin'), 'Call/trap in observer window')
    require(m not in ('ldmxcsr','stmxcsr','fxsave','fxrstor','xsave','xrstor'), 'FP state operation in observer window')
    if m=='ret':require(not i.op_str,'Observer changed cdecl stack cleanup')

def verify_sample(ins):
    require(ins and any(i.mnemonic=='ret' for i in ins),'Missing sampler return')
    control=[i for i in ins if i.mnemonic=='fnstcw']
    require(len(control)==1,'Sampler needs exactly one non-mutating control read')
    known={i.address for i in ins};by_address={i.address:i for i in ins}
    for i in ins:
        plain_integer(i,True)
    todo=[(ins[0].address,0)];seen=set();returned=False
    while todo:
        address,reads=todo.pop()
        if (address,reads) in seen:continue
        seen.add((address,reads));i=by_address[address]
        reads+=int(i.mnemonic=='fnstcw');require(reads<=1,'Sampler resamples control')
        if i.mnemonic=='ret':require(reads==1,'Sampler returns without control read');returned=True;continue
        for edge in successors(i,known):todo.append((edge,reads))
    require(returned,'No reachable sampler return')
    return {'instructions':len(ins),'calls':0,'control_reads':1,'fp_arithmetic':0,'fp_state_writes':0}

def bodies(obj):
    text=subprocess.check_output(['objdump','-drC','-Mintel','--insn-width=16','--disassemble-zeroes',str(obj)],text=True)
    require('file format pe-i386' in text,'Expected x86 COFF observer')
    parts=re.split(r'^[0-9a-f]+ <(.+)>:\n',text,flags=re.M)
    return dict(zip(parts[1::2],parts[2::2]))

def instructions(body):
    out=[]
    for line in body.splitlines():
        m=re.match(r'^\s*([0-9a-f]+):\s+((?:[0-9a-f]{2} )+)[ \t]+\S',line)
        if m:
            code=bytes.fromhex(m[2]);decoded=decode(code,int(m[1],16))
            require(len(decoded)==1 and decoded[0].size==len(code),'Incomplete/wrapped compiled instruction')
            if out:require(out[-1].address+out[-1].size==decoded[0].address,'Unexplained compiled instruction gap')
            out.extend(decoded)
        elif re.match(r'^\s*[0-9a-f]+:',line):
            require(re.match(r'^\s*[0-9a-f]+:\s+(?:DIR32|DISP32|SECREL32|SECTION)\b',line,re.I) is not None,
                    'Unexplained compiled bytes or relocation')
    require(out,'No compiled instructions')
    return out

def targets(i,address):
    return re.fullmatch(r'(?:0|0x[0-9a-f]+)',i.op_str) is not None and int(i.op_str,0)==address

def successors(i,known):
    if i.mnemonic=='ret':require(not i.op_str,'Unexpected callee cleanup');return []
    jump=i.group(capstone.CS_GRP_JUMP) or i.mnemonic.startswith('loop')
    if jump:
        require(re.fullmatch(r'(?:0|0x[0-9a-f]+)',i.op_str) is not None,'Unsupported indirect/far transfer')
        edges=[int(i.op_str,0)]
        if i.mnemonic!='jmp':edges.append(i.address+i.size)
    else:
        require(not any(i.group(g) for g in (capstone.CS_GRP_RET,capstone.CS_GRP_INT,capstone.CS_GRP_IRET,capstone.CS_GRP_PRIVILEGE)) and
                i.mnemonic not in ('ud2','xbegin','syscall','sysenter','sysret','sysexit','into'),
                'Unsupported control transfer')
        edges=[i.address+i.size]
    require(all(e in known for e in edges),'Transfer/fallthrough escapes instruction boundaries')
    return edges

def sample_prefix(ins,sample_address):
    by_address={i.address:i for i in ins}
    sample=[i for i in ins if i.mnemonic=='call' and targets(i,sample_address)]
    require(len(sample)==1,'Missing/ambiguous fog sample')
    following={};reverse={};reachable=set();todo=[ins[0].address]
    while todo:
        n=todo.pop()
        if n in reachable:continue
        reachable.add(n);following[n]=successors(by_address[n],set(by_address))
        for e in following[n]:reverse.setdefault(e,[]).append(n)
        todo.extend(following[n])
    reaches_sample=set();todo=[sample[0].address]
    while todo:
        n=todo.pop()
        if n in reaches_sample:continue
        reaches_sample.add(n);todo.extend(reverse.get(n,[]))
    require(ins[0].address in reaches_sample,'Fog sample unreachable from entry')
    prefix=(reachable & reaches_sample)-{sample[0].address}
    for address in prefix:plain_integer(by_address[address])
    return len(prefix)

def unique(b,suffix):
    found=[v for n,v in b.items() if n.endswith(suffix)]
    require(len(found)==1,'Missing/ambiguous compiled '+suffix)
    return found[0]

def native(game):
    result={}
    for name,pin in PINS.items():
        p=game/'Bin'/name;require(hashlib.sha256(p.read_bytes()).hexdigest()==pin,'Unsupported native '+name)
        with pefile.PE(str(p)) as pe:
            require(pe.FILE_HEADER.Machine==0x14c,'Expected native x86 '+name)
            base=pe.OPTIONAL_HEADER.ImageBase
            if name=='Engine.dll':
                exports={(s.name or b'').decode():s.address for s in pe.DIRECTORY_ENTRY_EXPORT.symbols}
                require(exports.get('?shaGetFogFactors@SeriousEngine@@YA?AVVector4f@1@XZ')==0x770b0,'Fog output ABI/export changed')
                code=decode(pe.get_data(0x770b0,0x129),base+0x770b0)
                require(code[-1].mnemonic=='ret' and not code[-1].op_str,'Fog no longer uses caller cleanup')
            else:
                helper=decode(pe.get_data(0x6920,0x5a),base+0x6920)
                require(helper[-1].mnemonic=='ret' and not helper[-1].op_str,'Slots helper no longer cdecl')
                call=decode(pe.get_data(0xf4fa,5),base+0xf4fa)
                require(len(call)==1 and call[0].mnemonic=='call' and call[0].op_str==hex(base+0x6920),'Wrong slots return boundary')
                table=pe.get_data(0x2834c,12)
                require(table==b'\x17\0\0\0\x07\0\0\0\x08\0\0\0','Wrong slots table')
                imports={(s.name or b'').decode():s.address for d in pe.DIRECTORY_ENTRY_IMPORT for s in d.imports}
                fog=imports.get('?shaGetFogFactors@SeriousEngine@@YA?AVVector4f@1@XZ')
                call=decode(pe.get_data(0xfc84,6),base+0xfc84)
                require(fog and len(call)==1 and call[0].mnemonic=='call' and
                        call[0].op_str=='dword ptr ['+hex(fog)+']','Wrong fog return boundary')
                code=decode(pe.get_data(0xf4ff,0xfc84-0xf4ff),base+0xf4ff)
                require(code[-1].address+code[-1].size==base+0xfc84,'Native producer boundary split')
                require(not any(i.mnemonic in ('call','fldcw','fninit','finit','ldmxcsr','fxrstor','xrstor') for i in code),
                        'Native producer interval gained call/control-state write')
                result['producer_instructions']=len(code)
    return result

def verify(game,obj):
    facts=native(game);b=bodies(obj)
    sample=unique(b,'captureProjectionSnapshot(ss2vr::IdleProjectionSnapshot&)')
    facts['sampler']=verify_sample(instructions(sample))
    sample_address=instructions(sample)[0].address
    slots=unique(b,'projectionSlots(int const*)');fog=unique(b,'projectionFog(void*)')
    for wrapper in (slots,fog):
        require(wrapper.count('DISP32\tss2vrNativeFinally')==1,'Wrapper lost native unwind boundary')
        require(not re.search(r'\bret\s+0x[1-9a-f]',wrapper),'Wrapper changed caller cleanup')
    hot=instructions(slots);calls=[i for i in hot if i.mnemonic in ('call','jmp') and targets(i,sample_address)]
    require(len(calls)==1,'Missing/ambiguous helper sample')
    # GCC may tail-jump to the pure sampler after restoring the wrapper frame;
    # the sampler then returns directly to the original native caller.
    by_address={i.address:i for i in hot};todo=[] if calls[0].mnemonic=='jmp' else [calls[0].address+calls[0].size]
    seen=set();returned=calls[0].mnemonic=='jmp'
    while todo:
        address=todo.pop()
        if address in seen:continue
        seen.add(address);i=by_address.get(address);require(i is not None,'Helper sampling tail escaped wrapper')
        plain_integer(i)
        if i.mnemonic=='ret':returned=True;continue
        todo.extend(successors(i,set(by_address)))
    require(returned,'Helper sampling tail never returns')
    facts['fog_prefix_instructions']=sample_prefix(instructions(fog),sample_address)
    symbols=subprocess.check_output(['objdump','-tC',str(obj)],text=True)
    for fn,slot in (('projectionSlots(int const*)','originalProjectionSlots'),('projectionFog(void*)','originalProjectionFog')):
        location=re.findall(r'0x([0-9a-f]+) ss2vr::game::remote_render::\(anonymous namespace\)::'+slot+r'$',symbols,re.M)
        require(len(location)==1,'Missing trampoline slot '+slot)
        runs=[v for n,v in b.items() if fn in n and '::Context::run(void*)' in n]
        original=r'\b(?:call|jmp)\s+DWORD PTR ds:'+re.escape(hex(int(location[0],16)))+r'\b'
        require(sum(len(re.findall(original,v)) for v in runs)==1,'Expected one original trampoline call-site: '+fn)
    facts.update(object_sha256=hashlib.sha256(obj.read_bytes()).hexdigest(),native_execution=False,
                 reference_replaced=False,single_original_call_site=True,unwind_boundary_presence=True,
                 forwarding_behavior_requires_source_review=True,
                 scope='pinned bookends, integer sampling/tails, cdecl and unwind/call-site presence; not forwarding behavior/runtime/cache provenance')
    return facts

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--game',type=Path,required=True);p.add_argument('--object',type=Path,required=True)
    a=p.parse_args()
    try:print(json.dumps(verify(a.game,a.object),indent=2))
    except (OSError,ValueError,subprocess.CalledProcessError) as e:p.exit(1,str(e)+'\n')
if __name__=='__main__':main()
