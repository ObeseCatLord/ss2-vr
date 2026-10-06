# Native world marker boundary: revised minimal design

Unreviewed main brief. Goal is native navigational beacon and objective markers in each actual XR eye, using original gameplay/marker state and draw functions. Flat quest/conversation/boss/death/score HUD is a separate subsequent adapter. No runtime testing authorized.

## Verified environment facts

| Fact | Bounded evidence |
|---|---|
| Current bridge invokes original Render3D per eye and once for desktop | src/game/engine.cpp render; bridge.cpp stereo; distinct target/readback/pair guards |
| Existing Render3D hook is entered inside original CPlayer RenderView | Sam RenderViewFEA20, callFEA84 -> virtual600; native drawport is already scoped byFEA7A |
| Native marker postlude is outside current capture | Native markersA01F0/A0710 calledFEB5E/FEB78 after native Render3D returns; flat RenderOverlay remains later brainEB857 |
| Parent has non-render side effects | NativeFEE82 GetViewOrigin, FEE90 GetVelocity, FEEA1 helper21B8B0; helper registers sound21B8FE and vibration21B91B listeners |
| Original marker arguments and setup | Parent copies native view12 floats/projection16 floats atFEAC2..FEAE6; gfuOrthoFEAE8, blend501FEAF3..FEAF9, raster togglesFEB02..FEB11; fade virtual60C atFEB30; drawport size from+4/+8/+C/+10; fade>0 draws both native markers |
| Exact native helper APIs | exported CPuppet RenderNavigationalBeaconsA01F0 and RenderMissionObjectivesA0710 each thiscall four stack arguments (view/P/dimensions references, float fade); CPlayer GetBeaconsAndObjectivesFadeRatioFEED0 returns native float |
| Marker bodies | Complete bodiesA01F0..A0700 andA0710..A0C20 use current-world info, native marker entity GetAbsPlacement, original mthProject3DPointTo2DSpace, original time/fade/resource/CString/font/texture draw; no observed movement/input/RPC/listener calls |
| Matrix provenance available | prepared/executedWeaponWorld copies actual per-eye native root view/P/depth after native clip adjustment; drawLasers already uses native ortho/ActivateExecution. Projection X/Y/W must remain actual eye frustum |
| Native fallback/desktop | Original parent remains single normal invocation and supplies stock crosshair/listener postlude once after stereo returns |

Evidence labels: table is [verified: owned fingerprinted native binary decoding and repo source]. Runtime marker appearance, performance, and pose correctness are [unverified]. Getter callback lifetime and scalar return convention still need source/compiled admission verification when implementing. No proprietary disassembly/content committed.

## Compare candidates

A. **Minimal incremental post-Render3D adapter (main lean).** Retain the existing stereo interception and native parent. Pass a small original-render callback to the same bridge: call original Render3D; only within the admitted local XR eye invoke original marker helpers, with the actual captured native eye view/P, original current drawport dimensions and original fade. Use original native 2D setup, then normal pair target/provenance validation/readback. Desktop callback performs only original Render3D; unchanged native parent draws markers/crosshair and registers listeners once. No new render hook, replayed command tree, listener interception, quest model, IPC or layer. This repeats original marker draw operations only, at their original color stage after Render3D. It does not implement marker contents itself.

B. **Move stereo interception outward to RenderView.** Preserve native postlude verbatim per eye but must isolate demonstrated listener mutations at exactFEEA6 origin, suppress stock center crosshair at exactFEB87 origin, and audit current matrix provenance. Adds at least caller-scoped listener and crosshair hooks and repeats native listener pose/velocity queries. Parent native resource/COW state is repeated. This is more complexity thanA after the new side-effect evidence.

C. **New marker UI/quest state or extra XR quad.** Rejected: duplicates working native quest/filter/fade/layout policies and weakens world projection. No demonstrated incompatibility requires it.

Reusable: current Render3D/stereo transport, native collection, root capture, physical weapons, collision lasers, current drawport scope, original marker draw APIs and fade, native later flat overlay, current head comfort HUD layer. Duplicated state/policy inA: none beyond temporary captured arguments/eye scope; reproduce only exact bounded native graphics setup necessary to call native draw APIs at the already intercepted boundary. No second renderer or view state machine.

## Implementation estimate and smallest vertical proof

One eye-only callback after original Render3D, bind original marker/fade/current-drawport/native 2D setup APIs, reuse copied actual eye matrices and existing pair-fault flag. Expected ~80–120 lines plus one production math/provenance helper and meaningful offline projection checks. Preserve head/seat identity, caller/native main thread, immutable request and actual drawport dimensions. If source needs further hooks, queues or lifecycle systems, reopen design.

Smallest proof: original Render3D completes into the actual eye target, both native marker helpers are called with that eye's actual executed view/asymmetric P and correct dimensions/fade before bridge readback, pair failure prevents partial/invalid output; original native desktop parent postlude/listeners and later flat overlay remain once. Cross-build + compiled reference/float argument and float-return ABI + projection/coherent-eye offline checks. No runtime correctness claim; user forbids launching.

## Decisions for Astra

Audit native evidence first. DoesA preserve native behavior sufficiently without recreating adjacent systems, and shouldB be deleted? What is the smallest exact per-eye matrix/target/drawport/identity admission guard needed after Render3D and before helper calls? Are setup/fade/resource side effects acceptable in repeated render-only helper calls? Does bypassing stock parent crosshair/listener in the two eye callbacks preserve desktop/native ownership? Rank real failures; do not require a full redesign when a narrow adapter closes them. Do not re-review mounted source/transport/optics/roomscale or implement flat HUD in this leg. Return max1200 words with prioritized verdict, conditions and concrete narrow changes, file/native-RVA evidence, verification and unknowns. Read-only, no runtime/no delegation.

## Additional main provenance checks

Exact imported raster toggles from the parent are Engine _gfxDisableDepthWrite at native IAT10294FB8, _gfxDisableDepthBuffer10294590 and _gfxDisableAlphaTest10294594. They reuse existing mod symbol pointers. Core mthProject3DPointTo2DSpace1D680..1D6C6 forwards to original float helper1A160..1A2B9; its view/P products use all four clip rows, test clip XYZ within[-W,+W], then convert X/W and Y/W to drawport pixels. Thus marker visibility must use the actual full captured native executed P, including its Z clip coefficients; X/Y/W provenance alone is only the request-match guard, not permission to substitute another depth row.

Implementation should borrow the actual active eye target/depth/viewport identity before native marker draws and again afterwards through the existing bridge device/eye surfaces; the existing final-target check only after the callback is insufficient as proof of which target received markers. Reuse the bridge's exact renderRequest/activeEye identity and fixed native drawport scope, without a new graphics state bank. If target/canvas/drawport/root/request/rider changes, reject the whole pair through existing sticky fault/commit guards. Original Render3D remains the complete color-stage boundary; this marker callback runs after it and before bridge readback, leaving native parent/listeners and flat overlay once.
