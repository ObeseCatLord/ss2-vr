# Roomscale finite-move query adapter — design fork

Main proposal after the bounded native cld investigation. No source approval,
physics mutation or relocation is implemented by this brief. Astra/xhigh must
verify-then-critique. Multiplayer remains mandatory, teleport excluded, and no
native/Windows/XR execution is authorized.

## Demonstrated incompatibility and reusable components

Native full-volume static overlap exists, but no public full-player-volume sweep
with acceptance was established. CPrimitiveHull CheckMove52030 skips small moves
and uses a zero-radius centre ray for larger movement; checked mechanism placement
rejects partial fractions. Native motor position is persistent partial correction
with combined post-solve displacement. Neither fulfills direct finite physical
translation attribution. Native body, solver, contacts, shapes, collision query
filters, shared ray/continuation and checked placement remain reusable.

Compare two minimal adapters before an independent controller/solver rewrite:

A: reconstruct a full moving-convex/body sweep and world collision traversal.
Rejected lean: duplicates substantial native collision policy and data ownership.

B: conservatively cover the actual existing upright player capsule with bounded
native moving-sphere queries, take a common accepted fraction, then use original
checked mechanism placement for that finite relative translation. This supplies
only the missing finite-move query; native bodies/physics keep ownership. It is
a hypothesis, not proof that existing thick-ray queries are safe sphere sweeps.
Do not guess capsule dimensions, identifiers, placement or native query semantics.

## Checkable capsule cover and smallest vertical

If actual root collision geometry is a rigid upright capsule with radius r>0 and
spine length H>=0, cover its hemispheres with exact endpoint spheres of radius r.
Split the spine into bounded cells of length h; one sphere at each cell midpoint
with radius sqrt(r*r+(h/2)*(h/2)) covers that entire cylindrical cell. This is a
continuous-volume conservative cover, not endpoint-overlap bisection or a set of
zero-radius point samples. The extra radial allowance is explicit; native target
intersection may be additionally conservative. No exact-contact equivalence claim.

Each covering sphere travels the SAME finite horizontal delta with the existing
native thick-ray/filter/continuation path. Minimum permitted fraction applies to
the whole body. Preserve actual orientation and dimensions, normal native velocity
and all ordinary motion. Use checked native placement only after query certification;
read back its immediate accepted delta in that isolated pre-physics transaction.
Do not use combined later physics displacement as roomscale acceptance.

First proof is one stationary native player on a flat floor, with the actual
root capsule, one finite horizontal tracking delta, an intervening thin wall,
ordinary floor contact, zero/partial accepted distance and coherent origin update.
If query continuation cannot distinguish starting contact/overlap from blocking
travel, or radius does not produce a continuous moving-sphere certificate across
model triangles and native primitive pairs, reject this candidate before code.
Unknown compound/noncapsule/tilted bodies cannot be silently declared complete.

## Required native evidence before implementation

1. Actual root shape/descriptor/absolute placement can be read safely under the
   existing player/mechanism handles; pose/geometry ownership survive the call.
2. Native thick-ray path with radius is conservative for all relevant world/model/
   primitive targets over the entire sphere travel, including initial touching,
   overlap, tangent floor travel and cldContinueRay behavior. A capsule-named
   target routine alone does not prove this.
3. Shared query filters/results/reentry guard have a narrow simulation-preparation
   owner and restoration contract; no live physics-worker query or borrowed hull
   survives the request. Do not add a global query registry.
4. Native checked placement actually moves the player rig while preserving normal
   velocities/solver/contact ownership; immediate readback is attributable to this
   one finite move and lifecycle revalidation is possible after native calls.
5. Native loaded/authoritative player replication can accept exactly this physical
   request without double application or ordinary-position correction undo. If
   original transport cannot express it, demonstrate the narrow incompatibility
   before choosing any request field/ACK addition. Never infer from setter success.

The adapter may not consume rejected physical distance as speed-driven locomotion,
accumulate movement debt, resize/replace native player bodies, force solver scratch,
or grow into another character controller. Reopen design if conservative cover
blocks ordinary standing floor travel, needs policy compensating many target
types, or duplicates native traversal/contact response. Full roomscale and all
other immersive requirements remain required beyond the first vertical.
