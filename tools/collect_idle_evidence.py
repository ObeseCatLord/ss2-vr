#!/usr/bin/env python3
"""Collect private OFFLINE idle evidence; never launches a game or changes settings."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import tempfile

from assess_idle_weapon import assess
from replay_idle_geometry import replay

ROOT = Path(__file__).resolve().parents[1]
MAX_BYTES = 16 * 1024 * 1024


def snapshot(path):
    if path.is_symlink():
        raise ValueError('Input symlinks are not admitted')
    before = path.stat()
    if not path.is_file() or before.st_size > MAX_BYTES:
        raise ValueError('Expected bounded regular input')
    with path.open('rb') as stream:
        raw = stream.read(MAX_BYTES + 1)
        after = os.fstat(stream.fileno())
    current = path.stat()
    identity = lambda s: (s.st_dev, s.st_ino, s.st_size, s.st_mtime_ns)
    if len(raw) > MAX_BYTES or identity(before) != identity(after) or identity(after) != identity(current):
        raise ValueError('Input changed during collection')
    return raw


def collect(log, candidates, private_root, output, expected_source, native_id, evaluator, evaluator_sha):
    private_root = private_root.resolve(strict=True)
    if private_root.is_relative_to(ROOT) or ROOT.is_relative_to(private_root) or not private_root.is_dir():
        raise ValueError('Choose a private root disjoint from source')
    for path in (log, candidates):
        if path.is_symlink() or not path.resolve(strict=True).is_relative_to(private_root):
            raise ValueError('Inputs must stay in the private root')
    output = output.absolute()
    if output.exists() or output.is_symlink() or not output.parent.resolve(strict=True).is_relative_to(private_root):
        raise ValueError('Choose a fresh output directory in the private root')
    if native_id not in (1, 2, 13):
        raise ValueError('Unsupported native weapon selector')
    log_bytes = snapshot(log)
    index_bytes = snapshot(candidates)
    evaluation_bytes = snapshot(evaluator)
    if hashlib.sha256(evaluation_bytes).hexdigest() != evaluator_sha:
        raise ValueError('Offline evaluator fingerprint mismatch')
    evidence = assess(log_bytes.decode('utf-8'), expected_source)
    observations = evidence['copied_event_pose_observations'] + evidence['rejected_or_missing_observations']
    if any(row['nativeId'] != native_id for row in observations):
        raise ValueError('Collected weapon does not match selected native ID')
    index = json.loads(index_bytes)
    # Decode/match/replay through existing consumers. Assets stay in their private
    # candidate root; every referenced channel is hash/length checked there.
    output.mkdir(mode=0o700, exist_ok=False)
    receipt = {'schema': 1, 'source_fingerprint': expected_source, 'native_id': native_id,
               'log_sha256': hashlib.sha256(log_bytes).hexdigest(),
               'candidate_index_sha256': hashlib.sha256(index_bytes).hexdigest(),
               'evaluator_sha256': evaluator_sha, 'game_launched': False,
               'settings_changed': False, 'native_execution_by_collector': False}
    receipt['candidate_root']=str(candidates.parent.resolve())
    (output / 'input-receipt.json').write_text(json.dumps(receipt, indent=2) + '\n')
    (output / 'input.log').write_bytes(log_bytes)
    (output / 'candidate-index.json').write_bytes(index_bytes)
    try:
        with tempfile.TemporaryDirectory(dir=output, prefix='owned-arithmetic-') as temp:
            temporary = Path(temp)
            # Execute the exact fingerprinted snapshot, not a path which could
            # be replaced between verification and a later draw's replay.
            checked_evaluator = temporary / 'checked-evaluator'
            checked_evaluator.write_bytes(evaluation_bytes)
            checked_evaluator.chmod(0o700)
            result = replay(evidence, index, candidates.parent, checked_evaluator, temporary)
        (output / 'idle-evidence.json').write_text(json.dumps(evidence, indent=2) + '\n')
        (output / 'position-replay.json').write_text(json.dumps(result, indent=2) + '\n')
        qualified = [row for row in result['draws'] if
                     row['result'] in ('unique-consumed-channel-match', 'unique-position-and-auxiliary-channel-match') and
                     row.get('reference_kind') in ('cold-first-material-pc24-nearest-staged-local', 'uploaded-transform-corroboration') and
                     row['position_replay']['position_replay_agrees_with_reference']]
        summary = {'schema': 1, 'native_id': native_id,
                   'completed_observations': len(evidence['copied_event_pose_observations']),
                   'rejected_observations': len(evidence['rejected_or_missing_observations']),
                   'qualified_unique_draws': len(qualified),
                   'qualified_draws': [{key: row[key] for key in
                        ('request', 'eye', 'hand', 'nativeId', 'geometry_index', 'reference_kind', 'candidates')} for row in qualified],
                   'native_shader_or_world_execution_by_collector': False,
                   'positive_grasp_verified': False, 'alignment_accepted': False,
                   'remaining': ['Confirm the qualified draw is the intended handle-bearing surface and reference assembly.',
                                 'Review model/muzzle/scope integration before activating a correction.']}
        (output / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
        return summary
    except BaseException as error:
        (output / 'failure.json').write_text(json.dumps({'schema': 1, 'error_type': type(error).__name__,
            'message': str(error), 'game_launched': False, 'preserve_output': True}, indent=2) + '\n')
        raise


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('log', 'candidates', 'private-root', 'output', 'evaluator'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--expected-source', required=True)
    parser.add_argument('--native-id', type=int, choices=(1, 2, 13), required=True)
    parser.add_argument('--evaluator-sha256', required=True)
    args = parser.parse_args()
    summary = collect(args.log, args.candidates, args.private_root, args.output,
                      args.expected_source, args.native_id, args.evaluator, args.evaluator_sha256)
    print(json.dumps({key: summary[key] for key in ('native_id', 'completed_observations',
        'rejected_observations', 'qualified_unique_draws', 'alignment_accepted')}))
