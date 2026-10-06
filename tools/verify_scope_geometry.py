#!/usr/bin/env python3
"""Check cap extraction with the user's pinned mesh; never execute Windows code.

The five private slices exist only in a temporary directory outside the repo.
No asset data, disassembly or absolute game path is emitted or retained.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile
import zipfile
from audit_sniper_geometry import ASSET, SHA256, inspect

ROOT = Path(__file__).resolve().parents[1]
EXPECTED = (
    ('positions', 0x5fd2, 884*12, 'a83d4d52827f2f95418f46c2ef8f8eca3ac594110ff35173bef4a526188a99c2'),
    ('indices', 0x1550, 928*3*2, '9e906c5c63a118742d857f0641ab39494ed1350374ea9e132fa5745d3637781b'),
    ('weights', 0x5fd2+151520, 884*4, '2852dade36b5c2b533147f17dc9018ae35eff637e68d7e85fee12c05271d30ad'),
    ('local_indices', 0x5fd2+155056, 884*4, '4bcd1d733a353b8de5512ea22b608ebc74c4a818fe00663ff32ea4f7a3dd808a'),
    ('uv', 0x5fd2+181824, 884*8, '199e21cfca88d708d605db3ee54f35bba76191bf698c725baf4a39034480b2fd'),
)


def verify(game: Path, checker: Path) -> dict:
    checker = checker.resolve(strict=True)
    if checker.read_bytes()[:4] != b'\x7fELF':
        raise ValueError('Only the portable Linux offline checker may execute')
    archive = game/'Patch_02_068.gro'
    geometry = inspect(archive, game/'All_PC_01.gro')
    with zipfile.ZipFile(archive) as owned:
        data = owned.read(ASSET)
    if hashlib.sha256(data).hexdigest() != SHA256:
        raise ValueError('Owned mesh changed during verification')
    slices = []
    for _, offset, length, digest in EXPECTED:
        channel = data[offset:offset+length]
        if len(channel) != length or hashlib.sha256(channel).hexdigest() != digest:
            raise ValueError('Pinned slice mismatch')
        slices.append(channel)
    with tempfile.TemporaryDirectory(prefix='ss2vr-scope-owned-') as private:
        sample = Path(private)/'slices.bin'
        sample.write_bytes(b''.join(slices))
        result = subprocess.run([str(checker),str(sample)],check=True,capture_output=True,text=True,timeout=10)
        checked = json.loads(result.stdout)
    if checked != {'owned_cap_vertices':24,'owned_cap_indices':66,'full_affine_reflection_preserved':True,
                   'owned_optical_frame':True,'owned_uv_pairs':True,'owned_uv_mapping':True}:
        raise ValueError('Unexpected owned-cap checker result')
    if any(type(checked[k]) is not int for k in ('owned_cap_vertices','owned_cap_indices')) or \
       any(type(checked[k]) is not bool for k in ('full_affine_reflection_preserved','owned_optical_frame','owned_uv_pairs','owned_uv_mapping')):
        raise ValueError('Owned-cap checker result types changed')
    return {'runtime_executed':False, 'offline_linux_checker_executed':True,
            'checker_sha256':hashlib.sha256(checker.read_bytes()).hexdigest(),
            'mesh_sha256':SHA256,'skeleton_sha256':geometry['scope_skin']['skeleton_sha256'],
            'copied_slice_bytes':sum(len(s) for s in slices),'slices_sha256':{s[0]:s[3] for s in EXPECTED},
            **checked,'live_loaded_content_verified':False}


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--game',type=Path,required=True)
    p.add_argument('--checker',type=Path,default=ROOT/'build-core/scope_geometry_checks')
    args = p.parse_args()
    print(json.dumps(verify(args.game,args.checker),indent=2))
