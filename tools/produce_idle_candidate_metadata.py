#!/usr/bin/env python3
"""Produce selected idle mesh metadata from exact, owned BMF asset pins.

This is a selected-field reader, not a general BMF decoder. No native loaded
resource, rendered instance, historical run, grasp or alignment is certified.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import struct

from build_idle_candidate_index import (MAX_BYTES, index_asset, integer,
                                        no_duplicates, private_path,
                                        read_bounded, reject_constant)

POLICY = Path(__file__).with_name('idle_candidate_metadata_layouts.json')


def encode(value):
    return (json.dumps(value, sort_keys=True, indent=2, allow_nan=False) + '\n').encode()


def load_policy():
    policy = json.loads(POLICY.read_bytes(), object_pairs_hook=no_duplicates,
                        parse_constant=reject_constant)
    if policy['schema'] != 1 or len(policy['layouts']) != 4:
        raise ValueError('Invalid bundled metadata policy')
    return policy['layouts']


class SelectedReader:
    def __init__(self, raw):
        self.raw, self.leaves = raw, []

    def span(self, value, parent=None):
        if not isinstance(value, list) or len(value) != 2:
            raise ValueError('Invalid selected span')
        start = integer(value[0], len(self.raw), 'span start')
        end = integer(value[1], len(self.raw), 'span end', start + 1)
        if parent is not None and not (parent[0] <= start < end <= parent[1]):
            raise ValueError('Selected span exceeds parent')
        return [start, end]

    def take(self, start, size, parent=None):
        span = self.span([start, start + size], parent)
        if any(span[0] < old[1] and old[0] < span[1] for old in self.leaves):
            raise ValueError('Overlapping selected leaves')
        self.leaves.append(span)
        return self.raw[span[0]:span[1]]

    def unpack(self, fmt, start, parent=None):
        raw = self.take(start, struct.calcsize(fmt), parent)
        values = struct.unpack(fmt, raw)
        if struct.pack(fmt, *values) != raw:
            raise ValueError('Selected scalar re-encoding differs')
        return values

    def array(self, span, tag, maximum, parent):
        span = self.span(span, parent)
        marker, count = self.unpack('<4sI', span[0], span)
        if marker != tag:
            raise ValueError('Wrong selected array tag')
        integer(count, maximum, 'selected array count')
        return count, [span[0] + 8, span[1]]


def declaration(card):
    name = card['name'].encode('ascii')
    raw = struct.pack('<4sII', b'DTTY', card['id'], len(name)) + name
    raw += struct.pack('<IIi', card['version'], card['kind'], card['x'])
    if card['kind'] == 5:
        raw += struct.pack('<4sI', b'STMB', len(card['fields']))
        for field, target in card['fields']:
            text = field.encode('utf-8')
            raw += struct.pack('<I', len(text)) + text + struct.pack('<I', target)
    elif card['kind'] == 4:
        raw += struct.pack('<4sII', b'ADIM', 1, card['count'])
    return raw


def validate_types(reader, cards):
    if not isinstance(cards, list) or not 1 <= len(cards) <= 64:
        raise ValueError('Invalid selected type budget')
    types, names = {}, {}
    for card in cards:
        tid = integer(card['id'], 4096, 'type identity')
        if tid in types or card['name'] in names:
            raise ValueError('Duplicate selected type')
        expected = declaration(card)
        if card['end'] != card['offset'] + len(expected) or \
                reader.take(card['offset'], len(expected)) != expected:
            raise ValueError('Selected serialized type declaration differs')
        types[tid], names[card['name']] = card, card

    def named(name, kind, width=None):
        card = names[name]
        if card['kind'] != kind or (width is not None and card['x'] != width):
            raise ValueError('Wrong selected type semantics: ' + name)
        return card

    def field(name, key):
        card = named(name, 5)
        fields = card['fields']
        if len({p[0] for p in fields}) != len(fields):
            raise ValueError('Duplicate selected struct field')
        return types[dict(fields)[key]]

    def target(card, kind, name):
        if card['kind'] != kind or types[card['x']]['name'] != name:
            raise ValueError('Wrong selected array/reference element type')

    named('INDEX', 13, named('SLONG', 0, 4)['id'])
    for name, width in [('IDENT', 4), ('ULONG', 4), ('UBYTE', 1)]:
        named(name, 0, width)
    for key in ('12', '13'):
        target(field('CRenderMesh', key), 6, 'VBufferHolder')
    target(field('VBufferHolder', '1'), 11, 'GFXHANDLE')
    target(field('CRenderMesh', '5'), 6, 'CRenderMeshLOD')
    target(field('CRenderMeshLOD', '5'), 6, 'CRenderMeshSection')
    target(field('CRenderMeshSection', '1'), 6, 'CRenderMeshSurface')
    for key in ('2', '3'):
        if field('CRenderMeshSurface', key)['name'] != 'INDEX':
            raise ValueError('Wrong selected signed count type')
    for key in ('6', '7', '10', '11'):
        if field('CRenderMeshSurface', key)['name'] != 'CRenderMeshElementPath':
            raise ValueError('Wrong selected descriptor type')
    for key, name in [('1', 'UBYTE'), ('2', 'UBYTE'), ('3', 'ULONG')]:
        if field('CRenderMeshElementPath', key)['name'] != name:
            raise ValueError('Wrong selected descriptor scalar type')
    uv = field('CRenderMeshSurface', '14')
    target(uv, 4, 'CRenderMeshElementPath')
    if uv['count'] != 8:
        raise ValueError('Wrong selected UV array length')
    target(field('CRenderMeshSurface', '18'), 7, 'IDENT')
    target(field('GFXHANDLE', '2'), 6, 'UBYTE')
    for card in cards:
        if card['kind'] == 5 and card['x'] != -1:
            if types[card['x']]['kind'] != 5:
                raise ValueError('Wrong selected base type')
            seen, current = set(), card
            while current['x'] != -1:
                if current['id'] in seen:
                    raise ValueError('Cyclic selected base type')
                seen.add(current['id'])
                current = types[current['x']]
    return names


def produce(raw, layouts=None):
    """Pure reader. Optional policy injection is for synthetic tests, never CLI."""
    if not raw or len(raw) > MAX_BYTES:
        raise ValueError('Invalid asset byte budget')
    digest = hashlib.sha256(raw).hexdigest()
    layouts = load_policy() if layouts is None else layouts
    if digest not in layouts or layouts[digest]['asset_bytes'] != len(raw):
        raise ValueError('Unknown or changed asset fingerprint')
    layout = layouts[digest]
    reader = SelectedReader(raw)
    names = validate_types(reader, layout['types'])
    table_start = layout['object_table']
    tag, count = reader.unpack('<4sI', table_start)
    if tag != b'OBTY' or not 1 <= count <= 4096:
        raise ValueError('Wrong object type table framing/count')
    pairs = reader.unpack('<' + 'II' * count, table_start + 8)
    table = dict(zip(pairs[::2], pairs[1::2]))
    if len(table) != count:
        raise ValueError('Duplicate object type table identity')
    if reader.unpack('<4sI', layout['objects_header']) != (b'OBJS', count):
        raise ValueError('Wrong objects header/count')
    mesh, buffers = layout['mesh'], layout['buffers']
    if not isinstance(buffers, list) or not 1 <= len(buffers) <= 128:
        raise ValueError('Invalid selected buffer budget')
    object_spans, objects = [], {}
    for obj, name in [(mesh, 'CRenderMesh')] + [(b, 'GFXHANDLE') for b in buffers]:
        oid, tid = obj['object'], names[name]['id']
        span = reader.span(obj['span'])
        extent = reader.span([obj['header'], span[1]])
        if any(extent[0] < p[1] and p[0] < extent[1] for p in object_spans + reader.leaves):
            raise ValueError('Selected object overlaps another object or framing')
        if obj['type'] != tid or table.get(oid) != tid or \
                reader.unpack('<II', obj['header']) != (oid, tid) or span[0] != obj['header'] + 8:
            raise ValueError('Selected object identity/type/header differs')
        if str(oid) in objects or any(span[0] < p[1] and p[0] < span[1] for p in object_spans):
            raise ValueError('Duplicate/overlapping selected objects')
        object_spans.append(extent)
        objects[str(oid)] = {'type': name, 'data': {}}
    buffer_ids = {b['object'] for b in buffers}

    def references(span):
        count, body = reader.array(span, b'STAR', 64, mesh['span'])
        if body[1] - body[0] != count * 4:
            raise ValueError('Wrong selected reference array extent')
        refs = reader.unpack('<' + 'i' * count, body[0], span) if count else ()
        if any(ref!=-1 and ref not in buffer_ids for ref in refs):
            raise ValueError('Selected reference does not identify an admitted GFXHANDLE')
        return [{'1': {'ref': ref}} for ref in refs]

    def array_children(node, span_key, children_key, tag, maximum, parent):
        span = reader.span(node[span_key], parent)
        count, body = reader.array(span, tag, maximum, parent)
        children = node[children_key]
        if not isinstance(children, list) or count != len(children):
            raise ValueError('Selected array count differs from pinned children')
        cursor = body[0]
        for child in children:
            child_span = reader.span(child['span'], body)
            if child_span[0] != cursor:
                raise ValueError('Selected child ordering/extent differs')
            cursor = child_span[1]
        if cursor != body[1]:
            raise ValueError('Selected array body extent differs')
        return children

    def desc(offset, parent):
        values = reader.unpack('<BBI', offset, parent)
        return dict(zip(('1', '2', '3'), values))

    data = objects[str(mesh['object'])]['data']
    data['12'], data['13'] = references(mesh['index_references']), references(mesh['vertex_references'])
    if {r['1']['ref'] for r in data['12'] + data['13'] if r['1']['ref']!=-1} != buffer_ids:
        raise ValueError('Selected buffers differ from mesh reference union')
    data['5'] = []
    for lod in array_children(mesh, 'lods_span', 'lods', b'STAR', 16, mesh['span']):
        lod_data = {'5': []}
        data['5'].append(lod_data)
        for section in array_children(lod, 'sections_span', 'sections', b'STAR', 64, lod['span']):
            section_data = {'1': []}
            lod_data['5'].append(section_data)
            for surf in array_children(section, 'surfaces_span', 'surfaces', b'STAR', 512, section['span']):
                parent = surf['span']
                out = {'2': reader.unpack('<i', surf['triangles'], parent)[0],
                       '3': reader.unpack('<i', surf['vertices'], parent)[0]}
                if set(surf['channels']) != {'6', '7', '10', '11'}:
                    raise ValueError('Wrong selected descriptor field set')
                out.update({key: desc(offset, parent) for key, offset in surf['channels'].items()})
                uv = reader.span(surf['uv_span'], parent)
                if uv[1] - uv[0] != 48:
                    raise ValueError('Wrong selected UV extent')
                out['14'] = [desc(uv[0] + i * 6, uv) for i in range(8)]
                count, body = reader.array(surf['palette_span'], b'SSAR', 64, parent)
                if body[1] - body[0] != count * 4:
                    raise ValueError('Wrong selected palette extent')
                out['18'] = list(reader.unpack('<' + 'I' * count, body[0], surf['palette_span'])) if count else []
                section_data['1'].append(out)
    for buf in buffers:
        count, body = reader.array(buf['bytes_span'], b'STAR', MAX_BYTES, buf['span'])
        if count != body[1] - body[0]:
            raise ValueError('Wrong selected byte buffer extent')
        values = reader.take(body[0], count, buf['bytes_span']) if count else b''
        objects[str(buf['object'])]['data']['2'] = list(values)
    result = {'objects': objects, 'result': {
        'parsed': True, 'scope': 'selected-serialized-fields',
        'asset_sha256': digest, 'asset_bytes': len(raw),
        'selected_serialized_fields_from_pinned_asset_verified': True,
        'whole_file_decoded': False, 'whole_file_roundtrip_verified': False,
        'native_loaded_resource_verified': False, 'rendered_instance_verified': False,
        'historical_run_association_verified': False, 'positive_grasp_verified': False,
        'alignment_accepted': False}}
    encoded = encode(result)
    if len(encoded) > MAX_BYTES:
        raise ValueError('Selected metadata byte budget exceeded')
    # Reuse the delivered geometry validator, preserving its evidence flags.
    index_asset(raw, encoded, 'selected',layout.get('native_id',1))
    return result


def write_private(output, payload, asset):
    output = Path(output)
    if output.exists() or output.is_symlink():
        raise ValueError('Refuse private output collision')
    output, asset = private_path(output), private_path(asset)
    if output == asset or output.exists() or output.is_symlink() or not output.parent.is_dir():
        raise ValueError('Refuse private output collision or missing parent')
    fd = os.open(output, os.O_WRONLY | os.O_CREAT | os.O_EXCL | getattr(os, 'O_NOFOLLOW', 0), 0o600)
    info = os.fstat(fd)
    try:
        try:
            destination = os.fdopen(fd, 'wb')
        except BaseException:
            os.close(fd)
            raise
        with destination:
            if destination.write(payload) != len(payload):
                raise OSError('Short private metadata write')
    except BaseException as failure:
        try:
            current = output.lstat()
            if (current.st_dev, current.st_ino) != (info.st_dev, info.st_ino):
                raise RuntimeError('Private metadata output ownership changed')
            output.unlink()
        except FileNotFoundError:
            pass
        except (OSError, RuntimeError):
            raise RuntimeError('Private metadata rollback incomplete') from failure
        raise


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--asset', type=Path, required=True)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument('--output', type=Path, help='Fresh private selected metadata JSON')
    mode.add_argument('--verify-metadata', type=Path, help='Compare canonical selected metadata to asset derivation')
    args = parser.parse_args()
    try:
        asset = private_path(args.asset)
        result = produce(read_bounded(asset))
        payload = encode(result)
        if args.verify_metadata is not None:
            supplied = json.loads(read_bounded(private_path(args.verify_metadata)),
                                  object_pairs_hook=no_duplicates, parse_constant=reject_constant)
            if encode(supplied) != payload:
                raise ValueError('Supplied selected metadata differs from pinned asset derivation')
        else:
            write_private(args.output, payload, asset)
        print(json.dumps({'selected_serialized_fields_from_pinned_asset_verified': True,
                          'native_loaded_resource_verified': False, 'alignment_accepted': False}))
    except (ValueError, KeyError, TypeError, OSError, struct.error, RecursionError, RuntimeError) as error:
        parser.exit(1, str(error) + '\n')


if __name__ == '__main__':
    main()
