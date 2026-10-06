#!/usr/bin/env python3
"""Inspect the owned stock optic; never extract or redistribute its mesh.

This is a fingerprint-bound format audit, not a general CTSEMETA importer.
Run with the installed game's Patch_02_068.gro as the sole argument.
"""
import argparse
from collections import Counter
import hashlib
import json
import math
from pathlib import Path
import struct
import zipfile

ASSET = 'Content/SeriousSam2/Models/Weapons/Sniper/Sources/Meshes/Sniper.bmf'
SHA256 = '4eb829b83b7d15068ac07ea3f1a942cf49bcd9bce5141ebd89ab5e3d8c4179b9'
SKELETON = 'Content/SeriousSam2/Models/Weapons/Sniper/Sources/Sniper.skl'
SKELETON_SHA256 = 'f4474f087deab8bb04eb379446f4514ae2eaa5831898e7f56e67fde73b1d5061'


def pinned_names(data: bytes, offset: int) -> dict:
    if data[offset:offset+4] != b'IDNT':
        raise ValueError('Pinned name table framing changed')
    count = struct.unpack_from('<I', data, offset+4)[0]
    cursor = offset+8
    names = {}
    if count > 64:
        raise ValueError('Pinned name table count changed')
    for _ in range(count):
        ordinal, length = struct.unpack_from('<II', data, cursor)
        cursor += 8
        if ordinal in names or length > 256 or cursor+length > len(data):
            raise ValueError('Invalid pinned name table entry')
        names[ordinal] = data[cursor:cursor+length].decode('utf-8')
        cursor += length
    return names


def inspect_skin(data: bytes, skeleton_archive: Path | None) -> dict:
    # These are serialized local palette indices, never runtime bone IDs.
    # Full stock BMF fingerprint was checked before reaching this helper.
    if struct.unpack_from('<BBI', data, 0x1350) != (0x80, 0, 151520) or \
       struct.unpack_from('<BBI', data, 0x1356) != (0x80, 0, 155056):
        raise ValueError('Pinned Scope skin paths changed')
    weights = data[0x5fd2+151520:0x5fd2+151520+884*4]
    bone_indices = data[0x5fd2+155056:0x5fd2+155056+884*4]
    if weights != bytes((255, 0, 0, 0))*884 or bone_indices != bytes(884*4):
        raise ValueError('Scope is not uniformly weighted to its first local palette slot')
    if data[0x13fc:0x1400] != b'SSAR' or struct.unpack_from('<II', data, 0x1400) != (1, 4):
        raise ValueError('Pinned Scope weight-map table changed')
    name = pinned_names(data, 0x1c6).get(4)
    if name != 'Sniper':
        raise ValueError('Pinned Scope weight-map name changed')
    result = {'local_palette_slot': 0, 'normalized_weight': 1,
              'weights_sha256': hashlib.sha256(weights).hexdigest(),
              'local_indices_sha256': hashlib.sha256(bone_indices).hexdigest(),
              'weightmap_name': name, 'skeleton_verified': False}
    if skeleton_archive is not None:
        with zipfile.ZipFile(skeleton_archive) as source:
            skeleton = source.read(SKELETON)
        if len(skeleton) != 1700 or hashlib.sha256(skeleton).hexdigest() != SKELETON_SHA256:
            raise ValueError('Unsupported stock skeleton fingerprint')
        # One LOD and one root bone. The LOD STAR is not the bone SSAR.
        if skeleton[0x604:0x608] != b'STAR' or struct.unpack_from('<I', skeleton, 0x608)[0] != 1 or \
           skeleton[0x614:0x618] != b'SSAR' or struct.unpack_from('<III', skeleton, 0x618) != (1, 1, 0):
            raise ValueError('Pinned skeleton topology changed')
        names = pinned_names(skeleton, skeleton.index(b'IDNT'))
        identity = (1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0)
        if names.get(1) != name or names.get(0) != '' or \
           struct.unpack_from('<12f', skeleton, 0x664) != identity:
            raise ValueError('Scope skin/rest-bone correspondence changed')
        result.update({'skeleton_verified': True, 'skeleton_sha256': SKELETON_SHA256,
                       'skeleton_root': name, 'inverse_bind_identity': True})
    return result



