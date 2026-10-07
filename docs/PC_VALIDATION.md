# PC validation — 2026-10-07

## Source and isolation

PC development begins from public commit
`8d172189aa0fbddadc1c1786eedbc38db8368487`, tree
`07b2508387d1a367107673020e6162eba9e17fd4`. Both queued source checkpoints are
present. The complete28-commit history passes the empty-email gate. Actions is
disabled; workflow files are absent. No old ancestry was imported.

The original installed-game checkout, seven unaccepted melee files and user data
are untouched. A separate complete game copy and new Proton prefix are used for
runtime checks. The original config was snapshotted with `um backup` under the
local name `ss2vr-pc-original-config-20261007`. Private paths, logs, crash dumps,
screenshots and game assets remain outside public source. Installer preflight,
installation and hash-checked removal operate only on the isolated lab copy.

## Local build and offline results

GNU MinGW16.2.0 builds the x86 proxy/server and x64 host/official loader. Clang
22.1.8 builds the existing native-finally C boundary. All59 portable groups pass
locally in Debug and Release with pinned Python dependencies. An initial failure
was missing Capstone in the test environment; pinned dependencies and an absolute
PYTHONPATH fix the environment without changing source or weakening the check.

Actual product/layout verification passes for IPC9/wire6. Initial local build
input fingerprint: `ac725760615945bf6b81947bb6d0f68dda69e3f3f40092a5ffac5fddb51fbb07`.
This identifies source/build inputs, not runtime success. Product SHA256 values:

| Product | SHA256 |
|---|---|
| x86 d3d9.dll | `cc9f75315df70f780a94e8d7dbcb481086c819c357fd1213ce6d63a786b6c74b` |
| x86 SS2VRServer.dll | `9bb0b93c1e548b6a3b28eff0c15d26d2f112cc5f364ab1f6cb95a84ae9de2a23` |
| x64 ss2vr_host.exe | `e0a1cd299815c4e164ec443d54515c11f46667dffe616fca8d1eddb1fa3802ed` |
| x64 official loader | `bb011caa82528c541a73967ce6408f82198ff4fd0358b38b54884719d863bd1d` |

A matching development package was staged privately and passed collision/hash
preflight before installation into the lab. It is incomplete and not a playable
release. Later source edits require rebuilding all products before repackaging.

## First actual Proton startup observation

Proton Hotfix `hotfix-20260828-ptr-x86_64` (prefix11.0-100), a new prefix and an
isolated1280x720 Xvfb display were used with the native D3D9 proxy override.
The native game reached Direct3D context creation, then displayed its crash
dialog and wrote a minidump. The captured image was inspected. No VR host log
was produced. This is a startup **failure**, not a headset or rendering pass.

The exact lab game PID was stopped; the installer verified and removed only
unchanged mod-owned payload. Stock reproduced the same exception. Astra/xhigh
identified integer divide-by-zero0xC0000094 at DXVK3.1 x86 D3D9 RVA0xCC34B:
GetRasterStatus divides1,000,000 by a zero D3DDISPLAYMODEEX refresh rate. The stack
reaches native GfxD3D/Engine; application proxy and system D3D9 load at distinct
addresses. This does not establish a mod regression or loader recursion.

The real Xwayland display reports approximately75Hz. Stock proceeds beyond the
failed point and completes native boot there. Restoring the exact verified proxy
package in the same lab/prefix/display also completes boot and logs native hooks
attached. A window-only boot config was created only in the lab. The original
config syntax warning persists on both controls; it is not claimed as the crash
cause. The game remains in the background. A captured window was black, so a
rendered-menu pass is not claimed from boot logs. No host process/log is observed
yet; a bounded lifecycle review and foreground observation remain open.

No active Monado/SteamVR/WiVRn service or USB headset was observed at initial
inventory. A Vulkan-capable RTX4090 is available. Hardware/runtime availability
is being clarified; no HMD/controller frame, network session, vehicle drive or
scope/UI visual acceptance has occurred. A virtual display launch cannot prove
headset comfort, performance or tracking.

## Remaining goal

Multiplayer body/origin settlement, physical melee consumption/lifetime and
observer delivery, wider vehicles including actual wheel grabbing, remote-head
worker/model lifetime/enablement and final full-project review/package readiness
remain unfinished. Inactive helpers are not completed features. The rejected
melee patch remains unapplied and native160/348 remain manual-only.

## IPC10 source checkpoint and focused desktop test

