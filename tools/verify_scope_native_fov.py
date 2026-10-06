#!/usr/bin/env python3
"""Verify the owned pinned projection seam without executing native code."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import capstone
import pefile

PIN = '5628b4ed30a966f10c8e8ea46bf0789257a35ea80127bacacebe28b1ce5303df'


def verify(game):
    path = game / 'Bin/Sam2Game.dll'
    if hashlib.sha256(path.read_bytes()).hexdigest() != PIN:
        raise ValueError('Unknown Sam2Game build')
    pe = pefile.PE(str(path))
    base = pe.OPTIONAL_HEADER.ImageBase
    decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)

    def decode(rva, size):
        return list(decoder.disasm(pe.get_data(rva, size), base+rva))

    def pair(instruction):
        return instruction.mnemonic, instruction.op_str

    if [pair(i) for i in decode(0xf8050,7)] != [('fld','dword ptr [ecx + 0x858]'),('ret','')]:
        raise ValueError('Player FOV getter is not the side-effect-free scalar load')
    if struct.unpack('<I',pe.get_data(0x29e878+0x5f8,4))[0] != base+0xf8050:
        raise ValueError('Player virtual getter slot differs')
    calls = [i for i in decode(0x9433c,0x9436f-0x9433c) if i.mnemonic == 'call']
    if [(i.address-base,i.op_str) for i in calls] != [
            (0x94340,'dword ptr [eax + 0x5f8]'),(0x94369,f'dword ptr [{hex(base+0x2941e4)}]')]:
        raise ValueError('Intervening callback or different getter/frustum route')
    imported = [(entry.dll,imp.name) for entry in pe.DIRECTORY_ENTRY_IMPORT for imp in entry.imports
                if imp.address == base+0x2941e4]
    if imported != [(b'Core.dll',b'?mthFrustumFOVX@SeriousEngine@@YA?AVMatrix44f@1@MMMM@Z')]:
        raise ValueError('Frustum import differs')
    if pair(decode(0x94393,5)[0]) != ('call',hex(base+0x94200)):
        raise ValueError('Root projection call differs')
    for rva,expected in ((0x942db,0x43070000),(0x942e2,0x42340000)):
        instruction = decode(rva,7)
        if len(instruction)!=1 or instruction[0].mnemonic != 'mov' or not instruction[0].op_str.endswith(hex(expected)):
            raise ValueError('Native base-angle clamp bound differs')
    if struct.unpack('<f',pe.get_data(0x29bbc4,4))[0] != struct.unpack('<f',struct.pack('<f',3.141592653589793/180))[0]:
        raise ValueError('Native angle unit differs')
    if pair(decode(0x94346,3)[0]) != ('fmul','dword ptr [ebp - 0x18]') or \
       pair(decode(0x94354,6)[0]) != ('fmul',f'dword ptr [{hex(base+0x29bbc4)}]'):
        raise ValueError('Native FOV multiplier/base/unit ordering differs')
    return {'sam2game_sha256':PIN,'runtime_executed':False,'root_projection_return':'0x94398',
            'frustum_return':'0x9436f','player_getter':'0xf8050','getter_slot':'0x5f8',
            'getter_scalar_offset':'0x858','unzoomed_horizontal_degrees':[45,135],
            'intervening_native_calls':0,
            'limits':['Static pinned route only; live vtable and frozen ownership still checked',
                      'No source view, zoom mutation, GPU image or runtime verification']}


if __name__ == '__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game',type=Path,required=True)
    args=parser.parse_args()
    print(json.dumps(verify(args.game),indent=2))
