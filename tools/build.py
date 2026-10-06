#!/usr/bin/env python3
"""Cross-compile both Windows architectures and run only native offline checks."""
import argparse
from pathlib import Path
import subprocess
ROOT = Path(__file__).resolve().parents[1]
p = argparse.ArgumentParser()
p.add_argument('--jobs', type=int, default=4)
p.add_argument('--minhook-source', type=Path)
p.add_argument('--openxr-source', type=Path)
a = p.parse_args()
for component, bits, dependency in [('game', 32, a.minhook_source), ('host', 64, a.openxr_source), ('core', 0, None)]:
    directory = ROOT / ('build-' + component)
    command = ['cmake', '-S', str(ROOT), '-B', str(directory), '-DSS2VR_COMPONENT=' + component,
               '-DCMAKE_BUILD_TYPE=' + ('Debug' if component == 'core' else 'Release')]
    if bits:
        command += ['-DCMAKE_TOOLCHAIN_FILE=' + str(ROOT / 'cmake' / ('mingw%d.cmake' % bits))]
    if bits and not dependency:
        command += ['-D' + ('MINHOOK_SOURCE' if component=='game' else 'OPENXR_SOURCE') + '=']
    if dependency:
        command += ['-D' + ('MINHOOK_SOURCE' if component == 'game' else 'OPENXR_SOURCE') + '=' + str(dependency.resolve())]
    subprocess.run(command, check=True)
    subprocess.run(['cmake', '--build', str(directory), '-j', str(max(1, a.jobs))], check=True)
subprocess.run(['ctest', '--test-dir', str(ROOT / 'build-core'), '--output-on-failure'], check=True)
print('Builds and offline checks complete. No game, host, Wine, or headset was launched.')
