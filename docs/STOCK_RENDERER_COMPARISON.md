# Stock-renderer reference and terrain-occlusion investigation

The reported desktop trees/waterfall-through-terrain defect remains open. Native
world/UI stereo and head translation/rotation were demonstrated in run154502;
that does not establish desktop restoration, terrain occlusion or full equivalence.
All runtime assets, captures, settings and operational records stay private.

## Implemented comparison boundary

SS2VR_LAB_STOCK_RENDER=1 is latched in the common D3D9 proxy initializer before
D3DPERF forwarding can create a VR worker. It skips the worker and device-hook
roots and forwards original D3D9. It requires the separate private online-isolation
opt-in and rejects the unvalidated9Ex route. The isolation owner and completed
installation are separate; partial/reentrant installation is rejected. Five native
binaries are checked before startup, with the normally loaded Sam2Game module
checked again at the native scene-stream boundary. Default production behavior
and multiplayer are unchanged. This is a stock renderer with fixture hooks,
not a claim that the entire process is unmodified stock.

The launcher now supports stock readiness, guarded native loading continuation,
exclusive owned-window capture and exact process-incarnation shutdown without an
OpenXR host/channel. The observer uses read-only process/module inspection;
subsequent messages require the same full game path and creation time. No passive
camera/getter hooks were added. All61 Debug groups and all products rebuilt for
the boundary. Product source fingerprint for the two completed stock probes:
a355862da6fe193cd9b461670763797754788df91c086e9f6c99fd8cbcbef2d1.

## Astra native camera disposition

A fresh Astra/max read-only investigation verified the owned native binaries.
Explicit current-turn Astra/max tags were confirmed locally; backend introspection
was unavailable. The ordinary Sam2Game Render3D caller943CC–943D2 obtains the
camera through slot5F4 into a temporary;945A4–945A9 copies seven words into the
persistent module value403084. Engine current view/projection values are2E6408
and2E6398; Gfx69AF–69BC copies the native projection into the latter.

| Recommendation | Disposition |
| --- | --- |
| Try external persisted camera/view/projection observations first | Adopted. The observer repeats the values, checks finite normalized pose, clear menu, inverse-view agreement and perspective shape. |
| Add passive getter observers only if the external route proves inadequate | Deferred. No getter hook or new renderer policy was introduced. |
| Do not promote repeated reads to frame/image ownership | Adopted. Repeated equality does not exclude ABA, stale state or another rendering pass. Captures remain camera-comparison-unverified. |

The local-transport witness separately reads Engine2EB6D8 and the expected
Sam2Game29D270 interface table twice. It certifies sampled transport identity,
not actor application/lifetime, a fresh frame or subsequent network correction.

## Actual stock probes

Both runs loaded the native Jungle stream10562049 bytes with SHA256
106f9e287110988720cc717aaaa22190cafcf90fb184fd5a83e9d467d93165df,
restored its original position0 and observed null-online normal shutdown without
cleanup errors. Neither launched a VR worker/host or installed graphics hooks.

Run164547 captured the scene, but camera orientation changed between observations;
its view cannot be compared with the earlier VR baseline. Cause was not proved.
A separate Astra/xhigh native input audit verified the existing FLOAT assignment
+inp_fMouseSensitivity0 before +level: Engine2C4214 scales native mouse axes
without a minimum clamp. Keyboard/buttons and the loading Enter route remain.
It also suppresses menu cursor motion, so this is an isolated fixture setting.

Run170604 used that verified assignment. External readings confirmed mouse_zero1;
the camera quaternion remained[0,0.790689647,0,0.612217188] across the capture and
matches the neutral VR baseline orientation. Position differed by less than1mm.
The first view/projection observation matched the native world camera; the second
sample caught orthographic UI state. The scene image was inspected, but these
samples do not establish exact draw/present ownership. Result remains
stock_image_captured_camera_unverified, not a terrain-regression pass.

## Next finite depth probe

Lab-only bounded device observations now record depth enable/write/function,
alpha/blend/stencil, viewport/range and actual color/depth dimensions/formats/
multisampling at entry, before/after each world pass, after each marker postlude,
and before/after desktop rebuild. They change no renderer policy. Each eye already
clears its depth attachment. The marker postlude calls original native depth/write
disable dispatchers; surface restoration alone does not restore those states.
Whether the next world pass reinitializes them is under independent bounded Astra
investigation and actual runtime observation. No depth repair is claimed yet.

