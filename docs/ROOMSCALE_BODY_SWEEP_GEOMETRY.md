# Actual root-target sweep geometry

This is inactive geometry preparation, not an authorized native query or enabled
body movement. It consumes an owned `BodyGeometry` capture and the exact root
candidate delivered by the checked-placement boundary.

## Native propagation evidence

Model pose and root pose are distinct. The reader now also copies root absolute
placement and each hull's stored relative placement. Parent-attached roots are
refused. Root and hull active-notification flags at `+0x4c` must contain bit0,
otherwise native placement can omit the callbacks that maintain descendants or
broadphase state. Changes to these fields invalidate capture comparisons.

Pinned Engine CAspect::SetRelPlacement `571d0` reads parent `+4`, uses its absolute
placement `+0x2c` (identity for a root), copies the new relative pose to `+0x10`,
writes absolute pose to `+0x2c`, and conditionally calls virtual `+0x1c`.
CHybridBody's slot targets CBody::OnMoved `10f460`, which retains the native body
broadphase update and calls CAspect::OnMoved `57b70`. That routine walks the
entire child/sibling chain (`+8`, `+0xc`), recomposes every child's absolute pose
from the new parent and stored relative pose, writes it, and invokes an active
child's moved callback. CPrimitiveHull's callback `509d0` retains native hull
broadphase maintenance before its own descendant propagation.

Thus translating old absolute hull centres alone is insufficient: native commit
recomputes orientation/position under its original arithmetic mode. The existing
root-target boundary intentionally does not move the FP math frame across these
native commits/callbacks.

## Conservative construction

`nativeChildPoseEnvelope` follows the audited raw quaternion/position arithmetic
without normalization. Every arithmetic intermediate is widened to outer
binary32 neighbours, enclosing x87 24/53/64-bit significand precision and each
rounding direction, including stores and subnormal values. Nonfinite/overflow
bounds are refused. This envelope is deliberately larger than exact native
output; it is not a replacement implementation of native placement.

`coverBodyRootSweep` bounds the quaternion component box between original and
candidate root orientations. The possible recomposed child offset is separated
from linear root displacement. Two outer float spacings at the complete world
coordinate range bound the final translation add/store. The existing actual
hull transform is included too, rather than assuming its cached pose exactly
matches recomposition. Forward and inverse-transpose raw-query transform bounds
feed the existing affine sphere-cover builder.

Each query direction is float-quantized, with positive outward distance as its
maximum ray parameter. An interval bound on the resulting endpoint error is
added to every query radius. This also bounds proportional error at every point
of the segment. All cover, recomposition, world-grid and endpoint uncertainty
must fit the caller's explicit radius allowance. The routine refuses the whole
request if any hull cannot fit; it never changes the avatar's own dimensions or
silently omits a hull. At most 68 sphere queries cover two admitted capsules.

The quaternion box defines a conservative interpolating geometric path; it does
not assert that the native setter animates through intermediate poses. Broad
orientation changes or far-away float grids may be rejected by the allowance.
Zero displacement is refused because it does not define this ray traversal.

## Validation and remaining work

46 portable groups pass Debug/Release. Generated scalar child-pose checks cover
3,000 arbitrary quaternion pairs with nearest/down/up binary32 and wide scalar
oracles. Sweep checks sample standing and two-hull swimming bodies, varied world
positions/yaw, small orientation changes, intermediate path points and capsule
surfaces. Regressions include missing active flags, attached roots, stale root
or relative poses, invalid second hulls, failed reads, zero movement, nonfinite
input, excess hull counts and unrepresentable world-grid allowance. The three
new/extended suites pass ASan/UBSan (workspace leak checking disabled) and compile
for Windows x86 and x64 without execution. Native layout verification passes
normally and with Python optimization.

These samples supplement the interval construction; they are not an exhaustive
native-geometry or runtime proof. Query ownership, actual native traversal,
checked setter integration, post-commit readback and local/network origin
settlement remain required. No Windows/Wine/game/headset session was run.
