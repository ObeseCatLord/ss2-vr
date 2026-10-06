"""Offline temporary-repository checks for the actual publication gate."""
import importlib.util
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('gate', Path(__file__).resolve().parents[1] / 'tools/verify_publication_history.py')
gate = importlib.util.module_from_spec(spec)
spec.loader.exec_module(gate)


class PublicationHistoryChecks(unittest.TestCase):
    def test_history_and_ref_validation(self):
        with tempfile.TemporaryDirectory() as directory:
            repo = Path(directory)
            env = dict(os.environ, GIT_AUTHOR_NAME='SS2VR contributors', GIT_COMMITTER_NAME='SS2VR contributors',
                       GIT_AUTHOR_EMAIL='', GIT_COMMITTER_EMAIL='', GIT_CONFIG_NOSYSTEM='1',
                       GIT_CONFIG_GLOBAL=os.devnull)
            def run(*args, input=None):
                return subprocess.check_output(['git', '-C', directory, *args], input=input,
                                               env=env, stderr=subprocess.DEVNULL).strip().decode()
            run('init', '-q')
            tree = run('mktree', input=b'')
            clean = run('commit-tree', tree, '-m', 'Clean root')
            self.assertEqual(gate.verify(repo, [clean]), 1)
            message_identity = run('commit-tree', tree, '-m', 'fixture@example.invalid')
            with self.assertRaises(ValueError):
                gate.verify(repo, [message_identity])
            env['GIT_COMMITTER_NAME'] = 'Private identity'
            named = run('commit-tree', tree, '-m', 'No address')
            with self.assertRaises(ValueError):
                gate.verify(repo, [named])
            env['GIT_COMMITTER_NAME'] = 'SS2VR contributors'
            env['GIT_AUTHOR_EMAIL'] = 'fixture@example.invalid'
            dirty = run('commit-tree', tree, '-p', clean, '-m', 'Fixture only')
            env['GIT_AUTHOR_EMAIL'] = ''
            descendant = run('commit-tree', tree, '-p', dirty, '-m', 'Clean tip cannot hide parent')
            for ref in (dirty, descendant, tree, '^'+dirty, clean+'..'+descendant):
                with self.assertRaises((ValueError, subprocess.CalledProcessError)):
                    gate.verify(repo, [ref])
            run('update-ref', 'refs/heads/main', clean)
            run('tag', '-a', 'annotated', clean, '-m', 'Tag metadata excluded')
            with self.assertRaises(ValueError):
                gate.verify(repo, ['annotated'])
            # Replacement objects must not conceal the actual outgoing identity.
            run('replace', dirty, clean)
            with self.assertRaises(ValueError):
                gate.verify(repo, [dirty])
            (repo / '.git/info/grafts').write_text(descendant + '\n')
            with self.assertRaises(ValueError):
                gate.verify(repo, [descendant])
            self.assertEqual(gate.verify(repo, ['main', clean]), 1)
            (repo / '.git/shallow').write_text(clean + '\n')
            with self.assertRaises(ValueError):
                gate.verify(repo, [clean])


if __name__ == '__main__':
    unittest.main()
