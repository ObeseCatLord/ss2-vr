#!/usr/bin/env python3
"""Finite ride-observer forwarding/cleanup gate; never executes native code."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import capstone
from verify_lab_preparation_abi import instructions,symbol_offset,require
from verify_saved_tls import State,UNKNOWN,pointer

def decoded_nodes(body):
    """Decode bytes, retaining each operand's actual COFF relocation."""
    code=instructions(body);relocations={a:[] for a,_,_ in code};operands={}
    decoder=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32);decoder.detail=True
    for line in body.splitlines():
        m=re.match(r'^\s*([0-9a-f]+):\s+((?:[0-9a-f]{2} )+)',line)
        if m:
            address=int(m[1],16);raw=bytes.fromhex(m[2]);ins=next(decoder.disasm(raw,address))
            operands[address]={address+offset for offset,size in
                               ((ins.disp_offset,ins.disp_size),(ins.imm_offset,ins.imm_size)) if offset and size==4}
    for line in body.splitlines():
        m=re.match(r'^\s*([0-9a-f]+): (dir32|secrel32|DISP32)\s+(.+)$',line)
        if m:
            address=int(m[1],16)
            owners=[a for a,_,_ in code if a<=address]
            require(owners,'Relocation has no owning instruction')
            owner=max(owners)
            require(address in operands[owner],'Relocation is not an encoded four-byte operand')
            relocations[owner].append((address,m[2],m[3]))
    return [(a,mn,op,relocations[a]) for a,mn,op in code]

def symbol_section(symbols,name):
    entries=re.findall(r'^.*\(sec\s+(\d+)\).*0x[0-9a-f]+ '+re.escape('ss2vr::game::'+name)+r'$',symbols,re.M)
    require(len(entries)==1,'Missing original symbol section')
    names=re.findall(r'^.*\(sec\s+'+entries[0]+r'\).*0x00000000 (\.[a-zA-Z0-9$]+)$',symbols,re.M)
    require(len(names)==1,'Missing original section identity')
    return names[0]

def forwards(body,slot,section):
    result=set()
    for a,mn,op,relocs in decoded_nodes(body):
        if mn in ('call','jmp') and op==f'dword ptr [{hex(slot)}]':
            require(relocs==[(a+2,'dir32',section)],'Original forwarding relocation differs')
            result.add(a)
    return result

def finally_delegates(body):
    delegates=set()
    for address,mn,op,relocs in decoded_nodes(body):
        if any(target=='ss2vrNativeFinally' for _,_,target in relocs):
            require(mn=='call' and op==hex(address+5) and
                    relocs==[(address+1,'DISP32','ss2vrNativeFinally')],
                    'Native-finally delegate opcode/relocation addend differs')
            delegates.add(address)
    require(len(delegates)==1,'Missing unique native-finally delegate')
    return delegates

def scalar_op(mn,op):
    # Adapt byte-decoded Capstone operands to the existing bounded scalar state.
    args=[]
    for part in op.split(', ') if op else []:
        part=part.replace('dword ptr ','DWORD PTR ').replace('byte ptr ','BYTE PTR ')
        part=part.replace('fs:[0x2c]','fs:0x2c')
        part=re.sub(r'\[([^]]+)\]',lambda m:'['+m[1].replace(' ','')+']',part)
        part=re.sub(r'(?<=[+-])([0-9]+)(?=\])',lambda m:hex(int(m[1])),part)
        if re.fullmatch('[0-9]+',part):part=hex(int(part))
        if part=='DWORD PTR [0]':part='DWORD PTR ds:0x0'
        args.append(part)
    return mn+' '+','.join(args)

