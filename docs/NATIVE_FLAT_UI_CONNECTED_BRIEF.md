# Connected once-only native UI

Source integration on7295b76; final Astra/xhigh SOURCE GO recorded in
NATIVE_FLAT_UI_CONNECTED_REVIEW.md. No runtime testing/deployment.
This implements the accepted geometry/boundary in the existing stereo transaction.
The smallest adapter keeps original callbacks, rendering resources, shaders,
blend/color/alpha/order and reliable multiplayer protocol. It adds three native
owner/fade hooks, two immediate draw adapters and rejection-only output observers;
no new renderer, HUD state model, frame queue, shader parser or XR layer.

## Native ownership and device output

Original BrainRenderViewEB7B0 owns original playerRenderView then original
RenderOverlay105480 once. Complete UI additionally requires exact world return
FEA8A, exact overlay returnEB85D/flag0, unchanged brain+28 resolved player and
native handle identity, native main thread and live frame/rider/epoch. The
existing Rendering slot remains Rendering through both original world images,
original desktop rebuild, original overlay and enclosing owner return. Partial,
nested, changed, absent or unsupported ownership rejects the entire pair.

Immediate DrawPrimitive/DrawIndexedPrimitive preserve original desktop operation
first and its HRESULT. The queried current native builtin VS/PS objects and
sourcec1..4, actual POSITION0/default/stream0/FLOAT3or4 declaration and bound
position stream/frequency admit original position semantics. The original
program's UV/color/resources/blend and draw arguments stay unchanged. Both added
eye draws use the accepted flat panel transform and six source clipping planes,
with destination depth tests/writes disabled for legibility. Source fog,
stencil, user planes and disabled clipping are rejected; active MRT is rejected.
Only triangle list/strip/fan panel footprints are admitted; point/line/UP/patch
outputs affecting the desktop reject the entire pair.
The original depth surface stays bound because matching nonMSAA targets and
formats/dimensions are preserved and UI depth/stencil are inactive.

The scoped color-output domain duplicates only those two draw families. UP and
patch draws, target clears, fills, copies and updates affecting the desktop reject
completion; offscreen outputs remain native and their later desktop composition
must admit. The actual original swapchain must have a nonlockable backbuffer,
excluding direct Surface LockRect/GetDC output. Another swapchain backbuffer is
rejected rather than classified as an intermediate. Device method addresses must match all installed routing/admission
hooks on the current vtable. Creation identity/thread, !PURE/!MULTITHREADED and
six user planes are queried, not forced. Unknown opportunity falls back to the
existing stats/world presentation, rather than claiming complete native UI.

Only modified raw state is saved: RT0, vertexc1..4, viewport/scissor, depthenable,
depthwrite, scissorenable, clipenable and six planes. Restoration attempts every
member, with target first and both raster rectangles last. Any unresolved
restore failure halts stereo adaptation until successful Reset/new device.
A whole-state block would desynchronize native caches and is unnecessary.
Persistent explicit owners cover queried/retained resources and pitched locks
before foreign entry; failed readback unlock retains its extra lock reference
through bounded cleanup and reset quarantine, halting world AND menu reuse.
Inner draw/observer and original owner/callback paths use
the accepted native-finally boundary. Native SEH is not converted, swallowed or
claimed recoverable. Standard COM HRESULT/nonthrow contracts remain required;
arbitrary crash recovery and unhooked engine concurrency are not established.

## Fade and presentation order

Exact native gfuFill returnsFE90A/FEA07 retain original native colors/gradients,
blend and once-only timing. Only those scopes admit full physical viewport,
scissor-off basic draws as full-eye fades. Other ordinary fills use the flat
panel transform. Head-comfort visibility<1 is applied before UI with the existing
CPU lookup quantization, writable pitched rows preserving native alpha, unlock
and matching SYSTEMMEM-to-DEFAULT UpdateSurface back into existing eye targets.
Visibility1 avoids the extra transfer. Final readback occurs after both UI images;
RGB is not dimmed again and transport alpha alone becomes255.

