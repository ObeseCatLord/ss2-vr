#!/usr/bin/env python3
"""Pin palette consumers and the limits of native loading-queue observations."""
import argparse
import hashlib
import json
from pathlib import Path
import capstone
import pefile


def require(ok, message):
    if not ok:
        raise ValueError(message)


def verify(game):
    pins = {'Engine.dll': 'da6efc9f72637eb3b6f48eadca2107be89b09c00618b6e72d5d3632938a7d851',
            'Core.dll': '7a1bd56b9bfa3edfbb23f4d3c96490e40a7c0b031b85e797e1af91b2ba3cf207'}
    images = {}
    for name, expected in pins.items():
        data = (game / 'Bin' / name).read_bytes()
        require(hashlib.sha256(data).hexdigest() == expected, 'Unsupported ' + name)
        image = pefile.PE(data=data, max_symbol_exports=100000)
        require(image.FILE_HEADER.Machine == 0x14c, 'Expected x86 native image')
        images[name] = image
    decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)

    def decode(name, start, end):
        image = images[name]
        base = image.OPTIONAL_HEADER.ImageBase
        result = {i.address-base: (i.mnemonic, i.op_str)
                  for i in decoder.disasm(image.get_data(start, end-start), base+start)}
        require(start in result, 'Missing native function start')
        return result

    def at(code, address, mnemonic, operands):
        require(code.get(address) == (mnemonic, operands), 'Native boundary changed: ' + hex(address))

    producer = decode('Engine.dll', 0xdde30, 0xddec7)
    at(producer, 0xdde80, 'mov', 'eax, dword ptr [0x102eac94]')
    at(producer, 0xdde8e, 'mov', 'eax, dword ptr [ecx + edx*8 + 4]')
    at(producer, 0xdde9f, 'mov', 'ecx, dword ptr [eax + 0x20]')
    at(producer, 0xddeb7, 'add', 'ebx, 0x30')
    at(producer, 0xddebc, 'rep movsd', 'dword ptr es:[edi], dword ptr [esi]')
    cpu = decode('Engine.dll', 0xdf5b0, 0xe11f3)
    at(cpu, 0xdf649, 'mov', 'esi, dword ptr [ecx + 4]')
    at(cpu, 0xdf847, 'add', 'ecx, dword ptr [0x102eac94]')
    for address, offset in [(0xdf85d, '0xc'), (0xdf875, '0x1c'), (0xdf88c, '0x2c')]:
        at(cpu, address, 'fadd', 'dword ptr [ecx + ' + offset + ']')
    at(cpu, 0xdfff7, 'mov', 'dword ptr [0x102eabc8], eax')
    at(cpu, 0xe02b4, 'add', 'eax, dword ptr [0x102eac94]')
    direction = [op for address, op in cpu.items() if 0xe02be <= address <= 0xe02f2]
    require(sum(m == 'fmul' for m, _ in direction) == 9 and
            all(m != 'fadd' for m, _ in direction), 'Direction transform must omit translation')
    at(cpu, 0xe08d7, 'mov', 'dword ptr [0x102eabcc], eax')
    at(cpu, 0xe09b6, 'add', 'ecx, dword ptr [0x102eac94]')
    at(cpu, 0xe11a4, 'mov', 'edx, dword ptr [ecx - 0xc]')
    at(cpu, 0xe11a7, 'mov', 'dword ptr [eax + 0xc], edx')
    at(cpu, 0xe11c4, 'mov', 'dword ptr [0x102eabd0], eax')
    require({address: op for address, (m, op) in cpu.items() if m == 'call'} == {
        0xdf63a: 'dword ptr [0x10206160]', 0xdf6ce: '0x10051770',
        0xe0047: '0x10051770', 0xe091f: '0x100dd1c0',
        0xe11dc: 'dword ptr [0x10206154]'}, 'CPU consumer call topology changed')
    gpu = decode('Engine.dll', 0x77590, 0x775d1)
    at(gpu, 0x775c1, 'lea', 'ecx, [eax + eax*2]')
    at(gpu, 0x775c6, 'call', 'dword ptr [0x102e62fc]')

    pending = decode('Core.dll', 0x63090, 0x630ba)
    at(pending, 0x630a8, 'mov', 'esi, dword ptr [ecx + 0x3c]')
    pop = decode('Core.dll', 0x62f00, 0x62f8e)
    at(pop, 0x62f69, 'dec', 'eax')
    at(pop, 0x62f6d, 'mov', 'dword ptr [esi + 0x3c], eax')
    worker = decode('Core.dll', 0x63140, 0x631fa)
    at(worker, 0x6317a, 'call', '0x10062f00')
    at(worker, 0x6319a, 'call', '0x10013f30')
    at(worker, 0x631a7, 'call', 'dword ptr [edx + 8]')
    at(worker, 0x631d6, 'mov', 'dword ptr [edi + 8], 1')
    wait = decode('Core.dll', 0x632f0, 0x6332e)
    at(wait, 0x63312, 'call', '0x10062f00')
    at(wait, 0x63322, 'call', 'dword ptr [eax + 8]')
    return {'fingerprints': pins, 'shared_palette_cpu_position_and_direction_channels': True,
            'gpu_matrix_register_stride': 3, 'pending_count_proves_worker_idle': False,
            'wait_until_completed_can_execute_jobs_on_caller': True,
            'complete_model_lifetime_proven': False, 'runtime_executed': False}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game', type=Path, required=True)
    print(json.dumps(verify(parser.parse_args().game), indent=2))
