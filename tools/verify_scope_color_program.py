#!/usr/bin/env python3
"""Independently decode the mod-owned PS2 fixture using a native Linux tool.

No game shader or Windows executable is run. This checks token encoding, not
actual native shader coverage or image rendering.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def verify(compiler: Path, checker: Path) -> dict:
    compiler, checker = compiler.resolve(strict=True), checker.resolve(strict=True)
    for executable in (compiler, checker):
        if executable.read_bytes()[:4] != b'\x7fELF':
            raise ValueError('Only native Linux ELF compiler/checker may execute')
    output = ROOT/'build-scope-shader/scope-color'
    output.mkdir(parents=True, exist_ok=True)
    environment = os.environ.copy()
    environment['LD_LIBRARY_PATH'] = str(compiler.parent) + (
        ':'+environment['LD_LIBRARY_PATH'] if environment.get('LD_LIBRARY_PATH') else '')
    binary, assembly = output/'direct.ps20.bin', output/'direct.ps20.asm'
    generated = json.loads(subprocess.run([str(checker),'--write-fixture',str(binary)],
        check=True,capture_output=True,text=True,timeout=10).stdout)
    if generated != {'mod_owned_color_fixture_words':15} or \
       type(generated['mod_owned_color_fixture_words']) is not int:
        raise ValueError('Unexpected mod-owned fixture producer result')
    data = binary.read_bytes()
    if len(data) != 60 or struct.unpack_from('<I',data)[0] != 0xffff0200:
        raise ValueError('Unexpected bounded PS2.0 fixture')
    subprocess.run([str(compiler),'-x','d3dbc','-b','d3d-asm','-o',str(assembly),str(binary)],
        check=True,env=environment,timeout=10)
    expected = ['ps_2_0','dcl t3','dcl_2d s3',
                'texld r0.xyzw, t3.xyzw, s3.xyzw','mov oC0.xyzw, r0.xyzw']
    if [line.strip() for line in assembly.read_text().splitlines() if line.strip()] != expected:
        raise ValueError('Independent compiler did not decode the expected color program')
    return {'runtime_executed':False,'windows_code_executed':False,'native_game_shader_observed':False,
            'offline_linux_compiler_executed':True,'offline_linux_checker_executed':True,
            'compiler_sha256':hashlib.sha256(compiler.read_bytes()).hexdigest(),
            'checker_sha256':hashlib.sha256(checker.read_bytes()).hexdigest(),
            'independent_instruction_register_decode':True,
            'declaration_mask_independently_verified':False,
            'fixture':{'name':binary.name,'sha256':hashlib.sha256(data).hexdigest(),'bytes':len(data)}}


if __name__ == '__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler',type=Path,required=True)
    parser.add_argument('--checker',type=Path,default=ROOT/'build-core/scope_color_program_checks')
    args=parser.parse_args()
    print(json.dumps(verify(args.compiler,args.checker),indent=2))
