#!/usr/bin/env python3
"""Inspect linked x86 primary entries/wrapper returns; execute no Windows code."""
import hashlib
import json
from pathlib import Path
import re
import subprocess

import capstone
import pefile

def require(ok, message):
    if not ok:
        raise ValueError(message)

ROOT = Path(__file__).resolve().parents[1]
ENTRIES = [("primaryDownPredicate", 0, 0), ("primaryPressPredicate", 24, 1),
           ("primaryReleasePredicate", 24, 2), ("primaryHistoryPredicate", 24, 3),
           ("primaryHeldPredicate", 28, 4)]
CALLBACKS = {"operatorFiring": 0, "fireButtonPressed": 4,
             "weaponFiringPressed": 4, "primaryPressed": 4, "primaryReleased": 4,
             "sawCopied": 4, "sawAssigned": 4, "sawPutDown": 4,
             "sawDeleted": 0, "sawWeaponPutDown": 0}


def inspect(path):
    native = pefile.PE(str(path), max_symbol_exports=65536)
    require(native.FILE_HEADER.Machine == 0x14c, 'Required boundary check failed: native.FILE_HEADER.Machine == 332')
    base = native.OPTIONAL_HEADER.ImageBase
    rows = subprocess.check_output(["i686-w64-mingw32-nm", str(path)], text=True).splitlines()
    symbols = {r.split()[2]: int(r.split()[0], 16) for r in rows
               if len(r.split()) == 3 and re.fullmatch("[0-9a-fA-F]+", r.split()[0])}
    text_addresses = sorted({int(r.split()[0], 16) for r in rows if re.search(" [tT] ", r)})
    decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    relocations = {entry.rva for block in native.DIRECTORY_ENTRY_BASERELOC
                   for entry in block.entries if entry.type == 3}
    entries = []
    for name, slot, kind in ENTRIES:
        address = symbols["_" + name]
        instructions = []
        for instruction in decoder.disasm(native.get_data(address - base, 128), address):
            instructions.append(instruction)
            if instruction.mnemonic == "jmp":
                break
        operand = "[ebp]" if slot == 0 else f"[ebp + 0x{slot:x}]"
        expected = [("pushfd", ""), ("pushal", ""), ("cld", ""), ("mov", "ebp, esp"),
                    ("and", "esp, 0xfffffff0"), ("sub", "esp, 0x200"), ("fxsave", "[esp]"),
                    ("fninit", ""), ("fldcw", "word ptr [esp]"), ("sub", "esp, 4"),
                    ("push", str(kind)), ("push", "dword ptr [ebp + 4]"),
                    ("push", "dword ptr " + operand),
                    ("call", f'0x{symbols["_ss2vrPrimaryPredicate"]:x}'),
                    ("add", "esp, 0x10"), ("mov", "dword ptr " + operand + ", eax"),
                    ("fxrstor", "[esp]"), ("mov", "esp, ebp"), ("popal", ""), ("popfd", ""),
                    ("jmp", f'dword ptr [0x{symbols["_" + name + "_original"]:x}]')]
        require([(i.mnemonic, i.op_str) for i in instructions] == expected, name + " entry differs")
        require(instructions[-1].address - base + 2 in relocations, name + " cell not relocated")
        entries.append({"name": name, "result_saved_offset": slot, "subject_saved_offset": 4,
                        "kind": kind, "trampoline_cell_relocated": True})
    callbacks = []
    for name, pop in CALLBACKS.items():
        matches = [n for n in symbols if re.fullmatch(r"@_ZN5ss2vr4gameL\d+" + name + r"EPvS\d_.*@\d+", n)]
        require(len(matches) == 1, (name, "definition missing or ambiguous"))
        address = symbols[matches[0]]
        end = next(a for a in text_addresses if a > address)
        returns = [i for i in decoder.disasm(native.get_data(address - base, end - address), address)
                   if i.mnemonic == "ret"]
        require(returns and all(i.op_str == str(pop) if pop else not i.op_str for i in returns), name + " return differs")
        body = list(decoder.disasm(native.get_data(address - base, end - address), address))
        require(any(i.mnemonic == "call" and i.op_str == f'0x{symbols["_ss2vrNativeFinally"]:x}'
                   for i in body), name + " missing native-finally boundary")
        callbacks.append({"name": name, "return_stack_bytes": pop, "return_sites": len(returns)})
    helper = symbols["_ss2vrPrimaryPredicate"]
    end = next(a for a in text_addresses if a > helper)
    code = list(decoder.disasm(native.get_data(helper - base, end - helper), helper))
    require(not any(i.mnemonic == "call" for i in code), "Scalar primary helper contains a call")
    require(any(i.mnemonic == "ret" for i in code), "Scalar primary helper has no return")
    return {"sha256": hashlib.sha256(path.read_bytes()).hexdigest(),
            "entries": entries, "callbacks": callbacks, "scalar_helper_call_count": 0}


print(json.dumps({
    "runtime_executed": False, "windows_code_executed": False, "hooks_installed": False,
    "method": "actual linked PE instructions, exact helper/cell targets, ASLR relocations and callback returns",
    "limits": ["No native control flow or installed MinHook trampoline execution",
               "Receiver/argument provenance and lifecycle require independent source review",
               "No stock weapon or multiplayer equivalence claim from entry inspection"],
    "artifacts": {name: inspect(ROOT / "build-game" / name) for name in ("d3d9.dll", "SS2VRServer.dll")}
}, indent=2))
