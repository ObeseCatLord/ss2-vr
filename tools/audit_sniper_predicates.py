#!/usr/bin/env python3
"""Verify candidate native zoom detour boundaries without executing game code."""
import argparse
import hashlib
import json
from pathlib import Path

import capstone
import pefile

ROOT = Path(__file__).resolve().parents[1]
EXPECTED = "5628b4ed30a966f10c8e8ea46bf0789257a35ea80127bacacebe28b1ce5303df"
# These are candidates, not installed hooks. Function ends exclude neighbouring
# methods/padding. The post-base seam is included because held-state precedes
# the interpolation predicate and can be changed by native callbacks.
SITES = [
    ("activate", 0x171D00, "eax, eax", 0x171CF0, 0x171DE2),
    ("deactivate_owner", 0x171E05, "eax, eax", 0x171DF0, 0x171F1E),
    ("deactivate_fov", 0x171E7B, "eax, eax", 0x171DF0, 0x171F1E),
    ("interpolate", 0x172854, "eax, eax", 0x172820, 0x172A62),
    ("sound_start", 0x17297F, "ecx, ecx", 0x172820, 0x172A62),
    ("sound_stop", 0x172A3C, "eax, eax", 0x172820, 0x172A62),
    ("before_view_toggle", 0x800E8, None, 0x7FFF0, 0x8010C),
    ("after_base_step", 0x172830, None, 0x172820, 0x172A62),
]


def audit(game):
    path = game / "Bin/Sam2Game.dll"
    digest = hashlib.sha256(path.read_bytes()).hexdigest()
    if digest != EXPECTED:
        raise ValueError("Unsupported native module fingerprint")
    pe = pefile.PE(str(path), max_symbol_exports=65536)
    decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    decoder.detail = True
    result = []
    for role, site, test, start, end in SITES:
        body = list(decoder.disasm(pe.get_data(start, end - start), start))
        if not body or body[-1].address + body[-1].size != end:
            raise ValueError("Incomplete bounded native function: " + role)
        by_address = {instruction.address: instruction for instruction in body}
        instruction = by_address.get(site)
        if instruction is None:
            raise ValueError("Candidate is not an instruction boundary: " + role)
        if test and (instruction.mnemonic != "test" or instruction.op_str != test):
            raise ValueError("Native predicate changed: " + role)
        if not test:
            expected = ("eax, dword ptr [edi + 0x558]" if role == "before_view_toggle"
                        else "eax, dword ptr [esi + 0xd4]")
            if instruction.mnemonic != "mov" or instruction.op_str != expected:
                raise ValueError("Native lifecycle seam changed: " + role)
        stolen = []
        cursor = site
        while cursor - site < 5:
            instruction = by_address[cursor]
            if instruction.mnemonic in ("ret", "retf"):
                raise ValueError("Native return inside detour window: " + role)
            stolen.append(instruction)
            cursor += instruction.size
        incoming = []
        stolen_branches = []
        for instruction in body:
            if not (instruction.group(capstone.CS_GRP_JUMP) or
                    instruction.group(capstone.CS_GRP_CALL)):
                continue
            if not instruction.operands or instruction.operands[0].type != capstone.x86.X86_OP_IMM:
                continue
            # Decoder addresses are RVAs, so relative targets are RVAs too.
            target = instruction.operands[0].imm
            if site < target < cursor:
                incoming.append(hex(instruction.address))
            if site <= instruction.address < cursor:
                stolen_branches.append({"from_rva": hex(instruction.address),
                                        "target_rva": hex(target)})
        if incoming:
            raise ValueError("Native branch enters the overwritten window: " + role)
        result.append({"role": role, "site_rva": hex(site),
                       "stolen_instruction_bytes": [x.size for x in stolen],
                       "stolen_bytes": cursor - site, "continuation_rva": hex(cursor),
                       "stolen_sha256": hashlib.sha256(pe.get_data(site, cursor-site)).hexdigest(),
                       "stolen_direct_branches": stolen_branches,
                       "in_function_branches_into_window": 0})
    return {"native_sha256": digest, "runtime_executed": False,
            "hooks_installed": False, "emitted_stub_abi_verified": False,
            "minhook_relocation_executed": False,
            "method": "bounded native instruction boundaries and direct branch targets only",
            "candidates": result}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--game", type=Path, default=ROOT.parent)
    print(json.dumps(audit(parser.parse_args().game.resolve()), indent=2))
