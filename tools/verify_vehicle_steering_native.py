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


def verify_seat_attachment(pe):
    """Bind the occupied-name → attachment route; never call its native getter.

    This is a finite instruction/dispatch check of the pinned module, not a
    lifetime proof, general CFG proof, or certification of a loaded resource.
    In particular GetSeatAttachment resolves its returned handle twice and
    does not null-check the second result before reading it.
    """
    base = pe.OPTIONAL_HEADER.ImageBase
    exports = {e.name.decode(): e.address for e in pe.DIRECTORY_ENTRY_EXPORT.symbols if e.name}
    for name, address in (
        ('?FindSeatDataByName@CPuppetEntity@SeriousEngine@@UAE?AV?$Handle@VCPuppetSeatData@SeriousEngine@@@2@VIDENT@2@@Z', 0x84880),
        ('?GetSeatAttachment@CPuppetEntity@SeriousEngine@@UAE?AVIDENT@2@V32@@Z', 0x84930),
        ('?GetSeatAbsPlacement@CPuppetEntity@SeriousEngine@@UAE?AVMatrix34f@2@VIDENT@2@@Z', 0x84d10),
    ):
        require(exports.get(name) == address, 'Native seat export changed')
    for table in (0x2a8558, 0x2b8420):
        for slot, target in ((0x20c, 0x84880), (0x214, 0x84930), (0x224, 0x84d10)):
            require(struct.unpack('<I', pe.get_data(table + slot, 4))[0] == base + target,
                    'Hover occupied-seat dispatch changed')
    imports = {i.address: (d.dll.lower(), i.name)
               for d in pe.DIRECTORY_ENTRY_IMPORT for i in d.imports}
    for address, expected in (
        (0x102941d0, (b'core.dll', b'?hvHandleToPointer@SeriousEngine@@YAPAXK@Z')),
        (0x1029405c, (b'core.dll', b'?_st_idInvalid@SeriousEngine@@3UInvalidIdent@1@B')),
        (0x10294cf4, (b'engine.dll', b'?GetAttachmentAbsolutePlacement@CModelRenderable@SeriousEngine@@QAE?AVMatrix34f@2@VIDENT@2@@Z')),
    ):
        require(imports.get(address) == expected, 'Native seat import changed')
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    decoded = {i.address - base: (i.mnemonic, i.op_str)
               for start, size in ((0x84880, 0xa5), (0x84930, 0x4f), (0x84d10, 0x3b))
               for i in md.disasm(pe.get_data(start, size), base + start)}
    expected = {
        # Find by *name*, distinct from seat index and attachment identity.
        0x8488c: ('call', 'dword ptr [eax + 0x200]'),
        0x848a9: ('call', 'dword ptr [edx + 0x204]'),
        0x848b3: ('call', 'dword ptr [0x102941d0]'),
        0x848b9: ('mov', 'ecx, dword ptr [eax + 4]'),
        0x848c2: ('cmp', 'ecx, ebx'),
        0x848c4: ('je', '0x10084916'),
        0x84919: ('mov', 'ecx, dword ptr [ebp + 0xc]'),
        0x8491e: ('mov', 'dword ptr [eax], ecx'),
        0x84922: ('ret', '8'),
        # IDENT uses a hidden output pointer; both exits pop two arguments.
        0x84933: ('mov', 'edx, dword ptr [ebp + 0xc]'),
        0x84939: ('push', 'edx'),
        0x8493a: ('lea', 'edx, [ebp + 0xc]'),
        0x8493d: ('push', 'edx'),
        0x8493e: ('call', 'dword ptr [eax + 0x20c]'),
        0x84947: ('mov', 'esi, dword ptr [0x102941d0]'),
        0x8494e: ('call', 'esi'),
        0x84953: ('test', 'eax, eax'),
        0x84955: ('jne', '0x10084969'),
        0x84957: ('mov', 'ecx, dword ptr [0x1029405c]'),
        0x8495f: ('mov', 'eax, dword ptr [ebp + 8]'),
        0x84962: ('mov', 'dword ptr [eax], edx'),
        0x84966: ('ret', '8'),
        0x8496d: ('call', 'esi'),
        0x8496f: ('mov', 'ecx, dword ptr [eax + 8]'),
        0x84972: ('mov', 'eax, dword ptr [ebp + 8]'),
        0x84978: ('mov', 'dword ptr [eax], ecx'),
        0x8497c: ('ret', '8'),
        # The native world-placement route consumes the resulting attachment.
        0x84d25: ('call', 'dword ptr [eax + 0x214]'),
        0x84d2b: ('mov', 'eax, dword ptr [esi + 0x120]'),
        0x84d32: ('call', 'dword ptr [0x102941d0]'),
        0x84d38: ('mov', 'ecx, dword ptr [ebp + 0xc]'),
        0x84d3e: ('push', 'ecx'),
        0x84d45: ('call', 'dword ptr [0x10294cf4]'),
    }
    for address, instruction in expected.items():
        require(decoded.get(address) == instruction, 'Native seat seam changed at ' + hex(address))
    return {'checked_instructions': len(expected), 'hover_classes_checked': 2,
            'attachment_abi': 'thiscall; hidden IDENT output then seat IDENT; ret8',
            'occupied_name_and_attachment_are_distinct_fields': True,
            'second_handle_resolution_null_checked': False,
            'loaded_resource_correspondence_proved': False,
            'sampling_getter_authorized_by_this_check': False,
            'runtime_executed': False}


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
            'seat_attachment': verify_seat_attachment(pe),
            'native_drive_input': 'raw move.x steering; raw move.z drive',
            'wheeled_target_selection': 'input sign selects zero or native joint limits',
            'proportional_steering_magnitude_proved': False,
            'steering_wheel_geometry_proved': False,
            'adapter_installed': False, 'runtime_executed': False}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game', type=Path, required=True)
    print(json.dumps(verify(parser.parse_args().game), indent=2))
