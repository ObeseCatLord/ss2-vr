#!/usr/bin/env python3
"""Export private decoded idle mesh candidates; no game, GPU or alignment evidence.

This consumes the existing decoded CRenderMesh/GFXHANDLE metadata domain. It
does not decode BMF files or certify that supplied metadata came from an asset.
Channel offsets address decoded buffer bytes, never offsets in the asset file.
"""
import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import re
import stat
import struct

ROOT = Path(__file__).resolve().parents[1]
MAX_BYTES = 16 * 1024 * 1024
MAX_OBJECTS, MAX_ASSETS, MAX_SURFACES = 4096, 16, 512
MAX_VERTICES, MAX_TRIANGLES = 2904, 2245  # Generic offline storage envelope; native admission remains per ID.
CHANNELS = ('positions', 'indices', 'weights', 'local_indices', 'uv')
FORMATS = dict(zip(CHANNELS, (133, 135, 128, 128, 132)))
STRIDES = dict(zip(CHANNELS, (12, 6, 4, 4, 8)))


def integer(value, maximum, label, minimum=0):
    if type(value) is not int or not minimum <= value <= maximum:
        raise ValueError('Invalid ' + label)
    return value


def mapping(value, label):
    if not isinstance(value, dict):
        raise ValueError('Expected object: ' + label)
    return value


def sequence(value, maximum, label):
    if not isinstance(value, list) or len(value) > maximum:
        raise ValueError('Invalid list/budget: ' + label)
    return value


