# Private neutral collection prerequisite

This procedure collects missing native ID1 event/draw/pose evidence. It is not a
completed VR feature or a substitute for hardware, firing, vehicles, physical
grasp or multiplayer acceptance. Proprietary fixtures/captures/settings stay
private. Do not copy this procedure into a production online launch.

The first approved private Xvfb attempt crashed before scene load. A later
approved private native prerequisite ran but yielded no admitted JSON result,
so SS2 was not launched. That attempt is consumed, and no alignment data exists.
The old collector command is historical; do not execute it. Current stdout/exit
reporting fixes are source/offline only and need review before another observation. Inspection
identified a zero-current-refresh division in the installed DXVK raster-status
query. A forceRefreshRate enumeration filter does not fix that current-mode
query. Preserve the failed run; do not retry it or switch to the user's desktop.
The source reference is [pinned DXVK D3D9 swapchain code](https://github.com/doitsujin/dxvk/blob/70d7508c01201ed3d4bfb33da42ba834eafe3857/src/d3d9/d3d9_swapchain.cpp).

The incremental alternative uses headless Weston with Xwayland and a fake seat.
A display-only check produced a positive current refresh. A later deeply nested
fixture exceeded the native UNIX socket path limit before its collector could
start. The launcher now keeps a short owned runtime directly beneath the sealed
private root, validates the byte limit before process launch, and binds that
actual runtime in its parent certificate. Actual Win32/D3D9 and
game startup on that display remain unverified. The source probe queries desktop,
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
  -static-libgcc -static-libstdc++ tools/display_timing_probe.cpp \
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
fields to `display-timing-probe.log`; probe-only Proton logging is disabled to
keep stdout on that handle, while subsequent game logging remains enabled.
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
