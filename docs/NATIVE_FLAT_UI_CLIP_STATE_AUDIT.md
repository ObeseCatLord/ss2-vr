# Native flat UI drawport, clipping and device restore

Main read-only, fingerprint-bound native inspection. This is a source gate for
the pending flat UI design, not an implemented draw adapter or runtime proof.
All disassembly remains private scratch; no proprietary bytes are retained here.

## Original drawport authority

Engine `_gfxCurrentDrawPort`8B5A0..8B652 activates the canvas when necessary,
compares the original drawport ID, invokes `_gfxCurrentRenderArea`, then requests
`_gfxScissor(0,0,-1,-1)`. Current logical dimensions alone are not the physical
device viewport or complete clipping region.

GfxD3D startup4CE2..4CE8 binds `_gfxCurrentRenderArea` to10F0..11CF. That body
uses physical drawport bounds14/18/1C/20 to construct the original device
viewport; invalid/empty bounds receive the native1x1 fallback. MinZ/MaxZ come
from native globals. It invokes device slotBC (`SetViewport`), then disables
render state174 (`SCISSORTESTENABLE`). Its BeginScene policy stays native.

Startup4BA7..4BAD binds `_gfxScissor` to65E0..679E. For an explicit subregion,
this body adds logical drawport origin4/8, intersects the rectangle with the
physical bounds14/18/1C/20, and compares the resulting region with the complete
physical region. Complete/reset requests disable174; a strict subregion
enables174 and invokes slot12C (`SetScissorRect`) with actual absolute physical
coordinates. The exported scissor cache is updated through the original path.
Future duplicate draws must use the actual queried device viewport, enabled
state and rectangle, preserving offset and resizing. The native cache is not
permission to assume a full-canvas viewport.

Engine `gfuOrtho`6E1A0..6E2FB requests original `mthOrtho` with left0,
right logical width, top0, bottom logical height and near0/far-1; then installs
the projection and identity matrix/cache state. This exact body does not
explicitly disable original depth, stencil or user clip-plane state. Neither
these calls nor default UI configuration establish all emitted vertex Z/W
values. Source clipping remains a separate gate; flattening invalid source
geometry would change the reference behavior.

## Main correction: HUD state is not glyph state

The complete CHUD body17C5E0..17C7EE sets its native640x480 orthographic
projection and explicitly invokes original depth-buffer disable, depth-write
disable and alpha-test disable at17C747/74F/756 before element callbacks.
That establishes the container setup, not all nested rendering state.

ProcRender glyph bodyF010..10122 explicitly invokes **depth-buffer enable**
atFF67 and depth-write disable atFF6F. Its actual effect-derived float3 vertex
buffer is transformed by original3x3/offset arithmetic atFD46..FED7, including
XYZ writes atFEC9..FECF. Dynamic text Z cannot be assumed zero. Its builtin3
draw binds position stride12, UV stride8 and color stride4 at10027/47/67, then
invokes original DrawVertices with primitive9C at1007C. Thus substituting a
fresh stencil/depth target cannot automatically preserve the glyph reference
depth result. Original compare, effective Z domain and depth independence
remain a material design/source gate.

Further main inspection bounds original Gfx depth-enable5000..5063: it only
updates the native enabled cache and requests D3DRS_ZENABLE7=true; it does not
force an always-pass compare. DepthFunc5640..569F maps the native comparison
through table11274 to D3DRS_ZFUNC23. Engine on-disk default2C3E50 is42, which
maps to D3DCMP_LESSEQUAL4, not ALWAYS8. This is the original default, **not**
proof of the current runtime compare after preceding native rendering.
[D3DCMPFUNC](https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dcmpfunc).

Native projection is also adjusted before reaching draws. Gfx startup4B3E..4B44
binds ProjectionMatrix to69A0..6CBF, preserving current source matrix then
calculating physical/logical drawport and pixel-center corrections into the
adjusted matrix. Original device constants at the immediate boundary are the
authority; rebuilding only the nominal640x480/gfuOrtho matrix would omit these
corrections. Main checked Core mthOrtho1EF00..1EF9C, but the authored near/far
arguments alone still do not establish the actual current glyph clip result.

## Admitted GPU families and program identity

Gfx startup4CBF..4CC5 binds DrawVertices9760..9C1F;4CCB..4CD0 binds
DrawIndices9C20..A057. The first body submits ordinary primitives through
device slot144 (`DrawPrimitive`) and special9C quad chunks through148
(`DrawIndexedPrimitive`). The second submits148. No UP submission appears
in these bounded bodies. Start with these two demonstrated families rather
than installing four draw hooks merely because the API offers UP variants.
This is not a claim that every custom native UI path uses these dispatchers.

Original vertex bind6EC0..6F51 and pixel bind6F60..706D resolve nonzero native
handles through Engine `_gfx_avppPrograms`2E6580. The stack-array data pointer
is field4, count8; records are12bytes, indexed by handle-1. **Both vertex and
pixel device pointers are record0.** Main corrected the earlier erroneous
vertex-record4 label: vertex6F25 reads data+12*handle-12; pixel6FD7..6FE6
reads the same record0. Actual CreateProgram7730 stores the returned device
shader at785A and uses record4/8 for other metadata. They reach SetVertexShader slot170
and SetPixelShader1AC. Pixel binding also applies its recorded sRGB sampler
policy. A future admission gate can compare the actual current device shader
objects against the bounded builtin handles and original current handles;
the enum alone is insufficient. Borrowed program collection lifetime and
readability must be checked; do not retain native record pointers.

Main additionally matched UseSimpleShader's matrix input2E6438 against the
actual exported `_gfx_mAdjustedProjection`, not raw current projection2E6398.
Instruction-aligned9456C onward combines that adjusted matrix with current
view2E6408. This closes the original constant producer's adjusted projection
identity; it does not establish every backend compiler transformation.
GfxCreateProgram7730 calls bounded compiler7220..772D, which calls original
Engine gfuParseShaderProgram then assembles/creates the device shader. Actual
compiled program position semantics remain a draw-admission gate; native source
strings alone do not authorize assuming an arbitrary generated device program.

## Immediate draw side effects

Microsoft specifies that `DrawPrimitiveUP` clears stream0 after the call and
consumes the supplied CPU data before returning. `DrawIndexedPrimitiveUP` also
clears the index-buffer binding. Consequently, a scoped duplicate adapter must
restore the **post-original** device state. Restoring pre-original bindings
would undo a native API side effect. Immediate duplicate UP operations can use
the same caller arguments only inside the wrapper, before the original caller
resumes; no future-frame CPU data retention is justified.
[DrawPrimitiveUP](https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-drawprimitiveup),
[DrawIndexedPrimitiveUP](https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-drawindexedprimitiveup).

The documented `D3DSBT_ALL` list includes vertex/pixel state, textures, streams
and their offsets/strides/frequencies, index buffer, viewport, scissor,
transforms and clip planes. Target/depth restoration must be explicit; the
adapter must check capture/apply/setter failures. A block scoped to additional
raw device calls does not authorize applying an old block across native render
callbacks and desynchronizing their caches.
[Saving all device states](https://learn.microsoft.com/en-us/windows/win32/direct3d9/saving-all-device-states-with-a-stateblock).

## Remaining bounded gates

Actual admitted program/device identity, draw families, source homogeneous
clip/Z admission, enabled stencil/user-plane behavior, owner-to-overlay
lifetime, all touched target restoration and full-field fade classification
still require proof. Device restoration must preserve post-original effects
on both success and rejected/partial duplicate paths. Appearance, concurrency
and performance remain unverified; no game, Wine, host or XR session was run.
