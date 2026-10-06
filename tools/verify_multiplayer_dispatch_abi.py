#!/usr/bin/env python3
"""Check compiled client RPC forwarding/cleanup. Does not execute native code."""
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
    require('file format pe-i386' in assembly, 'Expected x86 multiplayer object')
    pieces = re.split(r'^[0-9a-f]+ <(.+)>:\n', assembly, flags=re.M)
    bodies = dict(zip(pieces[1::2], pieces[2::2]))

    def select(fragment, suffix=None):
        found = [v for k, v in bodies.items() if fragment in k and 'clone' not in k and
                 (k.endswith(suffix) if suffix else k.startswith('@'))]
        require(len(found) == 1, 'Missing or ambiguous RPC boundary: ' + fragment)
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

    for fragment, stack in [('14clientReliable', '0x4'), ('16clientUnreliable', '0x4'),
                            ('13executeClient', '0x14')]:
        body = select(fragment)
        require('ss2vrNativeFinally' in body, 'RPC entry lost its native finally extent')
        require([i for i in code(body) if i.startswith('ret')] == ['ret ' + stack],
                'Native thiscall argument cleanup changed')
        if fragment != '13executeClient':
            require('_tls_index' in body and '.tls$' in body, 'RPC entry must capture prior TLS')
            require('mov edx,DWORD PTR [eax+0x20]' in code(body) and
                    'mov eax,DWORD PTR [eax+0x1c]' in code(body), 'Carrier fields changed')

    run = select('dispatchClient(', '::Context::run(void*)')
    instructions = code(run)
    require('and esp,0xfffffff0' in instructions[:7], 'Foreign callback must realign locally')
    require([i for i in instructions if i.startswith('call ')] == ['call DWORD PTR [edx]'],
            'Dispatch must forward to its original exactly once')
    call = instructions.index('call DWORD PTR [edx]')
    require(instructions[call-7:call] == [
        'mov DWORD PTR [edx+0x0],ecx', 'mov edx,DWORD PTR [eax+0x8]',
        'mov ecx,DWORD PTR [edx]', 'mov edx,DWORD PTR [eax+0x4]',
        'mov eax,DWORD PTR [eax+0xc]', 'mov eax,DWORD PTR [eax]',
        'mov DWORD PTR [esp],eax',
    ], 'Native receiver/RPC forwarding or TLS installation changed')
    cleanup = select('dispatchClient(', '::Context::finish(void*, int)')
    clean = code(cleanup)
    require('mov edx,DWORD PTR [edx+0xc]' in clean, 'Cleanup must load saved previous context')
    require('mov DWORD PTR [eax+0x0],edx' in clean and
            'mov DWORD PTR [ecx+0x0],edx' in clean, 'Normal/abnormal TLS restoration missing')
    require(cleanup.count('secrel32\t.tls$') == 2, 'Both cleanup paths must restore native TLS')
    require(set(re.findall(r'DISP32\s+([^\n]+)', cleanup)) == {'ss2vr::game::nativeInputFailed()'},
            'TLS cleanup gained an unreviewed callback')
    require(not any(i.startswith('call ') or re.match(r'f[a-z]', i) or
                    re.search(r'\b(?:xmm|ymm|zmm)[0-9]', i) for i in clean),
            'TLS cleanup must remain integer metadata with only the failure tail call')
    peer = select('withPeerLock<true, ss2vr::game::multiplayer::(anonymous namespace)::executeClientBody(',
                  '::Context::finish(void*, int)')
    require('ReleaseSRWLockExclusive@4' in peer and 'nativeInputFailed()' in peer,
            'Reentered client RPC must release peer ownership on native unwind')
    require(set(re.findall(r'DISP32\s+([^\n]+)', peer)) <=
            {'ReleaseSRWLockExclusive@4', 'ss2vr::game::nativeInputFailed()'},
            'Peer cleanup gained an unreviewed callback')
    return {'object_sha256': hashlib.sha256(obj.read_bytes()).hexdigest(),
            'client_rpc_finally_entries': 3, 'explicit_tls_and_lock_retirement': True,
            'runtime_executed': False}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--object', type=Path, required=True)
    print(json.dumps(verify(parser.parse_args().object), indent=2))
