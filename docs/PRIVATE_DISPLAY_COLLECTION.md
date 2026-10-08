# Private neutral collection prerequisite

This procedure collects missing native ID1 event/draw/pose evidence. It is not a
completed VR feature or a substitute for hardware, firing, vehicles, physical
grasp or multiplayer acceptance. Proprietary fixtures/captures/settings stay
private. Do not copy this procedure into a production online launch.

The first approved private Xvfb attempt crashed before scene load. Later setup
and native diagnostic attempts failed before SS2 launch; all records remain
private. The corrected prerequisite has now passed with actual native D3D9:
60 Hz desktop/adapter/swapchain at 1280 × 720, HRESULT 0 raster query and normal
native/owned cleanup. The old attempted collector commands remain historical.
A zero-current-refresh division in the installed DXVK raster-status query explains
the original Xvfb failure; forceRefreshRate enumeration filtering does not fix
that current-mode query. Never fall back to the user's display.
The source reference is [pinned DXVK D3D9 swapchain code](https://github.com/doitsujin/dxvk/blob/70d7508c01201ed3d4bfb33da42ba834eafe3857/src/d3d9/d3d9_swapchain.cpp).

The incremental alternative uses headless Weston with Xwayland and a fake seat.
A display-only check produced a positive current refresh. A later deeply nested
fixture exceeded the native UNIX socket path limit before its collector could
start. The launcher now keeps a short owned runtime directly beneath the sealed
private root, validates the byte limit before process launch, and binds that
actual runtime in its parent certificate. Actual Win32/D3D9 readiness now passes;
gameplay acceptance remains separate. The source probe queries desktop,
adapter and swapchain modes, declines unknown/zero/default frequency, then calls
one raster query. Successful return is only a display prerequisite.
[Microsoft's swapchain API documentation](https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3dswapchain9-getrasterstatus)
describes the raster query and its failure contract.

Prepare a fresh independent lab, known private profile and prefix before any
runtime launch. Pin the package fingerprint, all payload hashes, executable
probe hash, readers/evaluator, neutral config, settings/profile bytes and exact
user-link ledger. Mute only the five documented game audio values. Preserve the
original installation and all earlier labs; a failed prefix is not resealed into
a fresh initial fixture. The guide in that private lab supplies exact identities.
The x86 probe is a private diagnostic executable, never a public game payload:

```sh
i686-w64-mingw32-g++ -std=c++20 -O2 -Wall -Wextra -Werror \
  -static tools/display_timing_probe.cpp \
  -o "$PRIVATE_LAB/prerequisites/display_timing_probe32.exe" -ld3d9 -luser32
python3 tools/private_display_lab.py --config "$PRIVATE_LAB/config.json" --check
```

No process starts with `--check` or the default mode. Before collection,
`--display-only` exercises the actual private compositor/X11 ownership pipeline
and closes it without importing the collector or launching Wine/SS2. Xwayland
is started by a read-only X client query before its process identity is checked.
This preflight uses a separate receipt and cannot certify native/game readiness. A reviewed, separately
authorized replacement uses the same command with `--run`. It consumes an
exclusive attempt record even if startup fails. Weston starts its own Xwayland;
the child certifies its compositor parent and Xwayland identity, removes desktop
selectors and forces the collector onto that private X11 display. This isolates
the view/input route; it is not a security sandbox. No desktop focus, synthetic
movement/firing, global runtime changes or real device interaction occurs.

The timing probe runs inside the already validated collection extent, before
Monado or SS2. Logs remain private. It emits schema1 stage/mode/raster/HRESULT
fields to an exclusive bounded `display-timing-native.json` Win32 file after
releasing native resources/window. The probe uses the existing `runinprefix`
diagnostic route with probe-only Proton logging disabled; subsequent game
launch/logging remain unchanged. Console output is retained for diagnosis and
cannot substitute for the native receipt.
`display-timing-process.json` records exit/identity after exact retirement, even
when native output is absent. An accepted result is saved to
`display-timing.json`. SS2 loads the verified `+level` scene using the existing
neutral collector. Native world/eye/event receipts, raw grip/aim values and draw
channels are retained in the existing run directory. Existing offline readers
produce replay/reference measurements. No idle result certifies native melee
release or a physically correct grasp.

The private display record contains compositor/child identities, xrandr output,
explicit collector outcome and cleanup result. Normal compositor exit alone
cannot certify collection. Success requires exact native scene receipt, admitted
both-eye evidence, source-correlated geometry/replay, normal native isolation
shutdown and no exact-owned survivors. Original user-data preservation must also
be checked. Failure preserves diagnostics and requires a concrete next diagnosis;
never mask it with fallback to the user's display. The collection has a 180-second
shared readiness budget, bounded offline processing and a 285-second outer budget;
interrupt/failure cleanup may add up to45seconds. No unattended retry.

## Prerequisite-only diagnostic extent

A fresh configuration may seal `display_prerequisite_only: true`. The launcher
requires `--prerequisite-only` for that operation and rejects `--run`; default
and `--check` still create no process. It reuses the existing validated run and
returns immediately after the pinned D3D9 prerequisite, before Monado configuration
or game launch, even on success. Existing finally owns cleanup and the result.
The private display child does not import the idle collector and requires explicit
false game/Monado launch flags; geometry readiness stays false.

The diagnostic uses a separate attempt marker/output directory. Its native
readiness budget is45seconds in the prepared fixture, parent budget75seconds,
startup budget5seconds and outer failure cleanup up to45seconds. Allow2minutes15
seconds. The exact private guide supplies config/receipt/probe hashes and command.
This source/offline path has Astra/xhigh scoped GO and16 mocked control-flow/
ownership checks normally and with optimized Python. The actual native prerequisite has passed as described above. No alignment,
scope, steering or hardware pass follows from that graphics result.

A diagnostic success uses result `display_prerequisite_only`, simulation/hardware
false, game/Monado launch requested false and cleanup_errors empty. Native online
shutdown observed false is expected because SS2 never loads. The subsequent
geometry collector remains a separately controlled actual-game operation.

## Borrowed-window activation evidence

The replacement neutral capture loaded the verified scene and started the host,
but remained at native loading stage 3 with no Win32 host foreground. Its four
requests did not establish four successful activations: the known-channel branch
discarded code 8 and omitted the optional native receipt. The capture ended with
normal native isolation shutdown, clean owned retirement and preserved user data.
No weapon geometry was captured; the single SS2 replacement approval is consumed.

The source now passes a unique receipt for each existing bounded startup request.
`stock-focus` records process/incarnation, native association and target guards
even when rejected, plus target and foreground HWND/owner/thread before and after
the existing restore/foreground request. Code 8 and an absent receipt are explicit
evidence, never successful activation. The same four-request cap, deadline and
native loading/menu gates remain. No native state or loading flags are written.

A shared borrowed-window diagnostic is also used by the nongame D3D9 probe on its
own visible private window after verifying its process/thread/title/root. Its
`window_activation` receipt reports ownership, visibility, restoration and actual
foreground result before resource/window retirement. The probe cannot select a
foreign target. Graphics readiness is independent of activation success; this
check cannot certify SS2 foreground, gameplay, geometry or grasp. The shared header
is included in the exact prepared tool seal. Comprehensive gameplay stays
primarily user-operated, and repeated nongame private display checks are approved.

A separately sealed console/GUI comparison is available only in prerequisite-only
mode. Its owner probe must be PE32 GUI subsystem 2; two exact observer binaries
use the same source with console subsystem 3 versus GUI subsystem 2. The owner
holds its tool-created window for eight seconds, samples foreground every 20 ms
and pumps messages. Each observer reads foreground after 250 ms, including its
own PID/thread and console HWND. The harness preserves independent owner
observations before, during and after each child. It never reactivates the owner
during measurement. An absent target baseline before either variant, expired
owner opportunity or missing fresh bracketing cannot prove a difference; the
comparison remains inconclusive. This diagnostic stops before SS2/Monado and
cannot establish the historical SS2 cause. Stop this branch after a valid
comparison; an actual receipted neutral SS2 observation remains a separate gate.

The actual reviewed comparison completed with both console and GUI observers
seeing the exact same owned foreground, confirmed independently throughout their
entry-to-observation intervals. No subsystem difference was demonstrated. This
hypothesis is closed; changing native activation policy or selecting another
observer subsystem is not justified by this result. The remaining discriminating
observation is actual SS2 with the new per-guard receipts, under separately
renewed bounded authority. No native geometry or physical-release acceptance
follows from this nongame diagnostic.

## Eager Monado window-order diagnostic

The default prerequisite-only extent still stops before Monado and SS2.
A separately sealed `monado_window_order_probe: true` extent may start the same
private graphical Monado service, but always ends before SS2. It requires the
installed `XRT_COMPOSITOR_DISABLE_DEFERRED=1` option and a pinned native read-only
X11/XRes helper. Compile that helper with:

```sh
c++ -std=c++20 -Wall -Wextra -Werror tools/private_x11_windows.cpp -lXRes -lX11 -o /private/path/private_x11_windows
```

The service's live PID/start incarnation must match the XRes resource owner of
one exact `WM_NAME=Monado` client window, with InputOutput class, positive size
and IsViewable mapping. The title follows eager window/swapchain/renderer
initialization in the audited runtime. A socket or guessed `_NET_WM_PID` does
not satisfy this barrier. It does not prove submitted frames.

The diagnostic first requires Monado's XID to be active, then runs the same
GUI D3D9 probe on the private display. Native foreground receipts and independent
XRes owner/active-window evidence must agree for the later probe. Its normal
resource/window retirement must complete; the same Monado XID must remain
viewable afterward. All records remain private. The existing process owner,
PTY, prefix bootstrap and cleanup paths are reused. Unknown identities or an
unproved transition fail the diagnostic; they never justify changing native
loading guards. Ordered runs allow the sealed runtime budget plus 45 seconds
for existing cleanup and wrapper shutdown; normal prerequisite budgets remain.

The user now permits necessary private neutral SS2 captures within the explicit
startup/idle-geometry scope. Consumed one-run limits above are historical.
A negative ordering observation must produce a new evidence-based diagnosis,
not an identical capture retry or an assumed alignment/release state.

The actual Astra-reviewed ordered nongame run passed all those receipts and
owned cleanup, with protected files unchanged. SS2 and its collector were not
launched. Native and X11 observations are sequential; no sustained agreement,
pure map-order causality or SS2 repair is inferred. A subsequent fresh neutral
SS2 capture must independently establish gameplay readiness and geometry.
24 private-display regressions and22 runtime-evidence groups pass normal/
optimized Python; native helper/probe/observer builds use warnings as errors.
