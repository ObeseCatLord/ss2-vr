# Offline native weapon reference correction

The native first material stores Q=P_adjusted*V before computing and storing
R=Q*M. Its 24-bit x87 multiply/add order differs from collapsing M*L and V before
projection. The local palette L is applied separately by the vertex program.
The prior collapsed reference rejected valid copied weapon positions.

The V22 stationary private capture measured actual control127, eight cold0→6
producer pairs and eight reuse14→14 pairs. All cold Q/R words reproduce exactly
from independent M/V/P factors. Sixteen qualified copied geometries point to the
cold first producer, including both position-input layouts. Their3488 vertices
agree with the fixed staged reference: maximum absolute error1.177104891336711e-7,
zero components beyond the unchanged1e-5+relative1e-5 bound. The legacy reference
fails3924 components. Sixteen other copied geometries lack qualified association.
These are simulated, stationary, historical observations; no physical grasp,
winning second-eye event, GPU arithmetic or release acceptance follows.

## Implemented boundary

- `idle_native_reference.py` calculates fixed first-material Q/R addition order
  with bounded rational PC24/nearest arithmetic and binary32 stores. Only a
  source1, control127, cold0→6, already validated geometry association qualifies.
  Independent M/V/P are inputs. Cached matrices and actual c1-c4 corroborate
  outputs; they never seed the reference. Unknown earlier producers are excluded.
- `idle_position_replay` schema3 retains R and independent L separately. It
  explicitly rounds the local vertex stage and compares R*local_position using
  the existing bound. Schemas1/2 and program/input/influence guards remain intact.
- `replay_idle_geometry.py` computes legacy first, then selects a qualified native
  result regardless of pass/fail. `legacy_position_replay` preserves comparison;
  `reference_kind` identifies the selected method. Unsupported arithmetic retains
  per-copy legacy diagnostics but sets effective acceptance false. Structural or
  association corruption still rejects. A passing old result cannot rescue a
  failed native result. Newly derived geometry retains diagnostic provenance and
  false cleanup/outer-current/grasp/alignment claims.

The production renderer, collector raster reference, admission and tolerance are
unchanged. This correction unlocks offline geometry interpretation, not a guessed
controller offset. Native gun, muzzle/laser, scope and MP poses must remain coherent.
The recovered asset is the gun mesh, distinct from the authored hand annotation.

## Review disposition

Astra/xhigh gave source GO after these fixes. Main independently checked actual
outputs, selection, malformed-data behavior and compiled evaluator results.
Both local review turns report Astra/xhigh; backend identity remains unattested.

| Recommendation | Disposition |
| --- | --- |
| Preserve partial results on unsupported arithmetic | Adopted, per-copy typed exception; structural errors propagate |
| Restrict cold flags to observed0→6 | Adopted; flags1/8/9 and reuse are unsupported |
| Make native primary explicit and preserve legacy | Adopted; no caller selection or passing-choice search |
| Prove separate local rounding and both selection directions | Adopted, synthetic cancellation and native-pass/legacy-fail plus inverse tests |
| Keep rejected history outside readiness | Adopted, mixed/partial/historical integration checks |
| Infer cache lifetime, hand grasp or GPU precision | Deferred; missing evidence stays explicit |

All four products rebuilt on
`46e88dffe9e0e325faae37111f0be567f0d2dd1a0a4e66beed8a230bb07f3375`.
IPC10/wire7;72 Debug/72 Release groups and32 normal/optimized native/compiled gates
pass. Five pure helper groups and eight replay groups pass normally and optimized.
Tests use synthetic data; private assets/captures/disassembly are absent from source.
The new imported helper joins the twelve-tool seal for future fresh fixtures.
No new game launch, original deployment or individual firing test follows this fix.
