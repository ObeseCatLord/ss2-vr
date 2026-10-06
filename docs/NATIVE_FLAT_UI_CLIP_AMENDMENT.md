# Flat UI: clip-preserving minimal geometry amendment

Mostly-worked main amendment to780a37d. Solo mod, complete VR UI, OpenXR;
actual game/XR testing excluded. Original callbacks/assets/programs/RGB blends
remain native; no source GO or adapter is implied by this document.

Historical proposal retained for review provenance. Main's numerical proof
rejected the4mm volume. Astra selected a smaller synthetic clip-Z row retaining
strictly planarXYW and original programs, deleting both thickness and stencil.
The geometry-only helper now has bounded source GO after underflow and mixed
sourceZ/W corrections. See NATIVE_FLAT_UI_CLIP_SOURCE_REVIEW.md; GPU/pair
integration is still unfinished. No shader compiler dependency was introduced.

## Established facts and incompatibility

| Fact | Evidence |
|---|---|
| Glyph XYZ is dynamic, depth test enabled/write disabled | [verified: main native] F010..10122, FF67/6F and FD46..FED7. Original enable5000..5063 does not force ALWAYS; default native42 maps to LESSEQUAL4. Current comparison/domain remains unknown. NATIVE_FLAT_UI_CLIP_STATE_AUDIT.md. |
| Matrix-only flattening cannot preserve arbitrary sourceZ clipping | [verified: mathematics] Flatten source clipXYZW into planarXYW is rank deficient: differing sourceZ with identicalXYW have identical eye output, so output clip planes cannot distinguish source near/far admission. |
| Source bounds and shaders reusable | [verified: main native] Device actual MVPc1..4, texturec5, builtin object records; source physical viewport/scissor semantics and two draw families established. Same audit. |
| Programmable clip planes operate in output clip space | [verified: primary API] Microsoft SetClipPlane: plane dot output>=0, enabled separately. D3DCAPS9 MaxUserClipPlanes is runtime admission, not assumed. |
| Existing pair/comfort reusable | [verified: source] Rendering→Ready, hosthudAnchor, once-only native brain owner and executed eye fullP capture. Earlier Astra conditionalB decision stands. |

## Main selected presentation policy

VR flat overlays should stay legible over the world. Duplicate UI draws disable
destination depth test/writes while original desktop draws retain native depth.
This is an intentional VR presentation adaptation, not a claim of identical
desktop destination-depth visibility. Native effect-derived sourceXY geometry,
source near/far/viewport/scissor clipping, native texture/color/alpha/RGB blends
and original draw order must remain. No change to gameplay/world depth.

## Two narrow designs

1. **Main lean: invertible thin panel volume plus six user clip planes.** Keep
   the original GPU program pair. Source device clip coordinatesq are mapped
   through original physical viewport into the stable full-canvas panelXY;
   source normalizedZ0..1 maps into a small bounded panel thickness (proposed
   4mm, subject to numerical proof). The resulting4x4 source→eye clip mappingH
   is invertible. Duplicate MVP is H times original deviceMVP. Native source
   XYZ effects are evaluated originally, without CPU buffer copies.

   Four source viewport/scissor halfspaces and sourceZ>=0/W-Z>=0 are mapped
   through inverse transposeH to six D3D9 user clip planes. These preserve
   clipping before geometry reaches VR, including triangles crossing bounds.
   Eye frustum clipping remains natural. Ignore disabled stale scissor. Query
   actual source clip/stencil state; original enabled user planes/stencil are
   not silently discarded. Admit only demonstrated state/capability, reject
   the pair on incompatibility. Minimum six planes, finite/conditioned H and
   residual/sign preservation required. State/targets/viewport restored after
   original draw under scoped serialization. **Delete the proposed scratch
   stencil surface and mask draw entirely.**

   Native sourceZ changes add at most4mm of UI depth over its source clip range;
   this is an explicit small presentation difference. Preserve layout at
   sourceZ0, not a half-slab offset. Use relative panel/eye poses to avoid large
   native-world translation cancellation; the common tracking-rig correction
   cancels consistently. Use actual adjusted executed eye projection, not a
   guessed frustum or old root projection.

2. Flat panel with a narrow new UI shader retaining original source clip data
   and discarding before fragment blending. This could retain exact planarXY
   without thickness but replaces otherwise-working programs, introduces
   shader compiler/bytecode/formula/declaration admission, and still requires
   deliberate destination-depth policy. Main rejects it while route1 can be
   proven adequate. No broad shader rewrite or arbitrary effect reconstruction.

Original fresh-stencil-only matrix design remains rejected. Scalar-alpha capture
still loses destination-dependent blends; multipass RGB coefficient banks,
callback replay and future draw-buffer recording remain rejected.

## Scope and bounded proof

This amendment settles geometry/clipping/presentation depth only. Pair owner,
native fade scopes, pre-UI world dimming and host capability/comfort ownership
retain earlier reviewed requirements. No new transport/queue/protocol policy.

Main will implement/check the transformation as one portable production helper
and prove mapped clip signs with independent geometric oracles: asymmetric eyes,
tilted/translating panel, nested offset viewport, enabled subrect/disabled stale
scissor, dynamic sourceZ crossing both bounds, negativeW, thickness/conditioning
and nonfinite/empty rejection. Then wire actual builtin object/state and both
immediate draw families under existing transaction. Offline checks are math/
source evidence, not native rasterization/performance/stock coverage proof.
If numerical stability forces inflated thickness, a shader/state manager or
renderer-like growth, reopen instead of silently changing the policy.

## Astra contract

Explicit Astra/xhigh, verify effective settings; read-only, no edits/delegation/
runtime. Read AGENTS.md, senior-review skill, this amendment, existing design
review and clip audit. Objective: conditionalGO/NO-GO on thin-volume clip planes
versus narrow shader at this demonstrated incompatibility. Challenge sign,
interpolation, source clipping, depth policy, device availability/serialization
and numeric conditioning; seek simplification. No review of turret/scopes/
transport/head/roomscale/melee. Return <=900words evidence-backed terminal
verdict with smallest next production proof; don't re-investigate the native
bodies or introduce confirmation gates. Main owns all writes/integration.

[SetClipPlane](https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-setclipplane).
