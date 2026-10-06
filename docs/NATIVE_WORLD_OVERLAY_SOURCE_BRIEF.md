# Native world-marker source review brief

Bounded Astra/xhigh source review of the approved incremental design. Solo-user mod; native testing is expressly excluded. No game/host/Wine/XR/network session may be launched. Read-only; no delegation. Main owns all edits, records and integration.

## Verified facts and scope

| Fact | Evidence |
|---|---|
| Actual eye-only callback after original world render, before readback | [verified: source] bridge.cpp stereo uses optional EyePostRender only inside admitted two-eye loop. Recursion/fallback and desktop restoration still call original directly. engine.cpp render supplies callback only at native parent returnFEA8A. |
| Target/depth/viewport admission before and after callback | [verified: source/offline] eyeTargetCurrent and production finishEyeRender; world_marker_checks exercises initial mismatch, callback invalidation, partial phase and pair failure. |
| No retired root pointer use | [verified: source] nativeMarkerPostlude copies executedWeaponWorld; only matrices and dimensions passed to original helper APIs. Root destruction/lifetime recorded in approved design review. |
| Full adjusted native eye projection | [verified: source/offline] worldMarkerViewValid matches actual eye view and asymmetric X/Y/W, preserves captured Z. Tests compare Core's XYZ/W clipping against prepared-vs-executed Z distances. No new projection/render state bank. |
| Original marker state and raster setup | [verified: source/native audit] original virtual60C fade, ortho/blend501/disable depth-write/depth/alpha, native navigation then objectives. Hidden/empty fade is successful no-op. No listener calls or parent replay. |
| Frame/rider/world-info/drawport continuity | [verified: source] markerFrameCurrent borrows existing immutable eye request/snapshot and pair fault, native thread and live player/rider/frame guards. Native world-info IDENT is resolved around native callbacks. Full-size drawport dimensions are borrowed and validated; compatible ends with another frame check after getters. No Snapshot lock held across callbacks. |
| Original symbols and caller ABI | [verified: artifacts/compiled] all added decorated S names exist using complete PE parsing (max_symbol_exports65536), 38 hooks/5 internal seams unchanged. Both x86 products show ECX receiver, three reference arguments plus float at stack12, native16-byte cleanup, virtual60C x87 float. tools/verify_world_marker_abi.py reads bounded owned code. Main manually traced player and copied argument addresses. |
| Final products and portable checks | [verified: offline] full x86 proxy/server+x64 host/official loader cross-build PASS, all12 offline groups PASS. Private logs world-marker-build.private and world-marker-artifacts.private under /tmp/ss2-vr-equivalence. No installed writes/runtime execution. |

Review exact changes to src/game/engine.cpp nativeMarkerPostlude/frame/drawport/render/attach, src/game/bridge.cpp eyeTargetCurrent/stereo, src/game/game.hpp EyePostRender, src/common/world_markers.hpp, tests/world_marker_checks.cpp, and tools/verify_world_marker_abi.py. Existing commit eb0856f is baseline. Relevant native original parent/render/marker/fade/getter/color evidence is in NATIVE_WORLD_OVERLAY_DECISION_BRIEF.md and design review/disposition; inspect exact fingerprinted native bodies as needed, export no proprietary dumps.

## Main assessment / open review decisions

Minimal addition retains native graphics and marker behavior, no new hook or IPC/wire change. Confirm exact caller/native state/color stage, matrix dimensions/ABI, reentrant getters/lifetime and partial pair rejection. Challenge necessity/seek deletion if any policy or state duplicates existing systems. Test and compiled checker success does not establish marker appearance or runtime correctness. In particular, native getter side effects and virtual fade ownership need your independent audit, not assumptions from exported names.

Do not re-review mounted integration, transport, scope ownership, physical roomscale or remote-head policy. Flat mission/conversation/boss/score/death overlays, physical vehicle muzzle lasers and overall feature completion remain separate unfinished work. Runtime appearance/performance is [unverified]; no feature-completion claim follows this slice.

Return a final prioritized verdict and findings with file/line/native-RVA evidence, smallest concrete fixes, verification/unknowns, max1200 words. No edits. Do not emit separate informational final outputs that replace the full critique. If evidence is missing, report the bounded gap rather than broadening the task.