The connected weapon-wheel squeeze cancellation repair is documented in
SQUEEZE_ADMISSION.md. The earlier new-test Release result lacked assertion
coverage; target ordering was corrected, a compile-time NDEBUG guard added, and
all60 Debug/Release groups rerun successfully. UBSan passes. A deliberately
NDEBUG-disabled compile is rejected. Astra/xhigh approved the bounded follow-up.
All products rebuilt together. Artifact, primary, zoom, native-finally,
client/server RPC and head-volume compiled gates pass. Artifact/head JSON labels
both report10 after removing the old hardcoded head label.

Build-input fingerprint: `d2e0be422d2dd16d0d64a258eecc6b2ffc7bb07324d31137940a4ee4cf112dcc`.
Both architectures agree on IPC10/wire6: Input272, Request440, Shared83887840.

| Product | SHA256 |
|---|---|
| x86 d3d9.dll | `aa23c339927a33eaa6ce4fe4be652cc3ee1f181932167f64b081aa098bcb4814` |
| x86 SS2VRServer.dll | `e1a1be2bd0b8c77ed9f518dd2a25449e776b55b45da9a0961642288adf38763f` |
| x64 ss2vr_host.exe | `819e044f9ec0e585162d970f6dbd386b5ac95255e4f483bbf7e71761e7109636` |
| x64 official loader | `bb011caa82528c541a73967ce6408f82198ff4fd0358b38b54884719d863bd1d` |

The private initial IPC10 test archive is SHA256
`2941f8e5cb263bec86e0cc6f0e1e52c36357281d051c171dc44fbf05bf560ee2`.
It predates the head-report label/documentation correction; its tested binaries
have the identities above. It is not a public/test-ready release. Later packaging
must use a fresh directory and corrected source/docs.

The user explicitly permitted brief game focus. The focused proxy run rendered
the native Serious Sam2 main menu (v2.224.00:747079) correctly. A compositor
capture of the verified game window was inspected; the earlier direct-window
black capture was not accepted as rendering proof. No unrelated window was
captured or input sent. Native boot and hook attachment succeeded. All logged
MinHook create/enable results are successful. Device creation and the first
ordinary RT0 occurred before hooksReady on the recorded creation thread/device.
Despite a visible focused menu, there is no intercepted device Present,
post-ready RT0, channel creation or host-launch event. The exact lab PID was
stopped at the end of the bounded test and logs preserved privately.

This is a **desktop startup/menu pass**, not a VR pass. The pinned native renderer
has separate device/swapchain presentation routes; exact recurring route and
actual canvas ownership remain to prove. No unconditional host-start workaround
was added. Astra/Max is reviewing the smallest actual presentation-owner adapter,
including the implicit16x16 device buffer versus native window canvas.

No real HMD/controllers, stereo submission, scope/UI-in-headset, gameplay or
multiplayer runtime acceptance exists. Headset/runtime preference was pending at this test;
WinBoat is stopped and Windows GPU/OpenXR capability is unverified. Do not change
passthrough/security/credentials to make that platform appear tested.

## Confirmed target matrix and simulated baseline

The user subsequently confirmed SteamVR + Steam Frame and Envision/Monado +
Bigscreen Beyond as the required targets and explicitly approved simulated
Monado testing. Brief game focus is already authorized. Windows and Linux/Proton
remain separate platform requirements; actual hardware acceptance is separate
from simulation. No target device was detected during initial inventory.

The existing Envision simulated profile was started as a private per-process
service with a private runtime/socket directory. The first ordinary-pipe launch
failed at stdin epoll registration before compositor startup. A PTY launch
resolved that failure without global runtime/security changes. The service
created a Qwerty HMD and both synthetic controllers, selecting the RTX4090.
Runtime identifies itself as Monado25.1.0, build
`v25.1.0-710-g735e29e4e`.

The native Linux Vulkan2 `hello_xr` baseline created two896x1007 color/depth
swapchains (chosen color format43), enumerated View/Local/Stage spaces and
transitioned IDLE → READY → SYNCHRONIZED → VISIBLE → FOCUSED. Grab bindings
identify left/right Microsoft Mixed Reality Motion Controller Squeeze. The
runtime reports synthetic orientation/position tracking capabilities false;
this is not physical sensor tracking. No per-frame submission result or
compositor image has yet been accepted. This is neither the actual Windows
D3D11 host nor the x86 proxy/CPU-transport test.

The actual Proton host must use the Windows wineopenxr manifest resolving
`C:\\windows\\system32\\wineopenxr.dll`; the native Monado shared-library
manifest belongs to the Unix loader. WineD3D is not an equivalent D3D11 host
test: Proton's bridge requires DXVK Vulkan interop. Keep these runtime contexts
distinct and record runtime/Proton versions, enabled extensions, selected GPU,
swapchain formats, session transitions, frame results and both hand profiles.

