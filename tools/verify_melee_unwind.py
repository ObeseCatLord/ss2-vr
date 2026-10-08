#!/usr/bin/env python3
"""Inspect actual x86 melee cleanup and its scalar callees; execute no game code."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess


def require(ok, message):
    if not ok:
        raise ValueError(message)


def inspect(path):
    assembly = subprocess.check_output(['objdump', '-drC', '-Mintel', str(path)], text=True)
    require('file format pe-i386' in assembly, 'Expected x86 engine object')
    parts = re.split(r'^([0-9a-f]+) <(.+)>:\n', assembly, flags=re.M)
    bodies = {parts[i+1]: (int(parts[i], 16), parts[i+2]) for i in range(1, len(parts), 3)}
    starts = {address: name for name, (address, _) in bodies.items()}
    cleanup_names = ['dispatchMelee(', 'primaryPressed(', 'primaryReleased(', 'weaponFiringPressed(',
                     'baseWeaponStep(', 'labFireRelease(', 'sawDeleted(', 'sawWeaponPutDown(',
                     'sawCopied(', 'sawAssigned(', 'sawPutDown(']
    permitted_helpers = ['abortMeleeTicket(', 'nativeInputFailed(']
    permitted_system = {'AcquireSRWLockExclusive@4', 'ReleaseSRWLockExclusive@4',
                        'AcquireSRWLockShared@4', 'ReleaseSRWLockShared@4'}
    visited = set()
    symbols = subprocess.check_output(['objdump', '-tC', str(path)], text=True)
    tls_restores = {'dispatchMelee(': ('meleeDispatch', 2),
                    'weaponFiringPressed(': ('meleeHeld', 2),
                    'baseWeaponStep(': ('meleeStep', 1)}

    def instructions(body):
        result = []
        for line in body.splitlines():
            columns = line.split('\t')
            if len(columns) >= 3 and re.fullmatch(r'\s*[0-9a-f]+:', columns[0]):
                code = columns[-1].strip()
                if re.match(r'[a-z]', code):
                    result.append((int(columns[0].strip()[:-1], 16), code, []))
            elif result and re.search(r'\b(?:DISP32|dir32)\s', line):
                result[-1][2].append(re.split(r'\b(?:DISP32|dir32)\s+', line)[-1].strip())
        return result

    def audit(name):
        if name in visited:
            return
        visited.add(name)
        rows = instructions(bodies[name][1])
        require(rows, 'Empty cleanup implementation: ' + name)
        addresses = {row[0] for row in rows}
        for address, text, relocations in rows:
            code = ' '.join(text.split(' <', 1)[0].split())
            require(not re.match(r'f[a-z]', code) and
                    not re.search(r'\b(?:xmm|ymm|zmm)[0-9]', code),
                    'Cleanup must remain scalar: ' + name)
            if not re.match(r'(?:call|j[a-z]+|loop(?:e|ne)?)\b', code):
                continue
            branch = not code.startswith('call ')
            target = re.fullmatch(r'\S+ ([0-9a-f]+)', code)
            if branch and target and int(target[1], 16) in addresses:
                continue
            if relocations:
                require(len(relocations) == 1 and relocations[0] in permitted_system,
                        'Cleanup gained an unapproved external transfer: ' + name)
                continue
            require(target, 'Cleanup gained an indirect transfer: ' + name)
            callee = starts.get(int(target[1], 16))
            require(callee and any(callee.startswith('ss2vr::game::' + fragment)
                                   for fragment in permitted_helpers),
                    'Cleanup gained an unapproved resolved transfer: ' + name)
            audit(callee)

    cleanup = []
    for fragment in cleanup_names:
        found = [name for name in bodies if fragment in name and
                 name.endswith('::Context::finish(void*, int)') and 'clone' not in name]
        require(len(found) == 1, 'Missing/ambiguous native cleanup extent: ' + fragment)
        audit(found[0])
        if fragment in tls_restores:
            state, count = tls_restores[fragment]
            offset = re.findall(r'0x([0-9a-f]+) ss2vr::game::' + state + r'$', symbols, re.M)
            require(len(offset) == 1, 'Missing scope ownership symbol: ' + state)
            pattern = r'mov DWORD PTR \[e[a-z]{2}\+' + hex(int(offset[0], 16)) + r'\],e[a-z]{2}'
            stores = [' '.join(text.split(' <', 1)[0].split())
                      for _, text, _ in instructions(bodies[found[0]][1])]
            require(sum(re.fullmatch(pattern, code) is not None for code in stores) == count,
                    'Scope must restore saved outer TLS on every compiled path: ' + state)
        cleanup.append(fragment[:-1])
    return {'object_sha256': hashlib.sha256(path.read_bytes()).hexdigest(),
            'native_cleanup_extents': cleanup, 'transitive_scalar_bodies_checked': len(visited),
            'no_native_callbacks_or_allocation_in_cleanup': True,
            'runtime_executed': False, 'native_exception_executed': False}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--object', type=Path, required=True)
    print(json.dumps(inspect(parser.parse_args().object), indent=2))
