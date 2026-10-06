# Native flat overlay ownership and capture constraints

Main static inspection of fingerprinted installed binaries; no runtime or installed writes. This is evidence for the next UI adapter, not an implementation or reviewed design. Full native UI remains required. Existing comfort anchor, HUD/menu/wheel layers and four-layer budget are reusable.

## Original once-per-view owner

Sam2Game CPlayerBrainEntity RenderViewEB7B0..EB87C resolves its puppet, invokes virtual610 atEB833, then virtual608 atEB857 with one32-bit flag. The flag records whether the world supplied its alternate view. Player RenderOverlay105480..105558 is thiscall/ret4 and draws targeted player nameF8060, death messageFDED0, native TextQueue Render2D via puppet828, cheatsFE2B0, multiplayer player list104B90, local/demo HUD via puppet82C and17C5E0, deathmatch scores104E60, then RenderOverlayFadingsFE830. Base CPuppet RenderOverlay is the shared no-op453D0/ret4.

Do not replay this whole native overlay per eye. RenderOverlayFadingsFE830..FEA10 compares native uptime/simulation time, clears an expired native owner's Time at198/19C (FE8C2/FE8CB), writes the native gradient temporary at40D6FC (FE980/FE9DA), and emits gfuFill colors. This is demonstrated mutation, not inferred from a method name. Keeping the original invocation once avoids extra time/fade updates.

The HUD container17C5E0..17C7EE also invokes each original element's virtual20 at17C7B0 before conditional virtual28 rendering at17C7D7. Its flags/visibility remain native. The bounded body does not establish that the virtual20 family is read-only, so individual HUD replay is not yet justified either. It pushes/pops the native drawport, installs a640x480 ortho and native identity views, disables depth/depth-write/alpha-test, and starts blend501. Nested element renderers may change blend/state; their coverage is not yet audited.

## Alpha is not automatically a reusable UI image

Engine gfuBlendType8BD00..8BD63 maps501 to native factors26/27, and calls original gfxBlendFunc/enable. GfxD3D startup4960 assigns blend function54B0 at49F1; that function54B0..5564 maps factors through native table112A4 and sets only D3DRS_SRCBLEND19 and DESTBLEND20, caching native factors. Factors26/27 map to D3D5/6 (SRCALPHA/INVSRCALPHA). Thus a fresh transparent A8 surface under ordinary native blending does not by itself prove correct accumulated coverage alpha. The existing stereo/menu paths force opaque alpha and cannot be reused as transparent native HUD capture unchanged.

[Microsoft's render-state reference](https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3drenderstatetype) defines separate alpha blending and its default-disabled behavior. [Device caps](https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dpmisccaps) expose separate-alpha support. Those APIs are available candidates; they do not prove the game's element shaders, all blend modes, color-write masks or resulting images are suitable. [Alpha blending documentation](https://learn.microsoft.com/en-us/windows/win32/direct3d9/alpha-blending-state) confirms that different factor pairs can implement emission and destination modulation, so blindly imposing one coverage rule on every draw would change behavior.

## Narrow next decision

Compare capture around the existing once-only overlay with duplicating only its immediate GPU draw operations into an owned UI target. Both candidates should retain native quest/text/element policy and original desktop behavior, borrowing the existing shared image channel and comfort-following HUD layer instead of another quest model or unconditional XR layer. Per-eye native-overlay replay and replacing native messages with a custom quest model are unsupported.

Required evidence before source work: actual supported HUD draw/blend/alpha family and transparent readback; native target/cache restoration; world-vs-panel ownership of full-screen fades; correlation of the once-only overlay with the complete stereo pair/epoch; shared-image classification and expiry without menu-pointer crossover; native UI layout at readable physical size while preserving the35-degree comfort-follow threshold. A panel-sized black fill is not proof of full-field VR fading. Reopen architecture with Astra once these concrete boundaries are worked, rather than extend the marker callback into speculative flat-overlay replay. No feature completion follows this audit.
