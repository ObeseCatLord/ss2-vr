#!/usr/bin/env python3
"""Check pinned stock weapon command/render coverage; no native execution.

The result does not prove source-color completeness or scope-image admission.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path

import capstone
import pefile

SAM_SHA256 = '5628b4ed30a966f10c8e8ea46bf0789257a35ea80127bacacebe28b1ce5303df'
TABLES = {
    'CBaseWeaponEntity': 0x2a3fb8, 'CCircularSawBladesWeaponEntity': 0x2b2088,
    'CAutoShotgunWeaponEntity': 0x2ccc78, 'CBeamGunWeaponEntity': 0x2cd268,
    'CCannonWeaponEntity': 0x2cd960, 'CCircularSawWeaponEntity': 0x2cdd10,
    'CColtWeaponEntity': 0x2ce340, 'CDoubleShotgunWeaponEntity': 0x2cec60,
    'CFlamerWeaponEntity': 0x2cf250, 'CGrenadeLauncherWeaponEntity': 0x2cf918,
    'CKlodovikGunWeaponEntity': 0x2cff28, 'CMiniGunWeaponEntity': 0x2d0590,
    'CPlasmaRifleWeaponEntity': 0x2d0be8, 'CRocketLauncherWeaponEntity': 0x2d1258,
    'CSeriousBombWeaponEntity': 0x2d15d0, 'CSniperWeaponEntity': 0x2d1f00,
    'CUziWeaponEntity': 0x2d26a0, 'CZapGunWeaponEntity': 0x2d2ca0,
}


def inspect(path):
    digest = hashlib.sha256(path.read_bytes()).hexdigest()
    if digest != SAM_SHA256:
        raise ValueError('Unknown Sam2Game.dll fingerprint')
    with pefile.PE(str(path), max_symbol_exports=65536) as pe:
        if pe.FILE_HEADER.Machine != 0x14c:
            raise ValueError('Expected x86 Sam2Game.dll')
        base = pe.OPTIONAL_HEADER.ImageBase
        symbols = {s.name.decode(): s.address for s in pe.DIRECTORY_ENTRY_EXPORT.symbols if s.name}
        constructors = {n.split('@')[0][3:]: r for n, r in symbols.items()
                        if n.startswith('??0') and 'WeaponEntity@' in n and n.endswith('QAE@XZ')}
        if set(constructors) != set(TABLES):
            raise ValueError('Stock exported weapon constructor set differs')
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        decoder.detail = True
        rows = []
        for name, table in TABLES.items():
            assignments = set()
            for ins in decoder.disasm(pe.get_data(constructors[name], 0x400), base + constructors[name]):
                if ins.mnemonic == 'ret':
                    break
                if ins.mnemonic != 'mov' or len(ins.operands) != 2:
                    continue
                dest, source = ins.operands
                if (dest.type == capstone.x86.X86_OP_MEM and source.type == capstone.x86.X86_OP_IMM
                        and dest.mem.base and not dest.mem.disp):
                    assignments.add(source.imm - base)
            if table not in assignments:
                raise ValueError(f'{name} does not assign the expected table in its constructor')
            command, render = struct.unpack('<II', pe.get_data(table + 0x1f0, 8))
            expected_render = 0x171f20 if name == 'CSniperWeaponEntity' else 0x4c740
            if command != base + 0x4bf40 or render != base + expected_render:
                raise ValueError(f'{name} has an unclassified command/render target')
            rows.append({'class': name, 'table_rva': f'{table:x}', 'render_rva': f'{expected_render:x}'})
        # Independently enumerate aligned non-code references to the common
        # command constructor. Every occurrence belongs to the classified set.
        references = set()
        needle = struct.pack('<I', base + 0x4bf40)
        for section in pe.sections:
            if section.Characteristics & 0x20000000:
                continue
            data = section.get_data()
            start = 0
            while True:
                at = data.find(needle, start)
                if at < 0:
                    break
                rva = section.VirtualAddress + at
                if rva % 4 == 0:
                    references.add(rva - 0x1f0)
                start = at + 1
        if references != set(TABLES.values()):
            raise ValueError('Unclassified stock command-table references')
        instruction = next(decoder.disasm(pe.get_data(0x4bf34, 15), base + 0x4bf34), None)
        if instruction is None or (instruction.mnemonic, instruction.op_str) != ('call', 'dword ptr [edx + 0x1f4]'):
            raise ValueError('Common command virtual render dispatch differs')
    return {'sha256': digest, 'runtime_executed': False, 'scope_image_enabled': False,
            'common_command_rva': '4bde0', 'render_return_rva': '4bf3a', 'stock_tables': rows,
            'limits': ['Only this pinned stock module; external/replaced vtables are not admitted',
                       'Constructor site/table checks are not a complete control-flow or live-lifetime proof',
                       'No source world-content ordering, color equivalence or auxiliary-view admission proof']}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game', required=True, type=Path)
    args = parser.parse_args()
    try:
        result = inspect(args.game / 'Bin' / 'Sam2Game.dll')
    except (OSError, ValueError, pefile.PEFormatError) as exc:
        parser.exit(1, f'Audit refused: {exc}\n')
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()
