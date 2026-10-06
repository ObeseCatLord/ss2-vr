#!/usr/bin/env python3
"""Read-only COFF/PE boundary proof and compile-only GNU callback fixture.
Does not execute Windows code, raise a native exception or establish recovery.
"""
import hashlib
import json
import re
from pathlib import Path
import struct
import subprocess
import capstone
import pefile
ROOT = Path(__file__).resolve().parents[1]

def coff(path):
    raw = path.read_bytes()
    machine, count, _, symbols, symbol_count, optional, _ = struct.unpack_from('<HHIIIHH', raw)
    assert machine == 0x14c and not optional
    sections = {}
    for index in range(count):
        entry = 20+40*index
        name = raw[entry:entry+8].rstrip(b'\0').decode()
        size, data, reloc, _, nreloc = struct.unpack_from('<IIIIH', raw, entry+16)
        sections[name] = (raw[data:data+size],
            [struct.unpack_from('<IIH', raw, reloc+10*i) for i in range(nreloc)])
    strings = symbols+18*symbol_count
    names = {}
    i=0
    while i < symbol_count:
        entry=symbols+18*i
        name=raw[entry:entry+8]
        if name[:4] == b'\0'*4:
            offset=struct.unpack_from('<I',name,4)[0]
            end=raw.index(b'\0',strings+offset)
            name=raw[strings+offset:end]
        value, section, _, _, auxiliary = struct.unpack_from('<IhHBB',raw,entry+8)
        names[i]=(name.rstrip(b'\0').decode(),value,section)
        i += 1+auxiliary
    return sections,names

def instructions(raw, address=0):
    decoder=capstone.Cs(capstone.CS_ARCH_X86,capstone.CS_MODE_32)
    return [(i.address,i.mnemonic,i.op_str) for i in decoder.disasm(raw,address)]

