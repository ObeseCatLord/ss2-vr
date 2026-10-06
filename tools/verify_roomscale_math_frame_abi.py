#!/usr/bin/env python3
"""Inspect compiled x86 FP containment; never execute Windows code."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess

def require(ok, message):
    if not ok:
        raise ValueError(message)

def verify(obj):
    assembly = subprocess.check_output(['objdump', '-drC', '-Mintel', str(obj)], text=True)
    require('file format pe-i386' in assembly, 'Expected x86 COFF object')
    parts = re.split(r'^[0-9a-f]+ <(.+)>:\n', assembly, flags=re.M)
    bodies = dict(zip(parts[1::2], parts[2::2]))
    def body(prefix=None, suffix=None):
        found = [v for k,v in bodies.items() if
                 (k.startswith(prefix) if prefix else k.endswith(suffix)) and 'clone' not in k]
        require(len(found)==1, 'Missing or ambiguous FP boundary')
        return found[0]
    def code(text):
        result=[]
        for line in text.splitlines():
            cols=line.split('\t')
            if len(cols)>=3 and re.fullmatch(r'\s*[0-9a-f]+:', cols[0]):
                instruction=' '.join(cols[-1].split())
                if re.match('[a-z]', instruction): result.append(instruction)
        return result
    for name, opcode in [('saveHardwareFp', 'fxsave'), ('restoreHardwareFp', 'fxrstor')]:
        instructions=code(body('ss2vr::roomscale::'+name))
        require(instructions[:3]==['mov eax,DWORD PTR [esp+0x4]',opcode+' [eax]','ret'],
                'FP image helper must use the provided image, then return without FP work')
        require(all(i=='nop' for i in instructions[3:]), 'Unexpected image helper tail')
    install=code(body('ss2vr::roomscale::installRoomscaleFp'))
    require(install[:9]==['sub esp,0x8','mov eax,0x27f',
                         'mov WORD PTR [esp+0x2],ax',
                         'mov DWORD PTR [esp+0x4],0x1f80','fninit',
                         'fldcw WORD PTR [esp+0x2]','ldmxcsr DWORD PTR [esp+0x4]',
                         'add esp,0x8','ret'], 'Installed precision/control sequence changed')
    require(all(i=='nop' for i in install[9:]), 'Unexpected install helper tail')
    outer=body('ss2vr::game::runRoomscaleMathFrame(')
    oc=code(outer)
    require(oc[:5]==['lea ecx,[esp+0x4]','and esp,0xfffffff0',
                    'push DWORD PTR [ecx-0x4]','push ebp','mov ebp,esp'],
            'Outer native caller stack realignment changed')
    # EBP is aligned ESP-8. The saved image at EBP-0x218 is thus 16-byte aligned,
    # and its 512 bytes stay above all inner native-finally/callback frames.
    require('sub esp,0x258' in oc and 'lea ecx,[ebp-0x218]' in oc,
            'Aligned image extent changed; inspect frame layout again')
    require('_tls_index' in outer and '__emutls' not in assembly,
            'Containment must use allocation-free native TLS')
    require(outer.count('ss2vrNativeFinally')==1 and
            'GetCurrentThreadId@0' in outer and 'IsProcessorFeaturePresent@4' in outer and
            'mov DWORD PTR [esp],0xa' in oc, 'Owner/SSE2/finally boundary missing')
    run=body(suffix='::Context::run(void*)')
    rc=code(run)
    require('and esp,0xfffffff0' in rc[:8], 'Run callback must realign native stack')
    require(run.index('::saveHardwareFp')<run.index('::installRoomscaleFp'),
            'Must save before changing caller state')
    require(rc.count('call DWORD PTR [eax]')==1, 'Expected exactly one body callback')
    call=rc.index('call DWORD PTR [eax]')
    for half in (rc[:call],rc[call+1:]):
        require(any(i.startswith('stmxcsr ') for i in half) and
                'cmp eax,0x1f80' in half and 'and eax,0xffffffc0' in half and
                any(i.startswith('cmp WORD PTR ') and i.endswith(',0x27f') for i in half),
                'Both x87 and SSE modes must be checked before and after body')
    finish=body(suffix='::Context::finish(void*, int)')
    fc=code(finish)
    require(not any(i.startswith('call ') for i in fc), 'Cleanup must be call-free except tail restore')
    require(finish.count('::restoreHardwareFp')==1 and '_tls_index' in finish and
            'mov DWORD PTR [edx+0x0],0x0' in fc,
            'Cleanup must clear TLS and restore the saved image')
    require('mov edx,DWORD PTR [eax]' in fc and 'mov BYTE PTR [edx],0x1' in fc, 'Abnormal cleanup must fail the scope')
    return {'object_sha256':hashlib.sha256(obj.read_bytes()).hexdigest(),
            'aligned_fx_image_bytes':512, 'entry_exit_control_checks':True,
            'native_finally_structural_check':True, 'native_runtime_executed':False,
            'activated_in_game':False}

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--object',type=Path,required=True)
    args=parser.parse_args()
    print(json.dumps(verify(args.object),indent=2))
