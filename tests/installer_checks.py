"""Installer checks on synthetic files only; never modifies the actual installation."""
import hashlib
import importlib.util
import json
from pathlib import Path
import tempfile
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('installer',ROOT/'tools/install.py')
installer=importlib.util.module_from_spec(spec);spec.loader.exec_module(installer)
def digest(data):return hashlib.sha256(data).hexdigest()
with tempfile.TemporaryDirectory(prefix='ss2vr-installer-') as folder:
 root=Path(folder);game=root/'game';package=root/'package';(game/'Bin').mkdir(parents=True);package.mkdir()
 native=b'synthetic game fixture';(game/'Bin/Sam2.exe').write_bytes(native)
 files={'Bin/d3d9.dll':b'synthetic proxy','Bin/SS2VR/ss2vr_host.exe':b'synthetic host',
        'Bin/SS2VRServer.dll':b'synthetic native server module','Content/SS2VR.mod':b'synthetic module selection'}
 for relative,data in files.items():p=package/relative;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(data)
 manifest={'files':{p:digest(data) for p,data in files.items()},'version':'fixture','game_fingerprints':{'Sam2.exe':{'sha256':digest(native)}}}
 (package/'manifest.json').write_text(json.dumps(manifest))
 installer.install(game,package,True);assert not (game/'Bin/d3d9.dll').exists()
 collision=game/'Bin/d3d9.dll';collision.write_bytes(b'existing unrelated mod')
 try:installer.install(game,package,False);raise AssertionError('Collision accepted')
 except FileExistsError:pass
 assert collision.read_bytes()==b'existing unrelated mod';collision.unlink()
 content_collision=game/'Content/SS2VR.mod';content_collision.parent.mkdir();content_collision.write_bytes(b'other module selection')
 try:installer.install(game,package,False);raise AssertionError('Module collision accepted')
 except FileExistsError:pass
 assert content_collision.read_bytes()==b'other module selection' and not collision.exists();content_collision.unlink()
 installer.install(game,package,False);assert collision.read_bytes()==files['Bin/d3d9.dll']
 collision.write_bytes(b'modified payload')
 try:installer.uninstall(game,False);raise AssertionError('Modified payload removed')
 except ValueError:pass
 assert collision.read_bytes()==b'modified payload';collision.write_bytes(files['Bin/d3d9.dll'])
 installer.uninstall(game,False);assert not collision.exists();assert (game/'Bin/Sam2.exe').read_bytes()==native
 # Unsafe paths must be rejected before any mutation.
 for relative in ['../escape','Bin/../escape','Bin/Engine.dll','Bin/SS2VR/../../escape',
                  'Content/SeriousSam2.mod','Content/SS2VR.mod/child','Bin/SS2VRServer.dll/child']:
  try:installer.paths({'files':{relative:'0'*64}});raise AssertionError('Unsafe path accepted')
  except ValueError:pass
 print('Synthetic collision, integrity, install/remove and path checks passed; actual game untouched.')
