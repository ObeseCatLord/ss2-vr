#!/usr/bin/env python3
"""Check muzzle/placement callbacks and TLS cleanup, never native execution."""
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
    require('file format pe-i386' in assembly, 'Expected GNU x86 engine object')
    parts = re.split(r'^[0-9a-f]+ <(.+)>:\n', assembly, flags=re.M)
    bodies = dict(zip(parts[1::2], parts[2::2]))
    # GCC's fastcall attributes leave these two names partially mangled in -C
    # output. The shared shooting helper is inlined into each native entry.
    entry_names = ('@_ZN5ss2vr4gameL4shotEPvS1_PNS_4PoseE@12',
                   '@_ZN5ss2vr4gameL10sniperShotEPvS1_PNS_4PoseE@12')
    entries = [bodies.get(name, '') for name in entry_names]
    require(all(body.count('DISP32\tss2vrNativeFinally') == 1 and
                re.search(r'\bret\s+0x4\b', body) for body in entries),
            'Both native muzzle entries need one finally extent and ret4 ABI')
    finishes = [body for name, body in bodies.items() if 'shooting(' in name and
                name.endswith('::Context::finish(void*, int)') and 'clone' not in name]
    require(len(finishes) == 1, 'Missing or ambiguous muzzle cleanup callback')
    depth = re.findall(r'0x([0-9a-f]+) ss2vr::game::nativeShotDepth$', symbols, re.M)
    require(len(depth) == 1, 'Missing native muzzle TLS depth')
    body = finishes[0]
    instructions = []
    for line in body.splitlines():
        columns = line.split('\t')
        if len(columns) >= 3 and re.fullmatch(r'\s*[0-9a-f]+:', columns[0]):
            instruction = ' '.join(columns[-1].split()).split(' <')[0]
            if re.match('[a-z]', instruction):
                instructions.append(instruction)
    # The saved prior depth is captured by reference above the native extent.
    # Both completion paths share its unconditional restore before branching.
    restore = 'mov DWORD PTR [esi+' + hex(int(depth[0], 16)) + '],ebx'
    require(instructions.count(restore) == 1 and
            instructions.index('mov ecx,DWORD PTR [eax+0x4]') <
            instructions.index('mov eax,DWORD PTR [ecx]') <
            instructions.index('mov ebx,DWORD PTR [eax]') < instructions.index(restore),
            'Cleanup must restore the saved outer depth, not decrement or clear it')
    first_branch = next((i for i, item in enumerate(instructions) if item.startswith('j')), len(instructions))
    require(instructions.index(restore) < first_branch,
            'Normal and abnormal cleanup must both restore muzzle TLS')
    require('mov BYTE PTR [ecx],0x0' in instructions and 'mov BYTE PTR [eax],0x1' in instructions,
            'Abort must revoke the copied calibration witness and active input interval')
    require(not any(re.match(r'call\b|f[a-z]', item) or
                    re.search(r'\b(?:xmm|ymm|zmm)[0-9]', item) for item in instructions),
            'Muzzle cleanup must contain no callbacks, allocation or FP work')
    charge = bodies.get('@_ZN5ss2vr4gameL12weaponChargeEPvS1_PNS_8Matrix34E@12', '')
    charge_symbols = {}
    for symbol in ('originalWeaponCharge', 'weaponChargeReturn', 'placementCharge'):
        found = re.findall(r'0x([0-9a-f]+) ss2vr::game::' + symbol + '$', symbols, re.M)
        require(len(found) == 1, 'Missing charge-capture symbol: ' + symbol)
        charge_symbols[symbol] = hex(int(found[0], 16))
    returns = re.findall(r'\bret(?:\s+(0x[0-9a-f]+))?(?:\s|$)', charge)
    require(len(re.findall(r'\bcall\s', charge)) == 1 and
            'call   DWORD PTR ds:' + charge_symbols['originalWeaponCharge'] in charge and
            returns and all(value == '0x4' for value in returns),
            'Charge getter must forward once with its native ret4 ABI')
    require('mov    esi,ecx' in charge and 'mov    ebx,DWORD PTR [esp+0x14]' in charge and
            'mov    edi,DWORD PTR [esp+0x10]' in charge and 'mov    DWORD PTR [esp],ebx' in charge and
            'cmp    edi,DWORD PTR ds:' + charge_symbols['weaponChargeReturn'] in charge and
            'cmp    ebx,edx' in charge and 'mov    eax,edx' in charge,
            'Charge callback lost native receiver/output/return-address/result provenance')
    charge_finishes = [value for name, value in bodies.items() if 'nativePlacementWithCharge(' in name and
                       name.endswith('::Context::finish(void*, int)') and 'clone' not in name]
    require(len(charge_finishes) == 1, 'Missing charge-placement cleanup')
    finish = charge_finishes[0]
    require('mov    DWORD PTR [esi+' + charge_symbols['placementCharge'] + '],eax' in finish and
            'mov    BYTE PTR [ebx+0x15],0x1' in finish and 'mov    BYTE PTR [eax+0x15],0x1' in finish,
            'Charge cleanup must restore outer TLS and invalidate both interrupted observations')
    code = re.findall(r'^\s*[0-9a-f]+:\t[^\t]+\t([^\n]+)', finish, re.M)
    require(not any(re.match(r'call\b|f[a-z]', item.strip()) or
                    re.search(r'\b(?:xmm|ymm|zmm)[0-9]', item) for item in code),
            'Charge cleanup must remain scalar without callbacks or allocation')
    return {'object_sha256': hashlib.sha256(obj.read_bytes()).hexdigest(),
            'entry_count': len(entries), 'saved_depth_restored_on_all_cleanup_paths': True,
            'abort_calibration_and_input_revoked': True,
            'charge_native_output_and_ret4_provenance': True, 'charge_outer_tls_retired': True,
            'runtime_executed': False}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--object', type=Path, required=True)
    print(json.dumps(verify(parser.parse_args().object), indent=2))
