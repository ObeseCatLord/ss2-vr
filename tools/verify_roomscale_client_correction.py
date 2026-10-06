#!/usr/bin/env python3
"""Pin local-client correction boundaries; no gameplay or Windows execution."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import capstone
import pefile

PIN='5628b4ed30a966f10c8e8ea46bf0789257a35ea80127bacacebe28b1ce5303df'
def require(ok,message):
    if not ok:raise ValueError(message)
def verify(game):
    raw=(game/'Bin/Sam2Game.dll').read_bytes()
    require(hashlib.sha256(raw).hexdigest()==PIN,'Unknown Sam2Game image')
    pe=pefile.PE(data=raw,max_symbol_exports=100000);base=pe.OPTIONAL_HEADER.ImageBase
    require(base==0x10000000 and pe.FILE_HEADER.Machine==0x14c,'Native ABI changed')
    exports={s.name.decode():s.address for s in pe.DIRECTORY_ENTRY_EXPORT.symbols if s.name and not s.forwarder}
    for name,rva in {
        '?PostReceiveUpdate@CPlayerPuppetEntity@SeriousEngine@@UAEXXZ':0x104260,
        '?PostReceiveUpdate@CPuppetEntity@SeriousEngine@@UAEXXZ':0xa46e0,
        '?PostReceiveUpdate@CPlayerBrainEntity@SeriousEngine@@UAEXXZ':0xede60,
        '?ApplyClientPositionCorrection@CPuppetEntity@SeriousEngine@@UAEXXZ':0x90490,
        '?GetAbsPlacement@CPuppetEntity@SeriousEngine@@UAE?AVQuatVect@2@XZ':0x8e8e0,
        '?IsLocal@CPuppetEntity@SeriousEngine@@QAEHXZ':0x83530,
    }.items():require(exports.get(name)==rva,'Export changed: '+name)
    for slot,rva in [(0x54,0x8e8e0),(0x98,0x104260),(0x5bc,0x90490)]:
        require(struct.unpack('<I',pe.get_data(0x29e878+slot,4))[0]==base+rva,'Player vtable changed')
    imports={i.address-base:i.name.decode() for d in pe.DIRECTORY_ENTRY_IMPORT for i in d.imports if i.name}
    require(imports.get(0x294d4c)=='?SetAbsPlacement@CMechanism@SeriousEngine@@QAEHABVQuatVect@2@K@Z',
            'Native correction setter import changed')
    require(imports.get(0x295090)=='?GetAbsPlacement@CMechanism@SeriousEngine@@QAE?AVQuatVect@2@XZ',
            'Native body getter import changed')
    md=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
    expected={
        0x1042f6:('call','0x100a46e0'),
        0xa4750:('call','0x10083530'),
        0xa4755:('test','eax, eax'),
        0xa4757:('jne','0x100a483c'),
        0xa47b2:('call','dword ptr [0x10295090]'),
        0xa47bb:('fsub','dword ptr [esi + 0x4f0]'),
        0xa47c1:('lea','eax, [esi + 0x3b0]'),
        0xede8d:('call','0x10083530'),
        0xede94:('je','0x100ee016'),
        0xedebf:('mov','al, byte ptr [esi + 0x1cc]'),
        0xedec5:('cmp','al, byte ptr [esi + 0x152]'),
        0xedecb:('je','0x100edfe4'),
        0xedf06:('lea','eax, [esi + 0x1d0]'),
        0xedf85:('call','dword ptr [0x10295090]'),
        0xedf91:('lea','ecx, [ebx + 0x3b0]'),
        0xedfc9:('mov','byte ptr [esi + 0x152], al'),
        0xa6e0d:('call','dword ptr [eax + 0x5bc]'),
        0x905a9:('call','dword ptr [0x10294e90]'),
        0x905d0:('mov','edx, dword ptr [edi + 0x114]'),
        0x905e8:('call','dword ptr [0x10295090]'),
        0x906af:('push','0'),
        0x90706:('call','dword ptr [0x10294d4c]'),
        0x9070f:('fadd','dword ptr [esi]'),
        0x90711:('fstp','dword ptr [esi]'),
        0x8e910:('call','dword ptr [0x10295090]'),
    }
    for address,value in expected.items():
        i=next(md.disasm(pe.get_data(address,15),base+address))
        require((i.mnemonic,i.op_str)==value,'Correction boundary changed: '+hex(address))
    return {'sam_sha256':PIN,'local_actor_postreceive_skips_remote_position_error':True,
            'local_brain_correction_sequence_separate':True,
            'native_correction_applied_later_in_puppet_step':True,
            'actor_update_marker_is_not_local_body_settlement':True,
            'runtime_executed':False}
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--game',type=Path,required=True)
    print(json.dumps(verify(p.parse_args().game),indent=2))
