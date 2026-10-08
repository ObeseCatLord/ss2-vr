# Direct native-gameplay lab

The user requires repeatable in-game VR tests without menu clicking. The Linux/Proton
harness is tools/runtime_lab.py, with tools/runtime_observer.cpp and
 tools/monado_pose_driver.cpp. This is an active implementation; no world/stereo or
hardware acceptance is implied by a launcher completing.

## Verified native startup route

The owned build accepts these separate arguments from the game-root working directory:

```
+mod SeriousSam2 +sam_bBootSequence 0 +sam_bSkipMovies 1 +level Content/SeriousSam2/Levels/01_Mdigbo/1_1_Jungle/1_1_Jungle.wld
```

Engine vmProcessCmdArgs17CFA0 recognizes +name/value. Sam2Game metadata40F1A0 maps
level to sam_strLevelToStart40303C; deferred commands execute10EF8 before project
startup. Sam2.exe installs the project1541 and invokes startup1552. Sam2Game startup
26EF0 chooses its main menu only for an empty level270AB..270D6; nonempty valid
paths reach local start25D60, constructing the local interface/simulation and
calling native simStart_t at26089. This is static evidence from the pinned owned
binaries, not guessed flags. The Jungle world entry is present in the owned patch
archive with matching original/lab bytes. No proprietary bytes are in this repo.

Early configuration assignment is an alternative; invoking samStartGame in an
early config is not equivalent because its wrapper requires initialized project
state. Native +exec exists; no automatic startup.cfg route was found. Save loading
exists but no owned .sav candidate was found in the investigated trees.

## Lab boundaries and evidence

A private JSON config supplies the existing game lab/prefix, exact startup evidence,
archive/entry fingerprints, runtime paths, compiled observer/pose helper identities
and held-pose steps. A lab marker and installer receipt are required. The launcher
verifies these bytes, refuses another running SS2/host, starts a fresh private
runtime directory/config and PTY-backed Monado process, then passes the verified
stock +level arguments. It never clicks menus or silently falls back to them.

The exact-pose experiment uses a private copy of upstream Monado735e29e4e7552b254528dbb20e0e96ec8f32368c.
Existing installed Qwerty/Envision remains untouched. Upstream remote simulation
accepts full HMD/controller poses; the private listener binds loopback only. A
concrete unsynchronized packet read/write was found by Astra; a narrow private
mutex/snapshot adapter passed bounded source review. This changes simulation test
input, not game/host transport. Windows still resolves wineopenxr64.json and the
native bridge separately resolves the private Linux manifest.

The helper compiles the actual runtime wire header, checks376-byte layout/offsets,
handles the two-packet handshake with deadlines, rejects invalid input and sends
zero-padded fixed neutral controllers plus explicit stereo poses/FOV. A socket
write is not proof: the harness requires30 advancing matching actual OpenXR input
observations before capturing a newly requested native Ready eye pair. The observer
never acknowledges or changes slot state. Held pose transitions are not claimed
frame-atomic; acceptance uses steady observations after settling.

Timeouts retain private logs/result identities and stop exact owned processes,
first through the game's own WM_CLOSE and then bounded PID/start-time-verified
cleanup if required. Global runtime registration/security/credentials and original
game settings/saves are outside this harness. The opt-in native startup barrier suppresses construction of the Steam online
object before its initializer and observes a null interface at local gameplay
and shutdown. Its default is off: production multiplayer retains the stock online
route. The exact private runtime observations still have to certify isolation.

## Acceptance

Host diagnostics distinguish successful native world projection submission
xr_world from xr_menu and arbitrary xr_layers. Native camera observation is opt-in
SS2VR_LAB_TRACE=1, bounded to actual eye camera changes and does not inject game
poses. The required matrix is RUNTIME_ACCEPTANCE_MATRIX.md. Actual eye geometry,
near/far parallax, all translation axes and yaw/pitch/roll must be inspected; image
hashes, counters and fixture frames cannot replace that acceptance. Native built-in
dual-wield probes follow, preserving actual supported combinations/mechanics.

## Current local gates after follow-up review

The user renewed source-only public publication authority; the prior hold is historical. Observer compiles; run154502 reached verified direct gameplay and captured sustained complete native world/UI stereo with all seven head poses. Broader quality, lifecycle, hardware and network gates remain open. A strict scan found four different Jungle world providers, including Sam2Renovation.gro, so the harness records providers and requires a hash of the successfully opened native scene stream. It never changes archive precedence to obtain a desired result. The existing files remain preserved. Native Steam initialization/shutdown can write and delete remote profiles; no supported cloud-disable cvar was proved. A narrow opt-in process-local online-init barrier is implemented and reviewed, preserving Proton VR bootstrap and default production behavior. The scene observer admits the native loading worker, borrows synchronously, restores native position, and emits the digest only after restoration. Native-exception restoration remains outside normal-path runtime acceptance.

