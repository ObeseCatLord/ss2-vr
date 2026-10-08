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

    for name in ['palettePass()', 'modelPass()', 'animationEnd(void*)', 'freezePair()',
                 'commitPair(ss2vr::Slot&, ss2vr::Request const&, bool)']:
        entry = one('ss2vr::game::remote_render::', name)
        require(entry.count('DISP32\tss2vrNativeFinally') == 1,
                'Entry must retain one native unwind extent: ' + name)

    cleanup_names = ['palettePass()', 'modelPass()', 'animationEnd(void*)', 'freezePair()', 'commitPair(',
                     'postModelPass()', 'postPalette()', 'observeLocalScope()']
    fault = 'ss2vr::game::nativeUiFault(char const*)'
    # The diagnostic reason changed this ABI after the historical gate was
    # written. Verify its actual implementation, not just a trusted name.
    bridge = obj.with_name('bridge.cpp.obj')
    bridge_assembly = subprocess.check_output(['objdump', '-drC', '-Mintel', str(bridge)], text=True)
    bridge_parts = re.split(r'^[0-9a-f]+ <(.+)>:\n', bridge_assembly, flags=re.M)
    bridge_bodies = dict(zip(bridge_parts[1::2], bridge_parts[2::2]))
    fault_bodies = [body for name, body in bridge_bodies.items()
                    if name == fault or name.startswith(fault + ' [clone ')]
    require(fault in bridge_bodies and fault_bodies,
            'Missing UI-fault cleanup implementation')
    for fault_body in fault_bodies:
        require(not re.findall(r'DISP32\s+', fault_body) and
                not any(re.match(r'call\b|f[a-z]', i) or re.search(r'\b(?:xmm|ymm|zmm)[0-9]', i)
                        for i in code(fault_body)),
                'UI-fault cleanup must remain scalar with no callbacks or allocation')
        addresses = {int(address, 16) for address in
                     re.findall(r'^\s*([0-9a-f]+):\s', fault_body, re.M)}
        for instruction in code(fault_body):
            if re.match(r'(?:j[a-z]+|loop(?:e|ne)?)\b', instruction):
                target = re.fullmatch(r'\S+ ([0-9a-f]+)', instruction)
                require(target and int(target[1], 16) in addresses,
                        'UI-fault cleanup branch must stay inside its inspected body')
    allowed = {fault,
               'ss2vr::game::multiplayer::PresentationReadGuard::release()',
               'ReleaseSRWLockShared@4', 'ReleaseSRWLockExclusive@4',
               'operator delete(void*, unsigned int)'}
    for name in cleanup_names:
        body = one(name, '::Context::finish(void*, int)')
        instructions = code(body)
        calls = set(re.findall(r'DISP32\s+([^\n]+)', body))
        require(calls <= allowed and fault in calls,
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
        require(len(restores) == 2,
                'Both normal and abnormal paths must restore saved outer TLS: ' + name)
        from verify_saved_tls import verify_saved_tls
        verify_saved_tls(one('ss2vr::game::remote_render::',name),body,offset(state))
    idle_entry=one('ss2vr::game::remote_render::','copyIdleRaster(void*, ss2vr::IdleRasterCopy&)')
    require(idle_entry.count('DISP32\tss2vrNativeFinally')==1,'Idle raster scratch lacks local containment')
    require(sum(i.startswith('call ') for i in code(idle_entry))==9,'Idle entry lost actual finally/deletion calls')
    idle_cleanup=one('copyIdleRaster(', '::Context::finish(void*, int)')
    idle_calls=re.findall(r'DISP32\s+([^\n]+)',idle_cleanup)
    require(idle_calls==['operator delete(void*, unsigned int)']*8 and
            sum(i.startswith('call ') for i in code(idle_cleanup))==8,
            'All eight idle raster scratch vectors must retire; no native callbacks')
    require(not any(re.match(r'f[a-z]',i) or re.search(r'\b(?:xmm|ymm|zmm)[0-9]',i) or
                    re.match(r'call (?:DWORD PTR|(?:eax|ebx|ecx|edx|esi|edi|ebp|esp)$)',i) for i in code(idle_cleanup)),
            'Idle scratch cleanup gained FP or indirect callbacks')
    idle_cold=[b for n,b in bodies.items() if 'copyIdleRaster(' in n and '::Context::run(void*) [clone .cold]' in n]
    require(len(idle_cold)==1 and '__cxa_begin_catch' in idle_cold[0] and '__cxa_end_catch' in idle_cold[0],
            'Idle raster allocation failure must be contained locally')
    return {'object_sha256': hashlib.sha256(obj.read_bytes()).hexdigest(),
            'explicit_native_cleanup_boundaries': len(cleanup_names)+1, 'idle_scratch_vectors_retired':8,
            'scratch_and_lock_retirement': True,
            'ui_fault_object_sha256': hashlib.sha256(bridge.read_bytes()).hexdigest(),
            'ui_fault_cleanup_scalar': True, 'ui_fault_variants_checked': len(fault_bodies),
            'runtime_executed': False}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--object', type=Path, required=True)
    print(json.dumps(verify(parser.parse_args().object), indent=2))
