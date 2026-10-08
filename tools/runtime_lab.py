#!/usr/bin/env python3
"""Run a verified direct-to-scene SS2 test in an existing private Proton lab.

Configuration, owned game assets, prefixes, logs and images stay outside source.
This launcher never installs, changes global runtimes, joins multiplayer or clicks menus.
"""
from assess_runtime import (complete_pairs_for_pose,dual_topologies,successful_dual_fire,
    dual_identity,DualPhaseEvidence,require_dual_capture,dual_weapon_events)

import argparse
import ctypes
import hashlib
import json
import math
import os
from pathlib import Path
import pty
import re
import signal
import subprocess
import sys
import time
import zipfile
import uuid

ROOT = Path(__file__).resolve().parents[1]

def digest(path):
    with Path(path).open('rb') as f:
        return hashlib.file_digest(f, 'sha256').hexdigest()

def checked_file(path, expected):
    path = Path(path).resolve(strict=True)
    if not path.is_file() or digest(path) != expected:
        raise ValueError('Configured file fingerprint differs: ' + path.name)
    return path

def identity(pid):
    p = Path('/proc') / str(pid)
    try:
        stat = (p / 'stat').read_text().rsplit(')', 1)[1].split()
        return {'pid': int(pid), 'start': stat[19], 'parent': int(stat[1]),
                'name': (p / 'comm').read_text().strip(),
                'argv': (p / 'cmdline').read_bytes().split(b'\0')}
    except (OSError, IndexError):
        return None

def running(name):
    return [i for p in Path('/proc').iterdir() if p.name.isdigit()
            if (i := identity(p.name)) and i['name'] == name]

def still_owned(item):
    now = identity(item['pid'])
    return now is not None and now['start'] == item['start'] and now['argv'] == item['argv']

def stop_exact(item, budget=None):
    if not still_owned(item):
        return
    try: os.kill(item['pid'], signal.SIGTERM)
    except ProcessLookupError: return
    deadline = min(time.monotonic()+5,budget) if budget is not None else time.monotonic()+5
    while still_owned(item) and time.monotonic() < deadline:
        time.sleep(.1)
    if still_owned(item):
        try: os.kill(item['pid'], signal.SIGKILL)
        except ProcessLookupError: return
        deadline=min(time.monotonic()+2,budget) if budget is not None else time.monotonic()+2
        while still_owned(item) and time.monotonic()<deadline: time.sleep(.05)
        if still_owned(item): raise RuntimeError('Exact owned process did not terminate')

def remaining(deadline, limit=20):
    seconds=deadline-time.monotonic()
    if seconds<=0: raise TimeoutError('Absolute runtime deadline reached')
    return min(seconds,limit)

def private_processes(executable, token=None):
    expected=str(Path(executable).resolve()).replace('\\','/').casefold()
    found=[]
    for p in Path('/proc').iterdir():
        if not p.name.isdigit(): continue
        item=identity(p.name)
        if not item: continue
        original=[v.decode(errors='replace') for v in item['argv'][:16]]
        args=[v.replace('\\','/').casefold() for v in original]
        if any(v.removeprefix('z:')==expected for v in args) and (token is None or token in original):
            found.append(item)
    return found

def game_owned(item, lab):
    argv = b' '.join(item['argv']).decode(errors='replace').replace('\\', '/').casefold()
    return str(lab / 'Bin/Sam2.exe').casefold() in argv

def window_for(pid):
    x = ctypes.CDLL('libX11.so.6'); D = ctypes.c_void_p; U = ctypes.c_ulong
    x.XOpenDisplay.restype = D
    x.XDefaultRootWindow.argtypes = [D]; x.XDefaultRootWindow.restype = U
    x.XQueryTree.argtypes = [D,U,ctypes.POINTER(U),ctypes.POINTER(U),ctypes.POINTER(ctypes.POINTER(U)),ctypes.POINTER(ctypes.c_uint)]
    x.XFetchName.argtypes = [D,U,ctypes.POINTER(ctypes.c_char_p)]
    x.XFree.argtypes = [D]; x.XCloseDisplay.argtypes = [D]
    x.XInternAtom.argtypes = [D,ctypes.c_char_p,ctypes.c_int]; x.XInternAtom.restype = U
    x.XGetWindowProperty.argtypes = [D,U,U,ctypes.c_long,ctypes.c_long,ctypes.c_int,U,ctypes.POINTER(U),ctypes.POINTER(ctypes.c_int),ctypes.POINTER(U),ctypes.POINTER(U),ctypes.POINTER(ctypes.POINTER(ctypes.c_ubyte))]
    x.XSetInputFocus.argtypes = [D,U,ctypes.c_int,U]; x.XRaiseWindow.argtypes = [D,U]
    x.XSync.argtypes = [D,ctypes.c_int]
    class Attributes(ctypes.Structure):
        _fields_=[(name,ctypes.c_int) for name in ('x','y','width','height','border_width','depth')]+[
            ('visual',D),('root',U),('c_class',ctypes.c_int),('bit_gravity',ctypes.c_int),
            ('win_gravity',ctypes.c_int),('backing_store',ctypes.c_int),('backing_planes',U),
            ('backing_pixel',U),('save_under',ctypes.c_int),('colormap',U),('map_installed',ctypes.c_int),
            ('map_state',ctypes.c_int),('all_event_masks',ctypes.c_long),('your_event_mask',ctypes.c_long),
            ('do_not_propagate_mask',ctypes.c_long),('override_redirect',ctypes.c_int),('screen',D)]
    class XError(ctypes.Structure):
        _fields_=[('type',ctypes.c_int),('display',D),('resourceid',U),('serial',U),
                  ('error_code',ctypes.c_ubyte),('request_code',ctypes.c_ubyte),('minor_code',ctypes.c_ubyte)]
    x.XGetWindowAttributes.argtypes=[D,U,ctypes.POINTER(Attributes)]
    x.XSetErrorHandler.argtypes=[D];x.XSetErrorHandler.restype=D
    errors=[]
    @ctypes.CFUNCTYPE(ctypes.c_int,D,ctypes.POINTER(XError))
    def on_error(display,error):
        if len(errors)<16:errors.append((error.contents.error_code,error.contents.request_code))
        return 0
    class ClientMessage(ctypes.Structure):
        _fields_=[('type',ctypes.c_int),('serial',U),('send_event',ctypes.c_int),('display',D),
                  ('window',U),('message_type',U),('format',ctypes.c_int),('data',ctypes.c_long*5)]
    class Event(ctypes.Union):
        _fields_=[('client',ClientMessage),('pad',ctypes.c_long*24)]
    x.XSendEvent.argtypes=[D,U,ctypes.c_int,ctypes.c_long,ctypes.POINTER(Event)]
    d = x.XOpenDisplay(None)
    if not d: raise RuntimeError('No native desktop display')
    previous=x.XSetErrorHandler(ctypes.cast(on_error,D))
    try:
        root = x.XDefaultRootWindow(d); atom = x.XInternAtom(d,b'_NET_WM_PID',0)
        def walk(w):
            a=U(); b=U(); children=ctypes.POINTER(U)(); n=ctypes.c_uint()
            if not x.XQueryTree(d,w,ctypes.byref(a),ctypes.byref(b),ctypes.byref(children),ctypes.byref(n)): return
            nodes=[children[i] for i in range(n.value)]
            if children: x.XFree(children)
            for c in nodes:
                yield c
                yield from walk(c)
        found=[]
        for w in walk(root):
            a=U(); fmt=ctypes.c_int(); n=U(); after=U(); p=ctypes.POINTER(ctypes.c_ubyte)()
            x.XGetWindowProperty(d,w,atom,0,1,0,6,ctypes.byref(a),ctypes.byref(fmt),ctypes.byref(n),ctypes.byref(after),ctypes.byref(p))
            owner=ctypes.cast(p,ctypes.POINTER(U))[0] if p and n.value and fmt.value==32 else 0
            if p: x.XFree(p)
            if owner!=pid: continue
            title=ctypes.c_char_p()
            if x.XFetchName(d,w,ctypes.byref(title)) and title.value:
                attrs=Attributes()
                if (title.value==b'Serious Sam 2' and x.XGetWindowAttributes(d,w,ctypes.byref(attrs)) and
                    attrs.map_state==2 and attrs.c_class==1 and attrs.width>0 and attrs.height>0):found.append(w)
                x.XFree(title)
        if len(found)!=1: return None
        x.XSync(d,0);errors.clear()
        attrs=Attributes()
        if not x.XGetWindowAttributes(d,found[0],ctypes.byref(attrs)) or attrs.map_state!=2:return None
        event=Event();event.client.type=33;event.client.display=d;event.client.window=found[0]
        event.client.message_type=x.XInternAtom(d,b'_NET_ACTIVE_WINDOW',0)
        event.client.format=32;event.client.data[0]=2
        x.XSendEvent(d,root,0,(1<<20)|(1<<19),ctypes.byref(event))
        x.XRaiseWindow(d,found[0]);x.XSetInputFocus(d,found[0],2,0);x.XSync(d,0)
        # Mapping can change between inspection and focus. Reject that attempt;
        # the existing bounded launcher waits, without mapping arbitrary windows.
        if errors:
            if any(code not in (3,8) for code,request in errors):raise RuntimeError('Owned-window X operation failed')
            return None
        return found[0]
    finally:
        x.XSync(d,0);x.XSetErrorHandler(previous);x.XCloseDisplay(d)

