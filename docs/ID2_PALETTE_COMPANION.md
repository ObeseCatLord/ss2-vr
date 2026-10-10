# Auto Shotgun draw-local palette diagnostic

This opt-in diagnostic copies the selected native ID2 draw's world matrix,
one to three palette mappings, each referenced global canonical bone matrix,
and the actual D3D9 shader, constants, declaration and stream bindings. It is
separate from count-one affine geometry admission. It does not enable native
multi-palette replay or weapon alignment.

The existing submission owner reserves its ordinal before the first AddRef.
At most ten API payload attempts are retained independently of the geometry
copy counter; failed attempts consume capacity. The native palette stays in
the existing bounded submission row. Existing COM owner slots retain the
pre-sample shader identity through the post-sample and cleanup. No additional
allocator, native cache replacement or protocol is introduced.

The required joined bracket is native copy, API copy, native copy, original
draw, native copy, API copy, native copy. Every copied native bookend itself
requires stable draw/root/configuration/cache identities. Every mapping must
belong to that draw and rendered model, reference a supported canonical bone,
and fit the copied native spans. Raw world/canonical/palette finiteness is
checked without inventing a single affine for a multi-palette draw. Canonical
versus palette disagreement is retained as diagnostic evidence. Repeated
owner/generation/routing guards perform no foreign capability queries. Device
admission queries precede the sampled interval. When legacy geometry was
already certified, the companion serializes its copied program/constants and
retained canonical bindings without releasing or querying them again; the
post-original API snapshot must match, or that geometry observation is retired.

The `Lab idle paletteSummary`, `paletteRow` and `paletteData` schema1 records
join by request, eye, hand, submission index and ordinal. Detailed records
require normal successful original DIP return, matching native/API bookends,
post-Release generation/routing validity and normal complete outer weapon
cleanup. They are emitted after that cleanup from copied values only. A
rejected count-one geometry observation can still supply this diagnostic;
the rejection and geometry limits remain unchanged. Summary records alone
are not qualified copies.

No shader register base, local-index upload association, independent camera
reference, authored grip, rendered visibility or physical release is certified
by this receipt. The next offline consumer must establish those relationships
from the actual program and independent native/reference evidence. Unknown or
fractional shader addressing remains rejected. Do not derive an independent
reference from the GPU upload being checked or relax bounds to obtain a pass.

The existing selected `SS2VR_LAB_IDLE_WEAPON=2` exception is diagnostic only.
Source instrumentation does not authorize fixture grant/equip or game launches.
Current user capture scope, isolation and immutable-fixture rules still apply.
Raw constants, addresses, bytecode, captures and asset identities remain private.

## Offline reader

The existing `assess_idle_weapon.py`/`idle_submission_evidence.py` collector path
now reads the schema1 companion. It ties detailed rows to their original
submission, enforces the reserved API index and ordinal, checks all copied
array inventories, and compares the duplicated draw including signed base bits.
Native matrices must be finite; raw API constants and stream descriptors remain
diagnostic, including unused NaNs, zero stride and non-unit frequency. This
reader does not introduce position admission for those inputs.

Explicit ID2/schema4/copyLayout1 metadata can retain `bone=-1` as multiple or
unresolved scalar evidence. A detailed multi-mapping receipt additionally
requires `boneName=0`. IDs1/13 retain their single-bone bounds. A contradictory
equality flag, wrong owner/configuration/mapping, zero request, duplicate data
or a truncated detailed row rejects. Consistent canonical/palette disagreement
remains data for diagnosis.

Complete arrays establish completeness within an emitted detailed row only.
Failed/suppressed samples create gaps in the original numbering. Missing detail
cannot establish complete API coverage: the producer summary has no count of
qualified emissions. The reader labels completion as emitter-reported and keeps
source authentication, content, shader-index association, GPU visibility, grasp
and alignment verification false. Collection output stays private through the
existing collector path; no parallel launch or replay mechanism is added.

Reader verification:27normal/-O regression cases and7existing Debug/Release
collector/matcher/replay consumer groups pass. The preserved ID13 V42 log was
re-read with7complete copied observations and25diagnostics. This read of existing
evidence launched no target and does not add runtime acceptance. Current products
still match the native fingerprint below; Python-only changes require no rebuild.

The Astra/xhigh bounded reader review approved the incremental design and source
after independently reproduced zero-request and contradictory multi-bone-name
counterexamples were corrected. Local current-turn routing was verified; backend
attestation remains unavailable. The next unresolved relationship is the actual
ID2 draw/program/index upload association and independent camera/grasp reference.

| Reader recommendation | Disposition |
| --- | --- |
| Extend the existing submission reader | Adopted; existing collector and owner/ordinal grammar reused. |
| Preserve passive raw constants and descriptors | Adopted; unused NaN/zero-stride/non-unit-frequency regressions accepted as diagnostics. |
| Join duplicated draw words explicitly | Adopted; signed-base round-trip and crossed-draw rejection checked. |
| Reject zero requests and contradictory multi-bone names | Adopted; both counterexamples reproduced and fixed. |
| Do not infer coverage or position acceptance | Adopted; absent details stay unknown and verification claims remain false. |

Portable checks cover native bounds, crossed canonical/palette rows, every outer
completion gate, payload capacity, row/ordinal mismatch, missing/interrupted
API copies and Release-time retirement. Source-order checks state their bounded
lexical limits; compiled checks inspect the contained wrappers and eight-vector
scratch cleanup. These do not prove native execution or visual alignment.

## Bounded Astra review and verification

Current-turn local routing was verified as gpt-6-astra/xhigh; the effective
backend was not independently attested. The read-only review returned source
GO after these corrections:

| Finding | Disposition |
| --- | --- |
| Capability callbacks outside the sampled interval | Repeated guards are scalar; capability queries occur before the bracket. |
| Added queries disturb certified count-one geometry | Serialize certified pre-state while retaining original references; post-state mismatch retires geometry. |
| Outer completion precedes final cleanup | Companion emission occurs after normal original/physical-pass/finally completion. |
| Shared reader accidentally widens affine admission | Single-affine policy remains count-one and invertible; copied palette policy is separate. |

All four source-matched products rebuilt with fingerprint
d836a37c958f4e16c6efa8ca3e010adf8c1b3951afbe075c4e064353fbbdf8c1,
IPC10/wire7.81Debug and81assert-enabledRelease groups passed. Fifty existing
normal/-O game/server native/compiled/artifact gates and eight joined-submission
and palette-scratch checks passed against the final objects. No game, headset,
firing, movement, vehicle or network test ran. This is diagnostic source
readiness; actual ID2 association and all-weapon alignment remain open.
