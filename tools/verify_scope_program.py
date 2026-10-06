#!/usr/bin/env python3
"""Cross-check mod-owned UV token fixtures with native Linux vkd3d and the parser.

No owned game shader is embedded, compiled or executed. Actual native shader
compatibility and source-image admission remain unverified.
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
    output = ROOT/'build-scope-shader/scope-program'
    output.mkdir(parents=True, exist_ok=True)
    environment = os.environ.copy()
    environment['LD_LIBRARY_PATH'] = str(compiler.parent) + (
        ':'+environment['LD_LIBRARY_PATH'] if environment.get('LD_LIBRARY_PATH') else '')
    direct, direct_asm = output/'direct.vs11.bin', output/'direct.vs11.asm'
    generated = json.loads(subprocess.run([str(checker),'--write-fixture',str(direct)],
        check=True,capture_output=True,text=True,timeout=10).stdout)
    if generated != {'mod_owned_direct_uv_fixture_words':19} or \
       type(generated['mod_owned_direct_uv_fixture_words']) is not int:
        raise ValueError('Unexpected mod-owned fixture producer result')
    subprocess.run([str(compiler),'-x','d3dbc','-b','d3d-asm','-o',str(direct_asm),str(direct)],
        check=True,env=environment,timeout=10)
    expected = ['vs_1_1','dcl_texcoord0 v0','dcl_texcoord3 v3','mov oPos.xyzw, v0.xyzw',
                'dp4 oT3.x, v3.xyzw, c8.xyzw','dp4 oT3.y, v3.xyzw, c9.xyzw']
    if [line.strip() for line in direct_asm.read_text().splitlines() if line.strip()] != expected:
        raise ValueError('Independent compiler did not decode the expected direct UV interface')
    indirect = output/'indirect.vs11.bin'
    subprocess.run([str(compiler),'-x','hlsl','-b','d3dbc','-p','vs_1_1','--strip-debug',
        '-o',str(indirect),str(ROOT/'tests/shaders/scope_uv_interface.hlsl')],
        check=True,env=environment,timeout=10)
    checked = []
    for binary, admission in ((direct,True),(indirect,False)):
        data = binary.read_bytes()
        if len(data)<8 or len(data)%4 or len(data)>16384 or struct.unpack_from('<I',data)[0]!=0xfffe0101:
            raise ValueError('Fixture compiler did not produce bounded VS1.1')
        result = json.loads(subprocess.run([str(checker),str(binary),'accept' if admission else 'reject'],
            check=True,capture_output=True,text=True,timeout=10).stdout)
        if result != {'offline_uv_program_admission':admission} or \
           type(result['offline_uv_program_admission']) is not bool:
            raise ValueError('Unexpected shader checker result')
        checked.append({'fixture':binary.name,'sha256':hashlib.sha256(data).hexdigest(),
                        'bytes':len(data),'admitted':admission})
    return {'runtime_executed':False,'windows_code_executed':False,'native_game_shader_observed':False,
            'offline_linux_compiler_executed':True,'offline_linux_checker_executed':True,
            'compiler_sha256':hashlib.sha256(compiler.read_bytes()).hexdigest(),
            'checker_sha256':hashlib.sha256(checker.read_bytes()).hexdigest(),
            'independent_direct_uv_decode':True,'compiled_hlsl_fixture_rejected':True,'fixtures':checked}


if __name__ == '__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--compiler',type=Path,required=True)
    parser.add_argument('--checker',type=Path,default=ROOT/'build-core/scope_program_checks')
    args=parser.parse_args()
    print(json.dumps(verify(args.compiler,args.checker),indent=2))
