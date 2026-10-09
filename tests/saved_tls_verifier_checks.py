"""Offline counterexamples for the bounded saved-capture proof."""
from pathlib import Path
import sys,unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from verify_saved_tls import verify_saved_tls,State,pointer,SAVED,TEB,INDEX,UNKNOWN

def assembly(rows):
    result=[]
    for i,row in enumerate(rows):
        op,*relocations=row if isinstance(row,tuple) else (row,)
        result.append(f'{i*16:x}:\t90\t{op}\n')
        result.extend(f' {i*16+1:x}: {relocation}\n' for relocation in relocations)
    return ''.join(result)

ENTRY=assembly(['sub esp,0x40','mov ecx,DWORD PTR fs:0x2c',
    ('mov eax,ds:0x0','dir32\t_tls_index'),'mov eax,DWORD PTR [ecx+eax*4]',
    ('movzx eax,BYTE PTR [eax+0x5]','secrel32\t.tls$'),'mov BYTE PTR [esp+0xf],al',
    'lea eax,[esp+0xf]','mov DWORD PTR [esp+0x18],eax','lea eax,[esp+0x18]',
    'mov DWORD PTR [esp+0x30],eax','lea eax,[esp+0x2c]','mov DWORD PTR [esp+0x8],eax',
    ('call 0','DISP32\tss2vrNativeFinally')])

def finish(duplicated):
    prefix=['mov ebx,DWORD PTR [esp+0x4]','mov ebx,DWORD PTR [ebx+0x4]']
    load=['mov eax,DWORD PTR [ebx]','movzx edx,BYTE PTR [eax]']
    tls=['mov ecx,DWORD PTR fs:0x2c',('mov eax,ds:0x0','dir32\t_tls_index'),
         'mov eax,DWORD PTR [ecx+eax*4]',('mov BYTE PTR [eax+0x5],dl','secrel32\t.tls$')]
    prefix+=[] if duplicated else load
    normal=(load if duplicated else [])+tls+['ret']
    branch_index=len(prefix)+1;target=(branch_index+1+len(normal))*16
    return assembly(prefix+['test edi,edi',f'jne {target:x}']+normal+
                    (load if duplicated else [])+tls+[('jmp 0','DISP32\tss2vr::game::nativeUiFault(char const*)')])

class Checks(unittest.TestCase):
    def test_global_owner_cas_forgets_eax_and_rejects_tls_or_unbound_storage(self):
        state=State({'eax':pointer('tls',0),'edx':SAVED})
        state.step('lock cmpxchg DWORD PTR ds:0xc,edx',['100: dir32\t.bss'],5)
        self.assertEqual(state.register('eax'),UNKNOWN)
        for op,relocs in (
            ('lock cmpxchg DWORD PTR [eax+0x5],edx',['100: secrel32\t.tls$']),
            ('lock cmpxchg DWORD PTR ds:0xc,edx',[]),
            ('lock cmpxchg DWORD PTR ds:0xc,edx',['100: dir32\t.data']),
        ):
            with self.subTest(op=op,relocs=relocs),self.assertRaises(ValueError):
                state.step(op,relocs,5)
    def test_overlapping_store_truncation_displaced_index_and_prefixed_lea_fail_closed(self):
        state=State({'eax':pointer('tls',0),'dl':SAVED})
        state.step('mov BYTE PTR [eax+0x5],dl',['secrel32\t.tls$'],5)
        with self.assertRaises(ValueError):state.step('mov DWORD PTR [eax+0x4],0x0',[],5)
        state=State({'eax':pointer('tls',0),'dl':SAVED})
        state.set_register('eax',pointer('tls',0));state.step('movzx eax,al',[],5)
        with self.assertRaises(ValueError):state.step('mov BYTE PTR [eax+0x5],dl',['secrel32\t.tls$'],5)
        state=State({'ecx':TEB,'eax':INDEX,'dl':SAVED})
        state.step('mov eax,DWORD PTR [ecx+eax*4+0x4]',[],5)
        with self.assertRaises(ValueError):state.step('mov BYTE PTR [eax+0x5],dl',['secrel32\t.tls$'],5)
        with self.assertRaises(ValueError):state.step('cs lea eax,[eax+0x1]',[],5)
        state=State({'eax':pointer('tls',0),'dl':SAVED})
        state.step('movzx edx,BYTE PTR [eax+0x5]',['secrel32\t.tls$'],5)
        with self.assertRaises(ValueError):state.step('mov BYTE PTR [eax+0x5],dl',['secrel32\t.tls$'],5)
    def test_shared_and_duplicated_saved_loads_cover_both_exits(self):
        for duplicated in (False,True):
            with self.subTest(duplicated=duplicated):
                self.assertEqual(verify_saved_tls(ENTRY,finish(duplicated),5)['reachable_exits'],2)
    def test_constant_trace_wrong_capture_current_tls_and_clobber_are_not_saved_origin(self):
        body=finish(True)
        for mutation in ('mov edx,0x0','movzx edx,BYTE PTR [ebx]',
                         'mov eax,DWORD PTR [ebx+0x4]',
                         'movzx edx,BYTE PTR [eax+0x5]'):
            with self.subTest(mutation=mutation),self.assertRaises(ValueError):
                verify_saved_tls(ENTRY,body.replace('movzx edx,BYTE PTR [eax]',mutation,1),5)
        with self.assertRaises(ValueError):
            verify_saved_tls(ENTRY,body.replace('mov ecx,DWORD PTR fs:0x2c','xor edx,edx',1),5)
        with self.assertRaises(ValueError):
            verify_saved_tls(ENTRY,body.replace('mov BYTE PTR [eax+0x5],dl','mov BYTE PTR [eax+0x6],dl',1),5)
    def test_bypass_unsupported_edges_and_entry_origin_changes_fail_closed(self):
        body=finish(False)
        for mutation in (body.replace('mov BYTE PTR [eax+0x5],dl','nop',1),
                         body.replace('jne ', 'jmp dead',1),
                         body.replace('test edi,edi','call eax'),
                         body.replace('mov eax,DWORD PTR [ebx]','mov DWORD PTR [ebx],0x0',1)):
            with self.subTest(mutation=mutation),self.assertRaises(ValueError):verify_saved_tls(ENTRY,mutation,5)
        with self.assertRaises(ValueError):
            verify_saved_tls(ENTRY.replace('movzx eax,BYTE PTR [eax+0x5]','mov eax,0x1'),body,5)

if __name__=='__main__':unittest.main()