def inspect_scope_material(data: bytes) -> dict:
    """Pin the Scope-specific graph, not just the BMF's shared shader name.

    This is serialized metadata evidence. It does not establish live overrides
    or permit pixel substitution; no GPU program is imported or compiled here.
    """
    i32 = lambda offset: struct.unpack_from('<i', data, offset)[0]
    u32 = lambda offset: struct.unpack_from('<I', data, offset)[0]
    f32 = lambda offset: struct.unpack_from('<f', data, offset)[0]
    if u32(0x1320) != 2 or i32(0x1414) != 5 or \
       data[0x119c:0x11a0] != b'EXOB' or u32(0x11ac) != 0x41a:
        raise ValueError('Pinned Scope preset/module-resource association changed')
    # Config count, shader, args, platform, attributes and preset flags.
    if data[0x39c8a:0x39c8e] != b'STAR' or \
       tuple(i32(offset) for offset in (0x39c8e,0x39c92,0x39c96,0x39c9a,0x39c9e,0x39ca2)) != \
       (1,7,8,3,-1,255):
        raise ValueError('Pinned Scope shader config changed')
    if data[0x39ce2:0x39ce6] != b'DCON' or i32(0x39ce6) != 0:
        raise ValueError('Pinned Scope has unexpected modifiers')
    if (i32(0x39cda),i32(0x39cde)) != (8,0x34) or \
       data[0xd8f:0xdb7] != b'DTTY'+struct.pack('<II',0x34,20)+b'CPixelightShaderArgs'+struct.pack('<II',10,5):
        raise ValueError('Pinned Scope argument object/type/version changed')
    scalar_ints = {
        'blend': (0x39cea,500), 'depth_compare': (0x39cee,42),
        'alpha_test': (0x39cf2,0), 'double_sided': (0x39cfa,0),
        'full_bright': (0x39cfe,0), 'constant_color': (0x39d02,0xff5e5e5e),
        'diffuse_texture_object': (0x39d0a,10), 'diffuse_uv_id': (0x39d0e,3),
        'specular_color': (0x39d26,0xff4c4b55), 'normal_texture_object': (0x39d2e,11),
        'normal_uv_id': (0x39d32,3), 'height_texture_object': (0x39d46,0xffffffff),
    }
    scalar_floats = {
        'alpha_threshold': (0x39cf6,.5), 'bump_strength': (0x39d06,1.),
        'diffuse_stretch_u': (0x39d12,1.), 'diffuse_stretch_v': (0x39d16,1.),
        'diffuse_offset_u': (0x39d1a,0.), 'diffuse_offset_v': (0x39d1e,0.),
        'diffuse_rotation': (0x39d22,0.), 'specular_power': (0x39d2a,90.),
        'normal_stretch_u': (0x39d36,1.), 'normal_stretch_v': (0x39d3a,1.),
        'normal_offset_u': (0x39d3e,0.), 'normal_offset_v': (0x39d42,0.),
        'height_bias': (0x39d4a,1.),
    }
    if any(u32(offset) != value for offset,value in scalar_ints.values()) or \
       any(f32(offset) != value for offset,value in scalar_floats.values()):
        raise ValueError('Pinned Scope shader arguments changed')
    return {'scope_preset_object':5,'shader_object':7,'args_object':8,
            'native_shader_module':'Shaders','module_resource_id':'0x41a',
            'native_shader':'Poly Bump','serialized_argument_type':'CPixelightShaderArgs',
            'serialized_argument_version':10,'config_count':1,'modifier_count':0,
            'arguments':{**{k:u32(o) for k,(o,_) in scalar_ints.items()},
                         **{k:f32(o) for k,(o,_) in scalar_floats.items()}},
            'live_material_verified':False,'color_substitution_approved':False}