class ObserverError(RuntimeError):
    def __init__(self, command, code):
        self.code=code
        super().__init__('IPC observer '+command+' returned '+str(code))

def observer(cfg, env, command, token, output=None, timeout=20, deadline=None):
    args=[cfg['proton'], 'runinprefix', cfg['observer'], command, token]
    status_file=None
    if command in ('status','stock-status'):
        status_file=Path(cfg['observer_output_dir'])/('status-'+uuid.uuid4().hex+'.json')
        args.append('Z:'+str(status_file))
        if command=='status':args.append('Z:'+str(Path(cfg['game_lab'])/'Bin/Sam2.exe'))
    if output is not None:
        args.extend(map(str,output))
    before={i['pid'] for i in private_processes(cfg['observer'],token)}
    if deadline is None: deadline=time.monotonic()+timeout+4
    timeout=remaining(deadline,timeout)
    process=subprocess.Popen(args,env=env,stdout=subprocess.PIPE,stderr=subprocess.PIPE,start_new_session=True)
    try:
        stdout,stderr=process.communicate(timeout=timeout)
    except subprocess.TimeoutExpired:
        try:
            if process.poll() is None: process.terminate()
            try: process.communicate(timeout=remaining(deadline,1))
            except (subprocess.TimeoutExpired,TimeoutError):
                if process.poll() is None: process.kill()
                try: process.communicate(timeout=remaining(deadline,1))
                except (subprocess.TimeoutExpired,TimeoutError): pass
        finally:
            for item in private_processes(cfg['observer'],token):
                if item['pid'] not in before: stop_exact(item,deadline)
        raise
    if process.returncode:
        raise ObserverError(command,process.returncode)
    if command in ('status','stock-status'):
        if not status_file.is_file():
            (Path(cfg['observer_output_dir'])/'observer-output-diagnostic.txt').write_bytes(stdout+b'\n'+stderr)
            raise RuntimeError('Observer returned success without its private status record')
        return json.loads(status_file.read_text())
    return None

