Conditional GO for the bounded GPU implementation after the amendments below; NO-GO for enabling it or claiming completed remote head animation until the missing gates close. No production changes or game/host/Wine execution occurred; Atlas's CPU consumer investigation was not duplicated.

Verified baseline: HEAD a45c67b; only pre-existing documentation changes. Independently matched Engine/Core/GfxD3D fingerprints, disassembled the stated native boundaries, checked the stock skeleton, and exercised the existing affine helpers against an independent double-precision reference. The 256 cases covered scale, shear, reflection, rotation and translation; maximum component error was 5.72205e-6, with singular inversion rejected. Native evidence and offline check artifacts are in this scratch directory.

1. P1 — Pair validity is the highest-leverage decision; a boolean query alone is insufficient.

The brief overstates current binding validation. remote_render.cpp:118 checks wall-clock age, and :134 calls it even for frozen draws (:237). Consequently an otherwise unchanged sample can adapt the left eye and expire before the right. currentBinding also does not re-read the body CModelInstance or compare against current multiplayer incarnation: its incarnation comparison is between the supplied sample and the stored binding. During frozen rendering those are the same snapshot. commitStereo (engine.cpp:804) protects local snapshot state but has no remote validity condition. Multiplayer presentation (:642) itself folds expiry into valid=false, and relay replacement (:376) does not advance bindingEpoch.

Deep specification:
- freezePair must establish one transaction-scoped immutable bank before either eye: admitted sample, head validMask bit, normalized finite Q, body-eye E, handle/incarnation/tracking identity, exact body instance, and existing presentation eligibility. Admit freshness once. Empty/disabled banks remain valid native-only cases.
- During both eyes, use only that sample/E/eligibility. Separate lifecycle validation from age checks and sample selection. Re-resolve handle, reject local/dead/replaced avatars, and verify current body instance and raw multiplayer identity. Do not obtain a newer head pose or let presentation's age-expired valid flag become a second-eye veto.
- Maintain one sticky invalid flag for the transaction. A lifecycle change or validation fault affecting an admitted adaptation rejects the complete pair; never repair it by silently skipping only a later eye. Visibility differences or ordinary absence of mapped Head slots are not themselves faults.
- Serialize final remote validation through commitNativeFrame's Ready transition, alongside existing slot/snapshot ownership. A query that releases bindingLock before Ready has a check/use gap unless same-thread ordering is established. A narrow acquire/validate/release guard is sufficient; relevant deletion, incarnation replacement and tracking invalidation must participate, or their serialization must be proven. Establish lock order before adding nesting; do not hold render-long locks or add a coordinator.
- Existing bindingEpoch is conservative but increments on every observePlayer (:378). Prove observations cannot interrupt a pair, or narrow its invalidation semantics to incompatible lifecycle/eligibility changes. A newer compatible pose should wait for the next bank. Keep invalidity through desktop restoration and commit; endEye only disables frozen consumption. bridge.cpp:591 restores desktop before :605 commits.

2. P1 — Specify native mapping provenance and sentinel handling before touching copied matrices.

Verified bone stride 0x28, owner+0, parent+4, definition+0x24 (Engine E19E3–E19F5), and Core's hidden-output string-to-ID ABI (Engine 1BA0; Core export 5F930). The stock resource has Head parent Neck and no Head children; this proves no broader resource coverage.

Two native cases need explicit representation. E16FB–E1711 creates the synthetic root with owner 0, parent -1 and null definition. DDE92–DDEAE treats map bone index -1 as a fallback matrix. Do not dereference either as an ordinary named bone or reject a legitimate root as a cycle. Preserve an unmatched slot's existing palette bytes.

Require aligned membership of current record and draw entry in their bounded arrays before dereferencing. Draw entries have stride 0x20 at pointer2EAC54/count2EAC58/capacity2EAC50. E1C75/E1E24–E1E2D also proves map.first stores the draw-entry index: validate it against the current entry. Validate start/count against map and palette lengths/capacities using overflow-safe arithmetic, then match retained pointer/count exactly.

Resolve one owned Head across the native bones, not merely the current upload span. Traverse validated native parent indices, with bounded work and explicit root termination; reject ambiguous ownership/cycles atomically. Multiple palette references to the same Head are legitimate and all selected slots need adaptation. Leave other slots byte-identical. Same-instance, same-record ownership does not establish attached-model coverage; do not silently extend this slice across owners.

3. P2 — Delta algebra is sound; anatomical equivalence is intentionally narrower.

DDE30 copies model-local P; E12C5–E12D2 installs W separately; DB140 establishes B=P*inverse(S). Thus W*P' = (E*Q*inverse(E))*W*P when P'=inverse(W)*(E*Q*inverse(E))*W*P. Production packet generation (engine.cpp:531) uses bodyHeadTracking, supporting Q as a tracking delta. Reuse affineInverse/affineMultiply, check finite results, and take exact identity through native passthrough, including quaternion sign-equivalent identity.

This preserves native animated offsets; it does not force the skull origin onto the tracked eye. Pure rotation moves an offset anatomical point by (R-I)*(H-E). Deep XYZ crouch can substantially stretch Head/Neck blended vertices. Retain XYZ as requested, but do not claim anatomical tracking or IK. Position algebra does not independently prove lighting/normal correctness under nonuniform affine transforms; that remains a shader/visual gate.

4. P2 — Adopt the synchronous consumer seam; reject the retained-setter arena.

Verified shaBoneMatrices merely retains three values. shaBindBoneMatrices 775B0–775D0 performs only a cdecl callback(firstRegister,count*3,data); caller645A6 returns645AB and passes17. GfxD3D6330 submits those inputs at vslot94 and preserves native profiling. Microsoft's SetVertexShaderConstantF input-array contract supports call-scoped storage; the examined native wrapper adds no retained copy. Gate the callback to the fingerprinted backend, use bounded per-invocation storage, and preserve exactly one original-wrapper fallback or one native-callback adaptation. Reentrancy must not overwrite a shared buffer. No setter/global-pointer changes, bone evaluation calls, animation replacement, or protocol changes are justified.

Recommended disposition, for main's decision:
| Decision | Recommendation |
|---|---|
| Consumer adapter | Adopt |
| Affine tracking delta | Adopt; retain anatomical/normal limitations |
| Native Head/descendant mapping | Adapt with provenance, sentinels and bounded traversal |
| Frozen-bank commit predicate | Adapt into serialized commit validation plus sticky failure |
| Setter arena / skeletal replacement | Reject as unnecessary lifecycle/behavior duplication |
| GPU-only completion | Reject; CPU coverage remains required |

Missing gates: concrete x86 hook/callback ABI verification; production-path tests for identity/nonselected byte preservation, repeated mapped Head slots, malformed/ambiguous graphs, overflow, singular W, reentrancy and exactly-once fallback; transaction tests for age crossing, deletion/incarnation/body replacement, compatible new samples, freeze races and invalidation immediately around Ready; verified native draw/LOD/material and normal coverage. User-observable stereo behavior remains unverified under the no-runtime constraint. No human preference blocks this architecture; main retains final integration and acceptance.

Reviewer note (resolved by main): the senior-review skill requires “Verify the reviewer's effective settings.” Effective model/effort metadata is not exposed here, so the required Astra/xhigh designation cannot be certified independently.

Main independently verified the effective reviewer as gpt-6-astra/xhigh using only model/effort metadata. No telemetry was exported. This review preceded the producer alternative below; it does not approve that alternative.
