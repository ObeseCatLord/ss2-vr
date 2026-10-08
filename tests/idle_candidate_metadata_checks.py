"""Synthetic selected-field production checks; no game assets or runtime probes."""
import copy
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import produce_idle_candidate_metadata as producer
import build_idle_candidate_index as builder
from idle_candidate_index_checks import fixture as geometry_fixture
from replay_idle_geometry import channel_bytes


def fixture():
    """Synthetic serialized selected tree with distinct nonzero geometry offsets."""
    layout = copy.deepcopy(next(iter(producer.load_policy().values())))
    raw = bytearray(b'synthetic-selected-fields\0')

    def append(data):
        start = len(raw)
        raw.extend(data)
        return [start, len(raw)]

    for card in layout['types']:
        # Independent fixture encoder for observed serialized declarations.
        name = card['name'].encode()
        record = b'DTTY' + struct.pack('<II', card['id'], len(name)) + name
        record += struct.pack('<IIi', card['version'], card['kind'], card['x'])
        if card['kind'] == 5:
            record += b'STMB' + struct.pack('<I', len(card['fields']))
            for field, tid in card['fields']:
                text = field.encode()
                record += struct.pack('<I', len(text)) + text + struct.pack('<I', tid)
        if card['kind'] == 4:
            record += b'ADIM' + struct.pack('<II', 1, card['count'])
        card['offset'], card['end'] = append(record)
    ids = {c['name']: c['id'] for c in layout['types']}
    layout['object_table'] = append(b'OBTY' + struct.pack('<7I', 3, 1, ids['CRenderMesh'],
                                        2, ids['GFXHANDLE'], 3, ids['GFXHANDLE']))[0]
    layout['objects_header'] = append(b'OBJS' + struct.pack('<I', 3))[0]
    decoded, channels = geometry_fixture()
    surf = decoded['objects']['1']['data']['5'][0]['5'][0]['1'][0]
    mesh = {'object': 1, 'type': ids['CRenderMesh'],
            'header': append(struct.pack('<II', 1, ids['CRenderMesh']))[0]}
    body_start = len(raw)
    mesh['index_references'] = append(b'STAR' + struct.pack('<Ii', 1, 3))
    mesh['vertex_references'] = append(b'STAR' + struct.pack('<Ii', 1, 2))
    lod_start = append(b'STAR' + struct.pack('<I', 1))[0]
    section_start = append(b'STAR' + struct.pack('<I', 1))[0]
    surface_start = append(b'STAR' + struct.pack('<I', 1))[0]
    surface = {'span': [len(raw), None], 'channels': {}}
    surface['triangles'] = append(struct.pack('<i', 1))[0]
    surface['vertices'] = append(struct.pack('<i', 3))[0]
    for key in ('6', '7', '10', '11'):
        d = surf[key]
        surface['channels'][key] = append(struct.pack('<BBI', d['1'], d['2'], d['3']))[0]
    d = surf['14'][0]
    uv_start = len(raw)
    append(struct.pack('<BBI', d['1'], d['2'], d['3']))
    append(struct.pack('<BBI', 0, 255, 0xffffffff) * 7)
    surface['uv_span'] = [uv_start, len(raw)]
    surface['palette_span'] = append(b'SSAR' + struct.pack('<II', 1, 4))
    surface['span'][1] = len(raw)
    section = {'span': [surface_start, len(raw)], 'surfaces_span': [surface_start, len(raw)],
               'surfaces': [surface]}
    lod = {'span': [section_start, len(raw)], 'sections_span': [section_start, len(raw)],
           'sections': [section]}
    mesh.update(span=[body_start, len(raw)], lods_span=[lod_start, len(raw)], lods=[lod])
    layout['mesh'], layout['buffers'] = mesh, []
    for oid in (2, 3):
        data = bytes(decoded['objects'][str(oid)]['data']['2'])
        header = append(struct.pack('<II', oid, ids['GFXHANDLE']))[0]
        span = append(b'STAR' + struct.pack('<I', len(data)) + data)
        layout['buffers'].append({'object': oid, 'type': ids['GFXHANDLE'], 'header': header,
                                  'span': span, 'bytes_span': span})
    # Intentionally uninterpreted bytes demonstrate the bounded scope.
    append(b'uninterpreted-tail')
    layout['asset_bytes'] = len(raw)
    return raw, layout, channels


