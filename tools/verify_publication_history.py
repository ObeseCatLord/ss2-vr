#!/usr/bin/env python3
"""Read-only email-free commit ancestry gate; never prints identity values.

Run in the isolated publication checkout, against every ref that will be pushed.
This does not erase remote caches, inspect file contents, or authorize a push.
"""
import argparse
from pathlib import Path
import re
import subprocess


def git(repo, *args):
    return subprocess.check_output(
        ['git', '--no-replace-objects', '-C', str(repo), *args],
        stderr=subprocess.DEVNULL,
    )


def verify(repo, refs):
    if git(repo, 'rev-parse', '--is-shallow-repository').strip() != b'false':
        raise ValueError('Cannot certify incomplete shallow history')
    roots = set()
    for ref in refs:
        # Resolve separately, so a caller cannot supply rev-list exclusions or
        # flags that silently omit email-bearing ancestors.
        oid = git(repo, 'rev-parse', '--verify', '--end-of-options', ref).strip()
        if not re.fullmatch(rb'[0-9a-f]{40}|[0-9a-f]{64}', oid):
            raise ValueError('Invalid resolved object ID')
        if git(repo, 'cat-file', '-t', oid.decode()).strip() != b'commit':
            raise ValueError('Publication refs must point directly to commits; annotated tags require separate inspection')
        roots.add(oid)
    if not roots:
        raise ValueError('No publication commits found')
    commits = set()
    pending = list(roots)
    while pending:
        oid = pending.pop()
        if oid in commits:
            continue
        commits.add(oid)
        raw = git(repo, 'cat-file', 'commit', oid.decode())
        if b'@' in raw:
            raise ValueError('Possible email-bearing commit content in ' + oid.decode())
        headers = raw.split(b'\n\n', 1)[0].splitlines()
        if any(line.startswith((b'mergetag ', b'gpgsig ')) for line in headers):
            raise ValueError('Embedded tag/signature requires separate privacy inspection')
        # Walk raw parent headers: local grafts/replacements cannot hide ancestry.
        for line in headers:
            if line.startswith(b'parent '):
                parent = line[7:]
                if not re.fullmatch(rb'[0-9a-f]{40}|[0-9a-f]{64}', parent):
                    raise ValueError('Malformed parent header')
                pending.append(parent)
        for role in (b'author', b'committer'):
            identities = [line for line in headers if line.startswith(role + b' ')]
            if len(identities) != 1 or not re.fullmatch(
                role + rb' SS2VR(?: contributors)? <> [0-9]+ [+-][0-9]{4}', identities[0]
            ):
                raise ValueError('Nonempty or invalid identity metadata in commit ' + oid.decode())
    return len(commits)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--repo', type=Path, default=Path.cwd())
    parser.add_argument('refs', nargs='+', help='Every outgoing commit/ref; no revision ranges')
    args = parser.parse_args()
    try:
        count = verify(args.repo, args.refs)
    except (ValueError, subprocess.CalledProcessError) as error:
        # CalledProcessError can include a caller-supplied argument; don't echo it.
        print('FAIL: ' + (str(error) if isinstance(error, ValueError) else 'Git history inspection failed'))
        return 1
    print(f'PASS: {count} reachable commits have empty author and committer emails')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
