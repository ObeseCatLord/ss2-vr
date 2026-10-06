#!/usr/bin/env python3
"""Verify the embedded mod-owned shader corresponds to its checked build output."""
import hashlib
import json
from pathlib import Path
import re
import struct
ROOT=Path(__file__).resolve().parents[1]
header=(ROOT/'src/game/shaders/scope_image_opaque.hpp').read_text()
source=(ROOT/'src/game/shaders/scope_image.hlsl').read_bytes()
source_hash=hashlib.sha256(source).hexdigest()
if re.findall(r'^// HLSL SHA256: ([0-9a-f]{64})$',header,re.M)!=[source_hash]:
    raise ValueError('Stale source shader stamp')
words=[int(x,16) for x in re.findall(r'0x([0-9a-f]{8})u',header)]
if not words or words[0]!=0xffff0201 or words[-1]!=0xffff:
    raise ValueError('Incorrect embedded shader framing')
binary=struct.pack('<'+'I'*len(words),*words)
binary_hash=hashlib.sha256(binary).hexdigest()
if re.findall(r'^// Bytecode SHA256: ([0-9a-f]{64})$',header,re.M)!=[binary_hash]:
    raise ValueError('Incorrect bytecode hash')
# Optional reproduction after running the supported Linux compiler tool.
compiled=ROOT/'build-scope-shader/scope_image_opaque.ps2a.bin'
reproduced=compiled.exists()
if reproduced and compiled.read_bytes()!=binary:
    raise ValueError('Embedded image differs from reproduced bytecode')
print(json.dumps({'hlsl_sha256':source_hash,'bytecode_sha256':binary_hash,
                  'bytes':len(binary),'matches_local_compiler_output':reproduced,
                  'runtime_executed':False},indent=2))
