"""Synthetic private-file exporter checks; no proprietary data or runtime probes."""
import copy
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import build_idle_candidate_index as builder
from replay_idle_geometry import channel_bytes


def fixture():
    positions = struct.pack('<9f', 1, 2, 3, 4, 5, 6, -1, 2, 0)
    channels = {'positions': positions, 'indices': struct.pack('<3H', 0, 1, 2),
                'weights': bytes((255, 0, 0, 0)) * 3, 'local_indices': bytes(12),
                'uv': struct.pack('<6f', 0, 0, 1, 0, 0, 1)}
    vertex_buffer = bytearray(b'V' * 20)  # Offset exceeds the decoy asset length.
    declarations = {}
    for name in ('positions', 'weights', 'local_indices', 'uv'):
        declarations[name] = {'1': builder.FORMATS[name], '2': 0, '3': len(vertex_buffer)}
        vertex_buffer.extend(channels[name])
    index_buffer = b'I' * 10 + channels['indices']
    surface = {'2': 1, '3': 3, '6': {'1': 135, '2': 0, '3': 10},
               '7': declarations['positions'], '10': declarations['weights'],
               '11': declarations['local_indices'], '14': [declarations['uv']], '18': [4]}
    metadata = {'objects': {'0': {'type': 'CResourceFile', 'data': {}},
        '1': {'type': 'CRenderMesh', 'data': {
        '12': [{'1': {'ref': 3}}], '13': [{'1': {'ref': 2}}],
        '5': [{'5': [{'1': [surface]}]}]}},
        '2': {'type': 'GFXHANDLE', 'data': {'2': list(vertex_buffer)}},
        '3': {'type': 'GFXHANDLE', 'data': {'2': list(index_buffer)}}},
        'result': {'parsed': True}}
    return metadata, channels


def surface(metadata):
    return metadata['objects']['1']['data']['5'][0]['5'][0]['1'][0]


