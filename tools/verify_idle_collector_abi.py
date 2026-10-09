#!/usr/bin/env python3
"""Pinned idle-query boundaries and actual x86 observer entry; no native execution."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import capstone
import pefile

PIN='da6efc9f72637eb3b6f48eadca2107be89b09c00618b6e72d5d3632938a7d851'
ROOT=Path(__file__).resolve().parents[1]

def require(ok,message):
    if not ok:raise ValueError(message)

def verify_opt_in_source(remote,engine,policy):
    # Bounded source contract, not a compiled dominance/lifetime proof.
    def body(text,marker,immediate=False):
        start=text.find(marker)
        require(start>=0,'Missing opt-in selector extent')
        begin=text.find('{',start);require(begin>=0,'Missing selector body')
        if immediate:require(begin==start+len(marker),'Opt-in guard does not immediately own its block')
        depth=0
        for end in range(begin,len(text)):
            if text[end]=='{':depth+=1
            elif text[end]=='}':
                depth-=1
                if depth==0:return text[begin+1:end]
        raise ValueError('Unclosed selector body')
    compact=lambda text:re.sub(r'\s+','',text)
    guard='if(idleProbeWeaponSupported(selectedIdleProbeWeapon()))'
    normalized=compact(remote)
    guarded=body(normalized,guard,immediate=True)
    require(len(re.findall(r'install\(engine,0xbbf0,',normalized))==1,
            'Original-End hook must occur once in its guarded source extent')
    require(re.search(r'install\(engine,0xbbf0,',guarded) and 'originalAnimationEnd' in guarded,
            'Original-End hook is not inside the selected opt-in extent')
    selector=compact(body(engine,'int selectedIdleProbeWeapon() noexcept'))
    require('GetEnvironmentVariableW(L"SS2VR_LAB_IDLE_WEAPON",value,3)' in selector and
            'returnn<3?idleProbeWeaponId(std::wstring_view(value,n)):-1;' in selector and
            'returnselected;' in selector,'Changed bounded shared opt-in selector')
    pure=compact(body(policy,'inline int idleProbeWeaponId('))
    supported=compact(body(policy,'inline bool idleProbeWeaponSupported('))
    require(pure=='returnvalue==L"1"?1:value==L"13"?13:-1;' and
            supported=='returnid==1||id==13;','Opt-in selector admits unsupported IDs')

def verify(game,obj):
    path=game/'Bin/Engine.dll'
    require(hashlib.sha256(path.read_bytes()).hexdigest()==PIN,'Unsupported Engine fingerprint')
    with pefile.PE(str(path)) as pe:
        require(pe.FILE_HEADER.Machine==0x14c,'Expected native x86 Engine')
        exports={(s.name or b'').decode():s.address for s in pe.DIRECTORY_ENTRY_EXPORT.symbols}
        require(exports.get('?aniEndAnimationQuery@SeriousEngine@@YAXPAVCAnimQueue@1@@Z')==0xbbf0,
                'Original End export or cdecl signature changed')
        base=pe.OPTIONAL_HEADER.ImageBase
        decoder=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
        def instruction(rva,mnemonic,operand):
            i=next(decoder.disasm(pe.get_data(rva,15),base+rva),None)
            require(i is not None and (i.mnemonic,i.op_str)==(mnemonic,operand),f'Native boundary changed: {rva:x}')
        instruction(0xddde8,'call',f'0x{base+0xbbf0:x}')
        instruction(0xddded,'add','esp, 4')
        instruction(0xbc3a,'ret','')
        instruction(0xdd5e,'mov',f'dword ptr [0x{base+0x2d92a4:x}], ecx')
        instruction(0xbc11,'call','dword ptr [eax + 0xc]')
        instruction(0xbc33,'mov',f'dword ptr [0x{base+0x2d92c8:x}], ebx')
        instruction(0xd1936,'mov','ecx, dword ptr [eax]')
        instruction(0x3ae6,'mov','eax, dword ptr [eax + 4]')
        instruction(0x3af6,'mov','eax, dword ptr [eax + 8]')
        instruction(0x3b06,'fld','dword ptr [eax + 0xc]')
    source=(ROOT/'src/game/remote_render.cpp').read_text()
    verify_opt_in_source(source,(ROOT/'src/game/engine.cpp').read_text(),
                         (ROOT/'src/common/idle_probe_selection.hpp').read_text())
    require('nativeIdleQueryBorrow(caller,engineBase+0xddded' in source,'Observer uses wrong return-site gate')
    assembly=subprocess.check_output(['objdump','-drC','-Mintel',str(obj)],text=True)
    require('file format pe-i386' in assembly,'Expected compiled x86 observer')
    parts=re.split(r'^[0-9a-f]+ <(.+)>:\n',assembly,flags=re.M)
    bodies=dict(zip(parts[1::2],parts[2::2]))
    entry=[b for n,b in bodies.items() if n.endswith('animationEnd(void*)') and 'clone' not in n]
    require(len(entry)==1,'Missing or ambiguous compiled End wrapper')
    require(entry[0].count('DISP32\tss2vrNativeFinally')==1,'End wrapper lost native unwind boundary')
    require(not re.search(r'\bret\s+0x[1-9a-f]',entry[0]),'End wrapper changed cdecl stack convention')
    require(re.search(r'\bret\s*$',entry[0],re.M) is not None,'End wrapper lacks plain cdecl return')
    symbols=subprocess.check_output(['objdump','-tC',str(obj)],text=True)
    slots=re.findall(r'0x([0-9a-f]+) ss2vr::game::remote_render::\(anonymous namespace\)::originalAnimationEnd$',symbols,re.M)
    require(len(slots)==1,'Missing original-End trampoline slot')
    runs=[b for n,b in bodies.items() if 'animationEnd(void*)' in n and '::Context::run(void*)' in n]
    require(runs,'Missing actual native-finally End body')
    original_call=r'\b(?:call|jmp)\s+DWORD PTR ds:'+re.escape(hex(int(slots[0],16)))+r'\b'
    require(sum(len(re.findall(original_call,b)) for b in runs)==1,'End body must call its original exactly once')
    require(any('0xddded' in b for b in runs),'Compiled body lost exact caller return-site comparison')
    palette=[b for n,b in bodies.items() if n.endswith('observeIdlePalette()') and 'clone' not in n]
    # Source order is separately reviewed. This gate does not pretend literal
    # presence alone proves dominance or lifetime of every memory read.
    from verify_remote_render_unwind import verify as verify_unwind
    verify_unwind(obj)
    return {'native_sha256':PIN,'object_sha256':hashlib.sha256(obj.read_bytes()).hexdigest(),
            'original_end_rva':'bbf0','call_site':'ddde8','return_site':'ddded',
            'cdecl_argument_bytes':4,'original_calls':1,'native_execution':False,
            'scope':'event header and compiled unwind; not resource/grasp or runtime acceptance'}

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--game',type=Path,required=True);p.add_argument('--object',type=Path,required=True)
    a=p.parse_args()
    try:print(json.dumps(verify(a.game,a.object),indent=2))
    except (ValueError,OSError,subprocess.CalledProcessError) as e:p.exit(1,str(e)+'\n')

if __name__=='__main__':main()