def verify():
    obj=ROOT/'build-game/native-finally.o'
    sections,names=coff(obj)
    assert '.sxdata' not in sections  # GNU PE ld has no SafeSEH-index processing.
    text,relocations=sections['.text']
    table,table_relocations=sections['.xdata']
    assert len(table) == 12 and struct.unpack('<III',table) == (0xffffffff,0,0)
    assert len(table_relocations) == 1 and table_relocations[0][0] == 8
    handler=names[table_relocations[0][1]]
    assert handler[0].startswith('?dtor$') and handler[2] > 0
    code=instructions(text[:handler[1]])
    pairs=[(m,o) for _,m,o in code]
    assert pairs[:6] == [('push','ebp'),('mov','ebp, esp'),('push','ebx'),
                         ('push','edi'),('push','esi'),('sub','esp, 0x20')]
    assert ('mov','ecx, dword ptr [ebp + 0xc]') in pairs
    assert ('mov','eax, dword ptr [ebp + 0x10]') in pairs
    assert ('mov','dword ptr [ebp - 0x2c], eax') in pairs
    assert ('mov','dword ptr [ebp - 0x28], ecx') in pairs
    assert ('mov','esi, dword ptr fs:[0]') in pairs
    assert ('mov','dword ptr [ebp - 0x1c], esi') in pairs
    assert ('lea','edx, [ebp - 0x1c]') in pairs
    assert ('mov','dword ptr fs:[0], edx') in pairs
    body=pairs.index(('call','dword ptr [ebp + 8]'))
    assert pairs[body-1] == ('push','eax') and pairs[body+1] == ('add','esp, 4')
    cleanup=pairs.index(('call','eax'))
    assert pairs[cleanup-2:cleanup] == [('push','0'),('push','ecx')]
    assert pairs[cleanup+1] == ('add','esp, 8')
    unregistration=pairs.index(('mov','dword ptr fs:[0], eax'))
    assert body < cleanup < unregistration
    returned=pairs.index(('ret',''))
    assert all(m == 'nop' for m,_ in pairs[returned+1:])
    assert pairs[returned-4:returned+1] == [('pop','esi'),('pop','edi'),('pop','ebx'),('pop','ebp'),('ret','')]
    # Registration table and native CRT handler addresses are real relocations.
    targets=[names[symbol][0] for _,symbol,kind in relocations if kind == 6]
    assert targets == ['.xdata','__except_handler3']
    final=instructions(text[handler[1]:],handler[1])
    end=next(i for i,(_,m,_) in enumerate(final) if m == 'ret')
    assert all(byte == 0 for byte in text[final[end][0]+1:])
    assert [(m,o) for _,m,o in final[:end+1]] == [
        ('push','ebp'),('sub','esp, 8'),('add','ebp, 0xc'),('push','1'),
        ('push','dword ptr [ebp - 0x2c]'),('call','dword ptr [ebp - 0x28]'),
        ('add','esp, 0x10'),('pop','ebp'),('ret','')]
    products={}
    for name in ('d3d9.dll','SS2VRServer.dll'):
        path=ROOT/'build-game'/name
        pe=pefile.PE(str(path),max_symbol_exports=65536)
        assert pe.FILE_HEADER.Machine == 0x14c
        assert not (pe.OPTIONAL_HEADER.DllCharacteristics & 0x400) # NO_SEH forbidden.
        config=getattr(pe,'DIRECTORY_ENTRY_LOAD_CONFIG',None)
        assert not config or not getattr(config.struct,'SEHandlerCount',0)
        assert not any(s.Name.rstrip(b'\0') == b'.sxdata' for s in pe.sections)
        imports=[i.address for d in pe.DIRECTORY_ENTRY_IMPORT for i in d.imports
                 if i.name == b'_except_handler3']
        assert len(imports) == 1
        symbols=subprocess.run(['i686-w64-mingw32-nm','-an',str(path)],
                               check=True,capture_output=True,text=True).stdout
        match=re.search(r'^([0-9a-f]+) T _ss2vrNativeFinally$',symbols,re.MULTILINE)
        assert match
        address=int(match[1],16)
        rva=address-pe.OPTIONAL_HEADER.ImageBase
        linked=pe.get_data(rva,len(text))
        original=bytearray(text); normalized=bytearray(linked)
        for offset,_,kind in relocations:
            assert kind == 6
            original[offset:offset+4]=normalized[offset:offset+4]=b'\0'*4
        assert original == normalized
        scope_address=struct.unpack_from('<I',linked,relocations[0][0])[0]
        scope=pe.get_data(scope_address-pe.OPTIONAL_HEADER.ImageBase,12)
        assert struct.unpack('<III',scope) == (0xffffffff,0,address+handler[1])
        handler_address=struct.unpack_from('<I',linked,relocations[1][0])[0]
        thunk=pe.get_data(handler_address-pe.OPTIONAL_HEADER.ImageBase,6)
        assert thunk[:2] == b'\xff\x25' and struct.unpack_from('<I',thunk,2)[0] == imports[0]
        highlow={entry.rva for block in pe.DIRECTORY_ENTRY_BASERELOC for entry in block.entries
                 if entry.type == 3}
        assert pe.OPTIONAL_HEADER.DllCharacteristics & 0x40  # DYNAMIC_BASE.
        assert {rva+relocations[0][0], rva+relocations[1][0],
                scope_address-pe.OPTIONAL_HEADER.ImageBase+8,
                handler_address-pe.OPTIONAL_HEADER.ImageBase+2} <= highlow
        products[name]=hashlib.sha256(path.read_bytes()).hexdigest()
    fixture=ROOT/'build-game/native-finally-caller.o'
    subprocess.run(['i686-w64-mingw32-g++','-std=c++20','-O2','-Wall','-Wextra','-Werror',
        '-I'+str(ROOT/'src'),'-c',str(ROOT/'tests/native_finally_abi.cpp'),'-o',str(fixture)],check=True)
    dump=subprocess.run(['i686-w64-mingw32-objdump','-drC',str(fixture)],
                        check=True,capture_output=True,text=True).stdout
    for symbol in ('ss2vrNativeFinally','__cxa_begin_catch','__cxa_end_catch','consumeAligned'):
        assert symbol in dump,symbol
    assert 'rethrow_exception' not in dump
    callbacks=dump.split('>:\n')
    aligned=[part for part in callbacks if 'consumeAligned' in part]
    assert len(aligned) == 2
    for part in aligned:
        assert 'and' in part and '$0xfffffff0,%esp' in part
    # Manual source/ABI review still verifies callback storage and GNU catch
    # containment. Symbol presence is not an unwind or exception execution test.
    print(json.dumps({'accepted_compiled_shape':True,'runtime_executed':False,
        'native_finally_object_sha256':hashlib.sha256(obj.read_bytes()).hexdigest(),
        'callback_fixture_sha256':hashlib.sha256(fixture.read_bytes()).hexdigest(),
        'products':products,'checks':['x86 scope table/handler relocation',
        'FS registration and normal removal','stable callback/context slots',
        'normal cleanup0 and unwind cleanup1','cdecl argument cleanup',
        'legacy SEH image, linked scope/code/import identity and four HIGHLOW relocations','GNU callback compile-only catch containment and local alignment'],
        'limits':['no Windows/native exception execution','no GPU/owner integration',
        'no arbitrary crash recovery','fixture catch symbols/alignment are not native unwind execution']},indent=2))

if __name__ == '__main__': verify()