def validate(cfg):
    private=Path(cfg['private_root']).resolve(strict=True)
    if private.is_relative_to(ROOT) or ROOT.is_relative_to(private):
        raise ValueError('Runtime data must be outside the source tree')
    lab=Path(cfg['game_lab']).resolve(strict=True)
    prefix=Path(cfg['prefix']).resolve(strict=True)
    for p in (lab,prefix):
        if not p.is_relative_to(private): raise ValueError('Lab and prefix must be private owned paths')
    mode=cfg.get('renderer_mode','vr')
    if mode not in ('vr','stock'):raise ValueError('Unknown renderer comparison mode')
    from build_contract import read_contract
    contracts={role:read_contract(lab/relative,role) for role,relative in
        {'game':'Bin/d3d9.dll','server':'Bin/SS2VRServer.dll','host':'Bin/SS2VR/ss2vr_host.exe'}.items()}
    reference={k:v for k,v in contracts['game'].items() if k!='component'}
    if any({k:v for k,v in value.items() if k!='component'}!=reference for value in contracts.values()):
        raise ValueError('Installed product contracts disagree')
    cfg['compiled_product_contract']=reference
    expected=cfg.get('expected_product_source')
    if mode=='stock' or expected is not None:
        if not isinstance(expected,str) or not re.fullmatch(r'[a-f0-9]{64}',expected):
            raise ValueError('Comparator requires an exact compiled source identity')
        if reference['source_fingerprint']!=expected:
            raise ValueError('Installed products do not match the pinned source')
    marker=json.loads((lab/'.ss2vr-runtime-lab.json').read_text())
    checked_file(lab/'Bin/Sam2.exe',marker['game_sha256'])
    receipt=json.loads((lab/'Bin/SS2VR/install-receipt.json').read_text())
    for name, expected in receipt['files'].items(): checked_file(lab/name,expected)
    startup=cfg['startup']
    expected_arguments=['+mod','SeriousSam2','+sam_bBootSequence','0','+sam_bSkipMovies','1','+level',cfg['scene']['entry']]
    zero_mouse_arguments=expected_arguments[:6]+['+inp_fMouseSensitivity','0']+expected_arguments[6:]
    if startup['arguments'] not in (expected_arguments,zero_mouse_arguments):
        raise ValueError('Only the verified stock local +level startup is admitted')
    if not startup['evidence'] or not startup['arguments']:
        raise ValueError('Verified direct-start evidence and arguments are required')
    for item in startup['evidence']:
        path=checked_file(lab/item['relative'],item['sha256'])
        if not path.is_relative_to(lab): raise ValueError('Startup evidence escapes lab')
    scene=cfg['scene']; archive=checked_file(lab/scene['archive'],scene['archive_sha256'])
    if not archive.is_relative_to(lab): raise ValueError('Scene archive escapes lab')
    with zipfile.ZipFile(archive) as z:
        if hashlib.sha256(z.read(scene['entry'])).hexdigest()!=scene['entry_sha256']:
            raise ValueError('Owned scene bytes differ')
    # Record providers without guessing mount precedence. The successful native
    # stream receipt must later certify the configured exact scene bytes.
    providers=[]
    for candidate in lab.rglob('*'):
        if not candidate.is_file() or candidate.suffix.casefold()!='.gro': continue
        if not candidate.resolve().is_relative_to(lab): raise ValueError('External archive provider')
        with zipfile.ZipFile(candidate) as z:
            for name in z.namelist():
                if name.replace('\\','/').casefold()==scene['entry'].casefold():
                    data=z.read(name)
                    providers.append({'archive':str(candidate.relative_to(lab)),
                        'entry_sha256':hashlib.sha256(data).hexdigest(),'bytes':len(data)})
    if not providers: raise ValueError('No owned scene provider')
    if any(str(p.relative_to(lab)).casefold()==scene['entry'].casefold() for p in lab.rglob('*') if p.is_file()):
        raise ValueError('Loose scene override is not admitted')
    cfg['verified_scene_providers']=providers
    for name in ('proton','monado_service','native_manifest','observer','pose_driver'):
        checked_file(cfg[name],cfg['fingerprints'][name])
    if not 1 <= cfg['remote_port'] <= 65535 or cfg['monado_config'] != {'active':'remote','remote':{'version':0,'port':cfg['remote_port'],'view_count':2}}:
        raise ValueError('Exact private remote configuration required')
    allowed_environment={'P_OVERRIDE_ACTIVE_CONFIG','SDL_VIDEODRIVER','XRT_COMPOSITOR_FORCE_XCB',
        'XRT_COMPOSITOR_COMPUTE','U_PACING_APP_USE_MIN_FRAME_PERIOD','XRT_DEBUG_GUI','XRT_CURATED_GUI'}
    if set(cfg['monado_environment'])-allowed_environment or cfg['monado_environment'].get('P_OVERRIDE_ACTIVE_CONFIG')!='remote':
        raise ValueError('Only process-local simulation settings are admitted')
    if not 0<cfg.get('timeout',60)<=180: raise ValueError('Runtime budget must be between zero and 180 seconds')
    if cfg.get('native_input_probe') not in (None,'read-only-three-samples','owned-activation-three-samples'):
        raise ValueError('Unknown bounded native input probe')
    if cfg.get('native_input_probe') and cfg.get('renderer_mode','vr')!='vr':
        raise ValueError('Native input diagnostic requires the actual VR channel')
    if cfg.get('proton_environment',{}) not in ({},{'PROTON_NO_NTSYNC':'1'}):
        raise ValueError('Only the verified process-local Proton sync comparison is admitted')
    if cfg.get('native_dual_probe') not in (None,'zap-initial-inventory'):
        raise ValueError('Only the verified native ID1 charge/release fixture is admitted')
    if type(cfg.get('capture_desktop',True)) is not bool:
        raise ValueError('Desktop capture selection must be explicit boolean')
    if not cfg['pose_steps'] or cfg['pose_steps'][0]['name']!='baseline' or cfg['pose_steps'][0]['head']!=cfg['baseline_head']:
        raise ValueError('First held-pose case must certify the commanded baseline')
    for head in [cfg['baseline_head'],*[step['head'] for step in cfg['pose_steps']]]:
        if len(head)!=6 or not all(isinstance(v,(int,float)) and math.isfinite(v) and abs(v)<=10 for v in head):
            raise ValueError('Invalid held-pose specification')
    expected=cfg['expected_baseline_head']
    if expected!={'p':[0,0,0],'q':[0,0,0,1]} or cfg['baseline_head']!=[0,1.6,0,0,0,0]:
        raise ValueError('Initial remote STAGE head and native LOCAL reference must match the verified baseline')
    return private,lab,prefix