class CleanupState(State):
    """Reuse the scalar verifier; add pointer-width TLS and concrete case flags."""
    def __init__(self,regs,memory,offset):
        super().__init__(regs,memory);self.offset=offset;self.restores=0;self.zero=None
    def write(self,operand,value,relocations,state_offset):
        match=re.fullmatch(r'DWORD PTR (.+)',operand)
        if match and self.address(match[1])==pointer('tls',self.offset):
            require(any('secrel32\t.tls$' in r for r in relocations),'TLS store lacks its section relocation')
            require(value==pointer('previous_observation',0),'TLS value is not the saved outer observation')
            self.restores+=1
            require(self.restores==1,'Repeated outer TLS restore')
            self.memory[(pointer('tls',self.offset),4)]=value
            return
        super().write(operand,value,relocations,-1)
    def step(self,op,relocations,state_offset):
        mn,*rest=op.split(' ',1);args=rest[0].split(',') if rest else []
        if mn in ('test','cmp'):
            left=self.read(args[0],relocations,-1);right=self.read(args[1],relocations,-1)
            require(left[:1]==right[:1]==('constant',),'Unknown cleanup condition')
            self.zero=(left[1]&right[1])==0 if mn=='test' else left[1]==right[1]
        elif mn=='xor' and args[0]==args[1]:
            self.set_register(args[0],('constant',0));self.zero=True
        else:
            super().step(op,relocations,-1)

def cleanup_cases(body,outer,offset,model=False):
    nodes=decoded_nodes(body);locations={a:i for i,(a,_,_,_) in enumerate(nodes)}
    cases=0
    for abnormal in (0,1):
        for failed in (0,1):
            for entered in ((1,) if outer else (0,1)):
                context=pointer('context',0);closure=pointer('closure',0);obs=pointer('observation',0)
                memory={(pointer('stack',4),4):context,(pointer('stack',8),4):('constant',abnormal),
                        (pointer('context',4),4):closure,(pointer('context',8),1):('constant',failed)}
                if outer:
                    memory[(closure,4)]=pointer('saved_reference',0)
                    memory[(pointer('saved_reference',0),4)]=pointer('previous_observation',0)
                    memory[(pointer('closure',4),4)]=obs
                else:
                    memory[(closure,4)]=pointer('entered_reference',0)
                    memory[(pointer('entered_reference',0),1)]=('constant',entered)
                    memory[(pointer('closure',4),4)]=pointer('observation_reference',0)
                    memory[(pointer('observation_reference',0),4)]=obs
                flags=((0x24,1),(0x25,0),(0x26,0),(0x27,0),(0x28,0)) if model else \
                      ((0x10,1),(0x12,1),(0x13,0),(0x14,0))
                for off,val in flags:
                    memory[(pointer('observation',off),1)]=('constant',val)
                if model:
                    memory[(pointer('observation',0x1c),4)]=pointer('native_renderable',0)
                    memory[(pointer('observation',0x20),4)]=pointer('native_instance',0)
                state=CleanupState({'esp':pointer('stack',0)},memory,offset);index=0;visited=set()
                while True:
                    require(index not in visited,'Cleanup cycle');visited.add(index)
                    a,mn,op,relocs=nodes[index]
                    if mn=='ret':
                        require(op=='' and state.register('esp')==pointer('stack',0),'Cleanup return stack differs')
                        if outer:
                            require(state.restores==1 and
                                    state.memory.get((pointer('tls',offset),4))==pointer('previous_observation',0),
                                    'Cleanup bypasses or corrupts saved TLS restore')
                        if abnormal or failed:
                            if model:
                                require(all(state.memory.get((pointer('observation',off),1))==('constant',1)
                                            for off in (0x25,0x28)) and
                                        all(state.memory.get((pointer('observation',off),4))==('constant',0)
                                            for off in (0x1c,0x20)),
                                        'Abnormal cleanup failed to invalidate model join')
                            else:
                                require(state.memory.get((pointer('observation',0x14),1))==('constant',1) and
                                        state.memory.get((pointer('observation',0x12),1))==('constant',0),
                                        'Abnormal cleanup failed to invalidate observation')
                        if not outer and entered:
                            require(state.memory.get((pointer('observation',0x24 if model else 0x10),1))==('constant',0),'Callback busy state survives')
                            if not abnormal and not failed:
                                require(state.memory.get((pointer('observation',0x26 if model else 0x13),1))==('constant',1),
                                        'Normal callback not marked returned')
                        if model and outer and not abnormal and not failed:
                            require(state.memory.get((pointer('observation',0x27),1))==('constant',1),
                                    'Normal outer getter not marked returned')
                        cases+=1;break
                    if mn.startswith('j'):
                        require(mn in ('je','jne','jmp') and re.fullmatch('0x[0-9a-f]+',op),'Unknown cleanup edge')
                        take=mn=='jmp'
                        if mn!='jmp':
                            require(state.zero is not None,'Unknown cleanup flags')
                            take=state.zero if mn=='je' else not state.zero
                        if take:
                            require(int(op,16) in locations,'Cleanup edge escapes extent')
                            index=locations[int(op,16)];continue
                    else:
                        state.step(scalar_op(mn,op),[f'{ra:x}: {kind}\t{target}' for ra,kind,target in relocs],offset)
                    index+=1;require(index<len(nodes),'Cleanup falls out')
    return cases

