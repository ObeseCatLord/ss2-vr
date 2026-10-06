#!/usr/bin/env python3
"""Compile/disassemble only: six-slot cdecl/x87 boundary and production detour.

All objects are scratch files outside the repository. Reads only the owned
Core.dll export/ABI; never executes Windows code or writes native disassembly.
Requires GNU MinGW x86 tools and Python pefile. This is deliberately a pinned
compiler-shape check, not an emulator or proof of native TOI/SEH correctness.
"""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
import tempfile

import pefile

ROOT = Path(__file__).resolve().parents[1]
EXPORT = b"?mthIntersectThickRayTriangle@SeriousEngine@@YAMABVRay3f@1@ABVVector3f@1@111M@Z"
CORE_SHA256 = "7a1bd56b9bfa3edfbb23f4d3c96490e40a7c0b031b85e797e1af91b2ba3cf207"


def run(*args):
    return subprocess.check_output(args, text=True)


def instructions(assembly):
    result = []
    for line in assembly.splitlines():
        columns = line.split("\t")
        if len(columns) >= 3 and re.fullmatch(r"\s*[0-9a-f]+:", columns[0]):
            code = " ".join(columns[-1].split())
            if re.match(r"[a-z]", code):
                result.append(code)
    return result


def functions(assembly):
    result = {}
    parts = re.split(r"^[0-9a-f]+ <([^>]+)>:\n", assembly, flags=re.MULTILINE)
    for name, body in zip(parts[1::2], parts[2::2]):
        result[name] = body
    return result


def fixture_check(body, indirect):
    code = instructions(body)
    pointer_register = "edx" if indirect else "eax"
    first_argument = 12 if indirect else 8
    expected = ["push ebp", "mov ebp,esp", "sub esp,0x1c"]
    if indirect:
        expected.append("mov eax,DWORD PTR [ebp+0x8]")
    expected += [f"fld DWORD PTR [ebp+0x{first_argument+20:x}]",
                 "fstp DWORD PTR [esp+0x14]"]
    for index in range(4, -1, -1):
        destination = "[esp]" if index == 0 else f"[esp+0x{index*4:x}]"
        expected += [f"mov {pointer_register},DWORD PTR [ebp+0x{first_argument+index*4:x}]",
                     f"mov DWORD PTR {destination},{pointer_register}"]
    assert code[:len(expected)] == expected, code
    call = code[len(expected)]
    if indirect:
        assert call == "call eax", call
    else:
        assert re.fullmatch(r"call [0-9a-f]+ <_roomscaleAbiEntry\+0x[0-9a-f]+>", call), call
        assert "DISP32\t_ss2vrRoomscaleTriangleQuery" in body
    assert code[len(expected)+1:] == ["fstp DWORD PTR [ebp-0x4]",
                                     "fld DWORD PTR [ebp-0x4]", "leave", "ret"], code
    # The caller owns its 28-byte local frame (24 outgoing args + result).
    # There is no ret 24/callee cleanup. Input pointers retain native order.


def native_export_check(game):
    core = game / "Bin/Core.dll"
    raw = core.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == CORE_SHA256, "Core fingerprint differs"
    pe = pefile.PE(data=raw)
    assert pe.FILE_HEADER.Machine == 0x14C and pe.OPTIONAL_HEADER.ImageBase == 0x10000000
    matches = [s for s in pe.DIRECTORY_ENTRY_EXPORT.symbols if s.name == EXPORT]
    assert len(matches) == 1 and matches[0].address == 0x1D3C0 and not matches[0].forwarder
    # Exact pinned ABI evidence only; no native implementation is copied out.
    assert pe.get_data(0x1D3C0, 6) == bytes.fromhex("55 8b ec 83 ec 60")
    for offset, expected in {
        0x1D3C6: "8b 4d 18",  # fifth arg: normal reference
        0x1D3CA: "8b 75 08",  # first arg: ray reference
        0x1D3FA: "d9 45 1c",  # sixth arg: float radius, x87 load
        0x1D400: "8b 5d 10",  # third arg: B reference
        0x1D404: "8b 7d 0c",  # second arg: A reference
        0x1D66B: "d9 45 18 5f 5b 5e 8b e5 5d c3",  # ST0 return; plain ret
    }.items():
        expected_bytes = bytes.fromhex(expected)
        assert pe.get_data(offset, len(expected_bytes)) == expected_bytes, hex(offset)
    assert pe.get_data(0x81974, 4) == bytes.fromhex("e6 b1 61 7f")
    pe.close()


