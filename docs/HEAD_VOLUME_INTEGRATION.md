# Head-volume integration — source checkpoint

2026-10-07. Root-owned development; no game, Windows/Wine, OpenXR, headset or
network session was executed. HeadComfort remains disabled by default. This is
not a finished-device-test package or a guarantee of cockpit-interior protection.

## Native query and presentation path

The former opportunistic body-to-head ray ran only when a particular native
view-ray caller appeared and allowed missing observations to leave the world
visible. The new handheld and seated path runs after the complete native simulation/worker
join and after any admitted single-player roomscale settlement. It uses the same
positive world, manager, player/brain, camera, thread, cleanup and snapshot owner
checks as that controller. No body setter is called by the head path.

For each requested stereo frame, the native player collision category and actual
avatar/mechanism exclusions filter a sphere sweep from the current body anchor
to the requested head centre. At rest, a short nonzero conservative sweep also
checks initial occupancy. Both requested eye offsets and the observed native
near-plane extent enlarge the protected radius when necessary. Extra clearance
margin permits a bounded amount of later headset movement. Large/nonfinite or
unsupported geometry refuses the observation.

Triangle and primitive query adapters now have a separate initial-contact
rejection option. Head visibility cannot reuse the body controller's allowance
for nondeepening contact: touching, penetrating, unknown, pending-resource and
uncertified whole-path cases are unusable. Existing body-movement callers retain
their previous contact policy. The normal native cleanup, isolated FP frame and
resource cancellation scopes are shared; no native scratch repair is attempted
on abnormal unwind.

A request arriving after that simulation's query opportunity remains Requested
until a matching query attempt can run; it is not prematurely consumed as a
Rendering frame. Original desktop rendering continues. The attempt's body,
request/input identity, simulation revision, rig revision and age are checked
again before stereo. An obstructed or unavailable completed attempt produces
opaque world pixels in both eyes, while the normal native/fallback UI remains
available. This is a conservative opaque guard, not a smooth distance fade.

Actual eye placement is checked against the queried world sphere. The native
projection near plane is checked while the eyes render; an unexpectedly larger
plane rejects that pair and supplies the next query's size. The observed value
can decrease on later pairs, so a temporary near-plane change does not
permanently inflate every future query. Scope-source captures are separate.
Numerical margin is explicit and increases with world float spacing and stage
coordinate magnitude; excessive uncertainty rejects the query.

## IPC9 and cached images

Each completed image pair owns a fixed-width40-byte head-clearance response:
Disabled, Opaque or Clear. Clear carries the query time, original stage-space
head centre, available radius, minimum head radius and admitted near distance.
The game publishes it with the same guarded Ready transition as the pixels.
The host validates its mode/reserved fields and copies it with that exact pair.
No engine pointer or world object crosses IPC.

Before submission, including after potentially waiting HUD uploads, the host
checks the current tracked head and both eyes against that pair's clearance.
It bounds the cached image's near-plane corners for every eye orientation and
requires matching reference/session and a query age of at most100ms. Expired or
escaped clear images are hidden. Opaque images cannot reveal world pixels;
disabled-mode images retain the ordinary frame eligibility policy. Existing
tracking/focus/generation/deadline and two-eye ownership rules still apply.

Input remains264 bytes, Request432, and UI352. Slot becomes33,554,920 bytes and
Shared83,887,816 bytes. IPC9 rejects mixed older game/host products. Multiplayer
wire6 and input/weapon retention history are unchanged.

## Configuration and limits

HeadComfort Enabled=0 remains the development default. RadiusMeters is the
minimum head footprint. ClearanceMarginMeters (default0.05) is the extra checked
space available to later poses. The old FadeDepthMeters key is accepted as a
fallback configuration value; it no longer selects a translucent distance ramp.

The same world-volume query now admits a positively bound native seated rider.
It uses a read-only player/mechanism/pose-source binding rather than requiring
a foot capsule, zero mechanism parent or child-propagation flags. The pinned
CPlayer collision-category getter establishes the player category. A missing
view-resource link rejects the anchor: the native getter otherwise returns a
global fallback pose, which is not evidence of an actor world anchor.

Native self, parent-mechanism and ignored-mechanism exclusions remain intact.
In particular, the ridden mechanism can be excluded by the engine, so this does
not claim protection from its own cockpit or interior geometry. Seat transitions
and unsupported rider kinds remain Disabled; failed owned attempts are Opaque.
Unavailable native ownership fails closed. Readback cost, dynamic
world timing, actual comfort, and Windows/Proton headset behavior remain
unverified. Native broadphase/traversal remains the engine's responsibility;
portable geometry tests do not prove every real scene or renderer path.

## Offline verification

All55 portable groups pass Debug and Release; x86 game/server and x64 host build.
Changed head/triangle/primitive tests pass ASan/UBSan and the new head helpers
compile for both Windows architectures. Regression cases include static
occupancy, initially penetrating tangential movement, near-plane/eye volume,
late pose escape, stale/future clocks, reference changes, malformed receipts,
unknown modes, canonical opaque/disabled receipts and numerical sweep coverage.
The compiled integration verifier checks the true initial-contact-rejection
argument, zero penetration budget, metadata-only cleanup, FP/resource boundaries,
post-simulation order and request/host publication guards. A modified object that
turns initial-contact rejection off is rejected. The filter-binding fixture
covers read/resolve failures, replacement, nonunit poses and permitted parent
mechanisms. verify_head_query_binding.py pins the native category getter,
mechanism getter, missing-view fallback and self/parent filter branches; the
shared query ABI check covers both game and server objects. The binding tests
pass ASan/UBSan and Windows x86/x64 compile-only checks. Native runtime remains
untested.
