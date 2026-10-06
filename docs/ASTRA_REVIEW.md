**Approve the narrow adapter direction, but revise the stereo boundary before implementation.** The strongest risks are incomplete stereo reconstruction, native input policy coupling the hands, and a transport contract that promises more timing isolation than CPU readback provides.

[Verified] All five recorded SHA256 fingerprints match the installed binaries. Camera/shooting hidden-return conventions and weapon rendering’s 48-byte stack argument are supported by disassembly. One correction: `GfxD3D.dll` dynamically resolves `Direct3DCreate9` through `CDynamicLoader::GetSymbol_t` at RVA `0x2fdc`; it does not directly import it.

No game/headset execution, repository edits, or further agents were used. Effective CLI model/effort verification remains with the calling main agent.

1. **Highest impact: replacing view matrices and replaying `Execute` does not reconstruct an eye’s render state.**

   [Verified, installed `Engine.dll`] `CViewRenCmd::Execute` (`0x155ff0`) sorts its child-pointer array in place, executes virtual child commands, changes graphics state, and can reactivate its parent view. Its copy constructor (`0x14d340`) copies the pointer at `+0x10`, which `Execute` uses to find that array. Copying the view command therefore does **not** isolate the command graph.

   More importantly, `ActivateCollection` (`0x1559b0`) establishes `ren_mView` and `ren_vViewer` before collection. `CObjRenCmd::Prepare` (`0x1541a0`) consumes them, performs box culling, and calculates distance/LOD information. `ActivateExecution` updates graphics matrices later. Thus changing only execution matrices leaves previously calculated, view-dependent information intact. A conservative visibility union cannot by itself repair that.

   [Verified, source] The mod-menu hook calls the original `Execute` **once**, then draws overlays. It supplies a useful insertion point, not replay evidence. See [hooks.cpp:127](https://github.com/Nokama0/Serious-Sam-2-Mod-Menu/blob/df6669aaff5121961b4fc9efb15f1433b528df08/src/hooks.cpp#L127). Its freecam culling patches also demonstrate dependencies beyond a single frustum, including visibility groups and occlusion: [freecam_cull.cpp:4](https://github.com/Nokama0/Serious-Sam-2-Mod-Menu/blob/df6669aaff5121961b4fc9efb15f1433b528df08/src/freecam_cull.cpp#L4).

   **Recommended deep specification:** make stereo a scoped transaction spanning the minimum necessary **preparation, collection, and execution**, owned by the existing render thread. Preserve the engine renderer and command types; do not introduce an alternate scene renderer.

   The transaction must bind an immutable request containing session generation, frame sequence, reference-space/recenter generation, predicted display time, both eye poses/FOVs, and render dimensions. Both eyes use the same engine render snapshot and body-anchor transform. Simulation and weapon events execute independently, once through their normal paths.

   Identify the main local-player view positively. Classify nested views and command families before modifying them; sky, shadow, reflection, weapon, and HUD passes cannot all receive the main eye matrix indiscriminately. A recursion guard prevents the hook from stereo-expanding its own work, but is not a substitute for this classification.

   Audit every participating command family for collection-time view dependencies, execution mutations, resource lifetime, and nested rendering. Reuse view-independent preparation. Rebuild view-dependent preparation through native routines per eye. Permit shared conservative collection only where the audit establishes that eye-specific visibility, sorting, billboarding, occlusion, and prepared transforms remain correct. Do not adopt global culling-disable patches as production policy.

   Keep command allocations inside their original lifetime. [Verified] `renFinishRender` (`0x14bfa0`) executes the command graph and subsequently performs destruction/allocator cleanup. Do not retain its pointers in IPC or defer their execution beyond cleanup. Allocate separate native command structures where necessary; the observed copy constructor is insufficient.

   Isolate each eye’s color/depth and view-dependent intermediate targets. Restore render targets, depth-stencil, viewport/scissor, graphics state, **and engine-side cached state** through verified boundaries. A D3D state snapshot alone does not restore the engine’s bookkeeping.

   [Missing] The complete command-family audit, target-routing map, collection side-effect analysis, and final postprocess/HUD capture point. Until established, fail closed for stereo on unsupported paths. Never substitute identical eye images.

   The smallest useful implementation proof is one identified local-player world view producing two separately prepared perspective outputs through native rendering. Offline checks can establish ABI, projection math, ownership, and call sequencing; they cannot establish visual correctness. This limitation is compatible with the explicit prohibition on runtime testing.

2. **Independent hands require a narrow exception to native input routing.**

   [Verified, installed `Sam2Game.dll`] `GetWeaponFiringButton` (`0xfb590`) contains configuration-dependent routing. In its non-combo branch, equal weapon indices converge on button `0` (`0xfb628` → `0xfb64d`). Therefore two controller trigger states mapped only to the normal two buttons do not guarantee independent firing of identical weapons.

   `IsWeaponFiringPressed` (`0xf6f00`) obtains that button through virtual dispatch and queries the corresponding input. This is a promising narrow adapter boundary, **subject to verifying all relevant callers**.

   Recommend resolving the local player’s current native weapon handles on the simulation thread and mapping each weapon entity to its physical hand. Override the local weapon’s fire-query result where necessary; retain native firing, ammunition, cooldown, reload, and damage processing. Identify ownership by current handles and lifecycle generation, not weapon type or one global “currently firing weapon.”

   [Verified] Sniper shooting placement has a separate override at `0x172a70`, and weapon rendering also has separate base/sniper exports. [Source-verified; complete binary coverage missing] Beam behavior includes placement queries outside the ordinary `DoFirings` window: [hooks.cpp:3087](https://github.com/Nokama0/Serious-Sam-2-Mod-Menu/blob/df6669aaff5121961b4fc9efb15f1433b528df08/src/hooks.cpp#L3087). Audit continuous weapons, melee, grenades, and vehicle paths explicitly; a base shooting-pose hook is not evidence of universal coverage.

   Wheel selection should enqueue one hand-specific selection intent, validated against native inventory and changeability on the simulation thread. [Missing] The verified native selection path that preserves the other hand. Do not mutate inventory or weapon state directly to approximate it.

   Wheel opening, focus loss, stale input, invalid tracking, and ownership changes should release fire and require trigger return below the release threshold before rearming. Closing a wheel or recovering tracking must not synthesize a shot.

   Use one calibrated tracking-to-world transform for eyes, hands, rendered weapons, and shooting placement. Capture the anchor before VR modification; do not derive it from the already modified camera. [Missing] Verified world scale, camera/body separation, and weapon grip-to-muzzle transforms. Recenter and snap turn must change one shared transform generation atomically.

3. **The x64 CPU bridge is defensible, but does not guarantee bounded render latency or runtime compatibility.**

   Keep one x64 OpenXR/D3D11 host as the initial implementation choice. [Unknown] An x86 runtime path may work; its absence has not been established. Do not implement competing hosts merely to cover that uncertainty. An x64 executable also does not prove Proton/runtime interoperability.

   Specify the CPU transfer precisely: resolve multisampling first; use compatible source/system-memory surface dimensions and formats; honor row pitch; negotiate channel order, color encoding, and the OpenXR swapchain format. Microsoft documents that `GetRenderTargetData` fails for multisampled sources or mismatched dimensions/formats. [Microsoft API reference](https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-getrendertargetdata).

   **Bounded IPC waits do not bound the synchronous D3D9 readback call.** Do not move device calls onto an arbitrary worker to hide this without verifying device threading semantics. Treat CPU transfer as an initial correctness path with unverified performance, not a hard latency guarantee.

   Let the host alone own the OpenXR frame lifecycle. Submit the poses/FOVs actually used to render the images; OpenXR’s frame association is implicit, so the bridge’s sequence number is an application invariant. [Khronos frame-submission guide](https://github.com/KhronosGroup/OpenXR-Guide/blob/main/chapters/frame_submission.md).

   Use a bounded slot protocol with explicit producer/consumer ownership and cross-process synchronization. Never overwrite a slot while either process is copying it. Publish completion only after both eyes and their metadata are complete. Cancellation must not free a slot still owned by the renderer. Session, device-reset, dimension, and reference-space generations must invalidate obsolete results.

   Keep input publication independent of image completion so dropped frames cannot preserve held fire. Submit zero layers for incomplete/invalid frame transactions, while detecting sustained deadline failure rather than endlessly superseding work that cannot finish.

My final design choices are to retain native simulation and rendering; expand the stereo adapter only to demonstrated preparation dependencies; introduce weapon-specific input routing without duplicating gameplay policy; and retain one x64 host with an explicitly limited CPU transport. GPU sharing remains a later transport substitution. **The implementation gate is the renderer dependency audit—not additional IPC scaffolding.**