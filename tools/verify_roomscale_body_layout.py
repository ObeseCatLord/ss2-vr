#!/usr/bin/env python3
"""Verify native layout evidence for the inactive bounded body reader.

No game execution, live pointer access, shape admission or movement occurs.
"""
import argparse
import hashlib
import json
from pathlib import Path
import capstone
import pefile

PINS={
    'Engine.dll':'da6efc9f72637eb3b6f48eadca2107be89b09c00618b6e72d5d3632938a7d851',
    'Sam2Game.dll':'5628b4ed30a966f10c8e8ea46bf0789257a35ea80127bacacebe28b1ce5303df',
}


def require(value,message):
    if not value: raise ValueError(message)


def verify(game):
    images={}
    for name,pin in PINS.items():
        data=(game/'Bin'/name).read_bytes()
        require(hashlib.sha256(data).hexdigest()==pin,'Fingerprint mismatch: '+name)
        pe=pefile.PE(data=data)
        require(pe.OPTIONAL_HEADER.ImageBase==0x10000000 and pe.FILE_HEADER.Machine==0x14c,
                'Unsupported ABI: '+name)
        images[name]=pe
    engine=images['Engine.dll'];sam=images['Sam2Game.dll']
    exports={x.name:x.address for x in engine.DIRECTORY_ENTRY_EXPORT.symbols if not x.forwarder}
    for symbol,address in {
        b'??_7CHybridBody@SeriousEngine@@6B@':0x217af0,
        b'??_7CPrimitiveHull@SeriousEngine@@6B@':0x209268,
        b'?GetRootBody@CMechanism@SeriousEngine@@QAEPAVCBody@2@XZ':0x133b60,
        b'?GetAbsPlacement@CMechanism@SeriousEngine@@QAE?AVQuatVect@2@XZ':0x130b20,
        b'?GetAbsPlacement@CAspect@SeriousEngine@@QAE?AVQuatVect@2@XZ':0x26ff0,
    }.items():require(exports.get(symbol)==address,'Layout export changed: '+symbol.decode())
    md=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
    def check(pe,ranges,expected):
        decoded={i.address-0x10000000:(i.mnemonic,i.op_str)
                 for start,size in ranges
                 for i in md.disasm(pe.get_data(start,size),0x10000000+start)}
        for rva,instruction in expected.items():
            require(decoded.get(rva)==instruction,'Layout instruction changed: '+hex(rva))
    check(engine,[(0x26ff0,0x1a),(0x58d40,4),(0x58f50,4),(0x1bd5c0,4),
                  (0x108fb0,4),(0x133b60,0x11),(0x130b20,0x50),(0x52030,0x160)],{
        0x26ff8:('lea','esi, [ecx + 0x2c]'),0x26ffb:('mov','ecx, 7'),
        0x27007:('ret','4'),0x58d40:('mov','eax, dword ptr [ecx + 4]'),
        0x58f50:('mov','eax, dword ptr [ecx + 0xc]'),
        0x1bd5c0:('mov','eax, dword ptr [ecx + 8]'),
        0x108fb0:('mov','eax, dword ptr [ecx + 0x48]'),
        0x133b60:('mov','eax, dword ptr [ecx + 8]'),
        0x133b6a:('mov','eax, dword ptr [ecx + 4]'),
        0x133b6d:('mov','eax, dword ptr [eax + 8]'),
        0x130b26:('mov','eax, dword ptr [esi + 0x38]'),
        0x130b57:('lea','esi, [eax + 0x2c]'),
        0x52105:('mov','ecx, dword ptr [edi + 0x48]'),
        0x5210e:('mov','edx, dword ptr [edi + 0x70]'),
        0x52117:('mov','eax, dword ptr [edi + 0x54]'),
    })
    check(sam,[(0x83b90,0x2e0)],{
        0x83cf1:('mov','dword ptr [ebx + 0x114], eax'),
        0x83cf9:('mov','edx, dword ptr [ebx + 0x120]'),
        0x83d10:('call','dword ptr [0x10294d40]'),
        0x83d32:('call','dword ptr [0x10294d38]'),
        0x83d3b:('mov','dword ptr [ebx + 0x118], eax'),
    })
    imports={i.address:i.name for d in sam.DIRECTORY_ENTRY_IMPORT for i in d.imports}
    require(imports[0x10294d40]==b'?CreateMechanism@CMechanism@SeriousEngine@@QAEXVIDENT@2@PAVCModelRenderable@2@@Z',
            'Player model mechanism binding changed')
    require(imports[0x10294d38]==b'?GetRootBody@CMechanism@SeriousEngine@@QAEPAVCBody@2@XZ',
            'Player root-body binding changed')
    return {'fingerprints':PINS,'runtime_executed':False,'live_avatar_shape_verified':False,
            'body_reader_active':False,'native_model_and_root_are_distinct_roles':True}


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game',type=Path,required=True)
    print(json.dumps(verify(parser.parse_args().game),indent=2))
