#!/usr/bin/env python3
"""Compile/inspect the shared x86 entry fixture; never run it or game code."""
import hashlib
import json
import re
import subprocess
from pathlib import Path

def require(ok, message):
    if not ok:
        raise ValueError(message)

ROOT = Path(__file__).resolve().parents[1]
obj = ROOT / "build-game/native-predicate-abi.o"
subprocess.run(["i686-w64-mingw32-g++", "-std=c++20", "-O2", "-Wall", "-Wextra", "-Werror",
                "-I" + str(ROOT / "src"), "-c", str(ROOT / "tests/native_predicate_abi.cpp"),
                "-o", str(obj)], check=True)
nm = subprocess.check_output(["i686-w64-mingw32-nm", str(obj)], text=True)
symbols = {row.split()[2]: int(row.split()[0], 16) for row in nm.splitlines()
           if len(row.split()) == 3 and re.fullmatch(r"[0-9a-fA-F]+", row.split()[0])}
text_addresses = [int(row.split()[0], 16) for row in nm.splitlines()
                  if re.search(r" [tT] ", row)]
report = {}
for name, slot, subject, kind in [("predicateEax", 0x1c, 4, 1),
                                 ("predicateEcx", 0x18, 4, 2),
                                 ("predicateOwner", 0x1c, 0, 3),
                                 ("predicateEdi", 0, 4, 4)]:
    start = symbols["_" + name]
    following = [value for value in text_addresses if value > start]
    command = ["i686-w64-mingw32-objdump", "-d", "-r", "--start-address=" + str(start)]
    if following:
        command.append("--stop-address=" + str(min(following)))
    assembly = subprocess.check_output(command + [str(obj)], text=True)
    instructions = []
    for row in assembly.splitlines():
        columns = row.split("\t")
        if len(columns) < 3 or not re.fullmatch(r"\s*[0-9a-f]+:", columns[0]):
            continue
        instruction = " ".join(columns[-1].split())
        if re.match(r"^[a-z]", instruction):
            instructions.append(instruction)
        if instruction.startswith("jmp "):
            break
    expected = ["pushf", "pusha", "cld", "mov %esp,%ebp", "and $0xfffffff0,%esp", "sub $0x200,%esp",
                "fxsave (%esp)", "fninit", "fldcw (%esp)",
                "sub $0x4,%esp", f"push $0x{kind:x}",
                f"push 0x{subject:x}(%ebp)",
                f"push 0x{slot:x}(%ebp)"]
    require(instructions[:13] == expected, (name, instructions[:13]))
    require(len(instructions) == 21, (name, instructions))
    require(re.fullmatch(r"call [0-9a-f]+ <_predicateAbiHelper>", instructions[13]), instructions[13])
    require(instructions[14:20] == ["add $0x10,%esp", f"mov %eax,0x{slot:x}(%ebp)",
                                   "fxrstor (%esp)", "mov %ebp,%esp", "popa", "popf"], "Required boundary check failed: instructions[14:20] == ['add $0x10,%esp', f'mov %eax,0x{slot:x}(%ebp)', 'fxrstor (%esp)', 'mov %ebp,%esp', 'popa', 'popf']")
    cell = symbols["_" + name + "_original"]
    require(instructions[20] == f"jmp *0x{cell:x}", instructions[20])
    require(len(re.findall(r"\bdir32\s+\.bss\b", assembly)) == 1, "Required boundary check failed: len(re.findall('\\\\bdir32\\\\s+\\\\.bss\\\\b', assembly)) == 1")
    # Independent stack arithmetic: native ESP is restored through saved EBP,
    # not by guessing the number of bytes discarded during alignment.
    for initial in (0, 4, 8, 12):
        saved_frame = 0x1000 + initial - 4 - 32
        fx_base = (saved_frame & ~15) - 512
        call_stack = fx_base - 4 - 12
        require(call_stack % 16 == 0 and call_stack + 16 == fx_base, 'Required boundary check failed: call_stack % 16 == 0 and call_stack + 16 == fx_base')
        require(saved_frame + 32 + 4 == 0x1000 + initial, 'Required boundary check failed: saved_frame + 32 + 4 == 4096 + initial')
    report[name] = {"selected_saved_register_offset": slot,
                    "subject_saved_register_offset": subject,
                    "general_registers_and_flags_restored": True,
                    "direction_flag_cleared_for_helper_and_native_flags_restored": True,
                    "fx_state_bytes": 512, "fx_and_helper_call_alignment": 16,
                    "helper_has_empty_x87_stack_with_native_control_word": True,
                    "original_esp_restored_for_all_native_stack_alignments": True,
                    "indirect_jump_has_trampoline_cell_relocation": True}
print(json.dumps({"runtime_executed": False, "fixture_linked_into_mod": False,
                  "native_hooks_installed": False,
                  "fixture_sha256": hashlib.sha256(obj.read_bytes()).hexdigest(),
                  "shared_entry_source_sha256": hashlib.sha256(
                      (ROOT / "src/game/x86_predicate_entry.hpp").read_bytes()).hexdigest(),
                  "fixture_source_sha256": hashlib.sha256(
                      (ROOT / "tests/native_predicate_abi.cpp").read_bytes()).hexdigest(),
                  "verifier_source_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
                  "method": "compiled COFF defined-symbol instructions and independent stack arithmetic",
                  "entries": report}, indent=2))
