# Primitive initial-contact adapter

2026-10-06, root-owned source and offline verification. This is an inactive
component of the unfinished roomscale query, not implemented body movement.
The existing native triangle adaptation is unchanged.

## Native contract established

Pinned Core.dll fingerprint remains
7a1bd56b9bfa3edfbb23f4d3c96490e40a7c0b031b85e797e1af91b2ba3cf207.
mthIntersectThickRayPrimitive is at19A40, with four cdecl slots: explicit Box1
output pointer, Ray reference, PrimitiveDesc reference and float query radius.
It returns that output pointer in EAX, with ordinary caller stack cleanup. The
mod does not rely on the GNU compiler's aggregate-return convention.

The descriptor is16 bytes: kind at0, width at4, height at8 and depth at12.
The native jump table selects sphere0, box1, capsule2 and cylinder3; type4 shares
a cylinder ray helper but its underlying exact shape is not established and is
rejected in the optional scope. Sphere/capsule/cylinder width is a diameter;
box dimensions are full extents. Native constants are +/−0.5. The cylinder
quadratic uses X/Z, while slab/capsule height uses Y; the capsule spine half-length
is half-height minus radius. Malformed sizes and an inverted capsule are rejected.
Native empty Box1 uses entry3e38 and exit−3e38.

## Bounded mathematical addition

The demonstrated native negative-entry rule can discard an initial overlap even
when the sphere remains inside the primitive. Entry/exit values alone cannot
prove whether a proposed movement deepens that overlap. The new adapter computes
an outward support plane of the entire convex primitive from an approximate
nearest feature, then certifies its projection using outward-rounded binary64
interval arithmetic under the same FP-environment admission as the triangle
component.

For an axis n and support function h, the plane separation is
(n·p−h(n))/|n|. Over the entire finite ray its lower bound is the initial lower
bound plus min(0, bounded directional derivative) times the maximum parameter.
An independently bounded upper initial signed distance is retained. A skip is
allowed only when the plane's uncertainty fits the explicit contact-depth budget
and its whole-travel lower bound stays above initial upper distance minus that
budget. The support functions are those of the full sphere, box, capsule or
cylinder, so no later feature of that same primitive is hidden by a local normal
or whole-hull continuation shortcut. No source of impact normals is fabricated.

An initially separated sphere still invokes the original native interval kernel.
An unexpected origin-straddling native interval is rejected for the whole query:
expanded box/cylinder bounds can otherwise make native entry-only filtering hide
later contact. Nonfinite intervals and a different returned output pointer also
reject the query. Deepening, unsupported or uncertain initial contact similarly
marks the whole optional query failed and returns an empty interval. Outside a
scope the original call, arguments, output storage and result are unchanged.

## Verification and limits

All39 local Debug/Release groups pass. New tests cover tangent contact, shallow
and deep overlap, outward escape, inward movement, corners, sphere/capsule/box/
cylinder cases, unsupported descriptors, invalid arithmetic, output forwarding,
and native overlap rejection. Eight thousand deterministic random cases compare
admitted skips against independently computed long-double signed distances at65
points per path. Sampling is regression evidence, not the supporting-plane proof.
The same checks pass ASan/UBSan with leak checking disabled and compile for both
Windows architectures without execution.

verify_roomscale_primitive_abi.py checks the pinned export, jump table, scalar
constants, hidden-output ABI, axis/size evidence, compiled four-slot forwarding,
native-TLS entry alignment and output-pointer validation. Normal and Python -O
verification pass. Existing triangle ABI, resource-gate ABI, native-finally and
artifact checks are retained. Both x86 products build; no game/Windows/headset
or network runtime is executed.

Native separated-start TOI floating-point conservatism is still unproved. The
owner must establish actual query provenance, shared scratch/worker ownership,
resource cancellation, primitive coverage and orderly cleanup before enabling
these hooks. Actual avatar geometry, finite checked placement and authoritative
multiplayer/origin settlement remain unfinished. No movement is enabled by this
component, and a successful helper result does not certify a roomscale fraction.

## Local capsule-volume cover

The next pure geometry component covers a copied native capsule descriptor in
its own Y-axis frame. Endpoint spheres cover the hemispheres; each cylindrical
cell uses radius at least sqrt(r² + halfCellLength²). The implementation enlarges
the spine outward, bounds cell-centre/radius rounding and rounds final float
radii upward, so a finite set of spheres covers the continuous capsule volume.
The caller supplies the permitted relative query-radius increase; the helper
rejects shapes that cannot fit within that allowance and32 cells (34 total
queries). It never changes the actual native collision body or guesses its size.

Tests sample1025 cross-sections for20 size/aspect combinations using independent
long-double distances, and check exact spherical capsules, zero allowance,
overlong bodies, invalid descriptors and underflow. All40 local groups pass in
Debug and Release; the cover also passes ASan/UBSan (leak checking disabled) and
both Windows compile-only checks. World-space transform/quantization bounds,
reading actual player geometry, native query ownership and movement/replication
remain caller work; this local cover is not activated body movement.

