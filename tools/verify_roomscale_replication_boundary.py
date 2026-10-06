#!/usr/bin/env python3
"""Verify the pinned native update/transport distinction without executing it.

This is evidence for the unfinished roomscale settlement design, not a network
implementation or a guarantee that an actor update and mod ACK are atomic.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import capstone
import pefile

PIN='da6efc9f72637eb3b6f48eadca2107be89b09c00618b6e72d5d3632938a7d851'


def require(condition,message):
    if not condition:
        raise ValueError(message)


def verify(game):
    raw=(game/'Bin/Engine.dll').read_bytes()
    require(hashlib.sha256(raw).hexdigest()==PIN,'Engine fingerprint mismatch')
    pe=pefile.PE(data=raw)
    base=pe.OPTIONAL_HEADER.ImageBase
    require(base==0x10000000 and pe.FILE_HEADER.Machine==0x14c,'Native ABI changed')
    exports={x.name:x.address for x in pe.DIRECTORY_ENTRY_EXPORT.symbols if x.name and not x.forwarder}
    for name,rva in {
        b'?IsReliable@CNMUpdateEntity@SeriousEngine@@UAEHXZ':0x111f80,
        b'?IsReliable@CNMReliableRPC@SeriousEngine@@UAEHXZ':0x1110d0,
        b'?GetLastUpdateSequence@CEntity@SeriousEngine@@UAEJXZ':0x58f60,
        b'?SetLastUpdateSequence@CEntity@SeriousEngine@@UAEXJ@Z':0x124840,
        b'?UpdateEntity@CClientInterface@SeriousEngine@@MAEXPAVCNMUpdateEntity@2@@Z':0xf2fb0,
    }.items():
        require(exports.get(name)==rva,'Native export changed: '+name.decode())
    require(pe.get_data(0x111f80,3)==bytes.fromhex('33c0c3'),'Actor updates must retain false reliability result')
    require(pe.get_data(0x1110d0,6)==bytes.fromhex('b801000000c3'),'Reliable RPC result changed')
    require(struct.unpack('<I',pe.get_data(0x214ef4+0x18,4))[0]==base+0x111f80,
            'Actor update reliability slot changed')
    md=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
    expected={
        0x58f60:('mov','eax, dword ptr [ecx + 0x18]'),
        0x1020c8:('mov','eax, dword ptr [edi + 0x10]'),
        0x1020cc:('cmp','eax, ecx'),
        0x1020ce:('jne','0x101020d9'),
        0x1020d2:('call','0x100ed070'),
        0x1020f4:('call','0x100ed000'),
        0xed01a:('call','dword ptr [edx + 0x18]'),
        0xed01f:('je','0x100ed02f'),
        0xed08a:('call','dword ptr [edx + 0x18]'),
        0xed08f:('jne','0x100ed09f'),
        0xf32b3:('call','dword ptr [edx + 0x98]'),
        0xf32bf:('mov','ecx, dword ptr [edi + 8]'),
        0xf32ca:('call','dword ptr [edx + 0x8c]'),
        0x124846:('mov','dword ptr [ecx + 0x18], eax'),
    }
    for address,value in expected.items():
        instruction=next(md.disasm(pe.get_data(address,15),base+address))
        require((instruction.mnemonic,instruction.op_str)==value,'Native boundary changed: '+hex(address))
    return {'engine_sha256':PIN,'runtime_executed':False,
            'native_actor_update_reliable':False,'mod_rpc_reliable':True,
            'receive_can_select_unreliable_without_next_reliable':True,
            'roomscale_settlement_implemented':False}


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game',type=Path,required=True)
    print(json.dumps(verify(parser.parse_args().game),indent=2))
