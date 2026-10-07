#!/usr/bin/env python3
"""Pinned native steering-input evidence; no vehicle adapter or runtime execution."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import capstone
import pefile

PIN = '5628b4ed30a966f10c8e8ea46bf0789257a35ea80127bacacebe28b1ce5303df'


def require(condition, message):
    if not condition:
        raise ValueError(message)


def verify(game):
    data = (game / 'Bin/Sam2Game.dll').read_bytes()
    require(hashlib.sha256(data).hexdigest() == PIN, 'Unsupported Sam2Game build')
    pe = pefile.PE(data=data, max_symbol_exports=100000)
    base = pe.OPTIONAL_HEADER.ImageBase
    require(base == 0x10000000 and pe.FILE_HEADER.Machine == 0x14c, 'Unsupported native ABI')
    exports = {e.name.decode(): e.address for e in pe.DIRECTORY_ENTRY_EXPORT.symbols if e.name}
    expected_exports = {
        '?SetDriveSteerRatioAndLookDir@CPuppetEntity@SeriousEngine@@UAEXVVector3f@2@0@Z': 0x802c0,
        '?EnforcePuppetMoveLook@CWheeledVehiclePuppetEntity@SeriousEngine@@UAEXVVector3f@2@00@Z': 0x1563d0,
        '?LerpToDesiredVelocityAndLook@CWheeledVehiclePuppetEntity@SeriousEngine@@UAEXXZ': 0x15bfd0,
        '??_7CWheeledVehiclePuppetEntity@SeriousEngine@@6B@': 0x2a7ea0,
    }
    for name, address in expected_exports.items():
        require(exports.get(name) == address, 'Native steering export changed: ' + name)
    for slot, target in ((0x514, 0x1563d0), (0x58c, 0x802c0)):
        require(struct.unpack('<I', pe.get_data(0x2a7ea0 + slot, 4))[0] == base + target,
                'Wheeled native control dispatch changed')
    imports = {i.address: i.name for d in pe.DIRECTORY_ENTRY_IMPORT for i in d.imports}
    require(imports.get(0x102958a8) ==
            b'?SetDesiredSteeringPosition@CWheelJoint@SeriousEngine@@QAEXM@Z',
            'Native steering-joint target changed')
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    # Decode at actual function starts, including the entire transfer and branch
    # bodies. The full binary fingerprint additionally binds intervening code.
    ranges = [(0x802c0, 0x1d7), (0x1563d0, 0x3d), (0x15bfd0, 0x284)]
    decoded = {i.address - base: (i.mnemonic, i.op_str)
               for start, size in ranges for i in md.disasm(pe.get_data(start, size), base + start)}
    expected = {
        0x1563d6: ('mov', 'esi, dword ptr [ebp + 8]'),
        0x1563ec: ('mov', 'esi, dword ptr [ebp + 0x14]'),
        0x156402: ('call', 'dword ptr [eax + 0x58c]'),
        0x15640a: ('ret', '0x24'),
        0x80410: ('mov', 'edx, dword ptr [ebp + 8]'),
        0x8041c: ('lea', 'ecx, [esi + 0x8c]'),
        0x80422: ('mov', 'dword ptr [ecx], edx'),
        0x80433: ('lea', 'eax, [esi + 0xa4]'),
        0x80478: ('mov', 'dword ptr [esi + 0x4c4], 3'),
        0x80494: ('ret', '0x18'),
        0x15c074: ('cmp', 'eax, 3'), 0x15c077: ('je', '0x1015c0ad'),
        0x15c0ad: ('fld', 'dword ptr [ebx + 0x50c]'),
        0x15c0b3: ('fmul', 'dword ptr [ebx + 0x94]'),
        0x15c0be: ('fld', 'dword ptr [ebx + 0x8c]'),
        0x15c0c4: ('fchs', ''), 0x15c0c6: ('fstp', 'dword ptr [ebp - 0xc]'),
        0x15c147: ('fld', 'dword ptr [ebp - 0xc]'),
        0x15c150: ('fchs', ''), 0x15c152: ('fstp', 'dword ptr [ebp - 0x1c]'),
        0x15c158: ('fld', 'dword ptr [ebp - 0x1c]'),
        0x15c15b: ('test', 'eax, eax'), 0x15c15d: ('jne', '0x1015c167'),
        0x15c161: ('fld', 'dword ptr [0x10296244]'),
        0x15c167: ('cmp', 'dword ptr [esi], 0'), 0x15c16c: ('fchs', ''),
        0x15c16e: ('fcom', 'dword ptr [0x10296244]'),
        0x15c174: ('mov', 'dword ptr [ebp - 0xc], 0'),
        0x15c182: ('fld', 'dword ptr [esi + 0xc]'),
        0x15c187: ('fstp', 'dword ptr [ebp - 0xc]'),
        0x15c18a: ('fcomp', 'dword ptr [0x10296244]'),
        0x15c197: ('fld', 'dword ptr [esi + 0x10]'),
        0x15c19c: ('fstp', 'dword ptr [ebp - 0xc]'),
        0x15c20e: ('mov', 'edx, dword ptr [ebp - 0xc]'),
        0x15c214: ('push', 'edx'),
        0x15c21d: ('call', 'dword ptr [0x102958a8]'),
    }
    require(pe.get_data(0x296244, 4) == b'\0' * 4, 'Steering zero comparison changed')
    for address, instruction in expected.items():
        require(decoded.get(address) == instruction, 'Native steering seam changed at ' + hex(address))
    return {'native_sha256': PIN, 'checked_instructions': len(expected),
            'native_drive_input': 'raw move.x steering; raw move.z drive',
            'wheeled_target_selection': 'input sign selects zero or native joint limits',
            'proportional_steering_magnitude_proved': False,
            'steering_wheel_geometry_proved': False,
            'adapter_installed': False, 'runtime_executed': False}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game', type=Path, required=True)
    print(json.dumps(verify(parser.parse_args().game), indent=2))