## One shared cancellation state

The combined runRoomscaleCollisionScope entry requires both mathematical hook
bindings and nests them under the resource boundary using the SAME scope.failed
location. Resource cancellation cannot leave an earlier mathematical result
looking successful; nested/interrupted scopes retain their existing failure
semantics and native-finally cleanup. The small context lives above both foreign
unwind boundaries and contains no native resource ownership. This composition
still requires a separately established fresh-ray owner and orderly native
cleanup. It is not called by engine attach or an active movement path yet.

## Upright cover placement on the world float grid

The cover helper can now translate the already-built upright local spheres to
world coordinates. It outwardly grows each query radius by the centre's float
quantization bound and refuses a caller-owned absolute radius budget overflow.
It does not rotate a tilted body, normalize a pose or resize native geometry.
An exact zero local offset keeps its radius unchanged; large world coordinates
that cannot represent a nonzero offset accurately can reject the whole cover.

Independent long-double checks establish that each placed sphere contains its
translated local sphere at positive/negative world offsets through100000.
A ten-million-unit offset deliberately fails the tested budget. All41 local
Debug/Release groups, cover ASan/UBSan and both Windows compile-only fixtures
pass. Native body capture, query ownership and placement are still unconnected.

## Affine image cover for rotated hulls

`placeAffineCover` now encloses the image of each local cover sphere under a
supplied finite nonsingular3x4 matrix. An outward interval bound on the maximum
absolute row sum of A-transpose-A bounds the squared operator norm. Query radii
grow by that scale and the world-centre quantization bound; exceeding the caller's
radius budget rejects the entire cover. Reflection, rotation and shear are not
silently normalized away. The existing upright exact-centre shortcut remains.

Independent long-double sphere-boundary checks exercise the90-degree swimming
orientation and a sheared/stretched matrix, plus singular/nonfinite/budget refusal.
All41 local Debug/Release groups, cover ASan/UBSan and both Windows compile-only
fixtures pass. This does not yet bind an actual native hull matrix: the native
primitive path expands the raw quaternion with particular float spills, while
the general presentation `matrix(Pose)` helper normalizes it. That presentation
helper must not be substituted as evidence of the exact native collision frame.
Native transform admission and full movement integration remain unfinished.

## Raw native-quaternion envelope and complete copied-body cover

The new `roomscale_hull_transform.hpp` follows the pinned primitive-hull matrix
expansion, including its four intermediate binary32 spills and final coefficient
stores. It bounds the raw quaternion rather than normalizing it. The body-layout
verifier now pins those stores and the identity scalar. Interval cofactors also
bound the inverse transpose: the native local-ray path uses M-transpose, and
finite-precision M must not simply be assumed exactly orthogonal. The resulting
envelope contains both forward-placement and transpose-inverse query geometry.

The affine cover now accepts a coefficient envelope and covers every matrix
within it, including world-centre quantization. `coverBodyGeometry` builds the
complete captured one/two-hull union, reserving half the caller's explicit radius
allowance for frame/position uncertainty. Every final radius still obeys the
original total allowance. Any failed shape discards the entire result; no partial
body cover is usable and no native body is resized. A sphere descriptor is handled
as a spherical capsule with its actual diameter.

Two thousand generated normalized input quaternions are rounded to native float
storage and checked against an independent long-double reconstruction of the
spill sequence and inverse transpose. Both must fit the envelope. Tests also
cover the rotated two-hull body and all-or-nothing failure on its second hull.
All41 local Debug/Release groups, both cover/body sanitizer fixtures and both
Windows compile-only fixtures pass; native layout checks pass normally and with
Python optimization. This remains inactive geometry preparation. It does not
certify native floating-point TOI, acquire shared query scratch, move a body or
settle multiplayer origin changes. Unsupported FP environments still reject.

## Whole-finite-path clearance mode

The combined resource/triangle/primitive entry now requires an explicit
`requireWholePathClear` scope. For initially separated triangle and primitive
candidates, the existing outward supporting-plane bound must keep the entire
finite path beyond the query radius. An uncertain path invalidates the whole
query before a native floating-point miss could clear it. Existing initial
contact skips retain their complete nondeepening/budget proof. Unscoped native
calls and explicitly selected legacy contact-only fixtures remain unchanged.

This deliberately conservative mode can reject some genuinely free paths when
its chosen supporting plane cannot certify them. It does not compute a clipped
movement fraction, replace native traversal, certify broad-phase coverage or
supply shared scratch/body ownership. The future movement owner must still
consume the complete native query result and sticky failure together. It cannot
interpret a scope return alone as permission to move.

Regression controls use an intentionally false native-miss stub on paths that
cross a triangle or primitive; neither path is admitted. Two hundred plane-bound
cases and the existing8000 generated primitive paths check strict certificates
against independent long-double distances. All41 local Debug/Release groups,
x86 products, both kernel ABI verifiers, both kernel sanitizer fixtures and
x86/x64 compile-only fixtures pass. No native/game runtime was executed.