The private pose adapter follow-up passed its bounded source review after correcting default-view/head and hand-curl/pose snapshot consistency plus initialized thread-helper destruction. A real private remote-service preflight selected the remote builder, bound127.0.0.1, completed the376-byte native-header handshake/pose send, and exited0. No actual game/OpenXR pose or native world acceptance follows from that transport observation.

## Verified loading-screen continuation

The local startup presents CMSLoading and waits for native confirmation; no
auto-continue cvar was found on this inspected route. Native Enter is bound to
syscmdOk and dispatches MC_OK0x0B. Engine's current-thread message hook consumes
WM_KEYDOWN/UP scan code0x1C from lParam. The observer first verifies the private
process path, current object at Sam2Game+40A270, exact CMSLoading table+29F148,
ready field+6C==1, owned foreground HWND/thread and an immediate recheck.
The harness requires consistent fresh native scene/isolation receipts before
one down/up pair; release is always attempted and continuation is never retried.
It never writes game memory or remotely calls a native method. Asynchronous input
delivery can race other input; successful posting alone is not acceptance.

An actual run recorded completed native Jungle
loading in6.96 seconds, local simulation and thirty matching neutral OpenXR head
observations. The first-eye executed-view gate rejected the pair; no native world
images/projection stereo or translated/rotated camera acceptance followed.

The observer is built as a GUI-subsystem executable to avoid a console stealing
the game's focus. Compile with MinGW C++20, static runtime, -municode -mwindows,
-Isrc and -lshell32. It reports status through exclusive private JSON files.
The actual absolute deadline is configurable up to180 seconds; cleanup has a
separate common20-second budget. All process retirement remains ownership checked.

A later same-source x86 GUI observer run verified the complete guarded continuation
route: CMSLoading table29F148/ready1, one posted Enter down/up pair, then actual
Jungle simulation and normal null-interface shutdown. Cross-architecture x64
Toolhelp module enumeration had failed with error299 and could not certify menu
readiness. Distinct eye-image transport has since been observed for one fresh
pair; sustained UI-complete capture and translated/rotated/parallax acceptance
remain unverified. See PC_GAMEPLAY_READINESS.md for the precise limits.


## Stock reference and bounded depth regression

The launcher can select the lab-only stock renderer before worker/device-hook
creation, retaining native scene and isolated-profile startup receipts. It captures
owned process-incarnation stock camera candidates without a host/channel. See
STOCK_RENDERER_COMPARISON.md for actual observations and limits. Both renderer
modes verify matching installed game/server/host source, IPC and wire contracts;
expected_product_source can pin the exact compiled snapshot.

The verified +inp_fMouseSensitivity0 assignment may be inserted before +level
for stable fixture camera orientation. It affects mouse axes/menu cursor motion;
keyboard/button semantics remain native. Use only private lab settings.

Optional depth_range_probe=native-first-person-root-partition enables the assessor's
bounded initial-request draw-range report. It checks complete paired-eye and
desktop observations, read/unwind errors and the audited world partition. It
rejects the reproduced0..1 opaque/alpha-tested versus later0..0.9 mismatch. This
is a state regression for the verified first-person scene route; it does not
classify arbitrary vehicle/scope/postprocess draws or certify image occlusion.
Actual captures and visual terrain/tree/waterfall comparisons remain required.

The owned-window helper now requires mapped InputOutput windows and handles
mapping races as unavailable instead of a fatal XSetInputFocus BadMatch. Its
ctypes layouts were checked against the local Xlib headers. The native loading
command may return8 only before posting input; the launcher waits and rechecks
that rejection within its existing deadline. Successful posting is never repeated,
and partial posting remains a failure. Native acknowledgement/transition remains
under bounded review; a posted Enter alone is not scene readiness.

The observer now exposes existing published per-hand weapon/ammo/fire counters,
trigger validity/generations and UI health/age alongside input identity. These
scalars do not pin native entities or prove releases. The private remote pose
helper accepts optional left/right trigger values and hand yaw, using the actual
Monado wire fields and normal OpenXR input path. Zero/default, separate and both
trigger packets plus invalid-input rejection were transport-checked; gameplay
firing, aim and lifecycle acceptance remain open.


