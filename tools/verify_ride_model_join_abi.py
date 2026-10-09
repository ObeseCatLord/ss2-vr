#!/usr/bin/env python3
"""Finite x86 getter forwarding/cleanup check; no native execution."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
from verify_ride_control_abi import (bodies,cleanup_cases,decoded_nodes,extent,finally_delegates,forwards,
    instructions,paths,require,symbol_offset,symbol_section)


def verify_extent(assembly,symbols):
    table=bodies(assembly);cases=0
    for name,mangled,slot_name,outer in (
        ('observedRideRenderable','@_ZN5ss2vr4gameL22observedRideRenderable','originalRideRenderable',False),
        ('observedRideModelInstance','@_ZN5ss2vr4gameL25observedRideModelInstance','originalRideModelInstance',True),
    ):
        slot=symbol_offset(symbols,slot_name);section=symbol_section(symbols,slot_name)
        run=extent(table,name+'(void*','::Context::run(void*)')
        transfers=forwards(run,slot,section);require(len(transfers)==1,'Missing unique getter result call')
        paths(run,transfers,'')
        code=instructions(run);index=next(i for i,item in enumerate(code) if item[0] in transfers)
        require(code[index][1]=='call' and code[index+1][1:] in
                (('mov','dword ptr [esi], eax'),('mov','dword ptr [ebx], eax')),
                'Native EAX result is not immediately stored')
        entry=extent(table,mangled,'@8')
        delegates=finally_delegates(entry)
        paths(entry,forwards(entry,slot,section)|delegates,'')
        finish=extent(table,name+'(void*','::Context::finish(void*, int)')
        require(not any(mn=='call' or mn.startswith('f') or any(reg in op for reg in ('xmm','ymm','zmm'))
                        for _,mn,op in instructions(finish)),'Getter cleanup gained callbacks/FP work')
        offset=symbol_offset(symbols,'activeRideModelJoin') if outer else 0
        cases+=cleanup_cases(finish,outer,offset,model=True)
    return {'normal_paths_forward_once':True,'callee_relocation_identity_verified':True,
            'return_opcode_stack_pop_bytes':0,'immediate_native_eax_store_verified':True,
            'cleanup_cases_checked':cases,'outer_tls_final_value_verified':True,
            'scope':'original counts/relocations, ret opcode, immediate EAX store and finite cleanup effects; capture/result-reference origin and entry stack balance remain source review',
            'runtime_executed':False}


def verify(obj):
    assembly=subprocess.check_output(['objdump','-drC','-Mintel','--insn-width=16',str(obj)],text=True)
    symbols=subprocess.check_output(['objdump','-tC',str(obj)],text=True)
    require('file format pe-i386' in assembly,'Expected x86 object')
    result=verify_extent(assembly,symbols);result['object_sha256']=hashlib.sha256(obj.read_bytes()).hexdigest()
    return result


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--object',type=Path,required=True)
    try:print(json.dumps(verify(p.parse_args().object),indent=2))
    except (ValueError,OSError,subprocess.SubprocessError) as e:p.exit(1,str(e)+'\n')