def native_dual_probe(cfg,env,token,deadline,run_dir,state):
    """Bounded unchanged-inventory Zap probe via ordinary simulated input.

    Native receipts and visible impacts need subsequent review. This function
    never grants weapons, changes native eligibility or edits command history.
    """
    steps=[('neutral',0,0,2),('right',0,1,2),('neutral-after-right',0,0,3),
        ('left',1,0,2),('neutral-after-left',0,0,3),('both-first',1,1,2),
        ('right-retained',0,1,2),('neutral-after-both',0,0,3),('both-second',1,1,2),
        ('left-retained',1,0,2),('final-neutral',0,0,3)]
    identity_key=dual_identity(state)
    def ready(value):
        return (value['focused'] and value['head_valid'] and value['gameplay'] and not value['menu'] and
            value.get('health',0)>0 and value.get('current_weapon')==[1,1] and
            value.get('native_activation_repeated') and value.get('native_input_enabled') and
            value.get('native_input_exclusive') and value.get('native_core_foreground') and value.get('native_host_foreground') and
            value.get('hand_valid')==[1,1] and value.get('primary_active_mask')==3 and
            value.get('wheel_open')==[0,0] and value['input_age_ms']<=150 and
            value['input_tick_ms']+value['input_age_ms']>=value.get('ui_tick_ms',0) and
            value['input_tick_ms']+value['input_age_ms']-value.get('ui_tick_ms',0)<=250 and
            value.get('ui_tracking_generation')==value.get('tracking_generation') and
            dual_identity(value)==identity_key)
    report={'result':'incomplete','steps':[],'native_compatibility_acceptance':False}
    prior=[0,0];cycles={};last_release_tick=0
    def send(left,right):
        subprocess.run([cfg['pose_driver'],str(cfg['remote_port']),*map(str,cfg['baseline_head']),
            str(left),str(right),'-0.2','0.2'],check=True,timeout=remaining(deadline,3))
    def native_trace():
        text=(Path(cfg['game_lab'])/'Bin/SS2VR.log').read_text(errors='replace')
        if 'Lab native weapon trace saturated' in text:
            raise RuntimeError('Native dual observation trace saturated; evidence incomplete')
        return text
    try:
        if not ready(state):raise RuntimeError('Native dual Zap fixture lacks fresh owned two-hand/native foreground readiness')
        for name,left,right,duration in steps:
            boundary=state['input_sequence'];send(left,right)
            stop=min(deadline,time.monotonic()+duration);phase=DualPhaseEvidence(boundary,[left,right])
            while time.monotonic()<stop:
                state=observer(cfg,env,'status',token,timeout=remaining(deadline,2),deadline=deadline)
                if not ready(state):raise RuntimeError('Native dual probe lost its fixture/identity/readiness')
                phase.observe(state)
                time.sleep(.02)
            phase.require_progress();observations=phase.observations
            for hand,high in enumerate((left,right)):
                if high and not prior[hand]:cycles[hand]=phase.confirmed_boundary()
            report['steps'].append({'name':name,'input_boundary':boundary,
                'end_sequence':observations[-1]['input_sequence'],'expected_trigger':[left,right],'observations':observations})
            (run_dir/'native-dual-probe.json').write_text(json.dumps(report,indent=2))
            if left or right:
                native=native_trace()
                topologies=dual_topologies(native,identity_key,boundary,observations[-1]['input_sequence'])
                if not topologies:raise RuntimeError('Native independent two-hand topology was not compatible')
            report['steps'][-1]['successful_fire_hands_in_phase']=sorted(successful_dual_fire(
                native_trace(),identity_key,boundary,state['input_sequence'],dual_topologies(
                    native_trace(),identity_key,boundary,state['input_sequence'])))
            if left or right or name=='final-neutral':
                q=cfg['expected_baseline_head']['q'];p=cfg['expected_baseline_head']['p']
                capture=run_dir/('dual-'+name)
                observer(cfg,env,'capture',token,(capture,observations[0]['input_sequence'],*p,*q),timeout=remaining(deadline),deadline=deadline)
                metadata=json.loads(Path(str(capture)+'-metadata.json').read_text())
                state=observer(cfg,env,'status',token,timeout=remaining(deadline,2),deadline=deadline)
                if not ready(state):raise RuntimeError('Native dual capture lost fixture/identity/readiness')
                phase.observe(state)
                require_dual_capture(metadata,phase,identity_key,state)
                report['steps'][-1]['capture']=metadata
                report['steps'][-1]['end_sequence']=state['input_sequence']
                native_trace()
            closed={hand for hand,high in enumerate((left,right)) if prior[hand] and not high}
            for hand in closed:
                native=native_trace();start=cycles.pop(hand);end=state['input_sequence']
                topologies=dual_topologies(native,identity_key,start,end)
                bindings={(v['owner'],tuple(v['weapon']),tuple(v['receiver'])) for v in topologies}
                if len(bindings)!=1:raise RuntimeError('Zap cycle lacks one stable compatible native topology')
                releases=[v for v in dual_weapon_events(native,identity_key,boundary,end,topologies,'release')
                    if v['hand']==hand and v['trigger'][hand]<.01]
                if not releases or hand not in successful_dual_fire(native,identity_key,start,end,topologies):
                    raise RuntimeError('Zap press/release cycle lacks successful native fire/release receipts: '+name)
                last_release_tick=max(last_release_tick,max(v['completion_tick'] for v in releases))
                report['steps'][-1].setdefault('closed_cycles',[]).append({'hand':hand,'start_input':start,
                    'end_input':end,'normal_release_receipts':releases})
            if not left and not right and name!='neutral':phase.require_quiet_after(last_release_tick)
            prior=[left,right]
            native_trace()
        if cycles:raise RuntimeError('Native Zap probe ended with an unclosed ordinary input cycle')
        native_trace()
        report['result']='ordinary_input_sequence_observed_native_receipts_unreviewed'
    finally:
        # Release ordinary synthetic controls even if the runtime deadline expired.
        # This bounded release is not a native history/gesture manipulation.
        try:
            subprocess.run([cfg['pose_driver'],str(cfg['remote_port']),*map(str,cfg['baseline_head']),
                '0','0','-0.2','0.2'],check=True,timeout=3)
        except Exception as error:
            report['release_error']=str(error);report['result']='incomplete'
        (run_dir/'native-dual-probe.json').write_text(json.dumps(report,indent=2))
    return report

