#!/usr/bin/env python3
"""Run a verified direct-to-scene SS2 test in an existing private Proton lab.

Configuration, owned game assets, prefixes, logs and images stay outside source.
This launcher never installs, changes global runtimes, joins multiplayer or clicks menus.
"""
from assess_runtime import complete_pairs_for_pose

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
    class ClientMessage(ctypes.Structure):
        _fields_=[('type',ctypes.c_int),('serial',U),('send_event',ctypes.c_int),('display',D),
                  ('window',U),('message_type',U),('format',ctypes.c_int),('data',ctypes.c_long*5)]
    class Event(ctypes.Union):
        _fields_=[('client',ClientMessage),('pad',ctypes.c_long*24)]
    x.XSendEvent.argtypes=[D,U,ctypes.c_int,ctypes.c_long,ctypes.POINTER(Event)]
    d = x.XOpenDisplay(None)
    if not d: raise RuntimeError('No native desktop display')
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
                if title.value==b'Serious Sam 2': found.append(w)
                x.XFree(title)
        if len(found)!=1: return None
        event=Event();event.client.type=33;event.client.display=d;event.client.window=found[0]
        event.client.message_type=x.XInternAtom(d,b'_NET_ACTIVE_WINDOW',0)
        event.client.format=32;event.client.data[0]=2
        x.XSendEvent(d,root,0,(1<<20)|(1<<19),ctypes.byref(event))
        x.XRaiseWindow(d,found[0]);x.XSetInputFocus(d,found[0],2,0);x.XSync(d,0)
        return found[0]
    finally: x.XCloseDisplay(d)

class ObserverError(RuntimeError):
    def __init__(self, command, code):
        self.code=code
        super().__init__('IPC observer '+command+' returned '+str(code))

def observer(cfg, env, command, token, output=None, timeout=20, deadline=None):
    args=[cfg['proton'], 'runinprefix', cfg['observer'], command, token]
    status_file=None
    if command=='status':
        status_file=Path(cfg['observer_output_dir'])/('status-'+uuid.uuid4().hex+'.json')
        args.extend(['Z:'+str(status_file),'Z:'+str(Path(cfg['game_lab'])/'Bin/Sam2.exe')])
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
    if command=='status':
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
    marker=json.loads((lab/'.ss2vr-runtime-lab.json').read_text())
    checked_file(lab/'Bin/Sam2.exe',marker['game_sha256'])
    receipt=json.loads((lab/'Bin/SS2VR/install-receipt.json').read_text())
    for name, expected in receipt['files'].items(): checked_file(lab/name,expected)
    startup=cfg['startup']
    expected_arguments=['+mod','SeriousSam2','+sam_bBootSequence','0','+sam_bSkipMovies','1','+level',cfg['scene']['entry']]
    if startup['arguments'] != expected_arguments:
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
    if not cfg['pose_steps'] or cfg['pose_steps'][0]['name']!='baseline' or cfg['pose_steps'][0]['head']!=cfg['baseline_head']:
        raise ValueError('First held-pose case must certify the commanded baseline')
    for head in [cfg['baseline_head'],*[step['head'] for step in cfg['pose_steps']]]:
        if len(head)!=6 or not all(isinstance(v,(int,float)) and math.isfinite(v) and abs(v)<=10 for v in head):
            raise ValueError('Invalid held-pose specification')
    expected=cfg['expected_baseline_head']
    if expected!={'p':[0,0,0],'q':[0,0,0,1]} or cfg['baseline_head']!=[0,1.6,0,0,0,0]:
        raise ValueError('Initial remote STAGE head and native LOCAL reference must match the verified baseline')
    return private,lab,prefix

