# Research, 2026-10-01

## Existing asset tools, 2026-10-09

[Current tool research](EXISTING_SS2_TOOLS.md) identifies the official 2025 Edit
Data and SE2+ Blender importer as candidates to reduce authored mesh/skeleton/
animation work. Bundled editor/EditKit files exist locally; no editor was run.
Keep authored content inspection separate from live native ownership/cache/API
association. No current SS2 C++ SDK replacement was established.

## Verified installation
Steam application 204340; Serious Engine 2 modules are PE32/i386. Installed archives include Patch_02_100_800000.gro. Do not infer current upstream version from archive filename. Installed fingerprints:

| File | SHA256 |
|---|---|
| Sam2.exe | 727901f161133ff653fcdc196858335b991b743e67deb448c03c808e5b33e28b |
| Engine.dll | da6efc9f72637eb3b6f48eadca2107be89b09c00618b6e72d5d3632938a7d851 |
| Sam2Game.dll | 5628b4ed30a966f10c8e8ea46bf0789257a35ea80127bacacebe28b1ce5303df |
| GfxD3D.dll | 88749b79be36f0c0dccb623c4f1712685b5af603b451e25ed3c5029bedcdf3ed |
| Core.dll | 7a1bd56b9bfa3edfbb23f4d3c96490e40a7c0b031b85e797e1af91b2ba3cf207 |

`um kb search` found no Serious Sam 2 note. `um scan` failed to recognize the engine; DLL exports establish SeriousEngine namespace directly. Bin includes editor tools, not a verified native C++ SDK. Serious Engine 1 source is not Serious Engine 2 source.

## Primary references
- [Croteam's TFE VR product description](https://store.steampowered.com/app/552450/Serious_Sam_VR_The_First_Encounter/): one weapon per hand, locomotion options. Behavioral reference; no claim of copying game code.
- [Nokama0 SS2ModMenu](https://github.com/Nokama0/Serious-Sam-2-Mod-Menu), Unlicense: documented engine hooks, rendering-command boundary, hidden aggregate return ABI, per-game-module offsets. Research checkout kept outside repository. Claims checked against installed DLL bodies before use.
- [Khronos OpenXR SDK](https://github.com/KhronosGroup/OpenXR-SDK) and [SDK source samples](https://github.com/KhronosGroup/OpenXR-SDK-Source): loader, actions, predicted views, swapchain/frame lifecycle.
- [OpenXR specification](https://registry.khronos.org/OpenXR/specs/1.1/html/xrspec.html): D3D11 graphics binding; D3D9 is not a standard OpenXR binding.
- [Microsoft D3D9Ex resource sharing](https://learn.microsoft.com/en-us/windows/win32/direct3d9/dx9lh): sharing requires compatible devices/formats; classic D3D9 cannot be assumed to support this transport.
- [F.E.A.R. VR implementation](https://github.com/DR-89/fear-vr): example of x86 D3D9 game + x64 D3D11 OpenXR host. Different engine and available SDK; not evidence SS2 hooks are correct.

## Static graphics evidence
Engine.dll CViewRenCmd::Execute export RVA 0x155ff0 calls render-only child commands via vtable slot +4, between renderer setup and cleanup. Copy constructor RVA 0x14d340 copies Matrix34f at object +0x14 and Matrix44f at +0x44. Setup routine RVA 0x155a50 consumes these matrices. These are established offsets, not guessed camera globals. It also sorts child commands and has postprocess state: replay needs explicit review; no runtime safety proof.

`CPuppetEntity::GetCameraPlacement` uses a hidden QuatVect return pointer; QuatVect is 4 quaternion floats plus 3 position floats. `CBaseWeaponEntity::Render` receives Matrix34f by value (48 bytes), not a reference. Need verify coordinate mapping and inventory/action offsets locally.

## Decision constraints
OpenXR requested explicitly. Native game/headset testing excluded explicitly. Linux/Proton is the working environment; Windows PE cross-compilers for x86 and x64 are available. Proton OpenXR runtime availability and GPU interop are unverified. Do not promise Linux hardware support merely because cross-compilation succeeds.

## Implemented native boundary and further evidence
Astra rejected command replay. The implemented Render3D boundary regenerates collection and frees it per eye. GfxD3D dynamically resolves Direct3DCreate9 (RVA 0x2fdc); its gfxStartupAPI binds native canvas target control to RVA 0x2970, which issues D3D9 SetRenderTarget/SetDepthStencilSurface. See RENDER_AUDIT.md for limits.

Native player-command definitions at Sam2Game RVA 0xef410 associate X+ with strafe-right, X- with strafe-left, Z- with move-forward, and Z+ with move-backward. Movement uses these associations; it is not inferred from command spelling alone.

[Valve's Proton wineopenxr implementation](https://github.com/ValveSoftware/Proton/blob/proton_11.0/wineopenxr/openxr_loader.c) contains D3D11-to-native-Vulkan session handling. Installed Proton Hotfix includes the x64 wineopenxr manifest and copies it into a prefix during setup. That establishes a possible OpenXR route, not tested SS2VR/headset compatibility.

[Microsoft GetRenderTargetData](https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-getrendertargetdata) defines source/destination and multisampling constraints. [StretchRect](https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-stretchrect) supports non-MSAA resolve/copy subject to driver/format restrictions; it is used only for explicitly mono menu capture at Present.

## Completion-stage native and OpenXR refinements

The original camera includes native view/weapon animation. Static vtable/ABI evidence establishes a purpose-0 base view origin plus stance-height interpolation as the common tracking body anchor, while camera-relative authored weapon offsets use the original camera. See ANCHOR_REVIEW.md and COMPLETION_NATIVE_REVIEW.md. Native root view identifiers feed query CRC history; the root-only Prepare hook isolates the two eyes without replacing native culling. See RENDER_AUDIT.md and FINAL_INTEGRATION_REVIEW.md.

[The pinned Khronos rendering chapter](https://github.com/KhronosGroup/OpenXR-Docs/blob/release-1.1.53/specification/sources/chapters/rendering.adoc) defines latest-released-image composition, acquisition/wait retry ownership and LOCAL-space projection behavior. The final bridge reuses only a real complete eligible pair with original poses/FOV; both waits precede either upload/release and cached-index reacquisition hides reuse. See TRANSPORT_AMENDMENT.md and HOST_STATUS.md.
