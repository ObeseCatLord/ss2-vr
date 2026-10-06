#!/usr/bin/env python3
"""Inspect compiled DIP argument/caller forwarding; never execute Windows code."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import capstone
import pefile

ROOT = Path(__file__).resolve().parents[1]
GFX_SHA256 = '88749b79be36f0c0dccb623c4f1712685b5af603b451e25ed3c5029bedcdf3ed'


def symbols(path: Path) -> dict:
    result = {}
    output = subprocess.run(['i686-w64-mingw32-nm','-C',str(path)],check=True,capture_output=True,text=True)
    for line in output.stdout.splitlines():
        fields = line.split(maxsplit=2)
        if len(fields) == 3 and fields[1] in ('t','T'):
            result[fields[2]] = int(fields[0],16)
    return result


def verify(game: Path) -> dict:
    native = game/'Bin/GfxD3D.dll'
    if hashlib.sha256(native.read_bytes()).hexdigest() != GFX_SHA256:
        raise ValueError('Native DIP caller fingerprint changed')
    decoder = capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
    gfx = pefile.PE(str(native),max_symbol_exports=65536)
    call = list(decoder.disasm(gfx.get_data(0xa00b,6),gfx.OPTIONAL_HEADER.ImageBase+0xa00b))
    assert len(call)==1 and call[0].mnemonic=='call' and call[0].op_str=='dword ptr [ecx + 0x148]'
    assert call[0].address+call[0].size==gfx.OPTIONAL_HEADER.ImageBase+0xa011
    products = {}
    for filename in ('d3d9.dll','SS2VRServer.dll'):
        path = ROOT/'build-game'/filename
        table = symbols(path)
        hook = next(value for name,value in table.items()
                    if name.startswith('ss2vr::game::drawIndexed(') and name.endswith('@28'))
        target = next(value for name,value in table.items() if name.startswith('ss2vr::game::scopeGpuDraw('))
        pe = pefile.PE(str(path),max_symbol_exports=65536)
        assert pe.FILE_HEADER.Machine==0x14c
        code = list(decoder.disasm(pe.get_data(hook-pe.OPTIONAL_HEADER.ImageBase,96),hook))
        returned = next(i for i,insn in enumerate(code) if insn.mnemonic=='ret')
        code = code[:returned+1]
        pairs = [(i.mnemonic,i.op_str) for i in code]
        # 36-byte outgoing cdecl area: seven original arguments, actual native
        # return address, bridge forwarding callback. Incoming WINAPI retires28.
        assert pairs[0]==('sub','esp, 0x24') and pairs[-2:]==[('add','esp, 0x24'),('ret','0x1c')]
        assert pairs[1]==('mov','eax, dword ptr [esp + 0x24]')
        assert pairs[3]==('mov','dword ptr [esp + 0x1c], eax')
        assert pairs[2][0]=='mov' and pairs[2][1].startswith('dword ptr [esp + 0x20], 0x')
        assert pairs[4:18]==[
            ('mov','eax, dword ptr [esp + 0x40]'),('mov','dword ptr [esp + 0x18], eax'),
            ('mov','eax, dword ptr [esp + 0x3c]'),('mov','dword ptr [esp + 0x14], eax'),
            ('mov','eax, dword ptr [esp + 0x38]'),('mov','dword ptr [esp + 0x10], eax'),
            ('mov','eax, dword ptr [esp + 0x34]'),('mov','dword ptr [esp + 0xc], eax'),
            ('mov','eax, dword ptr [esp + 0x30]'),('mov','dword ptr [esp + 8], eax'),
            ('mov','eax, dword ptr [esp + 0x2c]'),('mov','dword ptr [esp + 4], eax'),
            ('mov','eax, dword ptr [esp + 0x28]'),('mov','dword ptr [esp], eax')]
        assert pairs[18]==('call',hex(target))
        products[filename]={'sha256':hashlib.sha256(path.read_bytes()).hexdigest(),
                            'dip_winapi_stack_retirement':28,'original_arguments_preserved':True,
                            'actual_native_caller_forwarded':True,'scope_gpu_cdecl_arguments':9}
    return {'runtime_executed':False,'windows_code_executed':False,
            'native_gfx_sha256':GFX_SHA256,'native_dip_return_rva':'0xa011','products':products,
            'limits':['Static compiled shape only; no COM invocation or exception recovery executed.']}


if __name__ == '__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game',type=Path,required=True)
    args=parser.parse_args()
    print(json.dumps(verify(args.game),indent=2))
