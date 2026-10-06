# Scope image adapter design brief

Solo local mod, preserving the native renderer and gameplay. Decide the smallest
adapter that can insert a real magnified image at the native Scope cap. This is
a design review, not a claim that image rendering is implemented. No game,
Windows binary, Wine, OpenXR or network session may execute. Linux compilation
and offline checks are authorized. Teleport is excluded; multiplayer remains a
requirement. Main owns integration; the investigation worker owns only the
shader interface/purpose evidence, not review or implementation.

## Checkable environment

| Fact | Status/evidence |
|---|---|
| Repository baseline | a75b141, existing x86 proxy/server and x64 host |
| Native render collection | [verified: bridge.cpp stereo and engine.cpp view hooks] original Render3D regenerates each eye; no brain/overlay replay |
| Request ownership | [verified: same files] one frozen Snapshot/Request; native-finally owner and resource generation; existing remote frozen pair |
| Cap geometry | [verified: scope_gpu.cpp, scope_geometry.hpp, scope-gpu-source-checks.json] actual native DIP, four exact copied hashes; 24 vertices, triangles901..922; full native affine |
| Remaining content admission | [verified: scope_pose.hpp] contentVerified remains false; geometry does not prove live material, skeleton or pass purpose |
| Root/gun boundary | [verified: owned Engine155FF0 and Sam4BDE0/4BF40/4C740, private bounded disassembly] sorted commands; local gun key8FF00; virtual Render dispatch4BF34; Render hook precedes gun view/projection/depth changes |
| Final color suffix | [verified: NATIVE_OPTICS_AUDIT.md] HDR disable is a copy, then separate final fade; a finished color source reinserted earlier receives suffix twice |
| Native Scope shader | [verified: NATIVE_OPTICS_AUDIT.md] Poly Bump LONG thiscall(self,args), ret4; stock single-sided helper7106 one shaRender; exact return7108 |
| Native fade | [verified: Shaders6E10/6FC0/70B0] stock500 changes to501 SRC_ALPHA/INV_SRC_ALPHA; alpha modulation; writes disabled. Double-sided args+38 selects extra culling passes; it is not an opaque flag |
| Shader input/purpose | [unknown] worker investigating actual VS version/outputs, live args association, color-versus-depth/shadow discriminator |
| Render target | [unknown] actual root RT format, viewport and native transfer settings must be observed; no assumption from HDR naming |

Existing native-finally cleanup cannot rely on GNU stack unwinding through native
SEH. Persistent owners live above the wrapper; cleanup must be idempotent,
allocation-free and must reject the entire pair on uncertain graphics state.

## Proposed incremental adapter

1. Reuse the existing native frame owner and frozen request. An ordinary eye0
   preview gathers the actual animated per-hand affine/geometry. These are
   copied values, never old native pointers or palette indices.
2. Up to two native scope-source Render3D invocations use explicit render purpose,
   exact current optic camera/FOV and distinct root query identity. Do not fake
   eyeIndex0 for a scope. Native collection/culling/animation still run normally;
   no simulation or HUD callback is repeated. Main world eyes then render and
   the original desktop rebuild remains once.
3. At the FIRST admitted local gun Render entry in each scope source, copy the
   current root scene color into a same-format owned RT texture, before any
   handheld gun or later effects. Continue native root execution normally.
   This replaces the rejected post-finish source; it does not build a new
   postprocess pipeline or require classifying every command after the gun.
4. At the exact actual native Poly Bump color draw, retain original VS, geometry,
   streams, raster/depth/stencil/scissor/blend and original native cleanup.
   Split the original928-triangle DIP into prefix901, cap22, suffix5, each
   triangle once. Only the cap uses an unlit scene-image pixel shader; changing
   the stock diffuse texture alone would relight the captured image.
5. Source image identity includes current request/input/generation, hand,
   weapon/model and copied actual cap affine/geometry. Final world cap must
   match the same frozen animated geometry. No previous-frame image/cache,
   revival after rejection, new IPC/wire, inventory or gameplay state machine.

This is more native views (at most preview+2sources+2eyes+desktop), not another
renderer. [Unverified] native animation/query/cache side effects under those
extra views are acceptable. Review must challenge this cost/assumption, rather
than merely approve local guards. A separate engine or finished-frame/depth
replacement is rejected because it duplicates renderer policy and loses native
foreground/blend ownership. Copying an XR eye and cropping is rejected because
it cannot supply an independently aimed magnified view.

## Optical frame and open decisions

Current lean: construct a proper rigid optical camera from the current full
cap affine. Transform the local outward normal by inverse transpose; choose
forward away from the rear cap. Keep reflections/shear in actual cap vertices,
not in the optical camera. Project the affine local Y onto the cap plane for
up; right/forward/up form a proper orthonormal camera. [Unknown] local rear
normal sign must be confirmed from owned cap winding/body extents before use.

Camera position is the actual cap center, not a guessed controller offset.
Physical radii come from actual cap points. FOV derives from those radii,
explicit eye-relief calibration and native per-hand zoom progress. [Unknown]
the exact native progress-to-magnification formula still needs a bounded read
proof; no field writes or shared owner858 FOV changes are proposed.

For image mapping, reconstruct the current world-eye ray from the actual native
adjusted projection/view/viewport. Convert direction into the rigid optic frame,
compress angular slope by magnification, then use source projection for UV.
Direct ray projection into a narrowed source FOV without angular compression
does not magnify content. All XYZ eye position affects an explicit exit-pupil/
eye-relief mask; reject malformed matrices and backwards rays. Reticle alignment
must reference the same native tracked ballistic direction as the laser.

Open fork: PS3 VPOS is convenient but its pairing with the actual native VS is
not established. Prefer the existing diffuse TEXCOORD interface if exact planar
UV-to-cap mapping can reconstruct the required ray without changing native VS.
Do not silently replace the original VS/skin program to make PS3 fit. Original
diffuse texture alpha multiplied by native effective color alpha must remain
under partial fade. Native alpha-test/state purpose require live admission.

Optional shader module hook would be one narrow Poly Bump Exec adapter, installed
only after the real module is loaded and fingerprinted, outside loader lock.
No additional shaRender hook solely to get return7108. [Unknown] exact safe
installation seam, original shader-args identity and pass discriminator remain
blocking implementation gates, not guessed production conditions.

## Review contract

Verify the load-bearing code/evidence before critique. Rank decisions by leverage,
choose your own deepest item, merge overlapping admission decisions, and seek
deletion/simplification. Return <=1800 words: prioritized critique, conditional
GO/NO-GO and exact smallest vertical proof. Distinguish decisions we can make
from genuinely human preferences. Do not re-review gameplay zoom/protocol/UI,
GPU lock containment or completed transport. Do not delegate or edit files.
Focus on pre-first-gun capture, extra native views, physical optical math,
native shader compatibility/color purpose, and failure containment.

Primary API constraints: [StretchRect](https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-stretchrect),
[D3D9 rasterization](https://learn.microsoft.com/en-us/windows/win32/direct3d9/rasterization-rules),
[HLSL semantics](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-semantics).
StretchRect color surfaces must satisfy actual format/pool/capability rules;
the additional outside-BeginScene restriction is documented for depth/stencil.
D3D9 pixel centers are integer window coordinates. VPOS is SM3, but that alone
does not prove compatibility with a native lower-version vertex program.
