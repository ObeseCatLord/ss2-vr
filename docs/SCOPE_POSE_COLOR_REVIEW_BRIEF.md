# Astra scope pose / color boundary brief

Solo native VR mod; incremental adapter, no alternate renderer/skeleton/gameplay state machine. Review the next scope slice after the completed physical gun projection/depth adapter. Actual game/headset/host/network execution is excluded by the user; static owned-file inspection and offline builds/checks are authorized. All reviews require Astra at explicit xhigh or higher. This is a design and evidence review, not permission to describe an optic as implemented.

## Environment / verified base

| Fact | Evidence and status |
|---|---|
| Workspace | ss2-vr repository in the installed game directory; owned modules in ../Bin. No installed-file writes. |
| Baseline | Commit c13de96; IPC6/wire4, native two-eye Render3D transactions, existing native reliable multiplayer and shooter authority. [verified: source] |
| Physical gun depth | Actual initial root execution P/view/depth is frozen; caller-scoped owned gun setup uses it while preserving original authored camera inputs and native model animation. [verified: engine.cpp, weapon_view.hpp, WEAPON_DEPTH_SOURCE_REVIEW.md] |
| Scope skin | All884 vertices weight255 to local palette slot0, named Sniper; pinned skeleton has one Sniper root, identity inverse-bind. [verified: tools/audit_sniper_geometry.py plus docs/sniper-skin-geometry.json; owned archives only] |
| Surface/resource | Runtime string ID translation, current weapon model instance, configured CMesh/selected section/draw route proved. [verified: NATIVE_OPTICS_AUDIT.md] Same-path loaded content still needs validation. [unknown] |
| Public bone query | Engine export mdlGetBoneAbsolutePlacement D8D60..D8F97, cdecl(instance,IDENT,Matrix34&)->int; identity-root native evaluation, animated/stretched world×bone result; DAD90 resets shared renderer arrays on ordinary0/1 returns. [verified: native bodies, audit] |
| Query reentrancy | Shared arrays clobbered; exception cleanup and complete lifetime/query-to-draw correspondence unproven. [unknown] Never insert it inside a draw. |
| Existing adapter interaction | Query setup invokes hooked DBC90; current postModelPass reads latest remote bindings outside frozen eyes. [verified: remote_render.cpp and E2670 native body] Must explicitly exclude public-query callers, not assume remote instances cannot overlap. |
| Root color stage | renFinishRender enables HDR before root Execute; resolves/disables HDR and applies final fade afterward. [verified:14BFA0..14C0DB] Current post-root laser phase is before those effects. |
| Magnified optics | No capture/aperture/zoom gesture implementation. Native right/left zoom guard semantics are being investigated independently; do not infer weapon+BC from the different Params+BC field. [verified status] |

Static scratch supporting instruction evidence: /tmp/ss2-vr-equivalence/scope-lens/{absolute-query.txt,query-setup.txt,world-stretch.txt,finish-render.txt}. inspect_native.py uses pefile/capstone from deps/python and owned files; known instruction starts only. Do not commit disassembly/assets. The newly added skin audit/tool changes are uncommitted and within this review's scope. Previous physical-gun review is closed and need not be repeated.

## Incremental choices / current lean

1. Lens pose: use the native public bone query once before any optic/world collection, only at an observed native main-thread/owner boundary with all shared collection/evaluation state demonstrably idle. Copy the result; ordinary query cleanup completes before Render3D. Retain native animated stretch and prove the left-hand temporary mirror order. Use the same tracked gun placement calculation as weaponAbs, extracted narrowly if necessary; do not make an independent pose policy. Later validate admitted actual draw world/palette against the predicted cap and hide optics on mismatch. This is preferable to nested query calls or a second skeleton evaluator. **Unknown:** idle-state proof, exception restoration, query-to-draw evaluation/lifetime. Review whether this query is necessary or whether one ordinary preceding gun draw can supply a smaller safe pose sample.

2. Color stage: change the previously approved full-finished-image retention detail. Continue extra full native Render3D capture(s) before both world eyes on an existing eye target, but retain the actual root output immediately after original root Execute, before HDR resolve/fade. Insert aperture at the matching post-root phase in ordinary eyes. Check actual target/format/dimensions rather than assuming the eye backbuffer is currently bound. Existing source/final depth/target identity gates remain. This avoids demonstrable final HDR/fade duplication. **Unknown:** root-local postprocessing already baked into the retained image, effect-specific parameter dependence, native target matching and shader transfer representation. Alternative: new after-FinishRender insertion boundary with complete retained world depth/native view restoration; rejected as the initial lean because root is destroyed and native depth/target may have changed. Reject a foreground host quad or a custom HDR pipeline.

3. Content/cap admission: runtime Scope name/path alone insufficient. Validate loaded native position/index/weight mapping plus loaded skeleton against the pinned profile at a native idle/resource boundary, with bounded buffer access and lifetime identity. Avoid every-vertex public GPU locks. Prefer one bounded native buffer lock per relevant slice when its ABI/lifetime is established. A later instance-specific native draw adapter can exclude exactly the rear cap triangles901..922; never mutate shared materials/index buffers. **Unknown:** native buffer lock/layout, actual draw subrange and cap suppression boundary. A guessed depth bias or textured disk over a retained opaque cap is insufficient.

These choices overlap: a prior ordinary native draw could establish both actual pose and loaded cap/draw identity with fewer query/ownership layers. Challenge whether that is the smaller vertical proof, including whether a previous-frame sample can remain valid for animated lenses.

## Review contract / output

Read-only Astra review. Verify load-bearing assertions cheaply from the artifacts/owned native files before critique. No delegation, edits or runtime. At most1200 words: prioritized flaws with evidence, smallest recommended next implementation slice, explicit GO/NO-GO per slice, and a deeper specification for your highest-leverage decision. Review the skin audit extension for correctness and honest provenance. Do not reopen transport, multiplayer codec, physical gun depth, roomscale/head/vehicle/melee/UI design. Missing evidence is a technical gate, not a user decision. Main owns spot-check, disposition and final integration.
