#!/usr/bin/env python3
"""Install/remove only SS2VR-owned files, with collision and hash checks."""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import shutil
import sys
RECEIPT = Path('Bin/SS2VR/install-receipt.json')

def digest(path): return hashlib.sha256(path.read_bytes()).hexdigest()

def paths(manifest):
    entries = manifest['files']
    for relative, expected in entries.items():
        p = PurePosixPath(relative)
        if p.is_absolute() or '..' in p.parts or '\\' in relative or ':' in relative:
            raise ValueError('Unsafe package path')
        if relative not in ('Bin/d3d9.dll', 'Bin/SS2VRServer.dll', 'Content/SS2VR.mod') and p.parts[:2] != ('Bin', 'SS2VR'):
            raise ValueError('File outside mod ownership')
        if len(expected) != 64 or any(c not in '0123456789abcdef' for c in expected):
            raise ValueError('Invalid manifest hash')
    return entries

def destination(game, relative):
    target = game/relative
    # Resolve existing parents to prevent writes/deletions through a mod-folder symlink.
    if target.resolve().is_relative_to(game) and not target.is_symlink(): return target
    raise ValueError('Destination escapes installation or is a symlink')

def install(game, package, dry):
    manifest = json.loads((package/'manifest.json').read_text())
    entries = paths(manifest)
    for name, metadata in manifest['game_fingerprints'].items():
        if Path(name).name != name: raise ValueError('Unsafe fingerprint filename')
        if digest(game/'Bin'/name) != metadata['sha256']:
            raise ValueError('Game build does not match verified fingerprints: ' + name)
    if (game/'Bin/SS2VR').exists() or (game/'Bin/SS2VR').is_symlink():
        raise FileExistsError('SS2VR destination already exists; refusing overwrite')
    for relative, expected in entries.items():
        source = package/relative
        if not source.resolve().is_relative_to(package) or source.is_symlink() or digest(source) != expected:
            raise ValueError('Package content/hash mismatch: ' + relative)
        target = destination(game, relative)
        if target.exists(): raise FileExistsError('Refusing to overwrite: ' + relative)
    if dry:
        print('Preflight passed: matching game, verified package, no collisions. No files changed.'); return
    created = []
    receipt_created=False
    try:
        for relative in entries:
            target = destination(game, relative)
            target.parent.mkdir(parents=True, exist_ok=True)
            # Exclusive creation prevents a race from overwriting another mod.
            with target.open('xb') as output, (package/relative).open('rb') as source:
                created.append(target); shutil.copyfileobj(source, output)
        with destination(game, str(RECEIPT)).open('x') as output:
            receipt_created=True
            json.dump({'files':entries, 'version':manifest['version']}, output, indent=2)
    except Exception:
        if receipt_created: destination(game,str(RECEIPT)).unlink()
        # Roll back only newly-created payload files, never existing files.
        for target in created:
            if target.is_file() and not target.is_symlink(): target.unlink()
        raise
    print('Installed SS2VR development build. No game/runtime was launched.')

def uninstall(game, dry):
    receipt = destination(game, str(RECEIPT))
    manifest = json.loads(receipt.read_text())
    entries = paths(manifest)
    for relative, expected in entries.items():
        target = destination(game, relative)
        if not target.is_file() or digest(target) != expected:
            raise ValueError('Modified/missing mod file; refusing removal: ' + relative)
    if dry:
        print('Removal preflight passed. No files changed.'); return
    for relative in entries: destination(game, relative).unlink()
    receipt.unlink()
    # Empty mod-owned directories only; runtime logs or other additions are retained.
    folders = sorted({(game/r).parent for r in entries if r.startswith('Bin/SS2VR/')},
                     key=lambda p:len(p.parts), reverse=True)
    for folder in folders:
        try: folder.rmdir()
        except OSError: pass
    print('Removed unchanged SS2VR payload; other files retained.')

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('action', choices=['install','uninstall'])
    parser.add_argument('--game', type=Path, required=True)
    parser.add_argument('--package', type=Path, default=Path(__file__).resolve().parent)
    parser.add_argument('--dry-run', action='store_true')
    args = parser.parse_args()
    try:
        game=args.game.resolve()
        if args.action=='install': install(game,args.package.resolve(),args.dry_run)
        else: uninstall(game,args.dry_run)
    except (OSError,ValueError,KeyError) as error:
        print('Stopped:',error,file=sys.stderr);sys.exit(1)
