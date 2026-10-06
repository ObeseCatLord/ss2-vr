# Roomscale query kernel — narrow adaptation decision

Main proposal after Astra's concrete native-kernel counterexamples. This
reopens the conservative-volume candidate, not the rejected collision/world/
physics rewrite. The completed Astra/xhigh decision below authorizes the model
kernel source vertical. Body placement and protocol are not implemented here;
no native execution occurred.

## Demonstrated incompatible boundary

Core mthIntersectThickRayTriangle1D3C0 returns miss at1D3ED–1D3F9 when
direction dot triangle normal>0, before finite edge/vertex tests. Exact horizontal
counterexample: triangle(0,-2,0),(0,2,0),(0,0,4), normal+X; radius1;
origin(1/8,0,-9/8), direction(3/5,0,4/5), length1/4. Edge distance crosses radius
at~.193039, but native query misses. Native continuation29290 excludes the whole
hit hull2D96EC/2F90B–2F911, so dropping a harmless floor contact can lose a wall in
the same model. Primitive initial intervals such as[-3,+3] can be discarded by
entry-only native validation rather than report existing overlap.

These are specific query-kernel/initial-contact incompatibilities. The native
world traversal, filtering, target shapes/transforms, actual body/solver and
checked placement remain reusable. The previous NO-GO does not demonstrate that
an independent character controller or collision world is needed.

## Minimal adapter versus broad reconstruction

A: reconstruct moving-convex collision and native traversal. Reject: duplicates
adjacent working geometry ownership, filtering/contact and solver policy.

B: narrowly adapt the two existing mathematical intersection boundaries ONLY
during one recognized roomscale query scope on the original simulation thread.
All other game rays/physics calls remain original. Native engine retains triangle
enumeration, transforms, category filters and nearest-hit reduction. No global
query tracker, entity cache, collision callbacks or custom world representation.

For model triangles, use a complete finite moving-sphere/triangle intersection
kernel: face, finite edge and vertex candidates; do not omit edges solely from
face-plane direction. Handle initial touch/overlap inside that SAME triangle
query before global nearest-hit reduction, so native traversal can still test
other triangles in the model. Do not use whole-hull continuation to skip support.

Possible initial-contact invariant: for a convex triangle, distance along a
straight ray to the closed triangle is convex. An already touching/overlapping
start with nonnegative directional distance derivative cannot deepen penetration
into that triangle; tangent floor travel can be ignored per triangle while a
different wall remains tested. This requires proof for feature transitions,
zero-distance/undefined-normal cases and numerical tolerance; not an approved
floor exception. A deeper initial move must block immediately. Avoid arbitrary
support-material/type exceptions or repeated ray restarts.

For primitive targets, preserve native positive-entry results. Initial-overlap
validation needs an actual geometric contract (native entry/exit alone is
insufficient for nondeepening contact). Review whether a narrow primitive-boundary
adapter can use native descriptor semantics without reproducing collision policy.
Do not invent tag values, dimensions, rotations or defaults. It is acceptable to
prove model-kernel vertical first, but final compound/primitive coverage remains
required; an unsupported-body restriction cannot become full equivalence.

## Native ABI and ownership requirements

Triangle entry is float cdecl(Ray3 const&,Vec3 const&a,Vec3 const&b,Vec3 const&c,
Vec3 const&normal,float radius), six stack arguments/x87 return. Primitive entry
returns Box1 through the verified hidden output buffer/EAX; do not rely on the
mod compiler's generic aggregate-return convention. Both Core mappings already
belong to the fingerprinted/pinned supported module set. Any hook installation
must reuse existing transactional rollback and cross-compiled ABI inspection.

Scoped query metadata is a small lexical value flag, with no native pointer
ownership or new persistent policy. Native SEH cleanup restores mod TLS only;
shared ray scratch ownership/restoration and physics-worker exclusion remain
separate evidence gates. Wrappers are pure math under that scope, no native
callback/allocation/body mutation. Full-volume capsule cover and exact native
dimensions remain the caller's responsibility, not inferred from ray radius.

## Senior decision and verification target

Is this the narrowest justified boundary after the demonstrated counterexample,
or does it still grow into custom collision policy? Challenge deletion and
complexity. Compare correcting/scoping native math with reconstructing whole body
sweep/traversal; identify native reusable lower helpers if they reduce code.

First source vertical would implement the model kernel and the exact missed-edge
counterexample, face/edge/vertex entries, grazing/tangent floor, initial overlap
escaping/deepening, reversed winding, degenerate/nonfinite cases and bounded
contact error. Wire only through the scoped actual native kernel before claiming
query behavior. Then certify actual capsule/primitive coverage, checked finite
placement, coherent origin publication and authoritative multiplayer acceptance.
No mock-only or mathematical pass completes body-follow collision.

If this needs target-specific compensating state machines, custom native collider
traversal/contact response or solver modification, reject B and record the precise
remaining incompatibility. Never replace adjacent systems merely because their
existing route is unverified. Full immersive objective stays intact.

## Final Astra/xhigh disposition and source extent

GO for the model triangle boundary, with substantial simplification: preserve
the original native TOI instead of implementing a new face/edge/vertex kernel.
When direction dot supplied normal is positive, swap B/C and negate the normal
before calling the original entry trampoline. This lets the existing edge/vertex
tests run. It is a conservative closed-triangle query, not exact one-sided native
gameplay equivalence.

Add only the missing initial-contact decision using the closest projection on
the closed convex triangle. Its separating support plane bounds distance over
the entire finite request. An explicitly budgeted nondeepening contact can be
ignored for that triangle; a deepening contact blocks at zero. Zero distance,
degenerate/nonfinite geometry, unsupported FP arithmetic or uncertain bounds
invalidate the entire scoped query. Never exclude the whole supporting hull.

| Finding | Disposition |
| --- | --- |
| Native early face-direction miss hides an edge collision | Winding adapter followed by original native TOI |
| Continuation excludes floor and wall in one hull | Per-triangle initial-contact decision before native nearest-hit reduction |
| Face normal cannot certify escape across closest-feature transitions | Closest-point support bound with explicit finite parameter/depth budget |
| Full moving-sphere kernel duplicates native math | Delete that proposal; retain original entry/trampoline |
| Shared scratch, primitives and actual body extent are not established | No engine activation, placement or body-follow claim from this source vertical |
| Native float TOI conservatism is unproved | Keep as an explicit certification gate; no guessed epsilon correction |

The six source/fixture/verifier files use a lexical POD query scope and the
existing hook registrar/native-finally boundary. CMake compiles the adapter into
both game and server DLLs. Compilation does not install or activate it. The
caller must establish exclusive native scratch ownership, compatible units,
minDistance0, positive actual radius, and an extent containing only the admitted
query. Independent source review and compile/offline checks precede acceptance.

The numerical contract is explicitly budgeted: distance(t) is at least
initialDistance minus contactDepthBudget, not exact zero deepening. The start
classification can include a bounded uncertainty band around contact; it does
not claim that a lower bound below radius proves overlap. Body certification
must account for that entire allowance rather than treating the helper's
ignoreNondeepening label as an exact-contact guarantee.