class Checks(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix='idle-candidate-synthetic-')
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.asset = self.root / 'synthetic.bmf'
        self.metadata = self.root / 'decoded.json'
        self.output = self.root / 'index.json'
        self.asset.write_bytes(b'decoy asset')
        self.decoded, self.channels = fixture()

    def build(self, metadata=None):
        value = self.decoded if metadata is None else metadata
        self.metadata.write_text(json.dumps(value))
        return builder.build_index([(self.asset, self.metadata)], self.output)

    def rejected(self, metadata):
        with self.assertRaises((ValueError, KeyError, TypeError)):
            self.build(metadata)
        self.assertFalse(self.output.exists())
        self.assertFalse(list(self.root.glob('*.bin')))
        self.assertEqual(self.asset.read_bytes(), b'decoy asset')

    def test_exact_export_and_existing_replay_consumer(self):
        result = self.build()
        self.assertEqual(json.loads(self.output.read_text()), result)
        asset = result[self.asset.name]
        row, = asset['candidate_channels']
        row = dict(row, asset_sha256=asset['asset_sha256'])
        self.assertEqual(channel_bytes(self.root, self.asset.name, row), self.channels)
        self.assertEqual(asset['asset_sha256'], hashlib.sha256(b'decoy asset').hexdigest())
        self.assertEqual(row['position_offset'], 20)
        self.assertGreater(row['position_offset'], self.asset.stat().st_size)
        self.assertEqual(row['bounds'], [[-1.0, 4.0], [2.0, 5.0], [0.0, 6.0]])
        self.assertEqual(row['palette_file_local_idents'], [4])
        self.assertTrue(row['single_body_influence'])
        for name, data in self.channels.items():
            self.assertEqual(row['channel_ranges'][name]['size'], len(data))
            self.assertEqual(row['channel_sha256'][name], hashlib.sha256(data).hexdigest())
            self.assertEqual((self.root / row['channel_files'][name]).read_bytes(), data)
        for flag in ('live_association', 'effective_pose', 'grasp_reference',
                     'decoded_asset_association_verified', 'historical_loaded_bytes_verified',
                     'positive_grasp_verified', 'alignment_accepted'):
            self.assertIs(asset[flag], False)
        self.assertEqual(self.asset.read_bytes(), b'decoy asset')

    def test_missing_channels_are_explicitly_unsupported(self):
        for field in ('6', '7', '10', '11', '14'):
            with self.subTest(field=field):
                metadata = copy.deepcopy(self.decoded)
                del surface(metadata)[field]
                result = self.build(metadata)[self.asset.name]
                self.assertEqual(result['candidate_channels'], [])
                self.assertEqual(len(result['unsupported_candidates']), 1)
                self.assertFalse(list(self.root.glob('*.bin')))
                self.output.unlink()
        surface(self.decoded)['10'] = {'1': 0, '2': 255, '3': 0xffffffff}
        self.assertEqual(self.build()[self.asset.name]['candidate_channels'], [])

    def test_bad_reference_type_and_identity(self):
        for ref in (-1, True, 0, 999, 0x100000000, '2'):
            bad = copy.deepcopy(self.decoded)
            bad['objects']['1']['data']['13'][0]['1']['ref'] = ref
            with self.subTest(ref=ref):
                self.rejected(bad)
        bad = copy.deepcopy(self.decoded)
        bad['objects']['2']['type'] = 'CRenderMesh'
        self.rejected(bad)
        bad = copy.deepcopy(self.decoded)
        bad['objects']['01'] = bad['objects'].pop('1')
        self.rejected(bad)

    def test_missing_channel_cannot_hide_malformed_supported_data(self):
        for key, value in (('2', 63), ('3', 0xffffffff)):
            bad = copy.deepcopy(self.decoded)
            del surface(bad)['10']
            surface(bad)['7'][key] = value
            self.rejected(bad)
        bad = copy.deepcopy(self.decoded)
        del surface(bad)['10']
        surface(bad)['18'] = [True]
        self.rejected(bad)
        bad = copy.deepcopy(self.decoded)
        del surface(bad)['10']
        bad['objects']['2']['data']['2'][20:24] = list(struct.pack('<f', float('nan')))
        self.rejected(bad)

    def test_bad_counts_offsets_and_buffer_slots(self):
        for field, values in [('2', (0, -1, True, 1333)), ('3', (0, -1, True, 1491))]:
            for value in values:
                bad = copy.deepcopy(self.decoded)
                surface(bad)[field] = value
                with self.subTest(field=field, value=value):
                    self.rejected(bad)
        for field in ('6', '7', '10', '11'):
            for key, values in [('2', (-1, True, 1, 256)), ('3', (-1, True, 0xffffffff, 0x100000000))]:
                for value in values:
                    bad = copy.deepcopy(self.decoded)
                    surface(bad)[field][key] = value
                    with self.subTest(field=field, key=key, value=value):
                        self.rejected(bad)
        bad = copy.deepcopy(self.decoded)
        bad['objects']['1']['data']['13'].append({'1': {'ref': 2}})
        surface(bad)['10']['2'] = 1
        self.rejected(bad)

    def test_malformed_indices_and_nonfinite_geometry(self):
        bad = copy.deepcopy(self.decoded)
        bad['objects']['3']['data']['2'][10:12] = list(struct.pack('<H', 3))
        self.rejected(bad)
        for value in (float('nan'), float('inf'), -float('inf')):
            for offset in (20, surface(self.decoded)['14'][0]['3']):
                bad = copy.deepcopy(self.decoded)
                bad['objects']['2']['data']['2'][offset:offset + 4] = list(struct.pack('<f', value))
                with self.subTest(value=value, offset=offset):
                    self.rejected(bad)

    def test_weight_and_palette_validation(self):
        for channel in ('weights', 'local_indices'):
            bad = copy.deepcopy(self.decoded)
            offset = surface(bad)['10' if channel == 'weights' else '11']['3']
            bad['objects']['2']['data']['2'][offset] = 1
            self.rejected(bad)
        for value in ([], [True], [-1], [0x100000000]):
            bad = copy.deepcopy(self.decoded)
            surface(bad)['18'] = value
            self.rejected(bad)

    def test_typed_bytes_and_metadata_budgets(self):
        for value in (-1, 256, True, 1.0, '0'):
            bad = copy.deepcopy(self.decoded)
            bad['objects']['2']['data']['2'][0] = value
            self.rejected(bad)
        bad = copy.deepcopy(self.decoded)
        bad['result']['parsed'] = False
        self.rejected(bad)
        with patch.object(builder, 'MAX_OBJECTS', 2):
            self.rejected(self.decoded)
        with patch.object(builder, 'MAX_SURFACES', 0):
            self.rejected(self.decoded)
        with patch.object(builder, 'MAX_BYTES', 10):
            self.rejected(self.decoded)

    def test_duplicate_keys_and_nonfinite_json(self):
        for text in ('{"objects":{},"objects":{}}', '{"objects":NaN}', '{"objects":Infinity}'):
            self.metadata.write_text(text)
            with self.assertRaises(ValueError):
                builder.build_index([(self.asset, self.metadata)], self.output)
            self.assertFalse(self.output.exists())

    def test_existing_output_and_channel_collisions(self):
        result = self.build()
        before = {p: p.read_bytes() for p in self.root.iterdir()}
        with self.assertRaises(ValueError):
            builder.build_index([(self.asset, self.metadata)], self.output)
        self.assertEqual({p: p.read_bytes() for p in self.root.iterdir()}, before)
        self.output.unlink()
        with self.assertRaises(ValueError):
            builder.build_index([(self.asset, self.metadata)], self.output)
        self.assertFalse(self.output.exists())
        for filename in result[self.asset.name]['candidate_channels'][0]['channel_files'].values():
            self.assertEqual((self.root / filename).read_bytes(), before[self.root / filename])

    def test_private_output_and_input_paths(self):
        repo = self.root / 'source'
        repo.mkdir()
        (repo / '.git').mkdir()
        (repo / '.git' / 'HEAD').write_text('ref: refs/heads/main\n')
        for output in (ROOT / 'refused-private-index.json', repo / 'index.json', self.root / 'missing' / 'index.json'):
            with self.assertRaises(ValueError):
                builder.build_index([(self.asset, self.metadata)], output)
        outside = self.root.parent / (self.root.name + '-outside.bmf')
        with self.assertRaises(ValueError):
            builder.build_index([(outside, self.metadata)], self.output)
        self.metadata.write_text(json.dumps(self.decoded))
        with self.assertRaises(ValueError):
            builder.build_index([(self.asset, self.metadata)] * 2, self.output)
        if hasattr(Path, 'symlink_to'):
            link = self.root / 'link.json'
            link.symlink_to(self.root / 'missing.json')
            with self.assertRaises(ValueError):
                builder.build_index([(self.asset, self.metadata)], link)
            self.assertTrue(link.is_symlink())
        self.assertFalse(self.output.exists())

    def test_two_assets_with_same_basename_remain_distinct(self):
        self.metadata.write_text(json.dumps(self.decoded))
        nested = self.root / 'other'
        nested.mkdir()
        asset = nested / self.asset.name
        asset.write_bytes(b'other asset')
        result = builder.build_index([(self.asset, self.metadata), (asset, self.metadata)], self.output)
        self.assertEqual(set(result), {self.asset.name, 'other/' + self.asset.name})
        a, b = (r['candidate_channels'][0]['channel_files'] for r in result.values())
        self.assertFalse(set(a.values()) & set(b.values()))
        for key, r in result.items():
            row = dict(r['candidate_channels'][0], asset_sha256=r['asset_sha256'])
            self.assertEqual(channel_bytes(self.root, key, row), self.channels)

    def test_late_collision_rolls_back_only_owned_exports(self):
        self.metadata.write_text(json.dumps(self.decoded))
        original_open = builder.os.open
        calls, competing = [], []

        def racing_open(path, flags, mode):
            calls.append(path)
            if len(calls) == 2:
                Path(path).write_bytes(b'competing private file')
                competing.append(Path(path))
            return original_open(path, flags, mode)

        with patch.object(builder.os, 'open', side_effect=racing_open):
            with self.assertRaises(FileExistsError):
                builder.build_index([(self.asset, self.metadata)], self.output)
        self.assertFalse(Path(calls[0]).exists())
        self.assertEqual(competing[0].read_bytes(), b'competing private file')
        self.assertFalse(self.output.exists())
        self.assertEqual(self.asset.read_bytes(), b'decoy asset')

    def test_failed_destination_wrapping_rolls_back_new_file(self):
        self.metadata.write_text(json.dumps(self.decoded))
        with patch.object(builder.os, 'fdopen', side_effect=OSError('synthetic wrapper failure')):
            with self.assertRaises(OSError):
                builder.build_index([(self.asset, self.metadata)], self.output)
        self.assertFalse(self.output.exists())
        self.assertFalse(list(self.root.glob('*.bin')))
        self.assertEqual(self.asset.read_bytes(), b'decoy asset')

    def test_rollback_continues_after_disappeared_or_replaced_export(self):
        self.metadata.write_text(json.dumps(self.decoded))
        original_open = builder.os.open
        for replaced in (False, True):
            with self.subTest(replaced=replaced):
                calls = []

                def racing_open(path, flags, mode):
                    calls.append(Path(path))
                    if len(calls) == 3:
                        if replaced:
                            calls[1].rename(self.root / 'moved-owned.bin')
                            calls[1].write_bytes(b'replacement private file')
                        else:
                            calls[1].unlink()
                        Path(path).write_bytes(b'late collision')
                    return original_open(path, flags, mode)

                expected = RuntimeError if replaced else FileExistsError
                with patch.object(builder.os, 'open', side_effect=racing_open):
                    with self.assertRaises(expected) as caught:
                        builder.build_index([(self.asset, self.metadata)], self.output)
                self.assertFalse(calls[0].exists())
                self.assertEqual(calls[2].read_bytes(), b'late collision')
                self.assertFalse(self.output.exists())
                if replaced:
                    self.assertEqual(calls[1].read_bytes(), b'replacement private file')
                    self.assertIsInstance(caught.exception.__cause__, FileExistsError)
                else:
                    self.assertFalse(calls[1].exists())
                for path in self.root.glob('*.bin'):
                    path.unlink()

    def test_cli_normal_and_optimized(self):
        self.metadata.write_text(json.dumps(self.decoded))
        for options in ([], ['-O']):
            output = self.root / ('optimized.json' if options else 'normal.json')
            result = subprocess.run([sys.executable, *options, str(ROOT / 'tools/build_idle_candidate_index.py'),
                                     '--asset', str(self.asset), str(self.metadata), '--output', str(output)],
                                    capture_output=True, text=True, timeout=10)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(json.loads(result.stdout), {'assets': 1, 'candidates': 1,
                'live_association': False, 'positive_grasp_verified': False, 'alignment_accepted': False})
            for filename in json.loads(output.read_text())[self.asset.name]['candidate_channels'][0]['channel_files'].values():
                (self.root / filename).unlink()
            output.unlink()


if __name__ == '__main__':
    unittest.main()
