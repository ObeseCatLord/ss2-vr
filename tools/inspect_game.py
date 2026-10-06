#!/usr/bin/env python3
"""Read-only PE fingerprint/export survey. Never extracts proprietary code."""
import argparse
import hashlib
import json
from pathlib import Path
import pefile

FILES = ('Sam2.exe', 'Core.dll', 'Engine.dll', 'GfxD3D.dll', 'Sam2Game.dll')

def inspect_game(game: Path):
    result = {}
    for name in FILES:
        path = game / 'Bin' / name
        pe = pefile.PE(str(path), max_symbol_exports=65536)
        symbols = [e.name.decode('ascii') for e in getattr(pe, 'DIRECTORY_ENTRY_EXPORT', []).symbols if e.name] if hasattr(pe, 'DIRECTORY_ENTRY_EXPORT') else []
        result[name] = {
            'sha256': hashlib.sha256(path.read_bytes()).hexdigest(),
            'machine': hex(pe.FILE_HEADER.Machine),
            'timestamp': pe.FILE_HEADER.TimeDateStamp,
            'image_size': pe.OPTIONAL_HEADER.SizeOfImage,
            'export_count': len(symbols),
            'vr_boundary_exports': [s for s in symbols if any(t in s for t in ('Render3D@CPuppet', 'GetCameraPlacement@CPuppet', 'GetProjectionMatrix@CPuppet', 'GetShootingPlacement@', 'SetCurrentWeapon@CPlayer', 'GetCurrentWeaponIndex@CPlayer', 'ToggleDualWielding@CPlayer'))],
        }
    return result

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('game', type=Path)
    args = parser.parse_args()
    print(json.dumps(inspect_game(args.game), indent=2))
