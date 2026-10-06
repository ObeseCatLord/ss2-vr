# Complete flat UI: Astra design review and main disposition

Baseline3332f60, mostly-worked NATIVE_FLAT_UI_DECISION_BRIEF.md. Explicitly
selected gpt-6-astra/xhigh; main independently verified those effective settings
for all three reviewer turns. Reviewer Volta the2nd was read-only and is closed.
No implementation, runtime or installation was performed by this review.

Main correction after this historical review: both vertex and pixel native
program-record device pointers are at record0, not vertex4. Adjusted projection
is the actual simple-shader constant producer. See
NATIVE_FLAT_UI_CLIP_STATE_AUDIT.md. The later synthetic clip-Z geometry source GO
is NATIVE_FLAT_UI_CLIP_SOURCE_REVIEW.md; complete GPU integration remains pending.

## Final consolidated Astra critique

Faithful normalized record of the terminal critique; not a verbatim transcript.

**Conditional B remains the preferred architectural direction; NO SOURCE GO.**
The matrix-only projection plus fresh-stencil-depth variant is not established
as a native-equivalent adapter. This consolidated verdict incorporates main's
dynamic-Z correction and supersedes earlier versions.

Selected architecture: retain the once-only native overlay invocation, duplicate
admitted immediate GPU operations into existing eye colors, and extend the
existing Rendering→Ready transaction through the original overlay owner.
Preserve native assets, effect evaluation, blending and layout.

A scalar-alpha capture cannot express demonstrated destination-modulating RGB
blends. Additional coefficient images introduce machinery and must preserve
intermediate UNORM saturation. Callback replay contradicts demonstrated text/
time/expiry/COW mutation. These favor B's ownership/draw boundaries, but do not
settle geometry/depth adaptation.

1. Dynamic effect Z makes source-depth semantics the first proof obligation.
   Main verified glyph FD46..FED7 transforms mutable float3 vertices through
   effect-derived XYZ calculations and writesXYZ atFEC9..FECF. The same path
   enables depth testing atFF67 and disables depth writes atFF6F. Earlier HUD
   depth disables and ortho setup do not establish constant-Z/depth-independent
   text. Dynamic Z disproves the premise; it does not itself prove rejected
   pixels. Fresh stencil/depth discards original contents. Eye-world depth is
   not automatically equivalent to native screen-space depth comparison.
   Establish actual compare, effect-Z domain and source-depth relationship.
   If redundant, justify a narrow override. Otherwise main may explicitly
   select VR panel legibility as a presentation difference; it must still
   preserve source near/far clipping and effect-derived screen geometry. No
   such policy is selected by this review.
2. A panel mask cannot repair discarded source clipping. CurrentRenderArea
   supplies physical viewport bounds and native depth range. Scissor applies
   logical origin and physical clamping; disabled scissor leaves stale device
   rectangle data. Map nested viewports into one stable panel, without
   stretching each. Ortho does not disable user planes/depth/stencil. With
   dynamic Z, flattening before source near/far admission can expose geometry.
   XY stencil cannot recover it. Solve source clipping before flattening,
   separately from destination-depth policy. Authorize scratch stencil only
   after this boundary is worked; avoid native-stencil emulation, mask caches
   and arbitrary-effect reconstruction.
3. Keep two demonstrated draw families and exact program admission. Native
   DrawVertices reaches DrawPrimitive9BB7 and indexed quad chunks9B82;
   DrawIndices reaches DrawIndexedPrimitiveA00B. Main connects glyph builtin3,
   its native streams and primitive9C at1007C. No UP appears in these bounded
   bodies, so two device hooks are the minimal demonstrated set. Actual
   12-byte native records supply vertex+4/pixel+0 device shader objects; admit
   the bound builtin pair rather than an enum. Duplicate immediately with
   original resources/arguments, exclude adapter mask draws, and eliminate
   command recording/borrowed histories. Other path coverage remains unproved.
4. Use one enclosing owner/completion transaction. Brain EB7B0..EB87C resolves
   the puppet separately before world/overlay. A scoped pending pair belongs
   to that invocation; stereo populates it and original overlay completes it.
   Missing overlay/alternate branch/nesting/owner or epoch change/partial
   failure retires the transaction. Freeze both executed eye transforms before
   current capture resets. Existing guards own final commit. Empty completed
   UI is valid; draw counts do not define completeness. No separate HUD queue,
   expiry or readiness state machine.
