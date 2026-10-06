## Audit result

The native per-eye collection transaction is verified. Final backbuffer capture for world plus injected weapons is conditionally supported by the existing D3D router, but not fully established for every render-command/postprocess family; keep stereo fail-closed outside the classified path.

### VERIFIED native control flow

- `Sam2Game.dll`, `CPuppetEntity::Render3D`, RVA `0x94380`: obtains camera placement and view/projection-derived state, then calls `Engine.dll` `renPrepareRender`, virtual `InjectRenderingCommands` (slot `+0x5fc`), `renFinishRender`, and clears the avatar. Each call is a full native render transaction; invoking it twice genuinely rebuilds collection/LOD/culling per eye under the scoped camera/projection hooks.

- `Engine.dll`, `CViewRenCmd::ActivateCollection`, RVA `0x1559b0`, installs the view/viewer state used while commands are prepared. `ActivateExecution`, RVA `0x155a50`, installs later graphics state. This confirms that the `Render3D`-twice approach is the correct narrow boundary and that command replay is not.

- `Engine.dll`, `renFinishRender`, RVA `0x14bfa0`, invokes the root command’s virtual execution and then performs renderer cleanup. Commands are not retained past their native lifetime.

- `Sam2Game.dll`, `CPlayerPuppetEntity::InjectRenderingCommands`, RVA `0xfda00`, resolves both current weapon handles at player offsets `+0x800` and `+0x804` and invokes each weapon’s render-injection virtual slot `+0x1f0` when eligible. Thus native local-weapon rendering is inside the `Render3D` transaction; it is not a separate manual draw in the adapter.

### VERIFIED canvas/target binding

- `Engine.dll`, `CCanvas::CurrentRenderTarget`, RVA `0x60e10`, compares the canvas to `_gfx_pcvCurrent`; when different, it calls the exported function-pointer binding `_gfxCurrentRenderTarget(canvas, -1)` at Engine RVA `0x2e6368`.

- `GfxD3D.dll`, `gfxStartupAPI`, RVA `0x4960`, installs the implementation for that Engine function pointer: `GfxD3D` RVA `0x2970`.

- `GfxD3D` RVA `0x2970` resolves the canvas surface/depth representation and issues D3D9 device calls at vtable offsets `+0x94` and `+0x9c`: `SetRenderTarget(0, …)` and `SetDepthStencilSurface(…)`. This is the concrete native WindowCanvas/backbuffer binding path.

- `CCanvas` also exposes `RenderCanvas`, `TextureCanvas`, and `CubeTextureCanvas`; the cube canvas has a distinct `CurrentRenderTarget` override at Engine RVA `0x60f40`. These are intermediate/offscreen families and should remain native-owned.

- `gfxPresent` is separately bound by `gfxStartupAPI` and implemented at `GfxD3D` RVA `0x1960`. It is not called by the verified `CPuppetEntity::Render3D` transaction, so Present is not the correct capture boundary.

The existing `bridge.cpp` router is aligned with the verified path: while an eye is active, it substitutes only RT slot 0 when the native target equals the original desktop surface, and substitutes the matching desktop depth surface. Native offscreen target binds therefore remain untouched. Because the engine can regard the WindowCanvas as current while the physical device target is redirected, it also handles the “already-current canvas” fast path.

### Capture assessment and concrete gates

Readback succeeding is not proof of final composition. The currently supported claim is narrower: if native world/weapon output renders to the full-size original WindowCanvas during `Render3D`, the verified canvas binding reaches the adapter’s eye surface and can be read back immediately after `Render3D` returns.

Add these fail-closed gates before calling that complete:

- Before each eye readback, query RT0 and depth-stencil; require identity with that eye’s owned color/depth surfaces. Reject otherwise.
- Record and reject any active-eye MRT binding (`SetRenderTarget` slot other than zero), rather than silently leaving it on the desktop path.
- Require the initial target to be the full-size backbuffer, as the code does, plus compatible eye-surface descriptors/format policy. The current allocator hard-codes `A8R8G8B8` and `D24S8`; it does not classify alternate window/depth formats or AA-sensitive paths.
- Treat a target change to an unclassified desktop alias, a device reset, or a failed target/depth bind as an invalid frame, not a partially captured eye.

The smallest adapter change is only this frame-scoped verification/rejection gate around the existing D3D target router. Do not replace the renderer, replay commands, or add a second render graph.

### UNKNOWN / unsupported command families

`CViewRenCmd::Execute` at Engine RVA `0x155ff0` executes a generic child-command array through virtual dispatch. It also has an optional clear path, but it does not itself prove what every child renders to. The remaining missing gate is a static target-routing classification for children that can own nested or offscreen views:

- nested `CViewRenCmd` views;
- `CEffectRenCmd` and effect/postprocess-like commands;
- texture/render/cube canvas passes, reflections, shadows, and render-to-texture;
- HUD/menu/drawport work, which is not established as part of the player weapon injection path.

No evidence found supports claiming those families’ final composition lands on the redirected WindowCanvas. They should remain unsupported for stereo capture until their final target return-to-window path is demonstrated. No game, host, headset, Wine, or runtime execution was performed, and no files were modified.
## Final static routing and history audit

Native target selection is centralized: Engine `_gfxCurrentDrawPort` RVA0x8b5a0 dispatches the canvas's CurrentRenderTarget virtual when its canvas changes, while nested view Execute restores the parent view's native matrices via ActivateExecution. CObj and CEffect command execution dispatch original renderable methods rather than applying the main-eye projection indiscriminately. Main camera/projection overrides are confined to the identified local puppet; weapon projection is separately scoped to owned weapon rendering. Native nested/shadow/reflection matrices retain their original construction.

A concrete previously missing source path was found: Engine CCanvas::CopyContent RVA0x60e80 calls `_gfxCopyRenderTarget`; GfxD3D gfxStartupAPI installs RVA0x1280 for that binding, whose copy uses device vslot34/offset0x88 (StretchRect) at RVA0x1373. The adapter now redirects both source and destination aliases of the original desktop surfaces in StretchRect, plus direct readback/fill, without changing canvas objects or intermediate texture ownership. Eye and CPU surfaces preserve the original A8R8G8B8/X8R8G8B8 format; output alpha is explicitly opaque. This removes the old format-conversion dependency for native window copies. Native gfxSubPixelHDR restores/copies its drawport inside renFinishRender; the copy follows this binding. Shader/effect visual quality remains untested.

Another concrete dependency: CViewRenCmd::Prepare RVA0x156370 CRCs its fourth identifier argument into view+0x98, including parent view identity. CObjRenCmd::GetUniqueID RVA0x1540e0 combines object identity with that view identity for native occlusion queries. renPrepareRender's main-root call at RVA0x14c512 supplies the same avatar-derived identifier across renders (return RVA0x14c517). A required exported Prepare hook tags only this verified call per eye before original preparation. The native CRC/query manager and parent propagation remain intact; shadow/nested callsites are forwarded unchanged. Thus left/right and restoring desktop views have distinct native history without disabling culling or introducing another query cache. Prepare's four reference/scalar arguments and ret16 were verified statically.

The static routing map now covers main WindowCanvas binding, native intermediate binding/return, original-surface copies and per-eye native query identity. It is not a GPU capture or proof that every shader, effect, cinematic or custom module produces correct VR visuals. Unsupported format/MSAA/MRT/final-target paths remain rejected.
