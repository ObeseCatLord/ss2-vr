"""Verification guards must remain active when Python optimization is enabled."""
import ast
from pathlib import Path
import subprocess
import sys
ROOT=Path(__file__).resolve().parents[1]
checked=0
for path in sorted((ROOT/'tools').glob('verify_*.py')):
    tree=ast.parse(path.read_text(),filename=str(path))
    if any(isinstance(node,ast.Assert) for node in ast.walk(tree)):
        raise ValueError('Verification must not depend on removable Python assert: '+path.name)
    for node in tree.body:
        if isinstance(node,ast.FunctionDef) and node.name=='require':
            # Exercise only this pure guard, not a validator's module-level
            # native-file/compiler work. No Windows code or game file is needed.
            source=ast.unparse(node)+'''
try:
    require(False, "negative guard fixture")
except (ValueError, AssertionError):
    pass
else:
    raise RuntimeError("Required verifier guard was optimized away")
require(True, "positive guard fixture")
'''
            subprocess.run([sys.executable,'-O','-c',source],check=True)
            checked+=1
if checked<20:
    raise ValueError('Unexpectedly few explicit production guards were checked')
print('Verified',checked,'production guards under Python -O; no removable validator assertions.')
