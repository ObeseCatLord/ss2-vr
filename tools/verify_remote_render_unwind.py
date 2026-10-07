#!/usr/bin/env python3
"""Inspect compiled remote-render unwind boundaries; never execute native code."""
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
    symbols = subprocess.check_output(['objdump', '-tC', str(obj)], text=True)
    require('file format pe-i386' in assembly, 'Expected GNU x86 remote-render object')
    parts = re.split(r'^[0-9a-f]+ <(.+)>:\n', assembly, flags=re.M)
    bodies = dict(zip(parts[1::2], parts[2::2]))

    def one(fragment, suffix):
        found = [body for name, body in bodies.items() if fragment in name and
                 name.endswith(suffix) and 'clone' not in name]
        require(len(found) == 1, 'Missing or ambiguous render boundary: ' + fragment + suffix)
        return found[0]

    def code(body):
        result = []
        for line in body.splitlines():
            columns = line.split('\t')
            if len(columns) >= 3 and re.fullmatch(r'\s*[0-9a-f]+:', columns[0]):
                instruction = ' '.join(columns[-1].split()).split(' <')[0]
                if re.match('[a-z]', instruction):
                    result.append(instruction)
        return result

    def offset(name):
        found = re.findall(r'0x([0-9a-f]+) ss2vr::game::remote_render::\(anonymous namespace\)::' +
                           re.escape(name) + r'$', symbols, re.M)
        require(len(found) == 1, 'Missing native state symbol: ' + name)
        return int(found[0], 16)

    for name in ['palettePass()', 'modelPass()', 'freezePair()',
                 'commitPair(ss2vr::Slot&, ss2vr::Request const&, bool)']:
        entry = one('ss2vr::game::remote_render::', name)
        require(entry.count('DISP32\tss2vrNativeFinally') == 1,
                'Entry must retain one native unwind extent: ' + name)

    cleanup_names = ['palettePass()', 'modelPass()', 'freezePair()', 'commitPair(',
                     'postModelPass()', 'postPalette()', 'observeLocalScope()']
    allowed = {'ss2vr::game::nativeUiFault()',
               'ss2vr::game::multiplayer::PresentationReadGuard::release()',
               'ReleaseSRWLockShared@4', 'ReleaseSRWLockExclusive@4',
               'operator delete(void*, unsigned int)'}
    for name in cleanup_names:
        body = one(name, '::Context::finish(void*, int)')
        instructions = code(body)
        calls = set(re.findall(r'DISP32\s+([^\n]+)', body))
        require(calls <= allowed and 'ss2vr::game::nativeUiFault()' in calls,
                'Cleanup gained a native callback or lost fault retirement: ' + name)
        require(not any(re.match(r'f[a-z]', i) or re.search(r'\b(?:xmm|ymm|zmm)[0-9]', i) or
                        re.match(r'call (?:DWORD PTR|(?:eax|ebx|ecx|edx|esi|edi|ebp|esp)$)', i) for i in instructions),
                'Cleanup must not use FP or indirect native callbacks: ' + name)
        require(f'mov BYTE PTR ds:{hex(offset("pairActive"))},0x0' in instructions and
                f'mov BYTE PTR ds:{hex(offset("pairInvalid"))},0x1' in instructions,
                'Abort must retire even an empty remote bank: ' + name)
        if name in ['freezePair()', 'commitPair(', 'postModelPass()', 'postPalette()']:
            unlock = 'ReleaseSRWLockExclusive@4' if name == 'freezePair()' else 'ReleaseSRWLockShared@4'
            require(unlock in calls and
                    'ss2vr::game::multiplayer::PresentationReadGuard::release()' in calls,
                    'Both presentation locks need explicit retirement: ' + name)
            require(any(re.fullmatch(r'mov BYTE PTR \[[a-z]{3}\+' + hex(offset('presentationBusy')) +
                                     r'\],0x0', i) for i in instructions),
                    'Adapter reentry ownership must retire: ' + name)
        if name in ['postModelPass()', 'postPalette()', 'observeLocalScope()']:
            require('operator delete(void*, unsigned int)' in calls,
                    'Invocation-owned scratch must retire across foreign unwind: ' + name)
    for name, state in [('palettePass()', 'paletteReentrant'), ('modelPass()', 'reentrant')]:
        body = one(name, '::Context::finish(void*, int)')
        instructions = code(body)
        restores = [i for i in instructions if re.fullmatch(
            r'mov BYTE PTR \[[a-z]{3}\+' + hex(offset(state)) + r'\],[abcd]l', i)]
        require(len(restores) == 2 and 'movzx ebx,BYTE PTR [eax]' in instructions,
                'Both normal and abnormal paths must restore saved outer TLS: ' + name)
    return {'object_sha256': hashlib.sha256(obj.read_bytes()).hexdigest(),
            'explicit_native_cleanup_boundaries': len(cleanup_names),
            'scratch_and_lock_retirement': True, 'runtime_executed': False}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--object', type=Path, required=True)
    print(json.dumps(verify(parser.parse_args().object), indent=2))
