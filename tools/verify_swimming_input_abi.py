#!/usr/bin/env python3
"""Static evidence for native water-input admission. Never executes the game."""
import argparse
import hashlib
import json
import struct
import re
import subprocess
from pathlib import Path
import capstone
import pefile

PIN = '5628b4ed30a966f10c8e8ea46bf0789257a35ea80127bacacebe28b1ce5303df'


def require(value, message):
    if not value:
        raise ValueError(message)


def verify(game):
    data = (game / 'Bin/Sam2Game.dll').read_bytes()
    require(hashlib.sha256(data).hexdigest() == PIN, 'Unsupported Sam2Game fingerprint')
    pe = pefile.PE(data=data, max_symbol_exports=100000)
    require(pe.FILE_HEADER.Machine == 0x14c and pe.OPTIONAL_HEADER.ImageBase == 0x10000000,
            'Unsupported native ABI')
    exports = {s.name.decode(): s.address for s in pe.DIRECTORY_ENTRY_EXPORT.symbols if s.name}
    expected_exports = {
        '?ProcessPlayerControls@CPlayerBrainEntity@SeriousEngine@@UAEXEVVector3f@2@0@Z': 0xee0f0,
        '?GetOperatorMoveDir@CLeggedPuppetEntity@SeriousEngine@@UAE?AVVector3f@2@V32@0@Z': 0x60d60,
        '?MovingIn3DArea@CLeggedPuppetEntity@SeriousEngine@@UAEHXZ': 0x61030,
        '?ProcessOperatorInput@CPuppetEntity@SeriousEngine@@UAEXXZ': 0x8d930,
        '??_7CPlayerPuppetEntity@SeriousEngine@@6B@': 0x29e878,
        '??_7CPlayerBrainEntity@SeriousEngine@@6B@': 0x29be90,
    }
    for name, rva in expected_exports.items():
        require(exports.get(name) == rva, 'Native export changed: ' + name)
    def u32(rva): return struct.unpack('<I', pe.get_data(rva, 4))[0]
    for slot, target in ((0x518, 0x60d60), (0x57c, 0x61030)):
        require(u32(0x29e878 + slot) == 0x10000000 + target, 'Player movement vtable changed')
    for slot, target in ((0x384, 0xee0f0), (0x388, 0xee260)):
        require(u32(0x29be90 + slot) == 0x10000000 + target, 'Brain control vtable changed')
    imports = {i.address: i.name for d in pe.DIRECTORY_ENTRY_IMPORT for i in d.imports}
    require(imports.get(0x10294298) == b'?mthEulerToMatrix@SeriousEngine@@YA?AVMatrix33f@1@ABVVector3f@1@@Z',
            'Native movement orientation import changed')
    require(imports.get(0x102951c8) == b'?GetCommandValue@CInputBindings@SeriousEngine@@QAEMVIDENT@2@@Z',
            'Native axis source changed')
    # Native enum records are {value, internal-name pointer, display-name pointer}.
    for rva, value, name in ((0x42816c, 3, b'PP_DIVE'), (0x428178, 4, b'PP_SWIM')):
        require(u32(rva) == value, 'Water pose value changed')
        require(pe.get_data(u32(rva + 4) - 0x10000000, len(name) + 1) == name + b'\0',
                'Water pose name changed')
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    ranges = [(0x61030, 0x27), (0x60d60, 0xaf), (0xee0f0, 0x164),
              (0xee260, 0x5e0), (0x8d930, 0xa0), (0xf2df0, 0xcf7)]
    decoded = {i.address - 0x10000000: (i.mnemonic, i.op_str)
               for start, length in ranges
               for i in md.disasm(pe.get_data(start, length), 0x10000000 + start)}
    expected = {
        0x61030: ('mov', 'eax, dword ptr [ecx + 0x4cc]'),
        0x61036: ('test', 'al, 2'), 0x61038: ('jne', '0x10061051'),
        0x6103a: ('test', 'al, 4'), 0x6103c: ('je', '0x1006104e'),
        0x6103e: ('mov', 'eax, dword ptr [ecx + 0x610]'),
        0x61044: ('cmp', 'eax, 4'), 0x61047: ('je', '0x10061051'),
        0x61049: ('cmp', 'eax, 3'), 0x6104c: ('je', '0x10061051'),
        0x6104e: ('xor', 'eax, eax'), 0x61051: ('mov', 'eax, 1'),
        0x60d6b: ('call', 'dword ptr [eax + 0x57c]'),
        0x60d73: ('cmp', 'dword ptr [esi + 0x610], 4'),
        0x60d7d: ('fld', 'dword ptr [ebp + 0x10]'),
        0x60d80: ('fcomp', 'dword ptr [0x102a749c]'),
        0x60d8b: ('jne', '0x10060d8f'), 0x60d8d: ('xor', 'ecx, ecx'),
        0x60da5: ('mov', 'dword ptr [ebp - 8], ecx'),
        0x60da8: ('mov', 'dword ptr [ebp - 4], ecx'),
        0x60db3: ('call', 'dword ptr [0x10294298]'),
        0x60dbf: ('mov', 'eax, dword ptr [ebp + 8]'),
        0x60e0c: ('ret', '0x1c'),
        0xee211: ('mov', 'eax, dword ptr [ebp + 0x18]'),
        0xee229: ('mov', 'eax, dword ptr [ebp + 0xc]'),
        0xee245: ('call', 'dword ptr [edx + 0x388]'),
        0xee251: ('ret', '0x1c'),
        0xee387: ('mov', 'eax, dword ptr [ebp + 0x18]'),
        0xee38a: ('mov', 'edx, dword ptr [ebp + 0x1c]'),
        0xee394: ('mov', 'eax, dword ptr [ebp + 0x20]'),
        0xee568: ('mov', 'edx, dword ptr [ebp + 0x18]'),
        0xee56b: ('mov', 'eax, dword ptr [ebp + 0x1c]'),
        0xee56e: ('lea', 'ecx, [edi + 0x144]'),
        0xee574: ('mov', 'dword ptr [ecx], edx'),
        0xee579: ('mov', 'dword ptr [ecx + 4], eax'),
        0xee57c: ('mov', 'dword ptr [ecx + 8], edx'),
        0x8d98e: ('add', 'eax, 0x144'), 0x8d9a8: ('add', 'ecx, 0x138'),
        0x8d9c9: ('call', 'dword ptr [ebx + 0x518]'),
        0xf2f5d: ('call', 'dword ptr [0x102951c8]'),
        0xf2f6c: ('call', 'dword ptr [0x102951c8]'),
        0xf2f72: ('fsubr', 'dword ptr [ebp - 0x1c]'),
        0xf2f85: ('mov', 'dword ptr [ebp - 0x24], 0x3f800000'),
        0xf35cd: ('call', 'dword ptr [edx + 0x384]'),
    }
    # Exact canonical disassembly, with explicit exceptions rather than asserts,
    # remains enforced under Python -O.
    for rva, instruction in expected.items():
        require(decoded.get(rva) == instruction, 'Swimming instruction changed: ' + hex(rva))
    require(struct.unpack('<f', pe.get_data(0x2a749c, 4))[0] == -0.5235987901687622,
            'Native surface-swimming pitch threshold changed')
    return {'sam_sha256': PIN, 'dive_pose': 3, 'swim_pose': 4,
            'native_movement_vector_offset': '0x144', 'native_look_offset': '0x138',
            'native_rpc_carries_all_three_movement_components': True,
            'runtime_executed': False, 'native_water_input_path_verified': True}