5. Restore post-original API state through one local guard. Desktop original
   draw once, preserve HRESULT, capture post-original state, duplicate/restore
   synchronously, return original HRESULT. Failure prevents pair publication.
   UP clears stream0 and indexedUP clears indices; these are conditional
   requirements for future admission/mask work, not native UP evidence.
   Scoped D3DSBT_ALL preserves draw state; explicitly preserve RT/depth.
   Restore mask-to-duplicate state and targets before final viewport because
   target changes reset it. Device serialization is required; TLS recursion
   alone is insufficient. Avoid a shadow state manager.
6. Preserve composition order and narrowly classify fades. bridge.cpp587 dims
   copied world pixels; retained eye colors remain undimmed, host HUD follows.
   Dimming after native UI changes brightness and destination-modulating RGB.
   Resolve the pre-UI color stage and cost. Fades follow earlier content and
   inherit blend. Tag exact two scopes, with source coverage/state admission
   for full-eye duplication. Eliminate accumulated-color replay and large-
   rectangle heuristics.
7. Reuse one comfort owner; remove duplicate presentation. Update existing
   host hudAnchor during request preparation and freeze panel pose/dimensions.
   Apply frozen eye rig correction, retaining thresholds/resets/comfort
   admission. Do not clamp panel offset as head position. Preserve native
   aspect instead of stats width/4. Suppress generated stats according to
   the actually presented pair, including cache reuse. No new image channel,
   XR layer or comfort scheduler.

Remaining proof starts with glyph depth compare/effect-Z, source near/far/user
planes and active stencil. Trace an effect text draw and nested clipped rectangle
through the existing owner/two draw boundaries, then finalize masking/policy.
Production helper checks should cover clipping, offset viewport, disabled stale
scissor, both eyes, owner change, partial failure, restore and composition order.
The350–600-line estimate is conditional. Do not build a renderer or multipass
RGB bank for unknown effects. Full coverage/appearance/performance/concurrency
remain unverified; main owns architecture, policy, edits and integration.

## Main verification and disposition

Main inspected the actual bridge.cpp477..628 and native instruction-aligned
boundaries recorded in NATIVE_FLAT_UI_CLIP_STATE_AUDIT.md. Dimming is indeed
CPU-only after readback; existing eye targets are undimmed. Glyph depth-enable
and dynamic XYZ are established; constant zero Z is not. Minimal two native
GPU families and current object record mapping were independently checked.
Microsoft separately confirms target changes reset viewport, scoped state-block
coverage and UP post-call clears. A review verdict is not native runtime proof.
[SetRenderTarget](https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-setrendertarget),
[state-block coverage](https://learn.microsoft.com/en-us/windows/win32/direct3d9/saving-all-device-states-with-a-stateblock),
[DrawPrimitiveUP](https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-drawprimitiveup),
[DrawIndexedPrimitiveUP](https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-drawindexedprimitiveup).

| Recommendation | Disposition and concrete consequence |
|---|---|
| B ownership/draw boundary | Adopt. Original callbacks once, two immediate GPU families, existing enclosing pair transaction. Source route remains gated. |
| DynamicZ/depth first | Adopt. Withdraw matrix-only/fresh-depth sufficiency. Prove actual glyph predicate/domain before adaptation; no assumed Z0. |
| Exact source clipping | Adopt. Physical viewport/scissor authority; disabled rectangle ignored. Do not allocate proposed mask resource yet. |
| Actual builtin objects | Adopt. Current actual VS/PS compared with native handle records, with lifetime/readability; no enum-only admission. |
| One owner and both copied views | Adopt. Pair cannot commit before original overlay. No new HUD queue or successful-draw counters. |
| Post-original restoration | Adopt. Preserve real HRESULT and binding side effects; target/depth explicit, viewport last; serialization remains a gate. |
| World dimming before UI | Adopt. Resolve existing color stage before destination-dependent UI; no silent order change. |
| Exact full-field scopes | Adopt. Two native scopes plus coverage/state, no heuristic rectangle classifier. |
| Existing comfort ownership | Adopt. Existing hudAnchor/settings, frozen request geometry and cached-pair capability; remove duplicate stats only for complete admitted native UI. |
| Rewrite/multipass/recording | Reject those alternatives as proposed production routes. They duplicate native policy/lifetime and exceed the narrow adapter estimate. |

## Next implementation gate

Complete the actual source-depth/clip proof, then refine the smallest geometry
adapter under this conditional decision. User-friendly destination UI depth is
main's implementation decision, but any intentional difference must be stated
and reviewed; it cannot be called native-equivalence proof. No user permission
question is needed for read-only proof or authorized reversible implementation.
No stock coverage or full-goal completion is claimed. Teleport stays excluded;
OpenXR/fullXYZ tracking/multiplayer and remaining immersive work stay required.
