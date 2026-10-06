#!/usr/bin/env python3
"""Verify native layout evidence for the inactive bounded body reader.

No game execution, live pointer access, shape admission or movement occurs.
"""
import argparse
import hashlib
import json
import struct
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
        pe=pefile.PE(data=data,max_symbol_exports=100000)
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
    require(struct.unpack('<d',engine.get_data(0x208d70,8))[0]==1.,'Quaternion matrix identity scalar changed')
    check(engine,[(0x525b0,0xc6)],{
        0x525be:('lea','esi, [ebx + 0x2c]'),
        0x525c1:('mov','ecx, 7'),
        0x525eb:('fstp','dword ptr [ebp - 4]'),
        0x525f6:('fstp','dword ptr [ebp - 0x10]'),
        0x52609:('fstp','dword ptr [ebp - 0x60]'),
        0x52614:('fstp','dword ptr [ebp - 0x5c]'),
        0x5262a:('fstp','dword ptr [ebp - 0x3c]'),
        0x52631:('fstp','dword ptr [ebp - 0x38]'),
        0x52639:('fstp','dword ptr [ebp - 0x34]'),
        0x5263e:('fstp','dword ptr [ebp - 0x30]'),
        0x52649:('fstp','dword ptr [ebp - 0x2c]'),
        0x52652:('fstp','dword ptr [ebp - 0x28]'),
        0x52658:('fstp','dword ptr [ebp - 0x24]'),
        0x52663:('fstp','dword ptr [ebp - 0x20]'),
        0x52671:('fstp','dword ptr [ebp - 0x1c]'),
    })
    expansion=list(md.disasm(engine.get_data(0x525cb,0xaa),0x100525cb))
    require(not any(i.mnemonic in ('call','jmp','ret') for i in expansion),
            'Raw quaternion expansion gained a normalization/callback boundary')
    # Root SetRelPlacement invokes OnMoved only when bit0 is active. The
    # child's OnMoved callback is equally required for broadphase maintenance.
    for table,target in ((0x217af0,0x10f460),(0x209268,0x509d0)):
        require(struct.unpack('<I',engine.get_data(table+0x1c,4))[0]==0x10000000+target,
                'Root/hull propagation vtable changed')
    check(engine,[(0x571d0,0x3f5),(0x57b70,0x400),(0x10f460,0x6c),(0x509d0,0x65)],{
        0x571db:('mov','eax, dword ptr [edx + 4]'),
        0x571f1:('lea','esi, [eax + 0x2c]'),
        0x57207:('lea','eax, [edx + 0x10]'),
        0x5741d:('rep movsd','dword ptr es:[edi], dword ptr [esi]'),
        0x574e7:('lea','edi, [edx + 0x2c]'),
        0x5759e:('test','byte ptr [edx + 0x4c], 1'),
        0x575bc:('call','dword ptr [eax + 0x1c]'),
        0x10f4b9:('call','0x10057b70'),
        0x50a29:('call','0x10057b70'),
        0x57b7a:('mov','ebx, dword ptr [ecx + 8]'),
        0x57b87:('mov','eax, dword ptr [ebp + 0xc]'),
        0x57c43:('fmul','dword ptr [ebx + 0x28]'),
        0x57c4c:('fmul','dword ptr [ebx + 0x20]'),
        0x57c57:('fmul','dword ptr [ebx + 0x24]'),
        0x57cac:('fld','dword ptr [ebx + 0x10]'),
        0x57d40:('lea','edx, [ebx + 0x2c]'),
        0x57d73:('test','byte ptr [ebx + 0x4c], 1'),
        0x57d7f:('rep movsd','dword ptr es:[edi], dword ptr [esi]'),
        0x57f59:('call','dword ptr [eax + 0x1c]'),
        0x57f5c:('mov','ebx, dword ptr [ebx + 0xc]'),
        0x57f6d:('ret','8'),
    })
    child_math=list(md.disasm(engine.get_data(0x57b87,0x1fa),0x10057b87))
    require(not any(i.mnemonic.startswith(('call','j')) for i in child_math),
            'Child recomposition gained a callback or alternate formula')
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