## Native loading acknowledgement follow-up

Astra/xhigh verified that CMSLoading6C is the confirmation-readiness latch, not an
acknowledgement. Accepted MC_OK closes the current menu and invokes native
UnblockWorldStart, clearing simulation48. The latch need not clear. The observer
now samples/rechecks current simulation and its blocked flag, exclusive input
and the dispatcher enable gate; the pre-post receipt records the actual tuple.
The launcher distinguishes posting, observed native transition and actual scene
readiness. External repeated reads do not pin native objects or prove that the
injected Enter specifically caused the transition.

The latest bounded launch174743 found ready CMSLoading and enabled dispatcher,
but exclusive input remained0 after four owned normal SetForegroundWindow
attempts. No Enter was posted; normal null-interface shutdown was observed.
The finite follow-up184951 sampled three stable native instance/window tuples:
input enabled1, running1, simulation present, blocking flag0, Core foreground0
and exclusive0. All three instance aliases and stock window/canvas associations
agreed. Probe185155 found the actual host window visible but minimized. One normal
owned ShowWindowAsync(SW_RESTORE) request produced foreground1, Core foreground1,
exclusive1 and minimized0 in two subsequent stable observations. The immediate
SetForegroundWindow return was0; subsequent native state, not request success,
establishes activation. No Enter/controller input was sent in either finite probe;
both shut down normally with no cleanup errors. The launcher now resolves the
native host window afresh, validates process incarnation/ownership, restores it
when minimized, and retains the original readiness guards before confirmation.
Source/native dual probe preparation proceeds independently. Full-mod acceptance
is open, and no direct native method invocation or memory write bypass is added.

Actual native eye captures now precede optional per-pose desktop captures. A
desktop mapping/capture error is recorded separately and cannot discard valid
native camera/pixel/projection evidence. Desktop occlusion comparison still needs
its own successful images; it is not accepted when those captures are absent.
See NATIVE_DUAL_PROBE.md for the subsequent unchanged-inventory firing sequence.

Optional native_input_probe selects read-only-three-samples or
owned-activation-three-samples. These bounded diagnostics record at most three
ready native tuples and return through normal cleanup. They never send loading
confirmation or controller inputs. The activation variant requests the same
ordinary owned-window restore/focus once; it does not synthesize WM_ACTIVATEAPP
or change native flags. Repeated external reads do not pin object lifetime.

## Post-repair seven-pose run and native firing prerequisite

Run190429 on source725425de9c8dd4315399e65bf7c28c353bc1390e176747935b4477a4f2d0eb48
completed all seven actual native eye pairs, matching cameras/world/UI/projection,
with normal isolated shutdown and no cleanup errors. The exact Jungle receipt is
10562049bytes, SHA256106f9e287110988720cc717aaaa22190cafcf90fb184fd5a83e9d467d93165df.
All seven attempted desktop captures failed and remain separately unaccepted.
The repeatable first-person depth regression passes; actual right-eye images were
inspected again. Native camera changes and static patch disparity are recorded
in IMPLEMENTATION_STATUS.md and PC_GAMEPLAY_READINESS.md.

The earlier run185525 exited before native scene receipt/host readiness and logged
EXCEPTION_WINE_ASSERTION while using ntsync. This is not a demonstrated mod cause.
The installed Proton wineserver contains PROTON_NO_NTSYNC; Valve's
[server source](https://github.com/ValveSoftware/wine/blob/proton_11.0/server/inproc_sync.c)
checks it before choosing the synchronization backend. Optional per-process
proton_environment={"PROTON_NO_NTSYNC":"1"} selected fsync in run190429. Global
runtime/kernel/security settings are untouched; other applications are not affected.

A background loading screen can precede the Present that starts the host. The
launcher therefore permits its same bounded native-owned restore/focus path
before that later channel exists. It still requires native loading readiness,
process incarnation, exact native host-window association, and ownership. Loading
confirmation remains guarded through the actual channel. No unconditional host
start or synthesized native activation event is introduced.

Config capture_desktop=false omits optional external desktop captures, retaining
actual native eye capture and its admission checks. This is useful for focused
controller probes; it supplies no desktop-comparison acceptance. Before native
firing probes, normal owned activation is rechecked separately from XR focus;
background/minimized gameplay images alone do not establish native input readiness.
Native initial Jungle weapons are ID1 Zap guns, not Colt. See NATIVE_DUAL_PROBE.md
for the required charge/release-cycle adaptation and unresolved actual firing gate.