def run(cfg):
    private,lab,prefix=validate(cfg)
    stock=cfg.get('renderer_mode','vr')=='stock'
    # A copied directory does not isolate native Steam remote mutations. Keep
    # actual game launch closed until the reviewed early process-local adapter
    # is compiled and explicitly selected in this private configuration.
    if cfg.get('online_isolation')!='native-init-barrier-verified':
        raise RuntimeError('Native save/profile isolation is not verified; no game launched')
    if running('Sam2.exe') or running('ss2vr_host.exe'):
        raise RuntimeError('An existing game or host is running; no duplicate test will start')
    observer_exclusions={(i['pid'],i['start']) for i in private_processes(cfg['observer'])}
    run_dir=private/'runs'/time.strftime('%Y%m%dT%H%M%S')
    run_dir.mkdir(parents=True,exist_ok=False)
    cfg['observer_output_dir']=str(run_dir/'observations')
    Path(cfg['observer_output_dir']).mkdir(mode=0o700)
    runtime=run_dir/'runtime';runtime.mkdir(mode=0o700)
    config_dir=run_dir/'config';config_dir.mkdir(mode=0o700)
    manifest={'source_head':subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),
              'source_diff_sha256':hashlib.sha256(subprocess.check_output(['git','diff','HEAD'],cwd=ROOT)).hexdigest(),
              'products':json.loads((lab/'Bin/SS2VR/install-receipt.json').read_text()),
              'harness_sha256':digest(Path(__file__)), 'runtime_helpers':cfg['fingerprints'],
              'scene':cfg['scene'], 'startup':cfg['startup'],
              'validated_config':cfg,
              'harness_sources':{name:digest(ROOT/'tools'/name) for name in
                  ('runtime_lab.py','runtime_observer.cpp','monado_pose_driver.cpp','assess_runtime.py')},
              'simulation':not stock,'renderer_mode':'stock' if stock else 'vr','hardware_acceptance':False,'result':'incomplete'}
    last_focus=0.0;last_state=None;loading_continue_sent=False;stock_creation=None
    loading_receipt=None;last_native_focus=0;native_focus_requests=0
    stock_token='Z:'+str(lab/'Bin/Sam2.exe')
    master,slave=pty.openpty(); service=None;launch=None;owned_game=None;owned_host=None;token=None;owned_window=None
    env=os.environ.copy()
    env.update({'STEAM_COMPAT_DATA_PATH':str(prefix),'STEAM_COMPAT_CLIENT_INSTALL_PATH':cfg['steam'],
        'SteamAppId':'204340','SteamGameId':'204340','XDG_RUNTIME_DIR':str(runtime),
        'XDG_CONFIG_HOME':str(config_dir),'XR_RUNTIME_JSON':cfg['native_manifest'],
        'WINEXR_RUNTIME_JSON':r'C:\openxr\wineopenxr64.json','VR_OVERRIDE':cfg['xrizer'],
        'LD_LIBRARY_PATH':cfg['runtime_libraries'],'WINEDLLOVERRIDES':'d3d9=n,b;d3d11,dxgi=n',
        'PROTON_LOG':'1','PROTON_LOG_DIR':str(run_dir),'SS2VR_LAB_TRACE':'1','SS2VR_LAB_ISOLATE_ONLINE':'1','SS2VR_LAB_SCENE':cfg['scene']['entry']})
    env.update(cfg['monado_environment'])
    env.update(cfg.get('proton_environment',{}))
    env.pop('SS2VR_LAB_STOCK_RENDER',None)
    if stock:env['SS2VR_LAB_STOCK_RENDER']='1'
    files=[]; cleanup_errors=[]
    deadline=time.monotonic()+cfg.get('timeout',60)
    try:
        # Proton consumes OpenVR paths before processing VR_OVERRIDE. Preserve
        # its bootstrap inside this run's private config, not global defaults.
        openvr=config_dir/'openvr';openvr.mkdir()
        (openvr/'openvrpaths.vrpath').write_text(json.dumps({
            'version':1,'jsonid':'vrpathreg','runtime':[cfg['xrizer']],
            'config':[str(openvr)],'log':[str(run_dir)]}))
        config=(config_dir/'monado');config.mkdir()
        (config/'config_v0.json').write_text(json.dumps(cfg['monado_config']))
        if not stock:
            f=(run_dir/'monado.log').open('xb');files.append(f)
            service=subprocess.Popen([cfg['monado_service']],stdin=slave,stdout=f,stderr=subprocess.STDOUT,env=env,start_new_session=True)
            os.close(slave);slave=-1
            while not (runtime/'monado_comp_ipc').exists():
                if service.poll() is not None:raise RuntimeError('Private Monado startup failed')
                if time.monotonic()>deadline:raise TimeoutError('Private Monado socket not ready')
                time.sleep(.1)
            # The native upstream listener is patched to loopback only. Certify the
            # actual listening inode belongs to this exact private service before control.
            while time.monotonic()<deadline:
                sockets=set()
                for p in Path('/proc',str(service.pid),'fd').iterdir():
                    try:sockets.add(p.readlink().name)
                    except OSError:pass
                listeners=[]
                for line in Path('/proc/net/tcp').read_text().splitlines()[1:]:
                    fields=line.split();address,port=fields[1].split(':')
                    if int(port,16)==cfg['remote_port'] and fields[3]=='0A':
                        listeners.append((address,fields[9]))
                if listeners:
                    if len(listeners)!=1 or listeners[0][0]!='0100007F' or ('socket:['+listeners[0][1]+']') not in sockets:
                        raise RuntimeError('Remote listener is not private-service loopback owned')
                    break
                if service.poll() is not None:raise RuntimeError('Private runtime stopped')
                time.sleep(.1)
            else:raise TimeoutError('Private exact-pose listener not ready')
            subprocess.run([cfg['pose_driver'],str(cfg['remote_port']),*map(str,cfg['baseline_head'])],check=True,timeout=remaining(deadline,5))
        # Existing runtime logs are preserved before each new process opens them.
        for path in (lab/'Bin/SS2VR.log',lab/'Bin/SS2VR/ss2vr_host.log',lab/'Sam2.log'):
            if path.exists():
                # Reversible move gives the fresh launch a new log boundary.
                path.rename(run_dir/('previous-'+path.name))
        f=(run_dir/'launch.log').open('xb');files.append(f)
        launch=subprocess.Popen([cfg['proton'],'run',str(lab/'Bin/Sam2.exe'),*cfg['startup']['arguments']],
            cwd=lab,env=env,stdout=f,stderr=subprocess.STDOUT)
        while time.monotonic()<deadline:
            games=[i for i in running('Sam2.exe') if game_owned(i,lab)]
            if len(games)>1:raise RuntimeError('Ambiguous game process ownership')
            if games:
                owned_game=games[0]
                if not cfg.get('native_input_probe') and (owned_window is None or ((not token or (last_state and last_state.get('menu',stock))) and time.monotonic()-last_focus>1)):
                    # X11 calls can block; keep them in a deadline-bounded child.
                    focus=subprocess.run([sys.executable,str(Path(__file__)),
                        '--focus-owned',str(owned_game['pid']),'--start',owned_game['start'],'--lab',str(lab)],
                        capture_output=True,text=True,timeout=remaining(deadline,2))
                    if focus.returncode:
                        # Preserve the concrete child failure in private evidence;
                        # a generic focus error cannot diagnose native startup.
                        manifest['focus_failure']={'returncode':focus.returncode,
                            'stdout':focus.stdout[-4096:],'stderr':focus.stderr[-4096:]}
                        raise RuntimeError('Owned-game focus helper failed')
                    owned_window=json.loads(focus.stdout);last_focus=time.monotonic()
            hosts=running('ss2vr_host.exe')
            for h in hosts:
                normalized=b' '.join(h['argv']).decode(errors='replace').replace('\\','/').casefold()
                if str(lab/'Bin/SS2VR/ss2vr_host.exe').casefold() not in normalized:continue
                args=[v.decode(errors='replace') for v in h['argv']]
                if '--channel' in args:
                    token=args[args.index('--channel')+1];owned_host=h
            # Native background/minimized loading can precede the presentation
            # that starts the host. Restore the verified game window without
            # requiring that later IPC channel; confirmation still needs it.
            if (not stock and not token and owned_game and not cfg.get('native_input_probe') and
                native_focus_requests<4 and time.monotonic()-last_native_focus>=2):
                incarnation=observer(cfg,env,'stock-status',stock_token,timeout=remaining(deadline,3),deadline=deadline)
                if incarnation.get('loading_native_ready')==1:
                    receipt=run_dir/('early-activation-'+str(native_focus_requests)+'.json')
                    try:observer(cfg,env,'stock-focus',stock_token,(incarnation['process_creation'],'Z:'+str(receipt)),timeout=remaining(deadline,3),deadline=deadline)
                    except ObserverError as error:
                        if error.code!=8:raise
                    native_focus_requests+=1
                    manifest['native_focus_requests']=native_focus_requests
                    if receipt.exists():manifest.setdefault('early_activation_requests',[]).append(json.loads(receipt.read_text()))
                last_native_focus=time.monotonic()
            if stock and owned_game and owned_window:
                if owned_host:raise RuntimeError('Stock comparator unexpectedly started a VR host')
                token=stock_token
                try:state=observer(cfg,env,'stock-status',token,timeout=remaining(deadline),deadline=deadline)
                except ObserverError as error:
                    if error.code==4:
                        time.sleep(.05);continue
                    raise
                if stock_creation is None:stock_creation=state['process_creation']
                if state['process_creation']!=stock_creation:raise RuntimeError('Stock process incarnation changed')
                last_state=state
                native_log=(lab/'Bin/SS2VR.log').read_text(errors='replace')
                if 'Native VR hooks attached' in native_log:raise RuntimeError('Stock reference has native VR hooks')
                receipts=re.findall(r'Lab native scene stream bytes=(\d+) positionRestored=(\d+) sha256=([a-f0-9]{64})',native_log)
                isolated=('Lab native online initializer suppressed; interface=0;' in native_log and
                    'Lab stock renderer selected before VR worker creation; graphics hooks disabled' in native_log)
                scene=(lab/'Sam2.log').read_text(errors='replace') if (lab/'Sam2.log').exists() else ''
                if state['loading_ready'] and not loading_continue_sent:
                    if not isolated or not receipts or any(h!=cfg['scene']['entry_sha256'] for size,pos,h in receipts):
                        raise RuntimeError('Stock continuation lacks consistent scene/isolation evidence')
                    try:observer(cfg,env,'stock-continue-loading',token,(stock_creation,),timeout=remaining(deadline),deadline=deadline)
                    except ObserverError as error:
                        if error.code==8:continue # Rejected before any input was posted.
                        raise
                    loading_continue_sent=True
                    manifest['loading_continue_messages_posted']=True
                if (isolated and state['local_transport'] and state['online_interface_null'] and receipts and
                    state['menu_clear'] and state['camera_repeated_equal'] and state['view_pose_match'] and state['perspective_projection'] and
                    ('+inp_fMouseSensitivity' not in cfg['startup']['arguments'] or state['mouse_zero']) and
                    all(h==cfg['scene']['entry_sha256'] for size,pos,h in receipts) and
                    re.search(r"Started simulation on '"+re.escape(cfg['scene']['entry'])+r"'",scene)):
                    (run_dir/'ready.json').write_text(json.dumps(state,indent=2));break
            elif token:
                try:state=observer(cfg,env,'status',token,timeout=remaining(deadline),deadline=deadline)
                except ObserverError as error:
                    if error.code==4:
                        time.sleep(.05);continue
                    raise
                last_state=state
                if cfg.get('native_input_probe'):
                    if state.get('native_state_repeated') and state.get('loading_native_ready')==1:
                        samples=manifest.setdefault('native_input_samples',[]);samples.append(state)
                        if len(samples)==1 and cfg['native_input_probe']=='owned-activation-three-samples':
                            if not (state.get('native_activation_repeated') and state.get('native_input_enabled') and
                                state.get('native_running') and state.get('native_simulation_present') and state.get('native_exclusive_block')==0):
                                raise RuntimeError('Native activation probe lacks verified eligible owned state')
                            incarnation=observer(cfg,env,'stock-status',stock_token,timeout=remaining(deadline),deadline=deadline)
                            if incarnation['game_pid']!=state['game_pid']:raise RuntimeError('Activation owner differs from channel')
                            receipt=run_dir/'activation-request.json'
                            try:observer(cfg,env,'stock-focus',stock_token,(incarnation['process_creation'],'Z:'+str(receipt)),timeout=remaining(deadline),deadline=deadline)
                            except ObserverError as error:
                                if error.code!=8:raise
                            if receipt.exists():manifest['activation_request']=json.loads(receipt.read_text())
                            manifest['activation_attempted_once']=True
                        if len(samples)==3:
                            manifest['game_pid']=owned_game['pid'];manifest['host_pid']=owned_host['pid'] if owned_host else None
                            manifest['result']='native_input_state_sampled_no_controls_injected'
                            return run_dir
                    time.sleep(.2);continue
                if (state.get('loading_native_ready')==1 and not state.get('loading_ready') and
                    native_focus_requests<4 and time.monotonic()-last_native_focus>=2):
                    incarnation=observer(cfg,env,'stock-status',stock_token,timeout=remaining(deadline),deadline=deadline)
                    if incarnation['game_pid']!=state['game_pid']:raise RuntimeError('Native focus owner differs from the channel')
                    try:observer(cfg,env,'stock-focus',stock_token,(incarnation['process_creation'],),timeout=remaining(deadline),deadline=deadline)
                    except ObserverError as error:
                        if error.code!=8:raise
                    native_focus_requests+=1;last_native_focus=time.monotonic()
                    manifest['native_focus_requests']=native_focus_requests
                if state.get('loading_ready') and not loading_continue_sent:
                    native_log=(lab/'Bin/SS2VR.log').read_text(errors='replace')
                    native_receipts=re.findall(r'Lab native scene stream bytes=(\d+) positionRestored=(\d+) sha256=([a-f0-9]{64})',native_log)
                    if ('Lab native online initializer suppressed; interface=0;' not in native_log or
                        not native_receipts or any(h!=cfg['scene']['entry_sha256'] for size,pos,h in native_receipts)):
                        raise RuntimeError('Loading continuation lacks consistent native scene/isolation evidence')
                    receipt=run_dir/'loading-post.json'
                    try:observer(cfg,env,'continue-loading',token,('Z:'+str(lab/'Bin/Sam2.exe'),'Z:'+str(receipt)),timeout=remaining(deadline),deadline=deadline)
                    except ObserverError as error:
                        if error.code==8:continue # Rejected before any input was posted.
                        raise
                    loading_continue_sent=True
                    manifest['loading_continue_messages_posted']=True
                    loading_receipt=json.loads(receipt.read_text())
                    manifest['loading_post_receipt']=loading_receipt
                if (loading_receipt and state.get('native_state_repeated') and
                    state.get('native_simulation')==loading_receipt['native_simulation'] and
                    state.get('native_world_start_blocked')==0 and state.get('native_current_menu')==0):
                    manifest['loading_native_transition_observed']=True
                if state['renderer'] and state['gameplay'] and not state['menu'] and state['focused'] and state['head_valid']:
                    (run_dir/'ready.json').write_text(json.dumps(state,indent=2));break
            if launch.poll() is not None and not games:raise RuntimeError('Game exited before world readiness')
            time.sleep(.2)
        else:raise TimeoutError('Verified scene did not reach gameplay readiness')
        manifest['game_pid']=owned_game['pid'];manifest['host_pid']=owned_host['pid'] if owned_host else None
        scene_log=(lab/'Sam2.log').read_text(errors='replace')
        if not re.search(r"Started simulation on '"+re.escape(cfg['scene']['entry'])+r"'",scene_log):
            raise RuntimeError('Fresh native log does not certify requested simulation')
        manifest['loaded_scene_confirmed']=True
        isolation_log=(lab/'Bin/SS2VR.log').read_text(errors='replace')
        if 'Lab native online initializer suppressed; interface=0;' not in isolation_log:
            raise RuntimeError('Fresh native log does not certify early online isolation')
        if (not stock and 'Lab local gameplay observed with online interface=0' not in isolation_log) or \
            (stock and not (state['local_transport'] and state['online_interface_null'])):
            raise RuntimeError('Fresh native local gameplay/isolation invariant not observed')
        receipts=re.findall(r'Lab native scene stream bytes=(\d+) positionRestored=(\d+) sha256=([a-f0-9]{64})',isolation_log)
        if not receipts: raise RuntimeError('No successful native scene-stream receipt')
        if any(h!=cfg['scene']['entry_sha256'] for size,pos,h in receipts):
            manifest['native_scene_receipts']=receipts
            raise RuntimeError('Actual native scene differs from the configured exact scene')
        manifest['native_scene_receipts']=receipts
        if 'Steam initialize (AppID' in scene_log or 'from Steam cloud:' in scene_log:
            raise RuntimeError('Native Steam interface unexpectedly initialized')
        if stock:
            (run_dir/'stock-camera-before.json').write_text(json.dumps(state,indent=2))
            desktop=run_dir/'stock-desktop.png'
            subprocess.run([sys.executable,str(Path(__file__)),
                '--focus-owned',str(owned_game['pid']),'--start',owned_game['start'],
                '--lab',str(lab),'--capture-owned',str(desktop),'--private-root',str(private)],
                capture_output=True,text=True,check=True,timeout=remaining(deadline,5))
            after=observer(cfg,env,'stock-status',stock_token,timeout=remaining(deadline),deadline=deadline)
            (run_dir/'stock-camera-after.json').write_text(json.dumps(after,indent=2))
            if after['process_creation']!=stock_creation:raise RuntimeError('Stock capture process incarnation changed')
            manifest['result']='stock_image_captured_camera_unverified'
            manifest['camera_comparison_acceptance']=False
            return run_dir
        baseline=cfg['expected_baseline_head'];baseline_packet=cfg['baseline_head']
        for step in cfg['pose_steps']:
            if not step['name'].replace('-','').isalnum():raise ValueError('Unsafe pose step name')
            boundary=state['input_sequence']
            head=step['head'];subprocess.run([cfg['pose_driver'],str(cfg['remote_port']),*map(str,head)],check=True,timeout=remaining(deadline,5))
            def mul(a,b):
                x,y,z,w=a;X,Y,Z,W=b
                return [w*X+x*W+y*Z-z*Y,w*Y-x*Z+y*W+z*X,w*Z+x*Y-y*X+z*W,w*W-x*X-y*Y-z*Z]
            yaw,pitch,roll=head[3:]
            q=mul(mul([0,math.sin(yaw/2),0,math.cos(yaw/2)],
                      [math.sin(pitch/2),0,0,math.cos(pitch/2)]),[0,0,math.sin(roll/2),math.cos(roll/2)])
            expected=[baseline['p'][i]+head[i]-baseline_packet[i] for i in range(3)]
            matches=0;previous=boundary;pose_deadline=min(deadline,time.monotonic()+10)
            while matches<30 and time.monotonic()<pose_deadline:
                try:state=observer(cfg,env,'status',token,timeout=remaining(pose_deadline),deadline=pose_deadline)
                except ObserverError as error:
                    if error.code==4:
                        time.sleep(.01);continue
                    raise
                values=state['head']['p']+state['head']['q']
                valid=(all(math.isfinite(v) for v in values) and abs(sum(v*v for v in state['head']['q'])-1)<.001 and
                    state['input_age_ms']<=150 and state['input_sequence']>boundary and
                    state['focused'] and state['head_valid'] and state['gameplay'] and not state['menu'] and
                    max(abs(state['head']['p'][i]-expected[i]) for i in range(3))<.001 and
                    abs(sum(a*b for a,b in zip(state['head']['q'],q)))>.99999)
                if state['input_sequence']>previous:
                    matches=matches+1 if valid else 0;previous=state['input_sequence']
                time.sleep(.01)
            if matches<30:raise TimeoutError('Held pose did not produce 30 matching OpenXR observations: '+step['name'])
            if step['name']=='baseline':
                sustained_deadline=min(deadline,time.monotonic()+60)
                count=0
                while time.monotonic()<sustained_deadline:
                    native_path=lab/'Bin/SS2VR.log';host_path=lab/'Bin/SS2VR/ss2vr_host.log'
                    native=native_path.read_text(errors='replace') if native_path.exists() else ''
                    host=host_path.read_text(errors='replace') if host_path.exists() else ''
                    count=len(complete_pairs_for_pose(native,host,{'p':expected,'q':q}))
                    if count>=30:break
                    if not still_owned(owned_game):raise RuntimeError('Owned game exited during sustained neutral rendering')
                    time.sleep(.1)
                manifest['distinct_neutral_complete_pairs']=count
                if count<30:raise TimeoutError('Neutral pose lacks 30 distinct complete native/UI/projection requests')
            (run_dir/(step['name']+'-input.json')).write_text(json.dumps(state,indent=2))
            desktop=run_dir/(step['name']+'-desktop.png')
            observer(cfg,env,'capture',token,(run_dir/step['name'],state['input_sequence'],*expected,*q),timeout=remaining(deadline),deadline=deadline)
            if cfg.get('capture_desktop',True):
                captured=subprocess.run([sys.executable,str(Path(__file__)),
                    '--focus-owned',str(owned_game['pid']),'--start',owned_game['start'],
                    '--lab',str(lab),'--capture-owned',str(desktop),'--private-root',str(private)],
                    capture_output=True,text=True,timeout=remaining(deadline,5))
                if captured.returncode:
                    manifest.setdefault('desktop_capture_errors',[]).append({'pose':step['name'],
                        'returncode':captured.returncode,'stderr':captured.stderr[-4096:]})
        if cfg.get('native_dual_probe')=='zap-initial-inventory':
            # Retain the same normal window activation policy for actual input
            # probes; native eye capture does not establish exclusive input.
            if not state.get('native_host_foreground') or not state.get('native_input_exclusive'):
                incarnation=observer(cfg,env,'stock-status',stock_token,timeout=remaining(deadline,3),deadline=deadline)
                receipt=run_dir/'dual-activation.json'
                try:observer(cfg,env,'stock-focus',stock_token,(incarnation['process_creation'],'Z:'+str(receipt)),timeout=remaining(deadline,3),deadline=deadline)
                except ObserverError as error:
                    if error.code!=8:raise
                stop=min(deadline,time.monotonic()+3)
                while time.monotonic()<stop:
                    state=observer(cfg,env,'status',token,timeout=remaining(stop,2),deadline=stop)
                    if state.get('native_host_foreground') and state.get('native_input_exclusive'):break
                    time.sleep(.05)
                else:raise RuntimeError('Native dual fixture did not regain normal exclusive input')
                if receipt.exists():manifest['dual_activation_request']=json.loads(receipt.read_text())
            manifest['native_dual_probe']=native_dual_probe(cfg,env,token,deadline,run_dir,state)
        manifest['result']='world_images_captured_unreviewed'
    except Exception as error:
        manifest['error']=str(error)
        raise
    finally:
        cleanup_deadline=time.monotonic()+20
        termination_requests=[]
        def defer_termination(signum,frame): termination_requests.append(signum)
        previous_handlers={sig:signal.signal(sig,defer_termination) for sig in (signal.SIGTERM,signal.SIGINT)}
        def cleanup(action):
            try:action()
            except Exception as error:cleanup_errors.append(str(error))
        def graceful_close():
            if token and owned_game and still_owned(owned_game):
                try:
                    if stock and stock_creation:
                        observer(cfg,env,'stock-close',stock_token,(stock_creation,),timeout=3,deadline=cleanup_deadline)
                    elif not stock:observer(cfg,env,'close',token,timeout=3,deadline=cleanup_deadline)
                except (RuntimeError,subprocess.TimeoutExpired,TimeoutError):pass
                end=min(cleanup_deadline,time.monotonic()+3)
                while still_owned(owned_game) and time.monotonic()<end:time.sleep(.1)
        cleanup(graceful_close)
        # Rediscover late descendants before and after stopping the launcher;
        # exact executable path/start identity prevents unrelated process cleanup.
        def retire_children():
            for executable in (lab/'Bin/Sam2.exe',lab/'Bin/SS2VR/ss2vr_host.exe',Path(cfg['observer'])):
                if executable==Path(cfg['observer']) and not token:continue
                for item in private_processes(executable,token if executable==Path(cfg['observer']) else None):
                    if executable==Path(cfg['observer']) and (item['pid'],item['start']) in observer_exclusions:continue
                    cleanup(lambda item=item:stop_exact(item,cleanup_deadline))
        cleanup(retire_children)
        def retire_launcher(process):
            if not process or process.poll() is not None: return
            cleanup(process.terminate)
            try: process.wait(timeout=remaining(cleanup_deadline,3))
            except (subprocess.TimeoutExpired,TimeoutError):
                cleanup(process.kill)
                cleanup(lambda:process.wait(timeout=remaining(cleanup_deadline,2)))
        cleanup(lambda:retire_launcher(launch))
        cleanup(retire_children)
        cleanup(lambda:retire_launcher(service))
        for f in files:f.close()
        os.close(master)
        if slave>=0:os.close(slave)
        import shutil
        for path in (lab/'Bin/SS2VR.log',lab/'Bin/SS2VR/ss2vr_host.log',lab/'Sam2.log'):
            if path.exists():shutil.copy2(path,run_dir/path.name)
        manifest['shutdown_observed']='Lab native online shutdown wrapper entered; interface=0;' in (run_dir/'SS2VR.log').read_text(errors='replace') if (run_dir/'SS2VR.log').exists() else False
        manifest['termination_requests']=termination_requests
        manifest['cleanup_errors']=cleanup_errors
        (run_dir/'result.json').write_text(json.dumps(manifest,indent=2))
        print('Private runtime record:',run_dir)
        for sig,handler in previous_handlers.items():signal.signal(sig,handler)
        if termination_requests and not cleanup_errors: cleanup_errors.append('Termination requested during cleanup')
        if cleanup_errors:raise RuntimeError('Private runtime cleanup failed; inspect preserved result')
    return run_dir

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--config',type=Path)
    parser.add_argument('--check',action='store_true')
    parser.add_argument('--focus-owned',type=int,help=argparse.SUPPRESS)
    parser.add_argument('--start',help=argparse.SUPPRESS)
    parser.add_argument('--capture-owned',type=Path,help=argparse.SUPPRESS)
    parser.add_argument('--private-root',type=Path,help=argparse.SUPPRESS)
    parser.add_argument('--lab',type=Path,help=argparse.SUPPRESS)
    args=parser.parse_args()
    if args.focus_owned is not None:
        item=identity(args.focus_owned)
        if not item or item['start']!=args.start or item['name']!='Sam2.exe' or not args.lab or not game_owned(item,args.lab.resolve()):
            raise SystemExit('Focus helper ownership mismatch')
        window=window_for(item['pid'])
        if args.capture_owned:
            capture_deadline=time.monotonic()+2
            while not window and time.monotonic()<capture_deadline:
                if not still_owned(item):raise SystemExit('Capture owner retired while waiting for its mapped window')
                time.sleep(.1);window=window_for(item['pid'])
        if args.capture_owned:
            if not window or not args.private_root:raise SystemExit('Owned capture unavailable')
            destination=args.capture_owned.resolve()
            private=args.private_root.resolve(strict=True)
            if private.is_relative_to(ROOT) or not destination.is_relative_to(private) or destination.exists():
                raise SystemExit('Unsafe capture destination')
            subprocess.run(['import','-window',str(window),str(destination)],check=True,timeout=3)
        print(json.dumps(window))
        raise SystemExit(0)
    if not args.config:parser.error('--config is required')
    def interrupted(signum,frame):raise RuntimeError('Runtime lab received termination signal')
    signal.signal(signal.SIGTERM,interrupted)
    configuration=json.loads(args.config.read_text())
    if args.check:
        validate(configuration);print('Private lab, installed products, startup evidence and scene bytes verified')
    else:run(configuration)
