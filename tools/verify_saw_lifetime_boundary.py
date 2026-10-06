#!/usr/bin/env python3
"""Verify pinned default/copy initialization facts without executing native code.

This does not admit a selected weapon as freshly constructed or install hooks.
Requires pefile and capstone. Output includes derived facts, not native bytes.
"""
import argparse
import hashlib
import json
from pathlib import Path

import capstone
import pefile

SAM_SHA256 = '5628b4ed30a966f10c8e8ea46bf0789257a35ea80127bacacebe28b1ce5303df'


def inspect(path):
    digest = hashlib.sha256(path.read_bytes()).hexdigest()
    if digest != SAM_SHA256:
        raise ValueError('Unknown Sam2Game.dll fingerprint')
    with pefile.PE(str(path)) as pe:
        if pe.FILE_HEADER.Machine != 0x14c:
            raise ValueError('Expected x86 Sam2Game.dll')
        base = pe.OPTIONAL_HEADER.ImageBase
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)

        def require(rva, mnemonic, operand):
            instruction = next(decoder.disasm(pe.get_data(rva, 15), base + rva), None)
            if instruction is None or (instruction.mnemonic, instruction.op_str) != (mnemonic, operand):
                raise ValueError(f'Lifetime boundary at RVA {rva:x} differs')

        require(0x1646a1, 'call', f'0x{base + 0x498a0:x}')
        require(0x49939, 'mov', 'eax, 1')
        require(0x49941, 'mov', 'dword ptr [esi + 0x38], eax')
        require(0x49977, 'mov', 'dword ptr [esi + 0xb0], eax')
        require(0x1646a6, 'xor', 'edi, edi')
        require(0x16471d, 'mov', 'dword ptr [esi + 0xe8], edi')
        require(0x165dab, 'call', f'0x{base + 0x43ff0:x}')
        require(0x44057, 'mov', 'ecx, dword ptr [edi + 0x38]')
        require(0x4405a, 'mov', 'dword ptr [esi + 0x38], ecx')
        require(0x4413a, 'mov', 'eax, dword ptr [edi + 0xb0]')
        require(0x44140, 'mov', 'dword ptr [esi + 0xb0], eax')
        require(0x165dfe, 'mov', 'eax, dword ptr [edi + 0xe8]')
        require(0x165e04, 'mov', 'dword ptr [esi + 0xe8], eax')
        require(0x43e7c, 'mov', 'ecx, dword ptr [edi + 0x38]')
        require(0x43e7f, 'mov', 'dword ptr [esi + 0x38], ecx')
        require(0x43f86, 'mov', 'eax, dword ptr [edi + 0xb0]')
        require(0x43f8c, 'mov', 'dword ptr [esi + 0xb0], eax')
    return {
        'sha256': digest,
        'runtime_executed': False,
        'native_integration_accepted': False,
        'default_constructor': {'rva': '164680', 'base': '498a0',
                                'primary_released': True, 'state': 1, 'sound_state': 0},
        'copy_constructor': {'rva': '165da0', 'base': '43ff0',
                             'copies_primary_release': True, 'copies_state': True,
                             'copies_sound_state': True},
        'base_assignment': {'rva': '43e40', 'copies_primary_release': True, 'copies_state': True},
        'limits': [
            'Pinned instruction sites plus manual path inspection, not a general dataflow proof',
            'First observation, selection and handle replacement do not prove fresh construction',
            'No selected-weapon lifetime, save/restore or callback non-reentry proof',
            'New consumption records must remain unknown until proven native completion',
        ],
    }


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
