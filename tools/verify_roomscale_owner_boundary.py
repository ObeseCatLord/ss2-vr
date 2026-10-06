#!/usr/bin/env python3
"""Pinned post-simulation/normal-ray-cleanup evidence; not a live ownership lock."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import capstone
import pefile
PINS={
    'Core.dll':'7a1bd56b9bfa3edfbb23f4d3c96490e40a7c0b031b85e797e1af91b2ba3cf207',
    'Engine.dll':'da6efc9f72637eb3b6f48eadca2107be89b09c00618b6e72d5d3632938a7d851',
    'Sam2Game.dll':'5628b4ed30a966f10c8e8ea46bf0789257a35ea80127bacacebe28b1ce5303df',
}
def require(ok,message):
    if not ok:raise ValueError(message)

def verify(game):
    images={}
    for name,pin in PINS.items():
        data=(game/'Bin'/name).read_bytes()
        require(hashlib.sha256(data).hexdigest()==pin,'Unsupported image: '+name)
        p=pefile.PE(data=data,max_symbol_exports=100000)
        require(p.FILE_HEADER.Machine==0x14c and p.OPTIONAL_HEADER.ImageBase==0x10000000,
                'Unsupported ABI')
        images[name]=p
    core,engine,sam=(images[n] for n in ('Core.dll','Engine.dll','Sam2Game.dll'))
    md=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
    def decode(image,rva,size):
        return list(md.disasm(image.get_data(rva,size),0x10000000+rva))
    def check(image,rva,size,expected):
        instructions=decode(image,rva,size)
        values={i.address-0x10000000:(i.mnemonic,i.op_str) for i in instructions}
        for at,value in expected.items():require(values.get(at)==value,'Boundary changed: '+hex(at))
        return instructions
    imports={i.address:i.name for d in engine.DIRECTORY_ENTRY_IMPORT for i in d.imports}
    require(imports[0x10206708]==b'?ExecuteThread@CBaseThread@SeriousEngine@@QAEXXZ',
            'Worker execute import changed')
    require(imports[0x10206704]==b'?WaitOnThread@CBaseThread@SeriousEngine@@QAEXXZ',
            'Worker join import changed')
    require(imports[0x10206104]==b'?Remove@CListNode@SeriousEngine@@QAEXXZ',
            'Cleanup unlink import changed')
    check(engine,0x10ee90,0x70,{
        0x10eea0:('mov','dword ptr [esi + 0x10], eax'),
        0x10eeb8:('call','ebx'),0x10eec8:('call','0x1010ca70'),
        0x10eede:('call','ebx'),0x10eee6:('jl','0x1010eed8'),
        0x10eee9:('mov','dword ptr [esi + 0x10], 0'),
        0x10eef6:('mov','dword ptr [esi + 0x10], edi'),
        0x10eef3:('ret','4'),0x10eefd:('ret','4')})
    check(engine,0x1b70e0,0x339,{
        0x1b739b:('call','0x101b3b60'),0x1b7406:('call','0x1010d7c0'),
        0x1b740b:('mov','dword ptr [edi + 0x4c], 0'),0x1b7418:('ret','')})
    tail=decode(engine,0x1b740b,0xe)
    require(not any(i.mnemonic=='call' for i in tail),'Unexpected callback after physics return')
    sam_imports={i.address:i.name for d in sam.DIRECTORY_ENTRY_IMPORT for i in d.imports}
    require(sam_imports[0x10294948]==b'?Step@CSimulation@SeriousEngine@@QAEXXZ','Simulation caller changed')
    check(sam,0x258ce,0x28,{
        0x258ce:('mov','ecx, dword ptr [esi + 8]'),
        0x258d8:('call','dword ptr [0x10294948]'),
        0x258de:('mov','ecx, dword ptr [0x1029419c]')})
    for table,target in ((0x209260,0x28b50),(0x214700,0xd8880)):
        require(struct.unpack('<I',engine.get_data(table+4,4))[0]==0x10000000+target,
                'Known ray cleanup callback changed')
    for rva,size in ((0x28b50,0x67),(0xd8880,0x26)):
        instructions=decode(engine,rva,size)
        require(instructions[-1].mnemonic=='ret' and
                not any(i.mnemonic.startswith(('call','j')) for i in instructions),
                'Scalar cleanup gained callback/control flow')
    require([(i.mnemonic,i.op_str) for i in decode(core,0xf270,0x18)]==[
        ('mov','eax, dword ptr [ecx]'),('mov','edx, dword ptr [ecx + 4]'),
        ('mov','dword ptr [eax + 4], edx'),('mov','dword ptr [edx], eax'),
        ('mov','dword ptr [ecx], 0'),('mov','dword ptr [ecx + 4], 0'),('ret','')],
        'Native list unlink changed')
    check(engine,0x1b35e0,0xc5,{
        0x1b35e6:('mov','eax, dword ptr [0x102f1a14]'),
        0x1b35f4:('mov','edi, dword ptr [eax]'),
        0x1b35fa:('lea','esi, [eax - 4]'),
        0x1b3601:('call','dword ptr [eax + 4]'),
        0x1b3607:('call','ebx'),0x1b36a4:('ret','')})
    return {'fingerprints':PINS,'native_simulation_caller_return':'Sam2Game+258de',
            'post_physics_tail_has_no_callbacks':True,'known_scalar_cleanup_callbacks':2,
            'runtime_executed':False,'live_worker_or_callback_exclusion_proven':False}
if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game',type=Path,required=True)
    print(json.dumps(verify(parser.parse_args().game),indent=2))
