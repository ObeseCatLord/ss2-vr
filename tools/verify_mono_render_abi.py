#!/usr/bin/env python3
"""Finite original-render receiver/forwarding check; no native execution."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
from verify_ride_control_abi import (bodies,decoded_nodes,extent,finally_delegates,forwards,instructions,
                                    paths,require,symbol_offset,symbol_section,scalar_op,fault_edges)
from verify_saved_tls import State,pointer
import re

class CallbackState(State):
    def write(self,operand,value,relocations,state_offset):
        if value[:1]==('constant',) and any(r.endswith('dir32\t.text') for r in relocations):
            value=('function',value[1])
        super().write(operand,value,relocations,state_offset)

def verify_completion_sequence(body,events):
    """All bounded normal paths must visit these exact events in order.

    Lexical address order is insufficient: branches can enter argument setup
    midway or execute completion before the original despite one call each.
    """
    code=instructions(body);by_address={a:i for i,(a,_,_) in enumerate(code)}
    expected={address:i for i,address in enumerate(events)}
    require(len(expected)==len(events),'Completion events alias')
    faults=fault_edges(body);pending=[(code[0][0],0,frozenset())];seen=set();returns=0
    while pending:
        address,phase,ancestors=pending.pop();state=(address,phase)
        require(state not in ancestors,'Unbounded normal mono completion path')
        if state in seen:continue
        seen.add(state);require(address in by_address,'Mono completion flow escapes extent')
        ancestors=ancestors|{state};i=by_address[address];_,mn,op=code[i]
        if address in expected:
            require(expected[address]==phase,'Mono path skips/reorders original, argument setup or completion')
            phase+=1
        if mn=='ret':
            require(phase==len(events),'Mono returns before ordered completion');returns+=1;continue
        if address in faults:
            require(mn=='jmp','Unexpected mono GNU-failure edge');continue
        if mn.startswith('j'):
            require(re.fullmatch(r'0x[0-9a-f]+',op),'Unknown mono completion branch')
            pending.append((int(op,16),phase,ancestors))
            if mn=='jmp':continue
        require(i+1<len(code),'Mono completion falls outside extent')
        pending.append((code[i+1][0],phase,ancestors))
    require(returns,'No ordered normal mono completion')


def verify_extent(assembly,symbols):
    table=bodies(assembly)
    entry=table.get('ss2vr::game::presentedRender(void*)')
    require(entry is not None,'Missing native mono-render entry')
    entry_code=instructions(entry)
    paths(entry,finally_delegates(entry),'')
    receiver=('native_receiver',)
    state=CallbackState({'esp':pointer('entry',0),'ecx':receiver})
    for address,mn,op,relocations in decoded_nodes(entry):
        if mn=='call':break
        state.step(scalar_op(mn,op),[f'{a:x}: {kind}\t{target}' for a,kind,target in relocations], -1)
    stack=state.register('esp')
    context=state.memory.get((pointer(stack[1],stack[2]+8),4))
    require(context and context[:1]==('pointer',),'Missing render finally context')
    closure=state.memory.get((context,4))
    require(closure and closure[:1]==('pointer',),'Missing original-render capture')
    reference=state.memory.get((pointer(closure[1],closure[2]+4),4))
    require(reference and state.memory.get((reference,4))==receiver,
            'Original-render capture does not originate in incoming thiscall ECX')
    run=extent(table,'presentedRender(void*)','::Context::run(void*)')
    finish=extent(table,'presentedRender(void*)','::Context::finish(void*, int)')
    for location,body,relocation_offset in (('esp',run,3),('esp + 4',finish,4)):
        expected=instructions(body)[0][0]
        displacement=0 if location=='esp' else 4
        require(state.memory.get((pointer(stack[1],stack[2]+displacement),4))==('function',expected),
                'Final callback argument differs from inspected relocated function')
        binding=[(a,relocs) for a,mn,op,relocs in decoded_nodes(entry)
                 if (mn,op)==('mov',f'dword ptr [{location}], {hex(expected)}')]
        require(len(binding)==1 and binding[0][1]==[(binding[0][0]+relocation_offset,'dir32','.text')],
                'Finally callback address/section differs from inspected body')
    slot=symbol_offset(symbols,'originalRender')
    transfers=forwards(run,slot,symbol_section(symbols,'originalRender'))
    require(len(transfers)==1,'Missing unique original native renderer')
    paths(run,transfers,'')
    code=instructions(run)
    index=next(i for i,item in enumerate(code) if item[0] in transfers)
    require(code[index-2][1:]==('mov','eax, dword ptr [ebx + 4]') and
            code[index-1][1:]==('mov','ecx, dword ptr [eax]'),
            'Original-render receiver not loaded from checked capture')
    prefix=[(mn,op) for _,mn,op in code[:index-2]]
    require(prefix[:6]==[('push','ebp'),('mov','ebp, esp'),('push','esi'),('push','ebx'),
                        ('and','esp, 0xfffffff0'),('sub','esp, 0x20')] and
            prefix[6:9]==[('mov','eax, dword ptr [ebp + 8]'),('mov','edx, dword ptr [0]'),
                          ('mov','ebx, dword ptr [eax]')] and
            decoded_nodes(run)[7][3]==[(code[7][0]+2,'dir32','_tls_index')],
            'Original-render context/closure load changed')
    supported={'push','pop','mov','sub','and','lea','test','cmp','js','je','jne','jmp','call','ret','xor','or','nop','xchg'}
    for i,(_,mn,op) in enumerate(code):
        require(mn in supported,'Unsupported renderer instruction')
        if mn=='xchg':require(op=='ax, ax','Only compiler no-op XCHG is admitted')
        if op.split(',')[0] in ('ebx','bx','bl','bh') and mn!='push':
            require((i==8 and (mn,op)==('mov','ebx, dword ptr [eax]')) or
                    (i>index and mn=='pop'), 'Original-render closure base overwritten')
    completion_name='ss2vr::game::remote_render::completeMonoPresentation(unsigned int, bool)'
    completion=[(a,mn,op,relocs) for a,mn,op,relocs in decoded_nodes(run)
                if any(target==completion_name for _,_,target in relocs)]
    require(len(completion)==1,'Missing unique normal mono completion')
    a,mn,op,relocs=completion[0]
    require(a>code[index][0] and mn=='call' and op==hex(a+5) and
            relocs==[(a+1,'DISP32',completion_name)],'Mono completion target/addend/order differs')
    paths(run,{a},'')
    complete_index=next(i for i,item in enumerate(code) if item[0]==a)
    require([x[1:] for x in code[complete_index-4:complete_index]]==[
            ('mov','dword ptr [esp + 4], 1'),('mov','eax, dword ptr [ebx]'),
            ('mov','eax, dword ptr [eax]'),('mov','dword ptr [esp], eax')],
            'Mono completion must consume captured owner after normal original return')
    verify_completion_sequence(run,[code[index][0]]+
                               [x[0] for x in code[complete_index-4:complete_index]]+[a])
    retirement=[(a,mn,op,relocs) for a,mn,op,relocs in decoded_nodes(finish)
                if any(target=='ss2vr::game::remote_render::retirePresentation(unsigned int)'
                       for _,_,target in relocs)]
    require(len(retirement)==1,'Mono owner cleanup missing')
    a,mn,op,relocs=retirement[0]
    require(mn=='jmp' and op==hex(a+5) and relocs==[(a+1,'DISP32',
            'ss2vr::game::remote_render::retirePresentation(unsigned int)')],
            'Owner retirement target/addend differs')
    require(not any(mn.startswith('f') or any(r in op for r in ('xmm','ymm','zmm'))
                    for _,mn,op in instructions(finish)), 'Mono cleanup gained FP work')
    return {'once_only_original_normal_paths':True,'native_thiscall_ecx_capture_verified':True,
            'return_opcode_stack_pop_bytes':0,'owner_retirement_connected':True,
            'normal_mono_completion_connected':True,
            'scope':'finite entry/capture/ECX load/original relocation and normal forwarding; native lifetime and owner retirement semantics are source/portable checks',
            'runtime_executed':False}


def verify(obj):
    assembly=subprocess.check_output(['objdump','-drC','-Mintel','--insn-width=16',str(obj)],text=True)
    symbols=subprocess.check_output(['objdump','-tC',str(obj)],text=True)
    result=verify_extent(assembly,symbols)
    result['object_sha256']=hashlib.sha256(obj.read_bytes()).hexdigest()
    return result


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--object',type=Path,required=True)
    try:print(json.dumps(verify(p.parse_args().object),indent=2))
    except (ValueError,OSError,subprocess.SubprocessError) as e:p.exit(1,str(e)+'\n')