def inspect(archive: Path, skeleton_archive: Path | None = None) -> dict:
    with zipfile.ZipFile(archive) as source:
        info = source.getinfo(ASSET)
        if info.file_size != 237010:
            raise ValueError('Unsupported mesh length')
        data = source.read(info)
    digest = hashlib.sha256(data).hexdigest()
    if digest != SHA256:
        raise ValueError('Unsupported stock mesh fingerprint; offsets cannot be reused')
    # In this exact mesh: Scope surface IDNT=2, 928 triangles, 884 vertices;
    # its indices/positions start at byte zero of their respective buffers.
    # STAR stores byte counts for these serialized BYTE arrays.
    for offset, length in ((0x1548, 19038), (0x5fca, 212128)):
        if data[offset:offset + 4] != b'STAR' or struct.unpack_from('<I', data, offset + 4)[0] != length:
            raise ValueError('Buffer framing disagrees with the pinned mesh')
    vertices = [struct.unpack_from('<3f', data, 0x5fd2 + i * 12) for i in range(884)]
    indices = struct.unpack_from('<2784H', data, 0x1550)
    if not all(math.isfinite(c) for point in vertices for c in point) or max(indices) >= len(vertices):
        raise ValueError('Invalid position or index')
    # Round-trip the parsed slices without writing any game-owned bytes.
    if b''.join(struct.pack('<3f', *v) for v in vertices) != data[0x5fd2:0x5fd2 + 884 * 12]:
        raise ValueError('Position round-trip failed')
    if struct.pack('<2784H', *indices) != data[0x1550:0x1550 + 2784 * 2]:
        raise ValueError('Index round-trip failed')
    # Rear-facing closed cap: identify its plane, then prove disk topology.
    # Plane selection is evidence for this asset only, not a runtime heuristic.
    cap_ordinals = [i // 3 for i in range(0, len(indices), 3)
                    if all(abs(vertices[indices[i + k]][2] - 0.0486) < 1e-5 for k in range(3))]
    if cap_ordinals != list(range(901, 923)):
        raise ValueError('Pinned rear cap triangle subrange changed')
    triangles = [indices[i*3:i*3 + 3] for i in cap_ordinals]
    used = {i for triangle in triangles for i in triangle}
    edges = Counter(tuple(sorted((t[k], t[(k + 1) % 3]))) for t in triangles for k in range(3))
    boundary = [edge for edge, count in edges.items() if count == 1]
    degrees = Counter(i for edge in boundary for i in edge)
    if len(triangles) != 22 or len(used) != 24 or len(boundary) != 24 or set(edges.values()) != {1, 2}:
        raise ValueError('Rear cap is not the expected triangulated disk')
    if set(degrees.values()) != {2} or set(degrees) != used or len(used) - len(edges) + len(triangles) != 1:
        raise ValueError('Rear cap boundary/Euler invariant failed')
    directed = Counter((t[k], t[(k + 1) % 3]) for t in triangles for k in range(3))
    for (a, b), count in edges.items():
        if count == 2 and (directed[(a, b)] != 1 or directed[(b, a)] != 1):
            raise ValueError('Rear cap has inconsistent interior edge winding')
    neighbors = {i: set() for i in used}
    for a, b in boundary:
        neighbors[a].add(b)
        neighbors[b].add(a)
    visited, pending = set(), [min(used)]
    while pending:
        node = pending.pop()
        if node not in visited:
            visited.add(node)
            pending.extend(neighbors[node] - visited)
    if visited != used:
        raise ValueError('Rear cap has more than one boundary loop')
    center = [sum(vertices[i][a] for i in used) / len(used) for a in range(3)]
    radii = [math.hypot(vertices[i][0] - center[0], vertices[i][1] - center[1]) for i in used]
    radius = sum(radii) / len(radii)
    signed_areas = [((vertices[b][0] - vertices[a][0]) * (vertices[c][1] - vertices[a][1]) -
                    (vertices[b][1] - vertices[a][1]) * (vertices[c][0] - vertices[a][0])) / 2
                   for a, b, c in triangles]
    if not all(a > 1e-12 for a in signed_areas):
        raise ValueError('Rear cap lost its audited outward +Z winding')
    area = sum(abs(a) for a in signed_areas)
    coverage = area / (math.pi * radius * radius)
    if max(radii) - min(radii) > 2e-6 or not 0.985 < coverage < 0.992:
        raise ValueError('Rear cap circularity/coverage failed')
    # Surface UV count/name table and the first packed UV descriptor. The
    # earlier offset90912 is FLOAT4 tangent/sign, never texture coordinates.
    if struct.unpack_from('<I',data,0x1330)[0] != 1 or \
       struct.unpack_from('<I',data,0x135c)[0] != 3 or \
       struct.unpack_from('<BBI',data,0x139c) != (0x84,0,181824):
        raise ValueError('Pinned Scope UV0 descriptor/name changed')
    uv_bytes=data[0x5fd2+181824:0x5fd2+181824+884*8]
    uv_digest=hashlib.sha256(uv_bytes).hexdigest()
    if uv_digest != '199e21cfca88d708d605db3ee54f35bba76191bf698c725baf4a39034480b2fd':
        raise ValueError('Pinned Scope UV0 bytes changed')
    uv={i:struct.unpack_from('<2f',uv_bytes,i*8) for i in used}
    if not all(math.isfinite(v) for pair in uv.values() for v in pair):
        raise ValueError('Nonfinite Scope cap UV')
    uv_center=[sum(uv[i][a] for i in used)/len(used) for a in range(2)]
    slopes=[]
    for axis in range(2):
        variance=sum((uv[i][axis]-uv_center[axis])**2 for i in used)
        if variance<1e-8:
            raise ValueError('Degenerate Scope cap UV axis')
        slope=sum((vertices[i][axis]-center[axis])*(uv[i][axis]-uv_center[axis])
                  for i in used)/variance
        error=max(abs(vertices[i][axis]-center[axis]-slope*(uv[i][axis]-uv_center[axis]))
                  for i in used)
        if error>1e-6 or abs(slope)<1e-5:
            raise ValueError('Scope cap UV-to-plane map changed')
        slopes.append(slope)
    return {'asset': ASSET, 'sha256': digest, 'scope_vertices': len(vertices),
            'scope_triangles': len(indices) // 3, 'rear_cap_triangles': len(triangles),
            'rear_cap_first_triangle': 901, 'rear_cap_end_triangle_exclusive': 923,
            'positions_sha256': hashlib.sha256(data[0x5fd2:0x5fd2 + 884*12]).hexdigest(),
            'indices_sha256': hashlib.sha256(data[0x1550:0x1550 + 2784*2]).hexdigest(),
            'rear_cap_boundary_vertices': len(boundary), 'rear_cap_center_model': center,
            'rear_cap_radius_model': radius, 'rear_cap_disk_coverage': coverage,
            'rear_cap_outward_normal_model':[0,0,1],
            'scope_uv':{'format':0x84,'buffer':0,'offset':181824,'stride':8,
                        'name_id':3,'sha256':uv_digest,'cap_planar_invertible':True,
                        'cap_uv_center':uv_center,'cap_uv_to_xy_scale':slopes,
                        'live_uv_verified':False},
            'slice_roundtrip_verified': True, 'scope_skin': inspect_skin(data, skeleton_archive),
            'scope_material': inspect_scope_material(data),
            'runtime_verified': False,
            'unknowns': ['material/UV lens meaning', 'animated model-to-world placement',
                         'native zoom input and multiplayer dispatch']}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('archive', type=Path)
    parser.add_argument('--skeleton-archive', type=Path, help='Owned All_PC_01.gro for pinned root-bone proof')
    args = parser.parse_args()
    print(json.dumps(inspect(args.archive, args.skeleton_archive), indent=2))
