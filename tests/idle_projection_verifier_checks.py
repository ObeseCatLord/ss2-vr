"""Instruction-level negative controls for observer-window verification."""
from pathlib import Path
import struct
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from verify_idle_projection_probe_abi import decode,verify_sample,sample_prefix,instructions,verify_producer_interval

class Checks(unittest.TestCase):
    def test_raw_source2_cannot_bypass_owner_or_promote_raster(self):
        from verify_idle_projection_probe_abi import verify_raw_diagnostic_source
        root=Path(__file__).resolve().parents[1]
        engine=(root/'src/game/engine.cpp').read_text();remote=(root/'src/game/remote_render.cpp').read_text()
        verify_raw_diagnostic_source(engine,remote)
        for token in ('!trace->admitted || trace->nativeId!=2',
                      'same==trace && identity==trace->binding',
                      'weaponPairFault || invocation->pass.failed || invocation->pass.stage!=4',
                      'trace->nativeId!=2 || p.source==1',
                      'break; // Only the latest corresponding producer'):
            with self.subTest(token=token),self.assertRaises(ValueError):
                verify_raw_diagnostic_source(engine.replace(token,'true'),remote)
        for token in ('if(raw && (!owner || owner->nativeId==2))owner=idleRawProjectionOwner();',
                      'raw && owner->nativeId==2?idleRawProjectionOwner():idleProjectionOwner();'):
            with self.subTest(token=token),self.assertRaises(ValueError):
                verify_raw_diagnostic_source(engine,remote.replace(token,'idleProjectionOwner();'))

    def test_native_bookend_interval_rejects_calls_returns_control_changes_and_escape(self):
        self.assertEqual(verify_producer_interval(decode(bytes.fromhex('90 d8 c1'),0x1000),0x1000,0x1003),2)
        for raw in ('ff d0','c3','d9 2c 24','db e3','0f ae 14 24','eb 7f'):
            code=bytes.fromhex(raw)
            with self.subTest(raw=raw),self.assertRaises(ValueError):
                verify_producer_interval(decode(code,0x1000),0x1000,0x1000+len(code))
    def test_shared_collector_opt_in_contract(self):
        from verify_idle_collector_abi import verify_opt_in_source,ROOT
        remote=(ROOT/'src/game/remote_render.cpp').read_text()
        engine=(ROOT/'src/game/engine.cpp').read_text()
        policy=(ROOT/'src/common/idle_probe_selection.hpp').read_text()
        verify_opt_in_source(remote,engine,policy)
        controls=[
            (remote.replace('if(idleProbeWeaponSupported(selectedIdleProbeWeapon()))','if(true)'),engine,policy),
            (remote.replace('if(idleProbeWeaponSupported(selectedIdleProbeWeapon()))','if(idleProbeWeaponSupported(selectedIdleProbeWeapon()));'),engine,policy),
            (remote.replace('install(engine,0xbbf0','install(engine,0xbbf1'),engine,policy),
            (remote,engine.replace('n<3?idleProbeWeaponId','n<4?idleProbeWeaponId'),policy),
            (remote,engine.replace('SS2VR_LAB_IDLE_WEAPON','SS2VR_UNGUARDED'),policy),
            (remote,engine,policy.replace('13:-1','13:1')),
            (remote,engine,policy.replace('id==1 || id==2 || id==13','id==1 || id==2 || id==13 || id==3'))]
        for args in controls:
            with self.subTest(control=args!= (remote,engine,policy)),self.assertRaises(ValueError):
                verify_opt_in_source(*args)

    def test_pure_word_sampler(self):
        result=verify_sample(decode(bytes.fromhex('d9 3c 24 8b 01 c3'),0x1000))
        self.assertEqual(result['calls'],0);self.assertEqual(result['control_reads'],1)
    def test_fp_changes_calls_traps_and_resampling_fail(self):
        prefix=bytes.fromhex('d9 3c 24')
        for extra in ('d9 2c 24','d8 c1','0f 58 c1','ff d0','cc','0f 0b','d9 3c 24','75 fb'):
            with self.subTest(extra=extra),self.assertRaises(ValueError):
                verify_sample(decode(prefix+bytes.fromhex(extra)+b'\xc3',0x1000))
        with self.assertRaises(ValueError):verify_sample(decode(bytes.fromhex('90 c3'),0x1000))
        with self.assertRaises(ValueError):verify_sample(decode(prefix+bytes.fromhex('c2 04 00'),0x1000))
        with self.assertRaises(ValueError):verify_sample(decode(prefix+b'\xe9'+struct.pack('<i',0x2000-0x1008),0x1000))
    def test_unselected_callback_path_is_separate(self):
        # Selected path jumps over an unrelated callback and reaches the sample.
        code=bytes.fromhex('83 f8 00 74 03 ff d0 c3')+b'\xe8'+struct.pack('<i',0x2000-0x100d)+b'\xc3'
        self.assertGreater(sample_prefix(decode(code,0x1000),0x2000),0)
        # Replace the unrelated return: it now falls through to the sample, so
        # the callback would occur in an observed prefix and must be rejected.
        bad=code[:7]+b'\x90'+code[8:]
        with self.assertRaises(ValueError):sample_prefix(decode(bad,0x1000),0x2000)
    def test_parser_rejects_wrapped_or_gapped_instructions(self):
        wrapped='''1000: d9 3c 24       fnstcw WORD PTR [esp]
1003: f3 0f 10 84 24 00 00   movss xmm0,DWORD PTR [esp]
100a: 00 00
100c: c3             ret
'''
        with self.assertRaises(ValueError):instructions(wrapped)
        full=wrapped.replace('24 00 00   movss','24 00 00 00 00   movss').replace('100a: 00 00\n','')
        with self.assertRaises(ValueError):verify_sample(instructions(full))
        with self.assertRaises(ValueError):instructions('1000: d9 3c 24 fnstcw WORD PTR [esp]\n1004: c3 ret\n')
    def test_unreachable_control_read_and_loop_escape_reject(self):
        with self.assertRaises(ValueError):verify_sample(decode(bytes.fromhex('c3 d9 3c 24 c3'),0x1000))
        with self.assertRaises(ValueError):verify_sample(decode(bytes.fromhex('d9 3c 24 e2 7f c3'),0x1000))
        # LOOP can also expose a callback path in the fog prefix.
        code=bytes.fromhex('e2 02 ff d0')+b'\xe8'+struct.pack('<i',0x2000-0x1009)+b'\xc3'
        with self.assertRaises(ValueError):sample_prefix(decode(code,0x1000),0x2000)
    def test_final_jump_cannot_hide_callback_to_sample(self):
        # JE selects a callback at the end, whose final JMP returns to SAMPLE.
        # The other path samples then returns normally.
        code=bytes.fromhex('83 f8 00 74 06')+b'\xe8'+struct.pack('<i',0x2000-0x100a)+b'\xc3\xff\xd0'+b'\xe9'+struct.pack('<i',0x1005-0x1012)
        with self.assertRaises(ValueError):sample_prefix(decode(code,0x1000),0x2000)
        safe=bytes.fromhex('83 f8 00 74 03 ff d0 c3')+b'\xe8'+struct.pack('<i',0x2000-0x100d)+b'\xc3\x90\x90'
        self.assertGreater(sample_prefix(decode(safe,0x1000),0x2000),0)

if __name__=='__main__':unittest.main()
