"""Dependency path discovery fixtures; native bytes and game execution are absent."""
from pathlib import Path
import sys
import tempfile
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from verify_sniper_predicate_sites import minhook_paths, MINHOOK_FILES

with tempfile.TemporaryDirectory(prefix='ss2vr-predicate-source-') as temporary:
    root=Path(temporary)
    for name in ['actual dependency','configured','explicit','deps/vendor/minhook','build-game']:
        (root/name).mkdir(parents=True)
    cache=root/'build-game/CMakeCache.txt'
    expected=root/'deps/vendor/minhook'
    assert minhook_paths(root=root)=={k:expected/v for k,v in MINHOOK_FILES.items()}
    cache.write_text('MINHOOK_SOURCE:PATH=configured\n')
    assert minhook_paths(root=root)['trampoline_c_sha256']==root/'configured/src/trampoline.c'
    cache.write_text('MINHOOK_SOURCE:PATH=configured\nminhook_SOURCE_DIR:STATIC='+str(root/'actual dependency')+'\n')
    assert minhook_paths(root=root)['trampoline_c_sha256']==root/'actual dependency/src/trampoline.c'
    assert minhook_paths(root/'explicit',root)['trampoline_c_sha256']==root/'explicit/src/trampoline.c'
    cache.write_text('MINHOOK_SOURCE:PATH=\nminhook_SOURCE_DIR:STATIC='+str(root/'actual dependency')+'\n')
    assert minhook_paths(root=root)['hde32_c_sha256']==root/'actual dependency/src/hde/hde32.c'
    cache.write_text('minhook_SOURCE_DIR:STATIC='+str(root/'missing')+'\n')
    try:
        minhook_paths(root=root)
    except FileNotFoundError:
        pass
    else:
        raise AssertionError('Missing configured dependency silently fell back to another source')
print('Explicit, configured, relative, spaced, fallback and missing dependency paths checked.')
