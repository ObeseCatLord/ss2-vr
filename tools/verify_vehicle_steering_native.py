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
ENGINE_PIN = 'da6efc9f72637eb3b6f48eadca2107be89b09c00618b6e72d5d3632938a7d851'


def require(condition, message):
    if not condition:
        raise ValueError(message)


def verify_draw_palette_reference(pe):
    """Finite native matrix-source check, not loaded grip or input admission.

    DDE30 copies the canonical cache matrix directly into a draw palette slot.
    DB140 obtains a bone placement by multiplying that canonical matrix by the
    native rigid inverse of the definition's stored inverse bind. Those frames
    differ for nonidentity bind data; this is not arbitrary affine inversion.
    No native query, renderer or resource is executed by this verifier.
    """
    base = pe.OPTIONAL_HEADER.ImageBase
    require(base == 0x10000000 and pe.FILE_HEADER.Machine == 0x14c,
            'Unsupported Engine palette ABI')
    imports = {i.address: (d.dll.lower(), i.name)
               for d in pe.DIRECTORY_ENTRY_IMPORT for i in d.imports}
    require(imports.get(0x102063a0) ==
            (b'core.dll', b'?mthInvertRTM34f@SeriousEngine@@YA?AVMatrix34f@1@ABV21@@Z'),
            'Native bone inverse-bind inversion import changed')
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    decoded = {i.address - base: (i.mnemonic, i.op_str)
               for start, size in ((0xdde30, 0x97), (0xdb140, 0x184))
               for i in md.disasm(pe.get_data(start, size), base + start)}
    expected = {
        # Map entry's global NativeBone index -> actual canonical cache slot.
        0xdde80: ('mov', 'eax, dword ptr [0x102eac94]'),
        0xdde85: ('mov', 'ecx, dword ptr [0x102eac74]'),
        0xdde8b: ('lea', 'edi, [ebx + eax]'),
        0xdde8e: ('mov', 'eax, dword ptr [ecx + edx*8 + 4]'),
        0xdde92: ('cmp', 'eax, -1'),
        0xdde95: ('je', '0x100ddea9'),
        0xdde97: ('lea', 'esi, [eax + eax*2]'),
        0xdde9a: ('mov', 'eax, dword ptr [0x102eab68]'),
        0xdde9f: ('mov', 'ecx, dword ptr [eax + 0x20]'),
        0xddea2: ('shl', 'esi, 4'),
        0xddea5: ('add', 'esi, ecx'),
        0xddea7: ('jmp', '0x100ddeae'),
        0xddea9: ('mov', 'esi, 0x102c8008'),
        0xddeb2: ('mov', 'ecx, 0xc'),
        0xddeb7: ('add', 'ebx, 0x30'),
        0xddebc: ('rep movsd', 'dword ptr es:[edi], dword ptr [esi]'),
        # Bone placement helper: P * inverse(definition + 0x48), not P alone.
        0xdb149: ('mov', 'edx, dword ptr [0x102eac64]'),
        0xdb150: ('lea', 'ecx, [eax + eax*4]'),
        0xdb153: ('lea', 'esi, [eax + eax*2]'),
        0xdb156: ('mov', 'eax, dword ptr [0x102eab68]'),
        0xdb15b: ('lea', 'ecx, [edx + ecx*8]'),
        0xdb15e: ('mov', 'edx, dword ptr [eax + 0x20]'),
        0xdb161: ('mov', 'eax, dword ptr [ecx + 0x24]'),
        0xdb164: ('shl', 'esi, 4'),
        0xdb167: ('add', 'esi, edx'),
        0xdb16c: ('je', '0x100db2b4'),
        0xdb172: ('add', 'eax, 0x48'),
        0xdb175: ('push', 'eax'),
        0xdb179: ('push', 'ecx'),
        0xdb17a: ('call', 'dword ptr [0x102063a0]'),
        0xdb180: ('fld', 'dword ptr [eax]'),
        0xdb182: ('fmul', 'dword ptr [esi]'),
        0xdb187: ('fld', 'dword ptr [esi + 8]'),
        0xdb18a: ('fmul', 'dword ptr [eax + 0x20]'),
        0xdb18f: ('fld', 'dword ptr [esi + 4]'),
        0xdb192: ('fmul', 'dword ptr [eax + 0x10]'),
        0xdb197: ('fstp', 'dword ptr [ebp - 0x30]'),
        0xdb1df: ('fadd', 'dword ptr [esi + 0xc]'),
        0xdb2b7: ('mov', 'ecx, 0xc'),
        0xdb2bc: ('rep movsd', 'dword ptr es:[edi], dword ptr [esi]'),
        0xdb2c3: ('ret', ''),
    }
    for address, instruction in expected.items():
        require(decoded.get(address) == instruction,
                'Native palette reference changed at ' + hex(address))
    return {'native_sha256': ENGINE_PIN, 'checked_instructions': len(expected),
            'canonical_is_draw_palette_source': True,
            'bone_placement_removes_stored_inverse_bind': True,
            'global_bone_index_preserved': True,
            'final_draw_palette_equality_verified': False,
            'loaded_grip_geometry_associated': False,
            'actual_draw_mapping_verified': False,
            'sampling_query_authorized': False, 'runtime_executed': False,
            'limits': 'finite pinned instruction/import check; no general CFG, loaded content, draw shader, grasp or input lifetime proof'}


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
    engine = (game / 'Bin/Engine.dll').read_bytes()
    require(hashlib.sha256(engine).hexdigest() == ENGINE_PIN, 'Unsupported Engine build')
    palette_reference = verify_draw_palette_reference(pefile.PE(data=engine, max_symbol_exports=100000))
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
            'draw_palette_reference': palette_reference,
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