def policy(raw, layout):
    return {hashlib.sha256(raw).hexdigest(): layout}


def surface(layout):
    return layout['mesh']['lods'][0]['sections'][0]['surfaces'][0]


class Checks(unittest.TestCase):
    def setUp(self):
        self.raw, self.layout, self.channels = fixture()
        self.temp = tempfile.TemporaryDirectory(prefix='selected-metadata-synthetic-')
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.asset, self.output = self.root / 'synthetic.bmf', self.root / 'selected.json'
        self.asset.write_bytes(self.raw)

    def produce(self, raw=None, layout=None):
        raw = self.raw if raw is None else raw
        layout = self.layout if layout is None else layout
        return producer.produce(raw, policy(raw, layout))

    def rejected(self, raw=None, layout=None):
        with self.assertRaises((ValueError, KeyError, TypeError, struct.error)):
            self.produce(raw, layout)

    def test_deterministic_selected_metadata_and_evidence_scope(self):
        metadata = self.produce()
        self.assertEqual(producer.encode(metadata), producer.encode(self.produce()))
        result = metadata['result']
        self.assertEqual(result['scope'], 'selected-serialized-fields')
        self.assertIs(result['selected_serialized_fields_from_pinned_asset_verified'], True)
        for flag in ('whole_file_decoded', 'whole_file_roundtrip_verified',
                     'native_loaded_resource_verified', 'rendered_instance_verified',
                     'historical_run_association_verified', 'positive_grasp_verified', 'alignment_accepted'):
            self.assertIs(result[flag], False)
        surf = metadata['objects']['1']['data']['5'][0]['5'][0]['1'][0]
        self.assertEqual(len(surf['14']), 8)
        self.assertEqual(surf['18'], [4])
        self.assertEqual(surf['2'], 1)
        self.assertEqual(surf['3'], 3)

    def test_exporter_and_existing_replay_interoperate(self):
        producer.write_private(self.output, producer.encode(self.produce()), self.asset)
        index = builder.build_index([(self.asset, self.output)], self.root / 'index.json')[self.asset.name]
        row, = index['candidate_channels']
        self.assertEqual(channel_bytes(self.root, self.asset.name,
                         dict(row, asset_sha256=index['asset_sha256'])), self.channels)
        for flag in ('live_association', 'decoded_asset_association_verified',
                     'historical_loaded_bytes_verified', 'positive_grasp_verified', 'alignment_accepted'):
            self.assertIs(index[flag], False)

    def test_fingerprint_checked_before_layout(self):
        self.assertRaisesRegex(ValueError, 'fingerprint', producer.produce, self.raw)
        changed = bytearray(self.raw)
        changed[-1] ^= 1
        self.assertRaisesRegex(ValueError, 'fingerprint', producer.produce, changed, policy(self.raw, self.layout))
        self.assertRaisesRegex(ValueError, 'fingerprint', producer.produce, self.raw[:-1], policy(self.raw, self.layout))
        bad = copy.deepcopy(self.layout)
        bad['asset_bytes'] -= 1
        self.rejected(layout=bad)

    def test_selected_type_bytes_ids_order_and_element_graph(self):
        for card in self.layout['types']:
            raw = bytearray(self.raw)
            raw[card['offset']] ^= 1
            self.rejected(raw)
        bad = copy.deepcopy(self.layout)
        card = next(c for c in bad['types'] if c['name'] == 'CRenderMesh')
        card['fields'].reverse()
        self.rejected(layout=bad)
        bad = copy.deepcopy(self.layout)
        bad['types'].append(copy.deepcopy(bad['types'][0]))
        self.rejected(layout=bad)
        # Matching serialized policy is still rejected when element semantics differ.
        for name, change in [('INDEX', {'x': 8}), ('IDENT', {'x': 1}),
                             ('CRenderMeshElementPath[8]', {'count': 7}),
                             ('Ptr<CVertexBuffer>', {'x': 1})]:
            bad, raw = copy.deepcopy(self.layout), bytearray(self.raw)
            card = next(c for c in bad['types'] if c['name'] == name)
            card.update(change)
            raw[card['offset']:card['end']] = producer.declaration(card)
            self.rejected(raw, bad)

    def test_object_tags_counts_headers_and_duplicate_identity(self):
        for offset in (self.layout['object_table'], self.layout['objects_header'],
                       self.layout['mesh']['header'], self.layout['buffers'][0]['header']):
            raw = bytearray(self.raw)
            raw[offset] ^= 1
            self.rejected(raw)
        raw = bytearray(self.raw)
        struct.pack_into('<I', raw, self.layout['object_table'] + 4, 4097)
        self.rejected(raw)
        raw = bytearray(self.raw)
        struct.pack_into('<I', raw, self.layout['object_table'] + 16, 1)
        self.rejected(raw)
        bad = copy.deepcopy(self.layout)
        bad['buffers'][1]['object'] = bad['buffers'][0]['object']
        self.rejected(layout=bad)

    def test_references_array_tags_counts_and_target_types(self):
        start = self.layout['mesh']['index_references'][0]
        for offset, value in [(start, 0), (start + 4, 65), (start + 4, 0),
                              (start + 8, 0xffffffff), (start + 8, 1), (start + 8, 99)]:
            raw = bytearray(self.raw)
            struct.pack_into('<I', raw, offset, value)
            self.rejected(raw)

    def test_nested_array_counts_order_extent_and_parent_containment(self):
        mesh = self.layout['mesh']
        for node, key in [(mesh, 'lods_span'), (mesh['lods'][0], 'sections_span'),
                          (mesh['lods'][0]['sections'][0], 'surfaces_span')]:
            raw = bytearray(self.raw)
            struct.pack_into('<I', raw, node[key][0] + 4, 2)
            self.rejected(raw)
        for mutate in (lambda b: b['mesh']['lods'][0]['span'].__setitem__(0, mesh['lods'][0]['span'][0] + 1),
                       lambda b: surface(b)['span'].__setitem__(1, len(self.raw)),
                       lambda b: b['mesh']['span'].__setitem__(1, mesh['span'][1] - 1)):
            bad = copy.deepcopy(self.layout)
            mutate(bad)
            self.rejected(layout=bad)

    def test_leaf_overlap_and_span_budgets(self):
        for mutate in (lambda b: surface(b)['channels'].__setitem__('7', surface(b)['channels']['6']),
                       lambda b: surface(b).__setitem__('triangles', -1),
                       lambda b: surface(b).__setitem__('vertices', len(self.raw)),
                       lambda b: surface(b)['uv_span'].__setitem__(1, surface(b)['uv_span'][1] - 1),
                       lambda b: b['buffers'][0]['bytes_span'].__setitem__(1, len(self.raw))):
            bad = copy.deepcopy(self.layout)
            mutate(bad)
            self.rejected(layout=bad)

    def test_palette_and_byte_buffer_framing(self):
        for span, tag in [(surface(self.layout)['palette_span'], b'SSAR'),
                          (self.layout['buffers'][0]['bytes_span'], b'STAR')]:
            for offset, value in [(span[0], 0), (span[0] + 4, 0xffffffff), (span[0] + 4, 0)]:
                raw = bytearray(self.raw)
                struct.pack_into('<I', raw, offset, value)
                self.rejected(raw)

    def test_malformed_geometry_is_rejected_before_output(self):
        surf = surface(self.layout)
        vb = self.layout['buffers'][0]['bytes_span'][0] + 8
        ib = self.layout['buffers'][1]['bytes_span'][0] + 8
        cases = [('<i', surf['triangles'], -1), ('<i', surf['vertices'], 1491),
                 ('<I', surf['channels']['7'] + 2, 0xffffffff),
                 ('<B', surf['channels']['7'] + 1, 64), ('<f', vb + 20, float('nan')),
                 ('<B', vb + 56, 254), ('<B', vb + 68, 1), ('<H', ib + 10, 3)]
        for fmt, offset, value in cases:
            with self.subTest(offset=offset, value=value):
                raw = bytearray(self.raw)
                struct.pack_into(fmt, raw, offset, value)
                self.rejected(raw)

    def test_unsupported_surface_preserved_and_cannot_hide_malformed_data(self):
        raw = bytearray(self.raw)
        struct.pack_into('<BBI', raw, surface(self.layout)['channels']['10'], 0, 255, 0xffffffff)
        result = self.produce(raw)
        row, _ = builder.index_asset(raw, producer.encode(result), 'synthetic')
        self.assertEqual(len(row['unsupported_candidates']), 1)
        self.assertEqual(row['candidate_channels'], [])
        struct.pack_into('<I', raw, surface(self.layout)['channels']['7'] + 2, 0xffffffff)
        self.rejected(raw)

    def test_private_output_mode_collision_git_and_symlinks(self):
        payload = producer.encode(self.produce())
        producer.write_private(self.output, payload, self.asset)
        self.assertEqual(self.output.stat().st_mode & 0o777, 0o600)
        self.assertEqual(self.output.read_bytes(), payload)
        for path in (self.output, self.asset, ROOT / 'forbidden.json'):
            self.assertRaises(ValueError, producer.write_private, path, payload, self.asset)
        link = self.root / 'link.json'
        link.symlink_to(self.asset)
        self.assertRaises(ValueError, producer.write_private, link, payload, self.asset)
        missing = self.root / 'missing-target.json'
        dangling = self.root / 'dangling.json'
        dangling.symlink_to(missing)
        self.assertRaises(ValueError, producer.write_private, dangling, payload, self.asset)
        self.assertTrue(dangling.is_symlink())
        self.assertEqual(dangling.readlink(), missing)
        self.assertFalse(missing.exists())
        git = self.root / 'git'
        git.mkdir()
        (git / '.git').write_text('gitdir: elsewhere')
        self.assertRaises(ValueError, producer.write_private, git / 'metadata.json', payload, self.asset)
        self.assertEqual(self.asset.read_bytes(), self.raw)

    def test_write_failure_rolls_back_owned_output(self):
        with patch.object(producer.os, 'fdopen', side_effect=OSError('synthetic write failure')):
            self.assertRaises(OSError, producer.write_private, self.output, b'example', self.asset)
        self.assertFalse(self.output.exists())

    def test_cli_cannot_override_policy_and_rejects_unknown_asset(self):
        for flags in ([], ['-O']):
            result = subprocess.run([sys.executable, *flags, '-B', str(ROOT / 'tools/produce_idle_candidate_metadata.py'),
                    '--asset', str(self.asset), '--output', str(self.output)], capture_output=True, text=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn('Unknown or changed asset fingerprint', result.stderr)
            self.assertFalse(self.output.exists())
        result = subprocess.run([sys.executable, '-B', str(ROOT / 'tools/produce_idle_candidate_metadata.py'),
                 '--help'], capture_output=True, text=True)
        self.assertEqual(result.returncode, 0)
        self.assertNotIn('--layout', result.stdout)
        self.assertIn('--verify-metadata', result.stdout)

    def test_cli_asset_derivation_verifier_rejects_changed_metadata(self):
        with patch.object(producer, 'load_policy', return_value=policy(self.raw, self.layout)):
            with patch.object(sys, 'argv', ['producer', '--asset', str(self.asset), '--output', str(self.output)]):
                producer.main()
            for mode in ([], ['changed']):
                if mode:
                    metadata = json.loads(self.output.read_bytes())
                    metadata['objects']['1']['data']['5'][0]['5'][0]['1'][0]['3'] = 4
                    self.output.write_bytes(producer.encode(metadata))
                with patch.object(sys, 'argv', ['producer', '--asset', str(self.asset),
                                              '--verify-metadata', str(self.output)]):
                    if mode:
                        self.assertRaises(SystemExit, producer.main)
                    else:
                        producer.main()


if __name__ == '__main__':
    unittest.main()
