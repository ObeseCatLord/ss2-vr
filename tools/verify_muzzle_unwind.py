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
    # Capture growth can split normal/abnormal cleanup into separate scalar
    # blocks. Prove every reachable return crosses exactly one saved-depth
    # restore instead of depending on the old register/capture layout.
    addresses=[];code={}
    for line in body.splitlines():
        match=re.match(r"^\s*([0-9a-f]+):\t[^\t]+\t(.+)",line)
        if match:
            address=int(match[1],16)
            instruction=' '.join(match[2].split()).split(' <')[0]
            addresses.append(address);code[address]=instruction
    next_address=dict(zip(addresses,addresses[1:]))
    offset=hex(int(depth[0],16))
    restore_pattern=r'mov DWORD PTR \[(?:eax|esi)\+'+re.escape(offset)+r'\],(?:esi|ebx)$'
    restores={address for address,ins in code.items() if re.fullmatch(restore_pattern,ins)}
    require(len(restores)==2,'Expected distinct saved-depth normal/abort stores')
    require('mov ebx,DWORD PTR [eax+0xc]' in instructions and
            'mov ebx,DWORD PTR [ebx]' in instructions and
            'mov esi,DWORD PTR [ebx]' in instructions,
            'Both depth stores must load the saved outer depth reference')
    pending=[(addresses[0],0)];seen=set();returned=False
    while pending:
        address,count=pending.pop()
        if (address,count) in seen:continue
        seen.add((address,count));count+=int(address in restores)
        require(count<=1,'Cleanup repeats depth restoration')
        ins=code[address]
        if ins=='ret':
            require(count==1,'Reachable cleanup return bypasses depth restore')
            returned=True;continue
        branch=re.match(r'(j[a-z]+) ([0-9a-f]+)$',ins)
        if branch:
            target=int(branch[2],16)
            require(target in code,'Cleanup branch escapes verified body')
            pending.append((target,count))
            if branch[1]=='jmp':continue
        require(address in next_address,'Cleanup falls out of verified body')
        pending.append((next_address[address],count))
    require(returned,'Cleanup has no verified reachable return')
    observed=re.findall(r'0x([0-9a-f]+) ss2vr::game::activeAttachmentObservation$',symbols,re.M)
    require(len(observed)==1,'Missing passive attachment TLS')
    receipt_offset=hex(int(observed[0],16))
    require('mov DWORD PTR [edi+'+receipt_offset+'],esi' in instructions and
            'mov DWORD PTR [esi+'+receipt_offset+'],edi' in instructions and
            'mov esi,DWORD PTR [eax+0x4]' in instructions and
            'mov edi,DWORD PTR [eax+0x4]' in instructions and
            'mov esi,DWORD PTR [esi]' in instructions and 'mov edi,DWORD PTR [edi]' in instructions and
            'mov BYTE PTR [ebx],0x0' in instructions,
            'Both published-receipt paths must restore the saved outer TLS and retire publication')
    require('mov BYTE PTR [eax],0x0' in instructions and 'mov BYTE PTR [eax],0x1' in instructions,
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