def verify_object(path):
    """Pinned GNU x86 production wrapper check, not Windows execution."""
    table = subprocess.check_output(['objdump', '-t', '-C', str(path)], text=True)
    assembly = subprocess.check_output(['objdump', '-dr', '-C', '-Mintel', str(path)], text=True)
    symbols = {}
    for name in ('originalPlayerControls', 'nativeOperatorMoveDir'):
        matches = re.findall(r'0x([0-9a-f]+) ss2vr::game::' + name + r'$', table, re.M)
        require(len(matches) == 1, 'Missing production symbol: ' + name)
        symbols[name] = int(matches[0], 16)
    blocks = re.split(r'^([0-9a-f]+ <[^\n]+>):\n', assembly, flags=re.M)
    bodies = [body for title, body in zip(blocks[1::2], blocks[2::2])
              if 'L19waterPlayerControlsEPvS1_hNS_4Vec3ES2_@36>' in title]
    require(len(bodies) == 1, 'Missing production fastcall wrapper')
    code = []
    for line in bodies[0].splitlines():
        columns = line.split('\t')
        if len(columns) >= 3 and re.fullmatch(r'\s*[0-9a-f]+:', columns[0]):
            instruction = ' '.join(columns[-1].split())
            if re.match('[a-z]', instruction): code.append(instruction)
    require(code[:3] == ['push edi', 'lea edi,[esp+0x8]', 'and esp,0xfffffff8'],
            'Incoming argument anchor changed')
    require('mov ebx,edi' in code[:12], 'Incoming argument anchor not retained')
    this = re.search(r'mov DWORD PTR (\[ebp-0x[0-9a-f]+\]),ecx', '\n'.join(code[:25]))
    fire = re.search(r'mov BYTE PTR (\[ebp-0x[0-9a-f]+\]),al', '\n'.join(code[:25]))
    require(this and fire and 'movzx eax,BYTE PTR [edi]' in code[:25], 'Receiver/fire capture changed')
    original = 'DWORD PTR ds:0x%x' % symbols['originalPlayerControls']
    calls = [i for i, value in enumerate(code) if value == 'call ' + original]
    require(len(calls) == 1 and code.count('jmp ' + original) == 1, 'Native forwarding paths changed')
    expected = []
    for offset in (0x14, 0x18, 4, 8, 12):
        expected += [f'mov eax,DWORD PTR [ebx+0x{offset:x}]',
                     f'mov DWORD PTR [esp+0x{offset:x}],eax']
    expected += ['movzx eax,BYTE PTR ' + fire.group(1), 'mov DWORD PTR [esp],eax',
                 'mov ecx,DWORD PTR ' + this.group(1)]
    i = calls[0]
    require(code[i-len(expected):i] == expected, 'Native look/fire/move slots changed')
    prefix = code[i-len(expected)-4:i-len(expected)]
    require(prefix[0] == 'mov eax,DWORD PTR [ebx+0x10]' and
            prefix[-1] == 'mov DWORD PTR [esp+0x10],eax', 'First movement component changed')
    require(code[i+1] == 'sub esp,0x1c' and code.count('ret 0x1c') == 1,
            'Native callee stack cleanup changed')
    direction = 'call DWORD PTR ds:0x%x' % symbols['nativeOperatorMoveDir']
    sites = [i for i, value in enumerate(code) if value == direction]
    require(len(sites) == 4, 'Expected three basis queries and one desired-direction query')
    for i in sites:
        require('sub esp,0x1c' in code[i+1:i+10], 'Direction query cleanup changed')
        require(any(v.startswith('cmp eax,') for v in code[i+1:i+12]), 'Hidden output identity unchecked')
    return {'object': str(path), 'stack_argument_bytes': 28, 'direction_calls': 4,
            'native_call_and_tail_forward_verified': True, 'windows_code_executed': False}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game', required=True, type=Path)
    parser.add_argument('--object', type=Path)
    args = parser.parse_args()
    report = verify(args.game)
    if args.object: report['compiled_wrapper'] = verify_object(args.object)
    print(json.dumps(report, indent=2))
