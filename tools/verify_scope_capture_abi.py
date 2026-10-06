#!/usr/bin/env python3
"""Inspect the compiled native capture-command call shapes; never execute PE code."""
import hashlib
import json
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def verify(path):
    text = subprocess.check_output(['i686-w64-mingw32-objdump','-drwC','-Mintel',str(path)],text=True)
    lines = text.splitlines()
    functions = []
    current = None
    for line in lines:
        match = re.match(r'^([0-9a-f]+) <(.*)>:$',line)
        if match:
            current = {'name':match.group(2),'instructions':[]}
            functions.append(current)
        elif current:
            match = re.match(r'^\s*[0-9a-f]+:\s+(?:[0-9a-f]{2}\s+)+\s*([^\t]+)',line)
            if match:
                current['instructions'].append(match.group(1).strip().split(' <')[0])
    candidates = [f for f in functions if 'ScopeCaptureCommands::queue' in f['name'] and
                  f['name'].endswith('::Context::run(void*)')]
    if len(candidates) != 1:
        raise ValueError('Expected exactly one production queue body')
    instructions = candidates[0]['instructions']
    alloc = [i for i,s in enumerate(instructions) if re.fullmatch(r'call\s+DWORD PTR \[eax\+0x8\]',s)]
    ctor = [i for i,s in enumerate(instructions) if re.fullmatch(r'call\s+DWORD PTR \[eax\+0xc\]',s)]
    if len(alloc) != 1 or len(ctor) != 1 or alloc[0] >= ctor[0]:
        raise ValueError('Unrecognized allocator/constructor member-call shape')
    a,c = alloc[0],ctor[0]
    if not re.fullmatch(r'mov\s+DWORD PTR \[esp\],0x18',instructions[a-1]):
        raise ValueError('Native allocation size/stack argument differs')
    saved = re.fullmatch(r'mov\s+DWORD PTR \[ebp-(0x[0-9a-f]+)\],eax',instructions[a+1])
    if saved is None or not re.fullmatch(r'mov\s+esi,eax',instructions[a+2]):
        raise ValueError('Allocator-return storage provenance differs')
    if not re.fullmatch(r'mov\s+ecx,esi',instructions[c-1]) or not re.fullmatch(
            r'mov\s+esi,DWORD PTR \[ebp-'+saved.group(1)+r'\]',instructions[c-3]):
        raise ValueError('Native constructor ECX is not the allocated storage')
    if not any(re.fullmatch(r'mov\s+DWORD PTR \[esi\+0x8\],0xaffff',s) for s in instructions):
        raise ValueError('Native capture sort rank differs')
    harmless = [f for f in functions if f['name'].endswith('::abandonedCapture(void*)')]
    if len(harmless) != 1 or not harmless[0]['instructions'] or harmless[0]['instructions'][0] != 'ret':
        raise ValueError('Harmless callback is not the expected plain cdecl return')
    return {'object':str(path.relative_to(ROOT)),'sha256':hashlib.sha256(path.read_bytes()).hexdigest(),
            'allocation_bytes':24,'allocator_argument':'stack','constructor_receiver':'allocated storage in ECX',
            'capture_rank':'0xaffff','harmless_callback_ret_bytes':0}


if __name__ == '__main__':
    paths = [ROOT/'build-game/CMakeFiles'/target/'src/game/scope_capture_command.cpp.obj'
             for target in ('ss2vr_game.dir','ss2vr_server.dir')]
    print(json.dumps({'runtime_executed':False,'native_commands_queued':False,
                      'artifacts':[verify(path) for path in paths],
                      'limits':['Compiler-specific structural check; source ownership remains separate',
                                'No native allocator/constructor, callback execution or failure injection']},indent=2))
