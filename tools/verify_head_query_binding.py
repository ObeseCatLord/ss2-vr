#!/usr/bin/env python3
"""Pin the read-only head filter and native parent-ignore semantics."""
import argparse
import hashlib
import json
from pathlib import Path
import capstone
import pefile

PINS={'Sam2Game.dll':'5628b4ed30a966f10c8e8ea46bf0789257a35ea80127bacacebe28b1ce5303df',
      'Engine.dll':'da6efc9f72637eb3b6f48eadca2107be89b09c00618b6e72d5d3632938a7d851'}
def require(ok,message):
    if not ok:raise ValueError(message)
def verify(game):
    images={}
    for name,pin in PINS.items():
        raw=(game/'Bin'/name).read_bytes();require(hashlib.sha256(raw).hexdigest()==pin,'Unknown '+name)
        images[name]=pefile.PE(data=raw,max_symbol_exports=100000)
    sam=images['Sam2Game.dll'];engine=images['Engine.dll']
    exports={s.name.decode():s.address for s in sam.DIRECTORY_ENTRY_EXPORT.symbols if s.name and not s.forwarder}
    require(exports.get('?GetMechanism@CPuppetEntity@SeriousEngine@@UAEPAVCMechanism@2@XZ')==0x83b70,
            'Player mechanism getter changed')
    require(exports.get('?GetCollisionCategory@CPlayerPuppetEntity@SeriousEngine@@UAE?AVIDENT@2@XZ')==0xf6930,
            'Player category getter changed')
    imports={i.address-sam.OPTIONAL_HEADER.ImageBase:i.name.decode()
             for d in sam.DIRECTORY_ENTRY_IMPORT for i in d.imports if i.name}
    require(imports.get(0x294030)=='?strConvertStringToID@SeriousEngine@@YA?AVIDENT@1@PBD@Z',
            'Player category conversion changed')
    require(sam.get_string_at_rva(0x2984d4)==b'player','Player collision identifier changed')
    native={s.name.decode():s.address for s in engine.DIRECTORY_ENTRY_EXPORT.symbols if s.name and not s.forwarder}
    require(native.get('?IsIgnored@CMechanism@SeriousEngine@@QAEHPAV12@@Z')==0x131f00,
            'Native mechanism-ignore method changed')
    md=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
    expected={
        'Sam2Game.dll':{
            0x83b70:('mov','eax, dword ptr [ecx + 0x114]'),
            0x83b77:('call','dword ptr [0x102941d0]'),
            0xf6937:('push','0x102984d4'),
            0xf693d:('call','dword ptr [0x10294030]'),
            0xf694b:('mov','dword ptr [eax], ecx'),
            0xf6950:('ret','4'),
            0x8effd:('mov','esi, dword ptr [ebx + 0x47c]'),
            0x8f02f:('cmp','dword ptr [edi], 0'),
            0x8f032:('jne','0x1008f03e'),
            0x8f034:('mov','esi, 0x10404e08'),
        },
        'Engine.dll':{
            0x28bc6:('mov','dword ptr [0x102d96f0], eax'),
            0x28bd6:('mov','dword ptr [0x102d96f4], eax'),
            0x2f902:('cmp','eax, dword ptr [esi + 0x48]'),
            0x2f905:('je','0x1002f9e7'),
            0x2f919:('mov','eax, dword ptr [esi + 0x70]'),
            0x2f922:('cmp','edx, eax'),
            0x2f924:('je','0x1002f9e7'),
            0x2f92d:('call','0x10131f00'),
            0x131f09:('mov','eax, dword ptr [edi + 0x34]'),
            0x131f1d:('cmp','eax, esi'),
            0x131f1f:('je','0x10131f65'),
            0x131f21:('mov','ecx, dword ptr [esi + 0x34]'),
            0x131f2e:('cmp','eax, edi'),
            0x131f30:('je','0x10131f65'),
            0x131f67:('mov','eax, 1'),
        },
    }
    for name,instructions in expected.items():
        pe=images[name]
        for rva,wanted in instructions.items():
            ins=next(md.disasm(pe.get_data(rva,15),pe.OPTIONAL_HEADER.ImageBase+rva))
            require((ins.mnemonic,ins.op_str)==wanted,'Native head boundary changed: '+name+' '+hex(rva))
    return {'fingerprints':PINS,'head_category':'native player',
            'avatar_and_mechanism_filters_preserved':True,'native_parent_ignore_preserved':True,
            'null_view_resource_is_not_world_anchor':True,'runtime_executed':False}
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--game',type=Path,required=True)
    print(json.dumps(verify(p.parse_args().game),indent=2))