def entry_epilogues(callback,outer,slot,section):
    """Finite compiler-shape check, not a general whole-function ABI proof."""
    code=instructions(callback);locations={a:i for i,(a,_,_) in enumerate(code)}
    nodes={a:(mn,op,relocs) for a,mn,op,relocs in decoded_nodes(callback)}
    original=forwards(callback,slot,section);faults=fault_edges(callback)
    work=[(code[0][0],0)];seen=set();returns=0
    while work:
        address,delta=work.pop()
        if (address,delta) in seen:continue
        seen.add((address,delta));require(len(seen)<4096,'Entry stack proof exceeds budget')
        require(address in locations,'Entry stack path escapes extent')
        i=locations[address];mn,op,relocs=nodes[address]
        if address in faults:continue
        if mn in ('add','sub') and op.startswith('esp, '):
            amount=int(op.split(', ')[1],0);delta+=amount if mn=='add' else -amount
        elif mn=='call':
            if address in original:delta+=4
            else:
                require(len(relocs)==1 and
                        ((relocs[0][1:] == ('DISP32','ss2vrNativeFinally')) or
                         (relocs[0][1:] == ('dir32','_imp__GetCurrentThreadId@0'))),
                        'Unknown entry callee stack effect')
        elif mn in ('push','pop','leave') or re.match(r'esp(?:,|$)',op):
            raise ValueError('Unsupported entry stack mutation')
        if mn=='ret':
            require(delta==0 and op=='4','Callback entry stack does not restore incoming ESP')
            returns+=1;continue
        if mn.startswith('j'):
            require(re.fullmatch('0x[0-9a-f]+',op),'Unknown stack edge')
            work.append((int(op,16),delta))
            if mn=='jmp':continue
        require(i+1<len(code),'Stack path falls out');work.append((code[i+1][0],delta))
    require(returns,'Missing callback stack return')
    # The mounted thunk realigns ESP and returns using its saved incoming EDI
    # stack reference. Verify the accepted bookends, not arbitrary interior calls.
    code=instructions(outer)
    prefix=[('push','edi'),('lea','edi, [esp + 8]'),('and','esp, 0xfffffff8'),
            ('push','dword ptr [edi - 4]'),('push','ebp'),('mov','ebp, esp'),
            ('push','edi'),('push','esi'),('push','ebx')]
    require([item[1:] for item in code[:9]]==prefix,'Mounted stack capture shape differs')
    suffix=[('lea','esp, [ebp - 0xc]'),('pop','ebx'),('pop','esi'),('pop','edi'),
            ('pop','ebp'),('lea','esp, [edi - 8]'),('pop','edi'),('ret','4')]
    exits=[i for i,(_,mn,_) in enumerate(code) if mn=='ret']
    require(len(exits)==1 and [item[1:] for item in code[exits[0]-7:exits[0]+1]]==suffix,
            'Mounted saved-stack return shape differs')
    return returns

def bodies(assembly):
    parts=re.split(r'^[0-9a-f]+ <(.+)>:\n',assembly,flags=re.M)
    return dict(zip(parts[1::2],parts[2::2]))

def extent(table,fragment,suffix):
    found=[b for n,b in table.items() if fragment in n and n.endswith(suffix)]
    require(len(found)==1,'Missing/ambiguous ride-observer extent')
    return found[0]

def fault_edges(body):
    edges=set();last=None
    for line in body.splitlines():
        m=re.match(r'^\s*([0-9a-f]+):\s+(?:[0-9a-f]{2} )+',line)
        if m:last=int(m[1],16)
        elif 'DISP32\t.text.unlikely' in line:edges.add(last)
    return edges