ABI8 freezes panel pose/full-canvas aspect/physical size into Request432. Response
NativeUiComplete is separate from immutable request data; Slot33554880 and
Shared83887736. Native wire6 remains unchanged. Host updates the existing35-degree
comfort anchor once, uses current menuWidth/menuDistance for the full native
canvas and retains generated stats at existinghudWidth/hudDistance as fallback.
Completion capability is cached only after both uploads/releases and cleared
with every cached-pair invalidation. Stats suppression follows the final surviving
complete projection, including reuse, with revalidation after HUD waits. A final current quit/session-loss guard
after all UI waits clears every layer and world accounting before submission.

## Evidence and limits

Full x86 proxy/server, x64 host/official loader and all14 portable offline groups
pass. New checks cover alpha preservation/channel quantization and exact immutable
UI/action-stream identity. Geometry witnesses and compiled native-finally evidence
remain bounded offline proofs. No native callback/GPU/exception, host/game, Wine,
OpenXR, multiplayer session or installed-game write was executed. Creation/main
thread opportunity, dynamic native UI coverage/appearance, performance/deadline
and Proton/runtime selection remain unverified.

The user explicitly excludes actual testing. Full goal remains active: native
zoom and optic images, roomscale body collision, physical swing melee, broader
vehicle implementations and complete remote head lifetime/enablement remain
required. This UI slice is not full immersive feature equivalence.

Primary API references: [programmable clip-plane space](https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-setclipplane),
[vertex input declaration mapping](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx9-graphics-reference-asm-vs-registers-input),
[D3D8/9 position semantics](https://learn.microsoft.com/en-us/windows/win32/direct3d9/mapping-between-a-directx-9-declaration-and-directx-8),
[UpdateSurface pool/format/locking requirements](https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-updatesurface).

## Corrected declaration evidence

Main rechecked exact native input mapping rather than adding the deleted shader
parser. All six builtin vertex sources are CTvs.1.1. Parser92C50 removesCT; its
92DFD version-upgrade branch is specifically guarded by ps.1. prefix211470,
not a vertex upgrade. Import206494 is strHasHeadS; pixel/vertex prefixes211478/
211480 establish the branch identity. The optional dclv0 at8FCF7 belongs to
upgraded pixel source. Thus builtin vertex sources stayVS1.1 and their position
v0 uses the legacy POSITION0→register0 declaration mapping. Earlier preparation
wording “optionalVS1.1→2.0” and initial connected-brief “dcl_positionv0” were
incorrect; original m4x4/c1 and assembler-path proof remain unchanged. No native
source/disassembly is retained in this repository.

[Surface GetDC restrictions](https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3dsurface9-getdc)
and [swapchain presentation parameters](https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3dswapchain9-getpresentparameters)
support that narrow output-domain gate. This is queried device admission, not
an instruction to force or rewrite the native swapchain.

## Runtime correction: backend input declarations

The historical POSITION0 admission above was contradicted by actual device
observation. The native builtin draw uses stream0/offset0/FLOAT3/default/TEXCOORD0;
UV/color are TEXCOORD2/3 in their original streams. Two-phase GetFunction reports
a124-byte VS1.1 program with a DCL of TEXCOORD0 into INPUTv0. Engine source strings
remaining VS1.1 does not establish the final backend declaration contract.
Previous bounded native findings identify API2 parsing (GfxD3D725E), injected
dcl_texcoordN vN (Engine93425..93461), assembly (GfxD3D6DAA), device shader creation
(77CC) and TEXCOORD/stream-index declaration construction (98D4/98DC,9D84/9D8C).
These are fingerprint-bound notes, not published proprietary code.

The current narrow gate uses that position element after original builtin program
identity validation. Other semantic indices, POSITION/POSITIONT, changed stream,
offset, type or method remain unsupported. Original draw arguments, buffers,
shader objects, constants, state restoration and once-only owner remain native.
No production shader parser or rewritten program is added. See
RENDERING_SENIOR_REVIEW.md and PC_GAMEPLAY_READINESS.md for current acceptance;
historical source GO did not certify runtime UI or the old POSITION0 assumption.
