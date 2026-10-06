#!/usr/bin/env python3
"""Inspect compiled MinGW x86 mod ABI; never executes native code."""
import subprocess,re,json,hashlib,argparse
from pathlib import Path
from abi_camera_forwarding import verify_camera_forwarding
ROOT=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--artifact',choices=['d3d9.dll','SS2VRServer.dll'],default='d3d9.dll')
artifact=parser.parse_args().artifact
p=ROOT/'build-game'/artifact
nm=subprocess.check_output(['i686-w64-mingw32-nm',str(p)],text=True)
entries={}
for function,needle,size in [('weaponRender','L12weaponRender',48),('sniperRender','L12sniperRender',48),('weaponAbs','L9weaponAbs',8),('weaponFrustum','L13weaponFrustum',0),('weaponMatrixInverse','L19weaponMatrixInverse',0),('weaponDepthRange','L16weaponDepthRange',0),('sniperAlternativePressed','L24sniperAlternativePressed',0),('mountedLookClamp','L16mountedLookClamp',4),('renderThirdPerson','L17renderThirdPerson',0)]:
 rows=[line.split() for line in nm.splitlines() if needle in line and ' t ' in line and '.cold' not in line]
 if not rows:
  rows=[line.split() for line in nm.splitlines() if function in line and ' t ' in line and '.cold' not in line]
 assert len(rows)==1,(function,rows)
 address=int(rows[0][0],16)
 # Next defined text symbol bounds compiler-generated function, then inspect
 # only own plugin machine code. Native code dumps are not exported.
 ends=[int(line.split()[0],16) for line in nm.splitlines() if re.search(r' [tT] ',line) and int(line.split()[0],16)>address]
 end=min(ends)
 assembly=subprocess.check_output(['i686-w64-mingw32-objdump','-d',f'--start-address={address}',f'--stop-address={end}',str(p)],text=True)
 rets=re.findall(r'\bret\s*(?:\$(0x[0-9a-f]+))?',assembly)
 if not rets and function=='mountedLookClamp':
  # The one-call wrapper may sibling-tail-call its __thiscall trampoline.
  # Prove the actual original argument slot, ECX and ESP are restored, rather
  # than accepting any indirect jump as equivalent to callee cleanup.
  trampoline=[line.split() for line in nm.splitlines() if 'L17originalLookClampE' in line]
  assert len(trampoline)==1,trampoline
  target=int(trampoline[0][0],16)
  instructions=[line.split('\t')[-1].strip() for line in assembly.splitlines()
                if re.match(r'^[0-9a-f]+:',line.strip()) and
                   re.match(r'^[a-z]+(?:\s|$)',line.split('\t')[-1].strip())]
  assert instructions[:6]==['push   %edi','lea    0x8(%esp),%edi','and    $0xfffffff8,%esp',
                            'push   -0x4(%edi)','push   %ebp','mov    %esp,%ebp'],instructions[:6]
  saved_entry=re.search(r'mov    %edi,(-0x[0-9a-f]+\(%ebp\))',assembly).group(1)
  saved_brain=re.search(r'mov    %ecx,(-0x[0-9a-f]+\(%ebp\))',assembly).group(1)
  load=instructions.index('mov    (%edi),%eax')
  saved_argument=next(re.fullmatch(r'mov    %eax,(-0x[0-9a-f]+\(%ebp\))',instruction).group(1)
                      for instruction in instructions[load+1:]
                      if re.fullmatch(r'mov    %eax,(-0x[0-9a-f]+\(%ebp\))',instruction))
  tail=[f'mov    {saved_entry},%eax',f'mov    {saved_argument},%edx','mov    %edx,(%eax)',
        f'mov    {saved_brain},%ecx','lea    -0xc(%ebp),%esp','pop    %ebx','pop    %esi',
        'pop    %edi','pop    %ebp','lea    -0x8(%edi),%esp','pop    %edi',f'jmp    *0x{target:x}']
  assert any(instructions[i:i+len(tail)]==tail for i in range(len(instructions))),tail
  exits=re.findall(r'\bjmp\s+\*([^\n]+)',assembly)
  assert exits==[f'0x{target:x}'],exits
  source=(ROOT/'src/game/engine.cpp').read_text()
  assert 'using LookClamp = void(__thiscall *)(void *, Vec3 &);' in source
  entries[function]={'return_cleanup_bytes':size,'cleanup_owner':'audited native __thiscall trampoline',
                     'tail_transfer_preserves_ecx_esp_and_original_reference':True}
  continue
 assert rets and all((int(x,16) if x else 0)==size for x in rets),(function,rets)
 if size==48:
  instructions=[line.split('\t')[-1].strip() for line in assembly.splitlines()
                if re.match(r'^[0-9a-f]+:',line.strip()) and
                   re.match(r'^[a-z]+(?:\s|$)',line.split('\t')[-1].strip())]
  camera_proof = verify_camera_forwarding(instructions, sniper=(function == 'sniperRender'))
 entries[function]={'return_cleanup_bytes':size}
 if size==48:
  entries[function]['by_value_camera_unchanged']=True
  entries[function].update(camera_proof)
print(json.dumps({'runtime_executed':False,'artifact':artifact,'artifact_sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'compiled_x86_abi':entries,'method':'defined-symbol bounded objdump, native functions never executed'},indent=2))
