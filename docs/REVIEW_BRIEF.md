# Astra review brief

Goal: implement SS2 native VR, independently tracked dual wield and convenient per-hand radial selection. Solo user; no enterprise ceremony. OpenXR explicitly required if possible; no game/headset testing. Review architecture before implementation. Depth: prioritize top 3 risks and one deep specification, <=1800 words. Read-only; no edits, no game launch, no further subagents. Return prioritized critique, concrete decisions and evidence labels. User decisions only if unavoidable.

## Facts
| Fact | Evidence |
|---|---|
| x86 native closed-source engine | [verified] file/PE exports in installed Bin |
| D3D9 game renderer | [verified] GfxD3D imports Direct3DCreate9 |
| Named camera, shooting, inventory exports | [verified] exported decorated symbols; weapon ABI investigation in progress |
| Left/right native weapons | [verified] archives carry _L/_R definitions, game exports per-slot accessors |
| Rendering command matrices | [verified] Engine.dll copy ctor 0x14d340, Execute 0x155ff0, setup 0x155a50; see RESEARCH.md |
| Windows cross-compilers available | [verified] i686/x86_64-w64-mingw32-g++ |
| Native SDK for engine 2 | [unknown] not found; no dependency on SDK planned |
| Actual headset/runtime availability | [unknown] no launches permitted |

Workspace: repository `ss2-vr` inside installed game folder, Bin one level above. Read docs/RESEARCH.md and AGENTS.md. External existing Unlicense mod source /tmp/ss2-vr-research/mod-menu/src/hooks.cpp and TECHNICAL.md offers ABI hints; installed exports /tmp/ss2-vr-research/*.pe.txt. Proprietary analysis artifacts kept outside repo.

## Proposed minimal adapter
An x86 D3D9 proxy loads hooks via pinned MinHook. Narrowly reuse game input/inventory/cooldown/ammo/projectile and per-hand slots. Hook hidden-return shooting pose per hand plus weapon render placement. Gather body anchor from camera placement without replacing physics. Wheel state handles presentation/selection only; native inventory determines available weapons. OpenXR actions support tracked hands, two wheels, independent trigger holds, stick locomotion, snap turn, menu/use/jump/recenter. Stale/focus-loss frames release all held input. No gameplay simulation on render thread.

Graphics: x64 OpenXR host owns D3D11 session/actions/view timing. A fixed-width shared memory ABI carries poses/input, frame ID and stereo pixels. Initial classic D3D9 GetRenderTargetData CPU readback -> shared RGBA pixels -> host D3D11 upload/copy avoids unverified D3D9Ex/DXVK shared texture compatibility. Costs acknowledged (bandwidth/readback stalls); preserve transport boundary for incremental GPU sharing later. Bound all waits, drop stale frames, use per-launch mapping token, include version/size, no pointers. Host begins/predicts frame and publishes views, game renders both views against that same pose/frame, host submits ONLY matching views/frames; incomplete frame submits zero layers.

Stereo lean: replay render-only CViewRenCmd::Execute with eye-specific matrices and separate color/depth targets, preserving source command matrices afterward. Do not replay game tick or fire events. Original visibility commands are prepared once; concern eye-specific frustum clipping, command sorting mutation, shadows/postprocess/viewmodels and GUI composition. Prefer modifying command construction/culling to conservative union before replay if evidence allows. Unknowns are NOT justification for replacing renderer.

## Alternatives compared
1. In-process x86 OpenXR + D3D11: simpler transport but runtime architecture support (SteamVR/VDXR/Proton) unverified. Could be preferred if supported; avoid claiming all runtimes work. Current lean x64 host for standard runtime bitness.
2. D3D9Ex shared textures vs CPU bridge: Ex upgrade impacts device semantics/HDR/DXVK compatibility. Start correctness-oriented CPU adapter and transparent performance limit; GPU only after static evidence.
3. Rendering-command replay vs outer scene render replay: command replay preserves simulation boundary, but visibility/state can be wrong. Outer render replay may rerun preparatory mutations; require evidence before use.
4. Engine/native game rewrite: rejected, no engine source, unnecessary duplicated simulation and asset loader policy.
5. Shared reticle/mouse emulation: rejected as fails independent six-DOF aim.

## Specific open decisions
A. Is command Execute replay sufficiently defensible without running game? What is minimum static evidence and where should it fail closed?
B. How to maintain predicted frame/view identity without blocking sim/render indefinitely?
C. Is x64 host complexity necessary given x86 loader support? CPU fallback acceptable as first implementation but not claim performant VR.
D. Correct body-relative/recenter transformation and wheel lifecycle, especially no firing on wheel-release/tracking recovery.
Suspected overlap A/B: pose identity should be integrated with rendering contract rather than separate frame scheduler policies.

Completion excludes runtime verification but requires actual hooked rendering/input path, buildable package and honest limitations. Do not equate mock test success to completed engine integration. Senior review should earn cost by changing design or catching a concrete hazard.

## Additional static finding during review
Sam2Game CPuppetEntity::Render3D RVA 0x94380 (321 disassembled instructions) is a stronger candidate boundary than command replay: GetProjectionMatrix -> camera placement via virtual slot +0x5f4 -> mthInvertRTM34f -> renPrepareRender -> InjectRenderingCommands slot +0x5fc -> renFinishRender -> renSetAvatar(nullptr). Disassembly with imports labeled: /tmp/ss2-vr-research/render3d.txt. Suggest replaying this narrow render-only entry with eye camera/projection hooks, so engine recomputes culling and builds/frees commands for each eye; no broad CPlayerPuppet RenderView (includes weapon/HUD/audio work). Main still verifying +0x5fc callees and matrix convention.
