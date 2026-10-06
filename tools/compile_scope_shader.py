#!/usr/bin/env python3
"""Compile mod-owned HLSL with an ELF vkd3d compiler; never run Windows code.

No compiled shader is installed or enabled by this tool. The binary/assembly
are build artifacts only, outside tracked source. Native draw admission remains
mandatory. vkd3d 1.17 supports HLSL -> legacy D3D9 bytecode on Linux.
"""
import argparse
import hashlib
import json
import os
import re
from pathlib import Path
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def compile_shader(compiler: Path, output: Path, entry: str = 'main') -> dict:
    if entry not in ('main','opaque'):
        raise ValueError('Unknown scope image entry')
    compiler = compiler.resolve(strict=True)
    # Automake's public executable can be a shell wrapper. Require the actual
    # ELF entry instead, preventing an accidental fxc.exe/Wine route.
    if compiler.read_bytes()[:4] != b'\x7fELF':
        raise ValueError('Compiler must be the native Linux ELF executable')
    output = output.resolve()
    if not output.is_relative_to(ROOT/'build-scope-shader'):
        raise ValueError('Shader output must stay inside build-scope-shader')
    output.mkdir(parents=True, exist_ok=True)
    source = ROOT/'src/game/shaders/scope_image.hlsl'
    stem = 'scope_image' if entry == 'main' else 'scope_image_opaque'
    binary = output/(stem+'.ps2a.bin')
    assembly = output/(stem+'.ps2a.asm')
    environment = os.environ.copy()
    environment['LD_LIBRARY_PATH'] = str(compiler.parent) + (
        ':'+environment['LD_LIBRARY_PATH'] if environment.get('LD_LIBRARY_PATH') else '')
    subprocess.run([str(compiler),'-x','hlsl','-b','d3dbc','-p','ps_2_a','-e',entry,
                    '--strip-debug','-o',str(binary),str(source)],check=True,env=environment)
    data = binary.read_bytes()
    if len(data) < 8 or len(data)%4 or len(data)>65536 or \
       struct.unpack_from('<I',data)[0] != 0xffff0201 or \
       struct.unpack_from('<I',data,len(data)-4)[0] != 0xffff:
        raise ValueError('Compiler did not produce bounded legacy PS2-extended bytecode')
    subprocess.run([str(compiler),'-x','d3dbc','-b','d3d-asm','-o',str(assembly),
                    str(binary)],check=True,env=environment)
    # This straight-line program uses single-slot opcodes only. Do not accept a
    # larger/changed shader merely because the compiler emitted a version token.
    lines=assembly.read_text().splitlines()
    declarations={'ps_2_x','ps_2_1','dcl','dcl_2d','def'}
    opcodes={'mov','mov_sat','dp3','add','max','rcp','mul','cmp','abs','texld'}
    instructions=[line.split()[0] for line in lines if line and line.split()[0] not in declarations]
    if not instructions or set(instructions)-opcodes or len(instructions)>512:
        raise ValueError('Unexpected shader opcode/slot budget')
    registers={kind:{int(x) for x in re.findall(r'\b'+kind+r'(\d+)\b',assembly.read_text())}
               for kind in ('r','c','s','t','v')}
    samplers, colors = ({0,3},{0}) if entry == 'main' else ({0},set())
    if max(registers['r'],default=-1)>=22 or max(registers['c'],default=-1)>8 or \
       registers['s']!=samplers or registers['t']!={3} or registers['v']!=colors:
        raise ValueError('Unexpected shader register interface')
    outputs=set(re.findall(r'\b(?:oDepth|oC\d+)\b',assembly.read_text()))
    if outputs!={'oC0'}:
        raise ValueError('Scope image shader must output only color0, never depth or MRT')
    return {'entry':entry,'profile':'ps_2_a','encoded_version':'2.1',
            'alpha_contract':'PP COLOR0 only; unadmitted' if entry == 'main' else
                             'RGB-only pass; requires masked alpha writes and opaque native admission',
            'instruction_slots':len(instructions),'temporary_registers':max(registers['r'])+1,
            'only_color0_output':True,
            'runtime_caps_required':{'PS20Caps.NumTemps':22,'PS20Caps.NumInstructionSlots':512,
                'PS20Caps.Caps':['ARBITRARYSWIZZLE','GRADIENTINSTRUCTIONS','PREDICATION',
                                'NODEPENDENTREADLIMIT','NOTEXINSTRUCTIONLIMIT']},
            'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),
            'bytecode_sha256':hashlib.sha256(data).hexdigest(),'bytecode_bytes':len(data),
            'native_linux_compiler_sha256':hashlib.sha256(compiler.read_bytes()).hexdigest(),
            'runtime_executed':False,'image_substitution_enabled_by_tool':False}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler',type=Path,required=True)
    parser.add_argument('--entry',choices=('main','opaque'),default='main')
    parser.add_argument('--emit-header',action='store_true',help='Emit mod-owned opaque shader header after verification')
    args = parser.parse_args()
    result=compile_shader(args.compiler,ROOT/'build-scope-shader',args.entry)
    if args.emit_header:
        if args.entry!='opaque': raise ValueError('Only the RGB-only entry may be embedded')
        data=(ROOT/'build-scope-shader/scope_image_opaque.ps2a.bin').read_bytes()
        words=struct.unpack('<'+'I'*(len(data)//4),data)
        lines=['// Generated by tools/compile_scope_shader.py --entry opaque --emit-header',
               '// HLSL SHA256: '+result['source_sha256'],
               '// Bytecode SHA256: '+result['bytecode_sha256'],
               '#pragma once','#include <cstdint>','namespace ss2vr::game {',
               'inline constexpr uint32_t ScopeImageOpaqueProgram[]{']
        lines += ['    '+','.join(f'0x{x:08x}u' for x in words[i:i+8])+',' for i in range(0,len(words),8)]
        lines += ['};','} // namespace ss2vr::game','']
        (ROOT/'src/game/shaders/scope_image_opaque.hpp').write_text('\n'.join(lines))
    print(json.dumps(result,indent=2))