def run(cfg):
    private,lab,prefix=validate(cfg)
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
              'simulation':True,'hardware_acceptance':False,'result':'incomplete'}
    last_focus=0.0;last_state=None;loading_continue_sent=False
    master,slave=pty.openpty(); service=None;launch=None;owned_game=None;owned_host=None;token=None;owned_window=None
    env=os.environ.copy()
    env.update({'STEAM_COMPAT_DATA_PATH':str(prefix),'STEAM_COMPAT_CLIENT_INSTALL_PATH':cfg['steam'],
        'SteamAppId':'204340','SteamGameId':'204340','XDG_RUNTIME_DIR':str(runtime),
        'XDG_CONFIG_HOME':str(config_dir),'XR_RUNTIME_JSON':cfg['native_manifest'],
        'WINEXR_RUNTIME_JSON':r'C:\openxr\wineopenxr64.json','VR_OVERRIDE':cfg['xrizer'],
        'LD_LIBRARY_PATH':cfg['runtime_libraries'],'WINEDLLOVERRIDES':'d3d9=n,b;d3d11,dxgi=n',
        'PROTON_LOG':'1','PROTON_LOG_DIR':str(run_dir),'SS2VR_LAB_TRACE':'1','SS2VR_LAB_ISOLATE_ONLINE':'1','SS2VR_LAB_SCENE':cfg['scene']['entry']})
    env.update(cfg['monado_environment'])
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
                if owned_window is None or ((not token or (last_state and last_state['menu'])) and time.monotonic()-last_focus>1):
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
            if token:
                try:state=observer(cfg,env,'status',token,timeout=remaining(deadline),deadline=deadline)
                except ObserverError as error:
                    if error.code==4:
                        time.sleep(.05);continue
                    raise
                last_state=state
                if state.get('loading_ready') and not loading_continue_sent:
                    native_log=(lab/'Bin/SS2VR.log').read_text(errors='replace')
                    native_receipts=re.findall(r'Lab native scene stream bytes=(\d+) positionRestored=(\d+) sha256=([a-f0-9]{64})',native_log)
                    if ('Lab native online initializer suppressed; interface=0;' not in native_log or
                        not native_receipts or any(h!=cfg['scene']['entry_sha256'] for size,pos,h in native_receipts)):
                        raise RuntimeError('Loading continuation lacks consistent native scene/isolation evidence')
                    loading_continue_sent=True
                    observer(cfg,env,'continue-loading',token,('Z:'+str(lab/'Bin/Sam2.exe'),),timeout=remaining(deadline),deadline=deadline)
                    manifest['loading_continue_messages_posted']=True
                if state['renderer'] and state['gameplay'] and not state['menu'] and state['focused'] and state['head_valid']:
                    (run_dir/'ready.json').write_text(json.dumps(state,indent=2));break
            if launch.poll() is not None and not games:raise RuntimeError('Game exited before world readiness')
            time.sleep(.2)
        else:raise TimeoutError('Verified scene did not reach gameplay readiness')
        manifest['game_pid']=owned_game['pid'];manifest['host_pid']=owned_host['pid']
        scene_log=(lab/'Sam2.log').read_text(errors='replace')
        if not re.search(r"Started simulation on '"+re.escape(cfg['scene']['entry'])+r"'",scene_log):
            raise RuntimeError('Fresh native log does not certify requested simulation')
        manifest['loaded_scene_confirmed']=True
        isolation_log=(lab/'Bin/SS2VR.log').read_text(errors='replace')
        if 'Lab native online initializer suppressed; interface=0;' not in isolation_log:
            raise RuntimeError('Fresh native log does not certify early online isolation')
        if 'Lab local gameplay observed with online interface=0' not in isolation_log:
            raise RuntimeError('Fresh native local gameplay/isolation invariant not observed')
        receipts=re.findall(r'Lab native scene stream bytes=(\d+) positionRestored=(\d+) sha256=([a-f0-9]{64})',isolation_log)
        if not receipts: raise RuntimeError('No successful native scene-stream receipt')
        if any(h!=cfg['scene']['entry_sha256'] for size,pos,h in receipts):
            manifest['native_scene_receipts']=receipts
            raise RuntimeError('Actual native scene differs from the configured exact scene')
        manifest['native_scene_receipts']=receipts
        if 'Steam initialize (AppID' in scene_log or 'from Steam cloud:' in scene_log:
            raise RuntimeError('Native Steam interface unexpectedly initialized')
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
            subprocess.run([sys.executable,str(Path(__file__)),
                '--focus-owned',str(owned_game['pid']),'--start',owned_game['start'],
                '--lab',str(lab),'--capture-owned',str(desktop),'--private-root',str(private)],
                capture_output=True,text=True,check=True,timeout=remaining(deadline,5))
            observer(cfg,env,'capture',token,(run_dir/step['name'],state['input_sequence'],*expected,*q),timeout=remaining(deadline),deadline=deadline)
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
                try:observer(cfg,env,'close',token,timeout=3,deadline=cleanup_deadline)
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
