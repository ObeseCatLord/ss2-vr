# Physical weapon projection/depth — Astra review brief

Decision scope: one native owned XR gun rendering transaction. Solo operator; preserve engine/game/render architecture. Full scope is independent native tracked guns with physical world occlusion, prerequisite to immersive optics. No game/headset/host/network execution is authorized. All reviews use Astra. No source changes yet; current committed baseline 24d3c2a.

## Verified environment

| Fact | Evidence/status |
|---|---|
| Windows x86 game hook + x64 OpenXR host; native D3D9 eye rendering | [verified: source src/game/engine.cpp, bridge.cpp; docs/RENDER_AUDIT.md] |
| Owned Sam2Game/Engine/Core/GfxD3D hashes gate attachment | [verified: docs/installed-build.json and native support gate] |
| Root CView Prepare caller is Engine14C512/return14C517 | [verified: owned Engine native disassembly; existing viewPrepare caller gate] |
| Root actual P/view passed from Engine2E6398/2E6408; Box1 0..1 | [verified: Engine14C4xx setup to156370, locals14C504/14C50B] |
| Base weapon Render receives Matrix34 by value, ret30 | [verified: Sam4C740..4CA9C] |
| Game and Core/Engine mappings pinned, GfxD3D not yet pinned | [verified: engine.cpp attach] |
| Shared immutable request/ownership/tracking guards and stereo epoch commit exist | [verified: engine.cpp beginStereo/beginEye/commitStereo and render wrappers] |
| No IPC/protocol change needed | [design inference] |

## Critical camera evidence

[verified: exact owned vtables and decoded function bodies] CPlayer vtable29E878+5FC -> FDA00. InjectRenderingCommands calls equipped right/left weapon +1F0. BaseWeapon vtable2A3FB8+1F0 ->4BF40 AddRenderingCommand. It allocates native CRenCmd20 bytes with vtable2A4720 and weapon pointer+10. Vtable+4 ->4BDE0 Execute. Execute resolves owner+28, calls owner virtual+AC with purpose0 at4BE14, builds camera Matrix34, copies12 floats to stack and calls weapon virtual+1F4 at4BF34. Native gun camera therefore is NOT automatically the actual XR eye camera. No caller ViewOrigin hook exists.

[verified: Sam4C740] BaseRender saves current P64 bytes at4C80C/4C829, depth status402 at4C82B. Calls Core mthFrustumFOVX at4C8E3 (return4C8E9), installs returned P at4C8F6. Inverts incoming camera with Core mthInvertM34f at4C90F (return4C915), copies returned12 floats to Engine currentView at4C922, derives camera globals. Calls native gfxDepthRange(0,.1) at4C9A4 (return4C9A6). Calls GetWeaponAbsPlacement with ORIGINAL incoming camera at4C9B6 (return4C9BB), preserving authored offsets before native mdlRenderModel. Restores saved P at4CA79 and depth at4CA91 (return4CA93). A GetWeaponAbsPlacement failure jumps4CA96 and skips native P/depth restore; inherited failure matters if new adapters rely on cleanup.

[verified: source] weaponAbs already computes original model then retargets world hand, calculating authored rotation relative to caller-supplied camera. Replacing that input camera would unnecessarily change this reference. Current weaponFrustum only substitutes asymmetric XR XY using stock gun near/far; no physical depth/view adapter exists.

## Depth/cache boundary

[verified: owned exports/imports] Engine exports current P2E6398, currentView2E6408, depthNear2E5BF0, depthFar2C3E3C, gfxDepthRange function pointer2E626C. GfxD3D startup binds that pointer to fingerprinted internal cdecl(float,float) callback56A0 at4A14. Callback entry bytes558bec83ec28. It clamps/updates native cached near/far then actual D3D9 GetViewport/SetViewport at5772/5790; ret57AF. Sam uses function pointer data export, not direct GetProcAddress code. Device-only SetViewport would desynchronize native cache and is rejected. At attachment, callback binding may not yet exist: fixed fingerprinted internal hook + actual-call function pointer identity check is preferable to guessing startup timing. New GfxD3D hook must pin that module too; headless branch must not require it.

