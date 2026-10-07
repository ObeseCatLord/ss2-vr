#!/usr/bin/env python3
"""Check compiled client/server RPC forwarding/cleanup; never execute native code."""
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

    for fragment, reliable in [('8reliable', 1), ('10unreliable', 0)]:
        entry = select(fragment)
        instructions = code(entry)
        require(entry.count('DISP32\tss2vrNativeFinally') == 1 and
                [i for i in instructions if i.startswith('ret')] == ['ret 0x4'],
                'Server RPC entry lost native cleanup or thiscall stack ownership')
        require(f'mov BYTE PTR [esp+0xe],{hex(reliable)}' in instructions and
                'mov DWORD PTR [esp+0x10],ecx' in instructions and
                'mov eax,DWORD PTR [esp+0x34]' in instructions and
                'mov DWORD PTR [esp+0x14],eax' in instructions,
                'Server receiver, RPC pointer or reliability capture changed')
    server_run = code(select('dispatchServer(', '::Context::run(void*)'))
    require('and esp,0xfffffff0' in server_run[:7], 'Server foreign callback must realign')
    require(len([i for i in server_run if i.startswith('call ')]) == 2 and
            server_run.count('call eax') == 1,
            'Server dispatch must inspect input and forward unhandled native RPC once')
    original = server_run.index('call eax')
    def native_pointer(name):
        matches = re.findall(r'0x([0-9a-f]+) ss2vr::game::multiplayer::\(anonymous namespace\)::' +
                             re.escape(name) + r'$', symbols, re.M)
        require(len(matches) == 1, 'Missing original server RPC pointer: ' + name)
        return hex(int(matches[0], 16))
    require(server_run[original-9:original] == [
        'mov edx,DWORD PTR [ebx]', 'mov eax,DWORD PTR [ebx+0x8]',
        'mov ecx,DWORD PTR [edx]', 'mov edx,DWORD PTR [ebx+0x4]',
        'cmp BYTE PTR [eax],0x0',
        'mov eax,ds:' + native_pointer('originalReliable'),
        'mov edx,DWORD PTR [edx]',
        'cmove eax,DWORD PTR ds:' + native_pointer('originalUnreliable'),
        'mov DWORD PTR [esp],edx',
    ] and
            server_run[original+1] == 'sub esp,0x4',
            'Server original receiver, argument or callee stack accounting changed')
    server_cleanup = select('dispatchServer(', '::Context::finish(void*, int)')
    require(set(re.findall(r'DISP32\s+([^\n]+)', server_cleanup)) ==
            {'ss2vr::game::nativeInputFailed()'} and
            not any(i.startswith('call ') for i in code(server_cleanup)),
            'Server failure cleanup must remain a bounded metadata tail call')
    peer = select('withPeerLock<true, ss2vr::game::multiplayer::(anonymous namespace)::receiveServer(',
                  '::Context::finish(void*, int)')
    instructions = code(peer)
    require(peer.count('DISP32\tReleaseSRWLockExclusive@4') == 2 and
            len([i for i in instructions if i.startswith('call ')]) == 2 and
            instructions.count('mov BYTE PTR [edx],0x0') == 2 and
            set(re.findall(r'DISP32\s+([^\n]+)', peer)) ==
            {'ReleaseSRWLockExclusive@4', 'ss2vr::game::nativeInputFailed()'},
            'Both server peer-lock cleanup paths must release recorded ownership')
    require(not any(re.match(r'f[a-z]', i) or re.search(r'\b(?:xmm|ymm|zmm)[0-9]', i)
                    for i in instructions), 'Server peer cleanup must not execute FP')
    return {'object_sha256': hashlib.sha256(obj.read_bytes()).hexdigest(),
            'client_rpc_finally_entries': 3, 'explicit_tls_and_lock_retirement': True,
            'server_rpc_finally_entries': 2, 'server_peer_lock_retirement': True,
            'runtime_executed': False}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--object', type=Path, required=True)
    print(json.dumps(verify(parser.parse_args().object), indent=2))
