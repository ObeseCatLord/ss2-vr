# Native thick-query counterexamples — Astra review

Astra/xhigh static kernel investigation and senior disposition. Main verified
effective routing in aggregate. No native execution, body move, edits or delegation.
The capsule-cover argument is geometrically conservative; the unchanged native
query kernel does not supply its required continuous-volume certificate. NO-GO
for that prototype as previously proposed. This does not justify a world/solver
rewrite. All addresses are RVAs; inspected Engine/Core match RESEARCH.md.

## Finite horizontal edge miss

Engine2F93A–2F990 selects effective radius from category flags. Model traversal
CB1ED→CAE80 computes scaled triangle vertices/normalCAFCF–CB054 and passes radius
CB05A–CB08D to Core mthIntersectThickRayTriangle1D3C0, float cdecl with six stack
arguments, x87 return. Positive direction dot normal returns miss~3e38 at
1D3ED–1D3F9 before any radius/finite-edge/vertex tests.

Synthetic geometry, not an extracted game asset:

| Value | Coordinates |
|---|---|
| Triangle A/B/C | (0,-2,0), (0,2,0), (0,0,4) |
| Face normal | +X |
| Sphere radius |1|
| Start centre |(1/8,0,-9/8)|
| Unit direction |(3/5,0,4/5)|
| Requested distance |1/4|

Centre remains on the front side. Squared distance to finite edgeAB changes
from41/32>1 to149/160<1. First contact is `(33-sqrt(639))/40`, approximately
.193039, inside the .25 move. Native dot=3/5 causes an immediate miss.

Otherwise the kernel tests a normal*radius translated triangle1D3FA–1D47F,
three finite edge tubes1D492/1D4B0/1D4CE and vertex spheres1D52E/1D588/1D5E0,
reducing entries1D5F3–1D66B. Existing geometry work is reusable; face-direction
rejection is the demonstrated incompatible boundary.

## Tangent support contact cannot be discarded by continuation

Core ray-sphere18F30 accepts zero discriminant18FC9–19005. Exact floor tangency
over a triangulation vertex can yield entry0 for a triangle extending ahead;
thick triangle allows dot=0. Engine model validationCAA60 accepts
`entry > rayMinDistance-rayRadius`, so0 qualifies for min0/positive radius.
Some tangent floor queries therefore permit no travel; not every floor query.

cldContinueRay29290 stores the previous hull in exclusion2D96EC at292A1;
rayContinue1B36B0 advances minimum by~max(1.2e-7,hitDistance*1.2e-7). Traversal
2F90B/2F911 excludes the entire hull. Skipping a floor contact in a model can
also hide its later wall. Returned triangle indices do not change this behavior.
Delete the assumption that continuation provides per-contact support filtering.

## Primitive initial-overlap inconsistency

Primitive CheckRay525B0/5278E uses Core19A40 radius expansion for sphere/box/
capsule/cylinder. Engine527C4–527EC validates only interval entry; at/below
minimum-radius it rejects even a positive exit. Query sphere radius1 inside
target sphere radius2 yields[-3,+3]; min0 threshold-1 rejects it. Shallow negative
entries may instead be accepted/clamped0 at291CF–291E2. There is no uniform
initial overlap/escape contract from the interval alone.

## Main disposition

No floor/material exceptions, whole-hull skipping, repeated restart policy or
unchecked placement is added. Initial-contact handling must retain other same-
model triangles and cannot rescue the finite edge miss by itself. Actual native
dimensions, body placement ownership and authoritative settlement remain gates.

ROOMSCALE_KERNEL_ADAPTER_REVIEW_BRIEF.md reopens a scoped mathematical-kernel
adaptation while retaining native traversal/filter/body/physics. That is a new
senior decision, not approval of collision implementation or full roomscale.
