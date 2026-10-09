# Private laser regression diagnostic

The ID1 alignment observation exposed missing aim lines while native stereo world
rendering continued. Their active rejection or visibility cause is unknown.
The next neutral capture uses passive records at existing boundaries, enabled
only by the private lab's existing `SS2VR_LAB_TRACE=1`. Normal runs do not log them.
This adds no ray, native model getter, alignment fallback or gameplay input.

Six stages each admit at most1024events: Cache, Muzzle, Query, Sample, Freeze and
Draw. Atomic counts include suppressed events; admitted events carry cumulative
per-stage drop counts and global event order. Each event can contain a fixed
number of companion lines. A saturation marker records the first dropped event.
Later silence after saturation is not evidence that an operation stopped.

Schema1 records request/input sequence, generation, owner/weapon/hand and eye where
available. Muzzle attempts join the corresponding Query request by event order
and exact input/owner/weapon/hand identity, never by stale object dereference.
Publication/invalidation copies calibration under its existing lock, then logs
after unlocking. Caller values are private native addresses, not public captures.

`muzzle` records cache validity/handle/tick and the existing sampled time;
alignment eligibility and whether capture was actually attempted; capture result
and failure block. A short-circuited capture is unattempted, with failure UINT_MAX.
`binding` companions retain copied identity fields and raw stretch bits for cache
and successful current capture. Failure blocks:1admission/phase/thread/type;
2owner/handles/selector;3instance/configuration;4post-read identity recheck. They
identify existing condition blocks, not each native predicate within a block.
No diagnostic repeats a getter to distinguish finer predicates.

Local attempt completion reasons:10context not adapted;11original muzzle/identity;
12rider;13owner/hand;14tracking anchor;15native camera/weapon;16alignment admission;
17target/weapon;18final alignment binding;0adaptation completed. Muzzle calibration
certification is recorded separately; completion alone does not certify a laser.
Native failures can unwind before a completion record.

Query reasons:1native-query admission block;2snapshot eligibility;3no matching
pending request;4body anchor;0admitted. Sample reasons:1settings/tracking;
2mounted muzzle;3handheld/wheel/selection;4weapon identity/scratch;
5muzzle/calibration/weapon/scratch;6current request/hand;7ray result/currentness;
8no final published sample;0published valid sample. These preserve each original
short-circuit expression and query order.

Freeze records the original sample and frame scalar/pose validity, time, ownership,
sequence, hand/UI state and drift/alignment alongside the actual eligibility
result. Existing eligibility remains authoritative. Vehicle identity rejection
requires its existing route, not a guessed handheld explanation. Drawing records
invalid/gated entries or three original line calls having returned, with copied
muzzle/end/hit and eye. Returned drawing does not prove visibility or GPU output.

Logging can affect timing. Interpret age failures alongside record volume and
saturation; never widen the100ms age, phase, ownership, binding or reach guards to
make the diagnostic pass. Actual before/after images and the failing stage must
support any subsequent fix. Keep capture data, native addresses, settings and
assets private. Firing/hardware/vehicle/multiplayer acceptance stays user-operated.

## Source review and build disposition

Astra/xhigh source GO, no MUSTFIX. Both local review turn tags were verified;
independent backend attestation is unavailable. Adopted: copied values only,
logging after locks, original getter/short-circuit preservation, bounded stages
and explicit unproved visibility. Adapted: record existing predicate blocks and
raw binding fields; no extra getter to subdivide a rejection. Drop counts are
observations, not a guaranteed terminal census.

All four products rebuilt on source fingerprint
`674ef5646c47e1d4d53e6837457ceb97e18394ad4ee3df2b623f611cd7ee55d2`;
IPC10/wire7.73Debug and73Release checks and36normal/optimized native/compiled gates
pass. Initial compile required declaration-order repair. The first Release CTest
invocation omitted the existing Python dependency environment and failed two
groups; the corrected invocation passes all73. No dependencies were reinstalled.
Lexical call-sequence comparisons in four changed extents match the preceding
source; source review, rather than that count alone, assesses branch behavior.
These are source/offline checks; the diagnostic has not yet run in a native scene.
