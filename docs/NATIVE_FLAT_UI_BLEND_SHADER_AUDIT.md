# Native flat UI blend and simple-program evidence

Read-only native implementation investigation, not an Astra review or an
implemented UI adapter. Terra worker inspected the bounded HUD/text/helper
family; main independently checked load-bearing facts and corrected the
preliminary claim that simple shader alpha could not be recovered statically.
No runtime, installed changes, proprietary shader source or disassembly retained
in the repository.

## Fingerprint-bound native facts

Sam/Engine/GfxD3D fingerprints remain installed-build.json. Newly inspected
ProcRender.dll SHA256 is
e93a51da739e6a4f0b66457af7de750befbd9514eb8180afe15513ca8d3427f2.
Any future adapter using its ABI must add a corresponding compatibility gate.

| Native blend type | Actual D3D9 source / destination RGB factors |
|---|---|
| <=500 | Blending disabled |
| 501 | SRCALPHA / INVSRCALPHA |
| 502 | DESTCOLOR / SRCCOLOR |
| 503 | ONE / ONE |
| 504 | SRCALPHA / ONE |
| 505 | DESTCOLOR / ZERO |
| 506 | ZERO / INVSRCCOLOR |
| 507 | ONE / INVSRCALPHA |

Worker decoded Engine8BD00..8BD63 factor table2C3E70/74 and original
GfxD3D54B0..5564 factor conversion112A4. CHUD17C758 requests501 before
original dynamic element prepare/render. ProcRender CTextQueue background
requests501 atD633..D678. CTextInQueue RenderF010..1012F reads current config
B4 atFF4F..FF58, then shader3 and native position/UV/color streams; no selector
range check is demonstrated there. Config constructor10810..10904 writes501
at108E9; assignment/copy preserve it at101B5..101BB/10746..1074C. Native
effects alter emitted vertices/alpha through original virtual callbacks.
Texture helpers inherit blend rather than forcing a universal mode.

Main independently inspected the original config constructor and text queue
Render2D10D20..10E80. The queue performs original per-entry render and expiry/
deletion, reinforcing once-only native ownership. Default501 is established;
all loaded/custom configurations being501 is not established.

## Main correction: zero cache is not absent shader source

The on-disk Engine runtime program cache2E6F2C/30 is zero-initialized, but this
does **not** imply program semantics are unrecoverable. Main found exact
padding-separated startup94440..9447D and deletion94480..944B3. Startup loops
over six pairs of static source-string pointers at2C40E0/4, submits them through
the original program creation dispatcher2E62F0, and stores the returned native
handles in2E6F2C/30. The deletion body calls original vertex/pixel delete for
those same pairs. Native source strings are present in the owned Engine image.
No replacement shader or speculative runtime inspection is needed to learn
these bounded built-in formulas.

Main inspected all six pairs without retaining their proprietary source.
Native vertex programs0..5 transform position by the four vector constants
c1..c4; textured2..5 also apply c5 to UVs. Color variants1/3/5 pass vertex color
through. Pixel1 passes interpolated vertex color; pixel3 multiplies sampled
texture RGBA by that color; rectangle variant5 does the equivalent rectangular
texture sampling. Thus these actual source programs establish position and
alpha semantics, while final compiled-device identity/resource admission still
requires a bounded gate. Shader0/2/4 use original uniform color. UseSimpleShader
944C0 selects program pairs and handles rectangle-texture variants; native
MVP/other uniforms remain original engine policy.

Further main instruction-aligned inspection establishes the bounded
UseSimpleShader body944C0..94C2F. Its final original call94C21 invokes exported
_gfxVertexProgramConstantsF2E62FC with start1, count5 and an invocation-local
five-vector span; first four are position MVP and fifth is texture scaling.
GfxD3D startup4BED..4BF3 binds that exact export pointer to6330. The original
6330..638A forwards start/pointer/count to IDirect3DDevice9 vtable178
(SetVertexShaderConstantF), with no register renumbering demonstrated. This
closes the specific native-source c1..c4→device-register mapping for these
built-in programs, without changing native cached matrices or shaders.

No claim is made about arbitrary user shaders, dynamic element paths, text
config values, GPU program translation, effect-derived texture/vertex alpha,
all color-write/separate-alpha state or target/cache restoration.

## Narrow architectural consequence (unreviewed)

Single transparent capture is smaller for admitted normal-alpha draws, but
destination-modulating RGB operations cannot generally be represented by one
background-independent scalar-alpha OpenXR quad. A fail-closed501-only subset
would not by itself establish complete native UI equivalence.

An alternative duplicates immediate **GPU draw operations** into the two
retained native eye targets during the original once-only overlay callback,
borrowing existing immutable eye views/P and substituting only the built-in
position transform c1..c4. Keep original shaders, texture/vertex streams and
actual RGB blend operations; do not replay element/text/time callbacks. Draws
from the exact two native full-field fade sites retain full-eye coverage,
rather than following a panel. Existing Rendering→Ready ownership must extend
through the actual original overlay owner; readback/commit happens only after
complete admission. Native desktop device constants/targets/cache state must
be restored exactly. Comfort panel geometry should reuse existing thresholds
without duplicating a parallel HUD policy. This is a candidate, not source GO.

Required next proof: actual built-in native program→device constant/register
identity, admitted draw-call/stream families and nesting; actual original UI
drawport/scissor transforms; complete target/depth/viewport/constant restoration;
once-owner and immutable pair lifetime. Compare this minimal draw adapter with
the smaller transparent-capture subset before Astra chooses. No shader-bytecode
rewrite, custom quest policy, extra model evaluator or transport queue is
justified by a zero runtime cache.

Microsoft documents separate target-alpha control and the required device
capability; the default uses the RGB factors for alpha too. The pinned OpenXR
chapter specifies source-alpha/premultiplication flags and linear layer
composition. These API facts support the stated constraints, not actual SS2
capture correctness. [Microsoft render-target alpha](https://learn.microsoft.com/en-us/windows/win32/direct3d9/render-target-alpha),
[Khronos rendering, release1.1.53](https://github.com/KhronosGroup/OpenXR-Docs/blob/release-1.1.53/specification/sources/chapters/rendering.adoc#composition-layer-blending).

Current host uses B8G8R8A8/R8G8B8A8 UNORM swapchain formats and unpremultiplied
flags for its generated HUD/wheels. A captured premultiplied image cannot inherit
those flags blindly; native/runtime color interpretation must be deliberate.
Existing comfort starts after35degrees, stops within8 and slews at90deg/s;
menus/wheels/native control policy remain reusable. No complete UI follows from
this audit, and the active full-immersion goal remains incomplete.