Steam Frame's optional native extension/profile must be enabled/suggested only
when supported. Its documented fallback is Frame → Generic Controller → Oculus
Touch. Index equivalence must not be inferred. Native Frame left inputs are
dpad/view; right inputs include a/b/x/y/menu, so left X/Y paths are invalid for
that profile. Existing Touch bindings have a documented fallback route. Beyond
is an HMD; actual controller model determines its interaction profile.

Primary references: [Frame inputs](https://partner.steamgames.com/doc/steamhardware/steamframe/input),
[custom-engine fallback](https://partner.steamgames.com/doc/steamhardware/steamframe/engines/custom),
[Proton D3D11 bridge](https://github.com/ValveSoftware/Proton/blob/proton_11.0/wineopenxr/openxr.c),
[Proton loader](https://github.com/ValveSoftware/Proton/blob/proton_11.0/wineopenxr/openxr_loader.c),
[Monado development](https://monado.freedesktop.org/developing-with-monado.html).

## Actual D3D11 host and native chain probe

The tracing-only source received Astra/xhigh follow-up GO after correcting
completion COM queries, failure-log bounds and duplicate creation identity.
The earlier Astra/Max design review remains conditional for the production
adapter. Current-turn settings were verified for both PC reviews. See
PRESENTATION_OWNER.md for dispositions and unresolved lifecycle gates.

All products rebuilt together; all60 portable groups, artifact/layout and
native-finally gates pass. Compiled chain callbacks have24-byte stdcall cleanup.
Source/build fingerprint:
`13dda1cfc9e9a8249d8aa6f816b92b59b5eea2764892b04d2a24ecef37838748`.

| Product | SHA256 |
|---|---|
| x86 d3d9.dll | `9d7c749e4900bb6b2d1311a005fec3e954fb9adc55c61325c3471f24d7517839` |
| x86 SS2VRServer.dll | `4d1f501f2bd3a2967b504f59b114c691f33002320459a0b569fd12ef3a35f840` |
| x64 ss2vr_host.exe | `31d40ce94d74670067a1d05af129da6270af269ef4affb3fdbc99f13dd1199b2` |
| x64 official loader | `bb011caa82528c541a73967ce6408f82198ff4fd0358b38b54884719d863bd1d` |

The tested private development archive is
`e4b375a50ce2a4562be7ea119f9a8766b66806a4cab5a324a4505d29c53e64b1`.
It predates this results documentation and is not a playable/public release.
Installer refused a residual generated host log; the log was preserved outside
the lab mod directory before hash/collision preflight and installation succeeded.
No unrelated collisions were overwritten.

The focused game probe observes eight recurring additional-swapchain calls
after hooksReady, at the pinned native presentation return site. Backbuffer is
1280x720 X8R8G8B8, non-MSAA and identical to RT0; destination is the foreground
game window, device/thread agree with creation, original HRESULTs are success.
Native depth is3440x1440 D24S8. The inspected compositor capture shows the
rendered startup scene. This probe did not establish a new menu capture or launch
a host from the game. The exact lab process was stopped after the bounded test.

A separate private synthetic IPC fixture launched the **actual matching x64
mod host** under the same Proton Hotfix/prefix and private simulated Monado
service. Native `XR_RUNTIME_JSON` selected Monado; `WINEXR_RUNTIME_JSON` selected
the Windows wineopenxr manifest. Proton's existing Steam helper initialized
vrclient. No global runtime registration/security change occurred. A read-only
Astra investigation supplied the environment plan; its effective-settings
introspection was unavailable, so that investigation is not formal source GO.

Observed host runtime: Monado25.1.0, build `v25.1.0-710-g735e29e4e`, required
`XR_KHR_D3D11_enable`, NVIDIA GeForce RTX4090 (10de:2684), feature level12.1.
Session transitions IDLE → READY → SYNCHRONIZED → VISIBLE → FOCUSED. Both hand
profiles are `/interaction_profiles/microsoft/motion_controller`. Two1024x1024
wheel swapchains, a1024x256 HUD chain and a640x360 synthetic menu chain use
DXGI_FORMAT_B8G8R8A8_UNORM (87), three images each.

At the final periodic snapshot:1201 successful xrEndFrame returns, including
300 submissions containing the synthetic menu quad. These new counters count
successful API returns after submission and distinguish menu layers from the
existing world-pair counter. Fixture snapshots show focused/head/both-hand
validity and advancing input sequence; no physical movement/button/haptic test
was performed. The fixture requested shutdown, host exited0 and tracking was
invalidated. Native game world/CPU transport counters remained0 by design.

This proves actual-host D3D11 runtime/session/swapchain/API submission and
synthetic input bring-up. It does not prove visible compositor image fidelity,
native proxy-to-host CPU readback, gameplay stereo, Frame/Beyond controls or
hardware acceptance. Those gates remain open.