def paths(body,transfers,ret_arg=None):
    code=instructions(body);by_address={a:i for i,(a,_,_) in enumerate(code)}
    faults=fault_edges(body);pending=[(code[0][0],0)];seen=set();returns=0
    while pending:
        address,count=pending.pop()
        if (address,count) in seen:continue
        seen.add((address,count));require(address in by_address,'Control flow escapes verified extent')
        i=by_address[address];_,mn,op=code[i]
        hit=address in transfers
        count+=int(hit);require(count<=1,'Path repeats native forward/TLS restore')
        if mn=='ret' or (mn=='jmp' and address in transfers):
            require(count==1,'Normal return bypasses native forward/TLS restore')
            if mn=='ret' and ret_arg is not None:require(op==ret_arg,'Native reference stack convention changed')
            returns+=1;continue
        if address in faults:
            require(mn=='jmp','Unexpected native GNU-failure edge')
            continue # Compiler catch section; no normal-return claim for faults.
        if mn.startswith('j'):
            require(re.fullmatch('0x[0-9a-f]+',op),'Unknown indirect branch')
            pending.append((int(op,16),count))
            if mn=='jmp':continue
        require(i+1<len(code),'Extent falls out without normal return')
        pending.append((code[i+1][0],count))
    require(returns,'No verified normal return')
    return returns

def verify_extent(assembly,symbols):
    table=bodies(assembly)
    slots={n:symbol_offset(symbols,n) for n in ('originalRideLookClamp','originalLookClamp')}
    sections={n:symbol_section(symbols,n) for n in slots}
    callback=extent(table,'@_ZN5ss2vr4gameL21observedRideLookClamp','@12')
    outer=extent(table,'@_ZN5ss2vr4gameL16mountedLookClamp','@12')
    cleanup_count=0
    for name,slot_name,entry in (('observedRideLookClamp','originalRideLookClamp',callback),
                             ('mountedLookClamp','originalLookClamp',outer)):
        slot=slots[slot_name];section=sections[slot_name]
        run=extent(table,name+'(void*','::Context::run(void*)')
        paths(run,forwards(run,slot,section))
        # Finally delegates to the already proved original-forward body. Resolve
        # its actual call instruction from the relocation, not a raw call count.
        delegates=finally_delegates(entry)
        paths(entry,forwards(entry,slot,section)|delegates,'4')
        finish=extent(table,name+'(void*','::Context::finish(void*, int)')
        cleanup=instructions(finish)
        require(not any(mn=='call' or mn.startswith('f') or re.search(r'\b(?:xmm|ymm|zmm)',op)
                        for _,mn,op in cleanup),'Cleanup contains callback/allocation/FP work')
        if name=='mountedLookClamp':
            location=re.findall(r'0x([0-9a-f]+) ss2vr::game::activeRideControlObservation$',symbols,re.M)
            require(len(location)==1,'Missing TLS observation slot')
            cleanup_count+=cleanup_cases(finish,True,int(location[0],16))
        else:
            cleanup_count+=cleanup_cases(finish,False,0)
    run=extent(table,'observedRideLookClamp(void*','::Context::run(void*)')
    require(not any(name in run for name in ('hvHandleToPointer','GetModelInstance','GetModelRenderable','GetProperties')),
            'Scalar sampler gained a native object/resource getter')
    stack_returns=entry_epilogues(callback,outer,slots['originalRideLookClamp'],sections['originalRideLookClamp'])
    return {'normal_paths_forward_once':True,'native_reference_stack_bytes':4,
            'outer_tls_restored_each_cleanup_return':True,'scalar_abort_invalidation':True,
            'cleanup_cases_checked':cleanup_count,'original_relocation_identity_verified':True,
            'callback_entry_balanced_returns':stack_returns,'mounted_stack_bookends_checked':True,
            'scope':'normal-path original counts, callback entry ESP, mounted stack bookends and finite cleanup case/value flow; argument/capture origin and mounted interior stack effects remain source review',
            'runtime_executed':False}

def verify(obj):
    asm=subprocess.check_output(['objdump','-drC','-Mintel','--insn-width=16',str(obj)],text=True)
    symbols=subprocess.check_output(['objdump','-tC',str(obj)],text=True)
    require('file format pe-i386' in asm,'Expected x86 object')
    result=verify_extent(asm,symbols);result['object_sha256']=hashlib.sha256(obj.read_bytes()).hexdigest()
    return result
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--object',type=Path,required=True)
    try:print(json.dumps(verify(p.parse_args().object),indent=2))
    except (ValueError,OSError,subprocess.SubprocessError) as e:p.exit(1,str(e)+'\n')
