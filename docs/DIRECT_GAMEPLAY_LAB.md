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

The user renewed source-only public publication authority; the prior hold is historical. Observer compiles; no direct no-menu gameplay run is accepted yet. A strict scan found four different Jungle world providers, including Sam2Renovation.gro, so the harness records providers and requires a hash of the successfully opened native scene stream. It never changes archive precedence to obtain a desired result. The existing files remain preserved. Native Steam initialization/shutdown can write and delete remote profiles; no supported cloud-disable cvar was proved. A narrow opt-in process-local online-init barrier is implemented and reviewed, preserving Proton VR bootstrap and default production behavior. The scene observer admits the native loading worker, borrows synchronously, restores native position, and emits the digest only after restoration. Native-exception restoration remains outside normal-path runtime acceptance.

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

An actual run now recorded continuation followed by completed native Jungle
loading in6.96 seconds, local simulation and thirty matching neutral OpenXR head
observations. The first-eye executed-view gate rejected the pair; no native world
images/projection stereo or translated/rotated camera acceptance followed.

The observer is built as a GUI-subsystem executable to avoid a console stealing
the game's focus. Compile with MinGW C++20, static runtime, -municode -mwindows,
-Isrc and -lshell32. It reports status through exclusive private JSON files.
The actual absolute deadline is configurable up to180 seconds; cleanup has a
separate common20-second budget. All process retirement remains ownership checked.
