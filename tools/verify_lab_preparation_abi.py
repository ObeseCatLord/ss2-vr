#!/usr/bin/env python3
"""Bounded production x86 preparation call/target-branch gate; no native execution.

This covers emitted argument/stack conventions and target dispatch. Native
ownership/lifetime admission remains the separately reviewed source contract.
Unsupported compiler output rejects rather than being interpreted approximately.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import capstone


def require(ok,message):
    if not ok:raise ValueError(message)


def instructions(body):
    decoder=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
    result=[]
    for line in body.splitlines():
        m=re.match(r'^\s*([0-9a-f]+):\s+((?:[0-9a-f]{2} )+)',line)
        if not m:continue
        address=int(m[1],16);raw=bytes.fromhex(m[2]);decoded=list(decoder.disasm(raw,address))
        require(len(decoded)==1 and decoded[0].size==len(raw),'Ambiguous or wrapped instruction')
        result.append((address,decoded[0].mnemonic,decoded[0].op_str))
    require(result,'Empty preparation body')
    for a,b in zip(result,result[1:]):require(a[0]<b[0],'Nonordered instruction extent')
    return result


def symbol_offset(symbols,name):
    found=re.findall(r'0x([0-9a-f]+) ss2vr::game::'+re.escape(name)+r'$',symbols,re.M)
    require(len(found)==1,'Missing/ambiguous preparation slot '+name)
    return int(found[0],16)


def callback_index(code,slot):
    found=[i for i,(_,mn,op) in enumerate(code) if mn=='call' and op==f'dword ptr [{hex(slot)}]']
    require(len(found)==1,'Missing/ambiguous native callback')
    return found[0]


def target_prefix(code,end,slot,register):
    candidates=[i for i in range(end) if code[i][1:] == ('mov',f'{register}, dword ptr [{hex(slot)}]')]
    require(candidates,'Missing target pointer load')
    start=candidates[-1]
    regs={register:'target'};stack={}
    for address,mn,op in code[start+1:end]:
        if mn.startswith('j'):
            require(int(op,16)>code[end][0],'Target-call prefix branch can reenter argument staging')
            continue
        require(mn not in ('call','ret','push','pop') and not (op.startswith('esp,') or op.startswith('ebp,')),
                'Target argument prefix contains unsupported call/stack mutation')
        if mn=='xor':
            args=op.split(', ')
            if len(args)==2 and args[0]==args[1]:regs[args[0]]=0;continue
        if mn!='mov':
            # Comparisons/test instructions leave provenance unchanged. Other
            # arithmetic in the emitted TLS bookkeeping invalidates its destination.
            if mn not in ('cmp','test','nop'):regs.pop(op.split(',')[0],None)
            continue
        dest,source=op.split(', ',1)
        if source in regs:value=regs[source]
        elif re.fullmatch(r'0x[0-9a-f]+|[0-9]+',source):value=int(source,0)
        else:
            indirect=re.fullmatch(r'dword ptr \[([a-z]+)(?: \+ (0x[0-9a-f]+|[0-9]+))?\]',source)
            if indirect and regs.get(indirect[1])=='target':value=('target',int(indirect[2] or '0',0))
            else:value=('load',source)
        outgoing=re.fullmatch(r'dword ptr \[esp(?: \+ (0x[0-9a-f]+|[0-9]+))?\]',dest)
        if outgoing:stack[int(outgoing[1] or '0',0)]=value
        elif re.fullmatch(r'e[a-z]{2}',dest):regs[dest]=value
    return regs,stack


def verify_extent(assembly,symbols,targets):
    parts=re.split(r'^[0-9a-f]+ <(.+)>:\n',assembly,flags=re.M)
    bodies=dict(zip(parts[1::2],parts[2::2]))
    names=[n for n in bodies if n.endswith('runPostSimulationSniper(ss2vr::game::SimulationInterval&)::{lambda()#1}::operator()() const')]
    require(len(names)==1,'Missing/ambiguous production preparation body')
    code=instructions(bodies[names[0]])
    slots={n:symbol_offset(symbols,n) for n in ('labPreparation','labSelectSniper','labSaveSniper','nativeZoomFlag','canChange','labAutoShotgunVtable')}
    select=callback_index(code,slots['labSelectSniper']);save=callback_index(code,slots['labSaveSniper'])
    zoom=callback_index(code,slots['nativeZoomFlag']);change=callback_index(code,slots['canChange'])
    regs,stack=target_prefix(code,select,slots['labPreparation'],'ecx')
    require([stack.get(i) for i in (0,4,8,12)]==[('target',0),1,0,0],'Native selection arguments differ')
    receivers=[op for _,mn,op in code[max(0,change-3):change] if mn=='mov' and op.startswith('ecx, dword ptr [ebp - ')]
    require(len(receivers)==1 and regs.get('ecx')==('load',receivers[0].split(', ',1)[1]),
            'Selection receiver differs from CanChange player receiver')
    after=code[select+1:]
    before_next=after[:next((i for i,(_,mn,_) in enumerate(after) if mn=='call'),len(after))]
    stack_adjustments=[(mn,op) for _,mn,op in before_next if op.startswith('esp,') or mn in ('push','pop')]
    require(stack_adjustments==[('sub','esp, 0x10')],'Selection lost four-argument callee-pop repair')
    regs,stack=target_prefix(code,save,slots['labPreparation'],'ecx')
    require(stack.get(0)==('target',12),'Native cdecl save argument differs from target resource path')
    after=code[save+1:];before_next=after[:next((i for i,(_,mn,_) in enumerate(after) if mn=='call'),len(after))]
    require(not any(op.startswith('esp,') or mn in ('push','pop') for _,mn,op in before_next),
            'Save call has unexpected callee-pop repair')
    # Actual descriptor flags select the emitted branch. Follow each successor
    # until leaving the finite readiness region; reject cycles/unknown transfers.
    guards=[i for i in range(zoom) if code[i][1:] == ('cmp','byte ptr [eax + 0x14], 0')]
    require(guards,'Missing target-specific zoom guard');guard=guards[-1]
    require(code[guard+1][1]=='je','Zoom guard no longer skips sniper block on zero')
    loads=[i for i in range(guard) if code[i][1:] == ('mov',f'eax, dword ptr [{hex(slots["labPreparation"])}]')]
    require(loads,'Zoom flag no longer comes from selected descriptor')
    require(not any(mn in ('call','ret') or (op.startswith('eax,') and mn not in ('cmp','test'))
                    for _,mn,op in code[loads[-1]+1:guard]),'Selected descriptor provenance was clobbered')
    require(targets=={2:False,13:True},'Compiled target descriptor ID/zoom flags differ')
    index={a:i for i,(a,_,_) in enumerate(code)};alternate=int(code[guard+1][2],16)
    require(alternate in index and alternate>code[zoom][0],'AutoSG branch does not skip sniper zoom block')
    region_start=code[guard+2][0];region_end=alternate
    require(any('[edx + 0xd4]' in op for _,_,op in code[guard+2:index[alternate]]),'Sniper readiness field disappeared')
    copies=[i for i in (zoom-2,zoom-1) if code[i][1:]==('mov','ecx, edx')]
    require(len(copies)==1,'Sniper zoom receiver convention changed')
    for _,mn,op in code[copies[0]+1:zoom]:
        require(mn=='mov' and re.fullmatch(r'dword ptr \[ebp - 0x[0-9a-f]+\], edx',op),
                'Sniper zoom receiver clobbered after its accepted copy')
    # Positive class equality continues to owner validation. Its mismatch must
    # take the same explicit return epilogue as the common native-idle mismatch.
    common=[i for i in range(guard) if code[i][1:]==('cmp','dword ptr [edx + 0xb0], 1')]
    require(common and code[common[-1]+1][1]=='jne','Common idle mismatch polarity changed')
    failure=int(code[common[-1]+1][2],16)
    require(failure in index,'Common readiness failure escapes body')
    epilogue=code[index[failure]:]
    terminal=next((i for i,(_,mn,_) in enumerate(epilogue) if mn=='ret'),None)
    require(terminal is not None and terminal<=12,'Readiness failure has no bounded return epilogue')
    require(all(mn in ('lea','pop','nop') for _,mn,_ in epilogue[:terminal]) and
            epilogue[terminal][2]=='','Readiness failure is not the scalar return epilogue')
    i=index[alternate]
    require(code[i][1:]==('mov','eax, dword ptr [edx]') and
            code[i+1][1:]==('cmp',f'dword ptr [{hex(slots["labAutoShotgunVtable"])}], eax') and
            code[i+2][1:] == ('jne',hex(failure)),
            'AutoSG class mismatch polarity/destination differs from explicit readiness exit')
    # Both successors are now bounded: failure returns, equality advances to
    # the separately source-reviewed currentOwner callback with no new branch.
    success=code[i+3:]
    first_call=next((j for j,(_,mn,_) in enumerate(success) if mn=='call'),None)
    require(first_call is not None and first_call<=4,'AutoSG readiness lacks bounded owner revalidation')
    require(not any(mn.startswith('j') or mn=='ret' or '[edx + 0xd4]' in op
                    for _,mn,op in success[:first_call]),'AutoSG equality path can reenter sniper readiness')
    require(success[first_call][2]!=f'dword ptr [{hex(slots["nativeZoomFlag"])}]',
            'AutoSG equality calls sniper zoom')
    return {'selection_arguments':4,'selection_thiscall_receiver_matches_can_change':True,
            'selection_callee_pop_bytes':16,'save_cdecl_arguments':1,'id2_sniper_zoom_excluded':True,
            'scope':'compiled argument staging, stack repair and target readiness dispatch; not complete native lifetime proof',
            'runtime_executed':False}


def descriptor(obj,name):
    section='.rdata$_ZN5ss2vr'+str(len(name))+name+'E'
    dump=subprocess.check_output(['objdump','-s','-j',section,str(obj)],text=True)
    data=b''
    for line in dump.splitlines():
        m=re.match(r'^\s*[0-9a-f]+\s+((?:[0-9a-f]{8}\s+){1,4})',line)
        if m:data+=bytes.fromhex(m[1])
    require(len(data)>=24 and data[20] in (0,1),'Missing/invalid native target descriptor')
    return int.from_bytes(data[:4],'little'),bool(data[20])


def verify(obj):
    assembly=subprocess.check_output(['objdump','-drC','-Mintel','--insn-width=16',str(obj)],text=True)
    symbols=subprocess.check_output(['objdump','-tC',str(obj)],text=True)
    require('file format pe-i386' in assembly,'Expected production x86 object')
    targets=dict(descriptor(obj,n) for n in ('LabAutoShotgunTarget','LabSniperTarget'))
    result=verify_extent(assembly,symbols,targets)
    result['object_sha256']=hashlib.sha256(obj.read_bytes()).hexdigest()
    return result

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--object',required=True,type=Path)
    try:print(json.dumps(verify(parser.parse_args().object),indent=2))
    except (ValueError,OSError,subprocess.SubprocessError) as e:parser.exit(1,str(e)+'\n')