Do not use raw D3D state restoration without checking native caches; do not hide
terrain, vegetation, waterfall or markers. Match scene, stable camera/settings and
stock/mod desktop, retaining both actual VR-eye comparisons. The user's exact
reported location remains unspecified. Remaining full-mod features and device/
platform/network acceptance are unchanged.

## Reproduced right-eye failure and measured depth-range divergence

The user clarified that the left eye is correct and the right eye shows the
terrain leak. Private run171315 reproduces a waterfall over the foreground statue/
hut and extra trunks over grass/road only in the right eye. The matched stock
starting view and mod desktop remain part of comparison; no effect was hidden.

Stage-only samples did not distinguish the eyes. Actual forwarded world draw
observations show depth testing/writing enabled for opaque and alpha-tested draws
in both eyes. Run171915 additionally sampled viewport depth range at those draws:
left opaque137 calls and all144 alpha-tested calls used0..0.9, with no0..1 calls.
Right opaque105 calls and alpha-tested132 calls used0..1, while later transparent
calls mostly used0..0.9. Identical first-pair geometry counts rule out an entirely
missing second world pass. The desktop redraw also mixed these ranges. This is a
measured device-range divergence, not yet a completed fix or cache diagnosis.

The mod directly resets each eye viewport to0..1, and restores the saved desktop
viewport afterward. A bounded Astra native cache/dispatcher audit is checking the
minimal correction: preserve the existing depth endpoints while changing target
and viewport dimensions. Raw stateblocks and a replacement renderer remain
unjustified. Scope scratch and desktop restoration must follow the same boundary.

One earlier diagnostic run171619 captured seven poses but exhausted the bounded
camera/pair/projection receipts before pitch/roll. Its final assessor correctly
rejects full correlation. Run171315 has all seven correlated;171915 is an explicitly
baseline-only depth probe. Diagnostic capacity is being adjusted; none of these
limits is a graphics or hardware acceptance pass.

## Astra cache review and implemented correction

Astra/max independently verified GfxD3D56A0:571D–5734 compares the cached native
endpoint bits and returns immediately when they match, without obtaining or
updating the device viewport. A changed range updates the cache and device
viewport at5772/5790. Engine exports the dispatcher pointer2E626C and cached
near/far values2E5BF0/2C3E3C. Native viewport geometry setup10F0 preserves these
endpoints before SetViewport119B. Main spot-checked the load-bearing comparisons
and geometry setup. Explicit local current-turn Astra/max tags were verified;
backend introspection was unavailable. No new native investigation is required
for this bounded repair.

Microsoft documents the target-change viewport reset and compatible attachment
size requirements in [SetRenderTarget](https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-setrendertarget).
A larger native desktop depth attachment is allowed, so that measured size
alone was not a defect diagnosis.

| Review recommendation | Disposition |
| --- | --- |
| Preserve actual depth endpoints before target changes, applying destination geometry afterward | Adopted for each eye and scope scratch. Invalid/unreadable endpoint observations reject setup. |
| Restore desktop geometry/scissor without overwriting current native depth endpoints | Adopted. Existing cleanup/quarantine and once-only desktop rebuild remain. |
| Keep native enable/write/comparison, root partition and cache ownership | Adopted. No stateblock, cache mutation, force-enable or renderer replacement. |
| Verify baseline → preservation → baseline, then restore fixed build | Completed baseline171915 → fixed172303 → old-build counterexample172428. The old build restored the early right-eye0..1/late0..0.9 divergence and the user saw the artifact return. The fixed product is restored for the full seven-pose run. Captures remain private. |

Run172303 source fingerprint
f945770801a867e9ec9713066ab9b4e200e1b578cbf307851b4055520b8b391f
uses the correction. Both actual eye images were inspected: the waterfall over
foreground statue/hut and the extra trunks over grass/road are absent in the right
eye; the left eye remains correctly occluded. Forwarded opaque/alpha-tested draws
use0..0.9 in both eyes and desktop. Native child weapon ranges remain distinct.
The same-request complete world/UI/camera/projection receipt accompanied the
capture;266 distinct neutral complete pairs preceded it. Normal null-interface
shutdown and no cleanup errors were observed. The user independently reports
that the issue appears fixed. This accepts the bounded Jungle reproduction, not
all levels, native scope lifecycle, hardware or full-mod acceptance.

The bounded lab receipt capacity was increased after a prior seven-pose diagnostic
run exhausted its camera/pair/projection logs. Assessment remains strict: missing
correlated receipts still fail, and cached world submissions do not become fresh
native pairs. The launcher now records matching compiled game/server/host source
and IPC/wire contracts and supports exact source pins for either comparison mode.