## Options and current lean

1. **Use existing root Prepare args, not a new renderer.** Lean: keep TLS current-eye root P/view/depth copies when exact root Prepare gate fires. Reset at begin/endEye. Render wrapper creates bounded stack invocation context for an actually owned/tracked local XR gun; nested/unowned calls do not inherit adaptation. Core frustum exactreturn4C8E9 copies root P. New Core inverse hook exactreturn4C915 copies root view. New GfxD3D56A0 hook exactreturn4C9A6 supplies root depth0..1 through original native callback. This aligns all P/view/depth while retaining original model input, authored gun offsets, native models/hands/stretch/render/state flow. Add only two native hooks; no render replay or replacement.
2. Capture current gfx caches at each gun instead. Initially preferred, but exact native gun owner ViewOrigin differs from XR and prior stock gun leaves current view changed. Root args are the authoritative per-eye native camera/projection at preparation; copies avoid dependence on preceding commands. Whether copying root args is sufficient across native current execution transformations is the key review question.
3. Replace Render's Matrix34 parameter with XR camera. Rejected lean because it also changes GetWeaponAbsPlacement authored reference unnecessarily. Inverse-result adapter is narrower and leaves original input bytes untouched.
4. Hook device SetViewport only. Rejected for native cache mismatch.
5. Render guns via a custom renderer or host compositor. Rejected: duplicates materials/animation/depth/occlusion, bypasses native architecture.

## Proposed admission and ordering

[design, not implemented] Capture root only from exact current-eye player transaction. Require finite P/view and valid depth; establish XR asymmetric XY projection provenance/actual eye view against native root args, preserving exact root coefficients. Admit one actual owned tracked gun context, scoped RAII; same current-eye root identity/request throughout. At exact frustum mark P changed; inverse changes only if admitted P succeeded; depth changes only after corresponding inverse succeeded and Engine function pointer still identifies hooked native callback. Nested calls clear context; restore previous context after original native call, exceptions included. Actual native restore4CA93 remains original. Native/non-XR/headless/unowned/shadow callers always call original once with exact native args.

[unknown] Is direct exact root P/view copy the correct matching execution stage for weapon materials, given native adjusted projection and current shader camera globals? Verify bounded native ActivateExecution/weapon setup consumers; do not assume D3D matrix conventions from API docs alone.
[unknown] Best fail-closed cleanup for native GetWeaponAbsPlacement failure: preserve native successful restore; add narrow per-invocation native P/depth restore if physical adapter changed them and native restore not observed. Do not introduce global raster-state reconstruction. Failed restoration should reject complete stereo pair rather than publish partial eyes. Existing stereo eligibility integration can receive a sticky per-pair fault if justified.
[unverified] Actual wall occlusion, gun scaling, clipping near face, other postpasses and performance; no runtime testing authorized. This step cannot be represented as completed immersive optics.

## Review request and depth budget

Verify load-bearing native instruction/caller/ABI claims from owned binaries before critique; audit actual source. Rank decisions, merge coupled ones, deep-spec the highest leverage failure family. Seek deletion/simplification. Return prioritized GO/reopen/gates, concrete smallest implementation and meaningful offline verification. Identify any human decision only if necessary; none currently expected. Max ~1600 words with exact file/symbol/RVA evidence. Read-only, no edits, no agents, no runtime execution. Do not rereview OpenXR transport, multiplayer, lasers, remote heads, roomscale, codec/UI or complete scopes. Missing evidence: report bounded gate rather than broadening architecture.

Primary API corroboration (not game-ABI evidence): [Microsoft D3DVIEWPORT9](https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dviewport9) defines MinZ/MaxZ as the depth mapping after projection. Native cache/callback evidence above is still decisive for the adapter. Fresh reviewer effective gpt-6-astra/xhigh was independently verified by selecting only model/effort fields from its private turn context; raw operational telemetry is not exported.
