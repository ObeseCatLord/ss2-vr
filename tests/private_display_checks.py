"""Offline prerequisite/ownership regressions. No display/Wine/game is launched."""
import hashlib,json,subprocess,sys,tempfile,unittest
from pathlib import Path
from unittest.mock import patch
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
import runtime_lab as lab
import private_display_lab as display
GOOD={'schema':1,'stage':'swapchain-query','desktop_hz':60,'adapter_hz':60,'swapchain_hz':60,
      'width':1280,'height':720,'raster_called':True,'hresult':0,'scanline':0,'in_vblank':True}
class Checks(unittest.TestCase):
    def test_zero_current_mode_failure_or_forged_success_is_not_readiness(self):
        lab.require_display_timing(GOOD)
        for key,value in [('schema',True),('stage','invalid-current-mode'),('raster_called',1),
                          ('hresult',False),('hresult',2289436780),('swapchain_hz',0),
                          ('adapter_hz',1),('desktop_hz',0),('width',False),('height',16385)]:
            with self.subTest(key=key),self.assertRaises(RuntimeError):lab.require_display_timing(GOOD|{key:value})
    def test_private_display_environment_removes_desktop_routes_and_ownership_is_exact(self):
        original={'DISPLAY':':1','WAYLAND_DISPLAY':'wayland-0','XAUTHORITY':'desktop',
                  'DBUS_SESSION_BUS_ADDRESS':'desktop','MIR_SOCKET':'desktop','OTHER':'keep'}
        env=display.private_environment(original,Path('/private/runtime'),'token')
        self.assertEqual(original['DISPLAY'],':1');self.assertEqual(env['OTHER'],'keep')
        for key in original:
            if key!='OTHER':self.assertNotIn(key,env)
        raw=b'SS2VR_PRIVATE_CAPTURE_TOKEN=token\0SS2VR_LAB_DISPLAY_ROOT=/private/runtime\0'
        self.assertTrue(display.tagged(raw,'token','/private/runtime'))
        for value in (raw.replace(b'token',b'other'),raw.replace(b'/private/runtime',b'/run/user'),
                      raw+b'SS2VR_PRIVATE_CAPTURE_TOKEN=token\0',b''):
            self.assertFalse(display.tagged(value,'token','/private/runtime'))
    def test_pinned_probe_requires_private_x86_executable(self):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);exe=root/'probe.exe';data=bytearray(70);data[:2]=b'MZ';data[60:64]=(64).to_bytes(4,'little');data[64:70]=b'PE\0\0L\x01';exe.write_bytes(data)
            cfg={'display_timing_probe':{'path':str(exe),'sha256':hashlib.sha256(data).hexdigest()}}
            lab.validate_display_probe(cfg,root)
            data[68:70]=b'd\x86';exe.write_bytes(data);cfg['display_timing_probe']['sha256']=hashlib.sha256(data).hexdigest()
            with self.assertRaises(ValueError):lab.validate_display_probe(cfg,root)
            with self.assertRaises(ValueError):lab.validate_display_probe({'display_timing_probe':True},root)
    def test_failed_or_timed_out_probe_prevents_progress_and_retires_exact_child(self):
        for fault in ('zero-mode','timeout','bad-status','duplicate-json'):
            with self.subTest(fault=fault),tempfile.TemporaryDirectory() as d:
                root=Path(d);exe=root/'probe.exe';exe.write_bytes(b'pinned fixture');calls=[]
                cfg={'proton':'not-launched','display_timing_probe':{'path':str(exe),'sha256':lab.digest(exe)}}
                class Process:
                    code=None
                    def poll(self):return self.code
                    def wait(self,timeout):
                        if fault=='timeout' and self.code is None:raise subprocess.TimeoutExpired('mock',timeout)
                        self.code=0 if fault!='bad-status' else 5;return self.code
                    def terminate(self):calls.append('term');self.code=-15
                    def kill(self):calls.append('kill');self.code=-9
                process=Process()
                def popen(args,**kw):
                    report=GOOD|({'swapchain_hz':0} if fault=='zero-mode' else {})
                    text=json.dumps(report)+'\n'
                    if fault=='duplicate-json':text*=2
                    kw['stdout'].write(text.encode());kw['stdout'].flush();return process
                discovers=[[],[{'pid':123}]]
                with patch.object(lab.subprocess,'Popen',side_effect=popen),patch.object(lab,'private_processes',side_effect=discovers),patch.object(lab,'stop_exact',side_effect=lambda item,budget:calls.append(item['pid'])):
                    with self.assertRaises((RuntimeError,subprocess.TimeoutExpired)):
                        lab.display_timing_probe(cfg,{'SS2VR_LAB_PRIVATE_DISPLAY':'1','DISPLAY':':2'},lab.time.monotonic()+40,root)
                self.assertIn(123,calls);self.assertFalse((root/'display-timing.json').exists())
                if fault=='timeout':self.assertIn('term',calls)
    def test_desktop_selector_or_wayland_never_reaches_proton(self):
        with patch.object(lab.subprocess,'Popen') as process:
            for env in ({'DISPLAY':':1'},{'SS2VR_LAB_PRIVATE_DISPLAY':'1','DISPLAY':':2','WAYLAND_DISPLAY':'wayland-0'}):
                with self.assertRaises(RuntimeError):lab.display_timing_probe({'display_timing_probe':{}},env,0,Path('.'))
            process.assert_not_called()
    def test_weston_exit_without_collector_receipt_is_not_success(self):
        self.wrapper_mock(False,False)
    def test_wrapper_preexec_certificate_and_cleanup_survive_failure(self):
        self.wrapper_mock(True,True)
    def test_late_cancellation_is_interrupted_after_complete_cleanup(self):
        self.wrapper_mock(True,False,True)
    def wrapper_mock(self, completed, fail_cleanup, late=False):
        with tempfile.TemporaryDirectory() as d:
            root=Path(d);cfg={'display_timing_probe':True,'expected_product_source':'a'*64}
            (root/'config.json').write_text(json.dumps(cfg));command=[];calls=[];live={101,102};identified=[0]
            class Process:
                pid=100;returncode=0
                def poll(self):return 0
            def popen(cmd,**kw):
                command[:]=cmd
                self.assertNotIn('DISPLAY',kw['env']);self.assertNotIn('WAYLAND_DISPLAY',kw['env'])
                self.assertEqual(kw['env']['XDG_RUNTIME_DIR'],str(root/'private-display/runtime'))
                if completed:(root/'private-display/collection-outcome.json').write_text(json.dumps({'completed':True,'data_ready_for_review':True}))
                return Process()
            def identify(pid):
                identified[0]+=1
                return {'pid':100,'start':'cookie','argv':[b'python-preexec'] if identified[0]==1 else [part.encode() for part in command]}
            def gather(owned,*args):
                for pid in live:owned[(pid,'cookie')]={'pid':pid,'start':'cookie','argv':[b'owned-helper']}
            faults=[fail_cleanup];handlers={};cancel=[late]
            def signals(sig,handler):
                old=handlers.get(sig);handlers[sig]=handler;return old
            def stop(item,deadline):
                calls.append(item['pid'])
                if cancel[0]:
                    cancel[0]=False;handlers[display.signal.SIGTERM](display.signal.SIGTERM,None)
                if faults[0]:faults[0]=False;raise RuntimeError('injected child cleanup fault')
                live.discard(item['pid'])
            with patch.object(display.subprocess,'Popen',side_effect=popen),patch.object(display,'identity',side_effect=identify),patch.object(display,'gather',side_effect=gather),patch.object(display,'still_owned',side_effect=lambda item:item['pid'] in live),patch.object(display,'stop_exact',side_effect=stop),patch.object(display.signal,'signal',side_effect=signals),patch.object(display.time,'sleep'):
                with self.assertRaises((FileNotFoundError,RuntimeError)):
                    display.launch(root/'config.json',cfg)
            report=json.loads((root/'private-display/result.json').read_text())
            self.assertEqual(report['survivors'],[]);self.assertTrue(report['display_stopped'])
            self.assertTrue((root/'agent-capture-attempt.json').exists())
            self.assertEqual(live,set());self.assertIn(102,calls)
            if fail_cleanup:self.assertTrue(report['cleanup_errors'])
            elif late:
                self.assertTrue(report['deferred_signals']);self.assertEqual(report['cleanup_errors'],[])
            else:self.assertIn('error',report)
            certificate=json.loads((root/'private-display/owner.json').read_text())
            self.assertEqual(certificate['argv'],command)
            with self.assertRaises(FileExistsError):display.launch(root/'config.json',cfg)
if __name__=='__main__':unittest.main()