def no_duplicates(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError('Duplicate metadata key')
        result[key] = value
    return result


def reject_constant(value):
    raise ValueError('Nonfinite JSON number: ' + value)


def read_bounded(path):
    if not path.is_file():
        raise ValueError('Input must be a regular private file')
    with path.open('rb') as source:
        if not stat.S_ISREG(os.fstat(source.fileno()).st_mode):
            raise ValueError('Input must be a regular private file')
        data = source.read(MAX_BYTES + 1)
    if not data or len(data) > MAX_BYTES:
        raise ValueError('Private input byte budget exceeded or empty input')
    return data


def private_path(path):
    path = Path(path).resolve()
    if path.is_relative_to(ROOT) or any((p / '.git').is_file() or (p / '.git' / 'HEAD').is_file()
                                       for p in (path, *path.parents)):
        raise ValueError('Inputs and output must stay outside source/Git checkouts')
    return path


def descriptor(value, label):
    if value is None:
        return None
    d = mapping(value, label)
    fmt = integer(d['1'], 255, label + ' format')
    buffer = integer(d['2'], 255, label + ' buffer')
    offset = integer(d['3'], 0xffffffff, label + ' offset')
    if fmt == 0 and (buffer != 255 or offset != 0xffffffff):
        raise ValueError('Malformed absent channel sentinel')
    return {'format': fmt, 'buffer': buffer, 'offset': offset}


def index_asset(asset_bytes, metadata_bytes, prefix):
    decoded = mapping(json.loads(metadata_bytes, object_pairs_hook=no_duplicates,
                                 parse_constant=reject_constant), 'decoded metadata')
    objects = mapping(decoded['objects'], 'objects')
    if not objects or len(objects) > MAX_OBJECTS:
        raise ValueError('Decoded object budget exceeded or empty objects')
    if 'result' in decoded and mapping(decoded['result'], 'result').get('parsed') is not True:
        raise ValueError('Metadata does not report completed parsing')
    for oid, obj in objects.items():
        if not re.fullmatch(r'0|[1-9][0-9]{0,9}', oid) or int(oid) > 0xffffffff:
            raise ValueError('Invalid decoded object identity')
        obj = mapping(obj, 'object')
        if not isinstance(obj['type'], str):
            raise ValueError('Invalid decoded object type')
        mapping(obj['data'], 'object data')
    buffers, total_buffer_bytes = {}, 0

    def buffer_table(value):
        nonlocal total_buffer_bytes
        table = []
        for entry in sequence(value, 64, 'mesh buffer table'):
            ref = integer(mapping(mapping(entry, 'buffer entry')['1'], 'buffer reference')['ref'],
                          0xffffffff, 'buffer reference')
            key = str(ref)
            if key not in objects or objects[key]['type'] != 'GFXHANDLE':
                raise ValueError('Buffer reference does not identify a decoded GFXHANDLE')
            if ref not in buffers:
                values = sequence(objects[key]['data']['2'], MAX_BYTES, 'buffer bytes')
                if any(type(v) is not int or not 0 <= v <= 255 for v in values):
                    raise ValueError('Invalid decoded buffer byte')
                total_buffer_bytes += len(values)
                if total_buffer_bytes > MAX_BYTES:
                    raise ValueError('Decoded buffer byte budget exceeded')
                buffers[ref] = bytes(values)
            table.append(buffers[ref])
        return table

    rows, unsupported, files = [], [], {}
    surface_count = 0
    for oid, obj in objects.items():
        if obj['type'] != 'CRenderMesh':
            continue
        mesh = obj['data']
        ib, vb = buffer_table(mesh['12']), buffer_table(mesh['13'])
        for lodnum, lod in enumerate(sequence(mesh['5'], 16, 'mesh LODs')):
            for section in sequence(mapping(lod, 'LOD')['5'], 64, 'LOD sections'):
                for surface in sequence(mapping(section, 'section')['1'], MAX_SURFACES, 'surfaces'):
                    surface_count += 1
                    if surface_count > MAX_SURFACES:
                        raise ValueError('Candidate surface budget exceeded')
                    surf = mapping(surface, 'surface')
                    triangles = integer(surf['2'], MAX_TRIANGLES, 'triangle count', 1)
                    vertices = integer(surf['3'], MAX_VERTICES, 'vertex count', 1)
                    uv = sequence(surf.get('14', []), 8, 'surface UVs')
                    declarations = {'positions': descriptor(surf.get('7'), 'positions'),
                                    'indices': descriptor(surf.get('6'), 'indices'),
                                    'weights': descriptor(surf.get('10'), 'weights'),
                                    'local_indices': descriptor(surf.get('11'), 'local indices'),
                                    'uv': descriptor(uv[0] if uv else None, 'UV0')}
                    missing = [name for name, d in declarations.items()
                               if d is None or d['format'] != FORMATS[name]]
                    palette = sequence(surf.get('18', []), 64, 'file-local palette identifiers')
                    for ident in palette:
                        integer(ident, 0xffffffff, 'file-local palette identifier')
                    channels, ranges = {}, {}
                    for name in CHANNELS:
                        d = declarations[name]
                        if name in missing:
                            continue
                        table = ib if name == 'indices' else vb
                        if d['buffer'] >= len(table):
                            raise ValueError('Channel buffer index exceeds decoded table')
                        if name != 'indices' and 'positions' not in missing and \
                                d['buffer'] != declarations['positions']['buffer']:
                            raise ValueError('Candidate vertex channels must share the decoded buffer')
                        count = triangles if name == 'indices' else vertices
                        size, offset = count * STRIDES[name], d['offset']
                        raw = table[d['buffer']]
                        if offset > len(raw) or size > len(raw) - offset:
                            raise ValueError('Candidate channel exceeds decoded buffer range')
                        channels[name] = raw[offset:offset + size]
                        ranges[name] = {'offset': offset, 'size': size,
                                        'format': d['format'], 'buffer': d['buffer']}
                    if ('weights' in channels and channels['weights'] != bytes((255, 0, 0, 0)) * vertices) or \
                            ('local_indices' in channels and channels['local_indices'] != bytes(vertices * 4)):
                        raise ValueError('Candidate is not exact single first-local-palette influence')
                    xyz = list(struct.iter_unpack('<3f', channels.get('positions', b'')))
                    uv_values = struct.iter_unpack('<2f', channels.get('uv', b''))
                    if any(not math.isfinite(v) for p in xyz for v in p) or \
                            any(not math.isfinite(v) for p in uv_values for v in p):
                        raise ValueError('Nonfinite candidate position or UV')
                    if any(t[0] >= vertices for t in struct.iter_unpack('<H', channels.get('indices', b''))):
                        raise ValueError('Candidate triangle index exceeds vertex channel')
                    if missing:
                        unsupported.append({'mesh_object': int(oid), 'lod': lodnum,
                                            'vertices': vertices, 'triangles': triangles,
                                            'reason': 'missing/unsupported channels: ' + ','.join(missing)})
                        continue
                    if not palette:
                        raise ValueError('Weighted candidate lacks a file-local palette')
                    channel_hashes = {name: hashlib.sha256(data).hexdigest()
                                      for name, data in channels.items()}
                    channel_files = {}
                    for name, data in channels.items():
                        filename = f'{prefix}.mesh{oid}.lod{lodnum}.surface{len(rows)}.{name}.bin'
                        files[filename] = data
                        channel_files[name] = filename
                    v = vb[declarations['positions']['buffer']]
                    i = ib[declarations['indices']['buffer']]
                    rows.append({'mesh_object': int(oid), 'lod': lodnum,
                                 'triangles': triangles, 'vertices': vertices,
                                 'positions_bytes': len(channels['positions']),
                                 'indices_bytes': len(channels['indices']),
                                 'positions_sha256': channel_hashes['positions'],
                                 'indices_sha256': channel_hashes['indices'],
                                 'whole_vertex_buffer_bytes': len(v),
                                 'whole_vertex_buffer_sha256': hashlib.sha256(v).hexdigest(),
                                 'whole_index_buffer_bytes': len(i),
                                 'whole_index_buffer_sha256': hashlib.sha256(i).hexdigest(),
                                 'position_offset': declarations['positions']['offset'],
                                 'index_offset': declarations['indices']['offset'],
                                 'palette_file_local_idents': palette,
                                 'channel_ranges': ranges, 'channel_sha256': channel_hashes,
                                 'channel_files': channel_files, 'single_body_influence': True,
                                 'bounds': [[min(p[c] for p in xyz), max(p[c] for p in xyz)]
                                            for c in range(3)]})
    return {'asset_sha256': hashlib.sha256(asset_bytes).hexdigest(),
            'metadata_sha256': hashlib.sha256(metadata_bytes).hexdigest(),
            'candidate_channels': rows, 'unsupported_candidates': unsupported,
            'live_association': False, 'effective_pose': False, 'grasp_reference': False,
            'decoded_asset_association_verified': False, 'historical_loaded_bytes_verified': False,
            'positive_grasp_verified': False, 'alignment_accepted': False}, files


def build_index(inputs, output):
    output = Path(output)
    if output.exists() or output.is_symlink():
        raise ValueError('Refuse existing candidate index output')
    output = private_path(output)
    root = output.parent
    if not root.is_dir() or not 1 <= len(inputs) <= MAX_ASSETS:
        raise ValueError('Choose an existing private output directory and bounded asset inputs')
    result, files = {}, {}
    for ordinal, (asset, metadata) in enumerate(inputs):
        asset, metadata = private_path(asset), private_path(metadata)
        if not asset.is_relative_to(root):
            raise ValueError('Assets must stay inside the index directory for private replay')
        if output in (asset, metadata) or asset == metadata:
            raise ValueError('Input/output path collision')
        name = asset.relative_to(root).as_posix()
        if name in result:
            raise ValueError('Duplicate candidate asset input')
        # Ordinal disambiguates equal basenames in separate private directories.
        slug = re.sub(r'[^A-Za-z0-9._-]', '_', asset.name)[:96]
        row, exported = index_asset(read_bounded(asset), read_bounded(metadata), f'asset{ordinal}.{slug}')
        if sum(map(len, files.values())) + sum(map(len, exported.values())) > MAX_BYTES:
            raise ValueError('Candidate output byte budget exceeded')
        result[name] = row
        files.update(exported)
    manifest = (json.dumps(result, indent=2, allow_nan=False) + '\n').encode('utf-8')
    if len(manifest) > MAX_BYTES or sum(map(len, files.values())) > MAX_BYTES:
        raise ValueError('Candidate output byte budget exceeded')
    targets = [(root / name, data) for name, data in files.items()] + [(output, manifest)]
    input_paths = {Path(p).resolve() for pair in inputs for p in pair}
    if len({p for p, _ in targets}) != len(targets) or any(
            p.exists() or p.is_symlink() or p in input_paths for p, _ in targets):
        raise ValueError('Refuse candidate channel/output collision')
    created = []
    try:
        for path, data in targets:
            fd = os.open(path, os.O_WRONLY | os.O_CREAT | os.O_EXCL | getattr(os, 'O_NOFOLLOW', 0), 0o600)
            info = os.fstat(fd)
            created.append((path, info.st_dev, info.st_ino))
            try:
                destination = os.fdopen(fd, 'wb')
            except BaseException:
                os.close(fd)
                raise
            with destination:
                if destination.write(data) != len(data):
                    raise OSError('Short private candidate write')
    except BaseException as failure:
        cleanup_errors = []
        for path, device, inode in reversed(created):
            try:
                info = path.lstat()
            except FileNotFoundError:
                continue
            except OSError:
                cleanup_errors.append('cannot inspect an export')
                continue
            if (info.st_dev, info.st_ino) != (device, inode):
                cleanup_errors.append('export ownership changed')
                continue
            try:
                path.unlink()
            except FileNotFoundError:
                continue
            except OSError:
                cleanup_errors.append('cannot remove an owned export')
        if cleanup_errors:
            raise RuntimeError('Candidate rollback incomplete: ' + '; '.join(cleanup_errors)) from failure
        raise
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--asset', nargs=2, type=Path, action='append', required=True,
                        metavar=('ASSET', 'DECODED_JSON'))
    parser.add_argument('--output', type=Path, required=True,
                        help='Fresh index JSON; parent is the existing private candidate root')
    args = parser.parse_args()
    try:
        result = build_index(args.asset, args.output)
        print(json.dumps({'assets': len(result), 'candidates': sum(len(r['candidate_channels']) for r in result.values()),
                          'live_association': False, 'positive_grasp_verified': False, 'alignment_accepted': False}))
    except (ValueError, KeyError, TypeError, OSError, struct.error, RecursionError, RuntimeError) as error:
        parser.exit(1, str(error) + '\n')


if __name__ == '__main__':
    main()
