#!/usr/bin/env python3
"""Summarize private passive laser records; returned line calls are not visibility."""
import argparse
from collections import Counter
import json
import math
from pathlib import Path
import re

KINDS = ('budget', 'saturation', 'event', 'muzzle', 'binding', 'freeze', 'dispatch')
LINE = re.compile(r'Lab laser (' + '|'.join(KINDS) + r')\s+(.*)')
FIELD = re.compile(r'([A-Za-z][A-Za-z0-9]*)=([^\s]+)')


def freeze_rejection(row):
    """Handheld predicate from copied values only; vehicle evidence is incomplete."""
    if row['kinds'] != '0,0':
        return 'vehicle-or-kind-change-unqualified'
    if not row['sampleValid']:
        return 'sample-invalid'
    if not row['sampleOwner'] or not row['sampleWeapon']:
        return 'sample-identity-empty'
    for sample, frame in [('sampleOwner', 'owner'), ('sampleWeapon', 'weapon'),
                          ('sampleGeneration', 'generation'), ('sampleInput', 'input'),
                          ('sampleRequest', 'request')]:
        if row[sample] != row[frame]:
            return sample + '-mismatch'
    if row['now'] < row['tick'] or row['now'] - row['tick'] > 100:
        return 'sample-age'
    if not row['handValid']:
        return 'hand-invalid'
    if row['finite'] != '1,1,1':
        return 'pose-nonfinite'
    if not all(math.isfinite(float(v)) for v in row['end'].split(',')):
        return 'end-nonfinite'
    if row['wheel'] or row['selecting']:
        return 'wheel-or-selection'
    if not (float(row['drift2']) <= .0009 and abs(float(row['alignment'])) >= .99995):
        return 'body-drift'
    return 'eligible'


def assess(lines):
    records = []
    for number, line in enumerate(lines, 1):
        match = LINE.search(line)
        if not match:
            continue
        fields = dict(FIELD.findall(match[2]))
        if fields.get('schema') != '1':
            raise ValueError('Unsupported laser schema at line ' + str(number))
        row = {'record': match[1], 'line': number}
        for key, value in fields.items():
            row[key] = int(value) if re.fullmatch(r'-?\d+', value) else value
        records.append(row)
    counts = Counter(row['record'] for row in records)
    events = Counter((row['event'], row['reason']) for row in records if row['record'] == 'event')
    freeze = Counter()
    mismatches = []
    bindings = {}
    binding_differences = []
    drops = [0] * 6
    for row in records:
        if row['record'] == 'budget':
            values = list(map(int, row['dropped'].split(',')))
            if len(values) != 6:
                raise ValueError('Malformed drop counters')
            drops = [max(a, b) for a, b in zip(drops, values)]
        elif row['record'] == 'saturation':
            drops[row['stage']] = max(drops[row['stage']], row['dropped'])
        elif row['record'] == 'binding':
            bindings.setdefault(row['order'], {})[row['kind']] = row
        elif row['record'] == 'freeze':
            reason = freeze_rejection(row)
            freeze[reason] += 1
            if reason != 'vehicle-or-kind-change-unqualified' and bool(row['valid']) != (reason == 'eligible'):
                mismatches.append({'line': row['line'], 'derived': reason, 'native_valid': row['valid']})
    fields = ['owner', 'weapon', 'model', 'instance', 'selector', 'hand', 'cfg', 'file', 'resource', 'stretch']
    for order, pair in bindings.items():
        if 'cache' in pair and 'current' in pair:
            changed = [name for name in fields if pair['cache'][name] != pair['current'][name]]
            binding_differences.append({'order': order, 'changed_fields': changed,
                'difference_mask': sum(1 << i for i, name in enumerate(fields) if name in changed)})
    dispatches = [row for row in records if row['record'] == 'dispatch']
    muzzles = [row for row in records if row['record'] == 'muzzle']
    return {'schema': 1, 'record_counts': dict(counts),
        'events': [{'event': event, 'reason': reason, 'count': count} for (event, reason), count in sorted(events.items())],
        'muzzle_outcomes': dict(Counter(f"admitted={r['admitted']},attempted={r['attempted']},captured={r['captured']},failure={r['failure']}" for r in muzzles)),
        'freeze_outcomes': dict(freeze), 'freeze_predicate_disagreements': mismatches,
        'binding_differences': binding_differences, 'observed_dropped_lower_bounds': drops,
        'dispatches_returned': len(dispatches), 'dispatch_eye_counts': dict(Counter(str(r['eye']) for r in dispatches)),
        'visible_lasers_verified': False, 'diagnostic_records_present': bool(records),
        'limits': ['Copied-value diagnostics only; no native queries.',
                   'Silence after saturation cannot prove nonexecution.',
                   'Logging can affect native freshness timing.',
                   'Returned drawing calls do not prove visibility.']}


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('log', type=Path)
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    result = assess(args.log.read_text(errors='replace').splitlines())
    text = json.dumps(result, indent=2) + '\n'
    if args.output:
        args.output.write_text(text)
    else:
        print(text, end='')