def verify(scratch, game):
    native_export_check(game)
    objects = {}
    for name, source, optimization in (
        ("fixture", "tests/native_roomscale_query_abi.cpp", "-O0"),
        ("production", "src/game/roomscale_query.cpp", "-O2"),
    ):
        target = scratch / (name + ".o")
        subprocess.run(["i686-w64-mingw32-g++", "-std=c++20", optimization,
                        "-fno-omit-frame-pointer", "-Wall", "-Wextra", "-Werror",
                        "-I" + str(ROOT / "src"), "-c", str(ROOT / source), "-o", str(target)], check=True)
        assert target.read_bytes()[:2] == b"\x4c\x01", "Expected i386 COFF"
        objects[name] = target
    fixture = functions(run("i686-w64-mingw32-objdump", "-dr", "-Mintel", str(objects["fixture"])))
    fixture_check(fixture["_roomscaleAbiCall"], True)
    fixture_check(fixture["_roomscaleAbiEntry"], False)

    assembly = run("i686-w64-mingw32-objdump", "-dr", "-Mintel", "-j", ".text", str(objects["production"]))
    bodies = functions(assembly)
    entry = bodies["_ss2vrRoomscaleTriangleQuery"]
    code = instructions(entry)
    assert "and esp,0xfffffff0" in code  # native caller can supply 4-byte alignment
    assert "__tls_index" in entry and "secrel32\t.tls$" in entry
    assert "dir32\t.bss" in entry and code.count("call esi") == 2  # original trampoline
    assert "roomscale8classify" in entry
    assert code.count("fchs") == 3  # negated native normal
    assert code.count("ret") == 2 and not any(c.startswith("ret ") for c in code)
    # Both native-call sites receive six slots. The outside-scope path returns
    # ST0 untouched; the scoped path examines ST0 for invalid native output.
    calls = [i for i, value in enumerate(code) if value == "call esi"]
    for i in calls:
        window = code[i-12:i]
        assert "fstp DWORD PTR [esp+0x14]" in window
        for slot in (0, 4, 8, 12, 16):
            operand = "[esp]" if slot == 0 else f"[esp+0x{slot:x}]"
            assert any(c.startswith(f"mov DWORD PTR {operand},") for c in window), window
    assert code[calls[0]+1] == "fld st(0)"
    assert code[calls[1]+1:calls[1]+6] == ["lea esp,[ebp-0x8]", "pop ebx", "pop esi", "pop ebp", "ret"]
    # The reversed branch gives original B to native C's slot, then loads
    # original C and joins the common store to native B's slot.
    reverse = code[calls[1]+6:]
    swapped = ["mov eax,DWORD PTR [ebp+0x10]", "mov DWORD PTR [esp+0xc],eax",
               "mov eax,DWORD PTR [ebp+0x14]"]
    index = reverse.index(swapped[0])
    assert reverse[index:index+3] == swapped
    assert reverse[index+3].startswith("jmp ")
    assert "lea eax,[esp+0x2c]" in reverse[:index]  # address of negated normal
    scope = next(body for name, body in bodies.items() if "27runRoomscaleModelQueryScope" in name and "withNativeFinally" not in name)
    assert "DISP32\t_ss2vrNativeFinally" in scope and "GetCurrentThreadId" in scope
    assert "fnstcw" in scope and "_fegetround" in scope
    finish = next(body for name, body in bodies.items() if "Context6finish" in name)
    assert "secrel32\t.tls$" in finish and "mov BYTE PTR [edx+0x10],0x1" in instructions(finish)
    queue = next(body for name, body in bodies.items() if "queueRoomscaleTriangleHook" in name)
    assert "RoomscaleTriangleExport" in queue and "dir32\t.bss" in queue
    assert EXPORT + b"\0" in objects["production"].read_bytes()
    nm = run("i686-w64-mingw32-nm", str(objects["production"]))
    assert re.search(r" T _ss2vrRoomscaleTriangleQuery$", nm, re.MULTILINE)
    assert "MH_EnableHook" not in nm and "MH_ApplyQueued" not in nm
    print(json.dumps({
        "scope_done": "compiled model triangle boundary only",
        "runtime_executed": False, "native_hooks_installed": False,
        "core_sha256": CORE_SHA256, "core_export_rva": "0x1D3C0",
        "export": EXPORT.decode(), "argument_bytes": 24, "return": "float in x87 ST0",
        "checks": ["exact owned Core export/fingerprint and pinned ABI instructions",
                   "fixture six slots in order, caller frame cleanup, x87 float spill/reload",
                   "production TLS gate, aligned detour, native trampoline calls and x87 return",
                   "production B/C swap, normal negation, original registrar export and finally relocations"],
        "object_sha256": {name: hashlib.sha256(path.read_bytes()).hexdigest() for name, path in objects.items()},
        "limits": ["compile-only; no Windows/native SEH execution or installation",
                   "no native TOI numerical accuracy/conservatism proof",
                   "no query/world lifetime, primitive contact, body placement or multiplayer proof"],
    }, indent=2))


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--game", type=Path, default=ROOT.parent,
                        help="owned game directory (defaults to the repository's parent)")
    parser.add_argument("--output-dir", type=Path, help="scratch directory outside the repository")
    args = parser.parse_args()
    game = args.game.resolve()
    if args.output_dir:
        directory = args.output_dir.resolve()
        if directory == ROOT or ROOT in directory.parents or directory == game or game in directory.parents:
            parser.error("objects must be outside the repository and game directory")
        directory.mkdir(parents=True, exist_ok=True)
        verify(directory, game)
    else:
        with tempfile.TemporaryDirectory(prefix="ss2vr-roomscale-abi-") as temporary:
            verify(Path(temporary), game)
