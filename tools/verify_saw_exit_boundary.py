#!/usr/bin/env python3
"""Read-only, fingerprinted fatal-exit boundary audit; never execute game code.

This proves call-site topology, not callback target closure or hook acceptance.
Requires pefile and capstone. Output contains derived facts, never native bytes.
"""
import argparse
import hashlib
import json
from pathlib import Path

import capstone
import pefile

CORE_SHA256 = '7a1bd56b9bfa3edfbb23f4d3c96490e40a7c0b031b85e797e1af91b2ba3cf207'
REGISTRATION_NAMES = (
    'conAddExitCallback', 'conRemExitCallback', 'conSetFatalErrorCallback',
    'conSetPreTerminationCallback', 'conSetPostTerminationCallback',
)


def inspect(core_path):
    digest = hashlib.sha256(core_path.read_bytes()).hexdigest()
    if digest != CORE_SHA256:
        raise ValueError('Unknown Core.dll fingerprint; no boundary claim is made')
    with pefile.PE(str(core_path)) as pe:
        if pe.FILE_HEADER.Machine != 0x14c:
            raise ValueError('Expected x86 Core.dll')
        base = pe.OPTIONAL_HEADER.ImageBase
        decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)

        def require(rva, mnemonic, operand):
            instruction = next(decoder.disasm(pe.get_data(rva, 15), base + rva), None)
            if instruction is None or (instruction.mnemonic, instruction.op_str) != (mnemonic, operand):
                raise ValueError(f'Boundary at RVA {rva:x} differs')

        # Verify the earlier callback as well as the eventual conExit call.
        require(0x451c, 'call', 'eax')
        require(0x457a, 'call', f'0x{base + 0x41e0:x}')
        require(0x4573, 'call', f'0x{base + 0x6afc0:x}')
        require(0x6afc3, 'call', f'0x{base + 0x6c390:x}')
        require(0x6afd8, 'call', f'dword ptr [0x{base + 0x80234:x}]')
        require(0x6afdf, 'jmp', f'0x{base + 0x6c3a0:x}')
        require(0x6c390, 'inc', f'dword ptr [0x{base + 0xc10ac:x}]')
        require(0x6c3a0, 'dec', f'dword ptr [0x{base + 0xc10ac:x}]')
        require(0x41fd, 'call', 'eax')
        require(0x421a, 'call', 'dword ptr [eax + esi*8]')
        require(0x4234, 'call', 'eax')
        require(0x423a, 'call', f'0x{base + 0x6afb0:x}')
        require(0x6afb7, 'call', f'dword ptr [0x{base + 0x801f0:x}]')
        imports = {entry.address: (dll.dll.decode(), (entry.name or b'').decode())
                   for dll in pe.DIRECTORY_ENTRY_IMPORT for entry in dll.imports}
        if imports.get(base + 0x801f0) != ('MSVCR71.dll', 'exit'):
            raise ValueError('sysExit no longer resolves to the audited CRT exit import')
        if imports.get(base + 0x80234) != ('USER32.dll', 'MessageBoxA'):
            raise ValueError('Fatal reporting message-box import changed')
        exports = {e.name.decode(): e.address for e in pe.DIRECTORY_ENTRY_EXPORT.symbols if e.name}
        registrations = {name: rva for name, rva in exports.items()
                         if any(name.startswith('?' + key + '@') for key in REGISTRATION_NAMES)}
        if len(registrations) != len(REGISTRATION_NAMES):
            raise ValueError('Expected callback-registration exports are missing or ambiguous')
        return {
            'sha256': digest,
            'runtime_executed': False,
            'native_integration_accepted': False,
            'fatal_error_callback_rva': '451c',
            'fatal_to_con_exit_rva': '457a',
            'exit_callbacks': {'pre': '41fd', 'list': '421a', 'post': '4234'},
            'terminal_import': 'MSVCR71.dll!exit',
            'pre_exit_modal_report': {'call_rva': '4573', 'import': 'USER32.dll!MessageBoxA',
                                     'paint_lock_counter_rva': 'c10ac'},
            'registration_exports': {name: f'{rva:x}' for name, rva in registrations.items()},
            'limits': [
                'Registered callback targets and descendants are not certified non-reentrant',
                'CRT exit is not a direct ExitProcess proof and may invoke termination callbacks',
                'No conclusion about dynamic plugins, callback exceptions, or live registrations',
                'No receipt hook, gameplay mutation, or physical-melee acceptance',
            ],
        }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game', required=True, type=Path)
    args = parser.parse_args()
    try:
        result = inspect(args.game / 'Bin' / 'Core.dll')
    except (OSError, ValueError, pefile.PEFormatError) as exc:
        parser.exit(1, f'Audit refused: {exc}\n')
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()
