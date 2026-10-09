# Passive ride-control scalar observation

This diagnostic is a prerequisite for physical cockpit controls. It does not
implement grabbing or steering, identify installed resources, or certify a live
control frame. No vehicle runtime test has been performed for it.

## Native boundary and behavior

The optional headful hook observes the original
`CPuppetEntity::ClampLookDirEulAsRide` callback at Sam2Game RVA `0x901b0`.
The existing mounted-look adapter retains an equality token from its existing
successful rider resolution, without adding a lookup. Immediately around its
one original brain-clamp call, it arms a stack observation containing that token,
the same `Vec3&`, and the current thread. The receiver must match all three;
duplicate callbacks, nesting, rejection and abnormal completion invalidate the
observation. Native-finally cleanup restores the prior TLS observation. Every
ordinary callback path forwards the original once, including declined samples.
Native exceptions propagate; readability checks do not supply fault recovery or
native ownership.

Only the native-borrowed callback receiver supplies these copied scalar fields:

| Field | Native offset / admission |
| --- | --- |
| Class | Sam2Game-relative vtable `0x2a8558` or `0x2b8420` |
| Movement mode | `+0x4c4` |
| Execution abilities | `+0x2d8` |
| Movement abilities | `+0x4cc` |
| Parameter equality token | `+0x47c`, nullable |
| Renderable handle token | `+0x120`, nullable |

The last two values are never dereferenced or resolved by this sampler. The
collector adds no resource/model getter, physics change or ClientAction change.
The original installation and default controls are unaffected. The optional
hook requires exact process-local `SS2VR_LAB_RIDE_CONTROL=1` plus pinned export
and player dispatch identities; it is disabled by default and on the headless
server. This is diagnostic enablement, not a ready user collection procedure.

## Private evidence contract

After normal completion, at most 64 accepted rows are written to the private
game log under `Lab rideControl`. Schema 1 binds the current 64-character build
source fingerprint, ordinal, input/generation/session/reference, rider/seat/
brain/thread identity, class and copied scalars. Each row explicitly states
`resourceAssociated=0 frameAssociated=0 steeringApplied=0`.

`tools/assess_ride_control.py` accepts only the exact schema/fingerprint,
consecutive ordinals, supported classes, one completed callback and unsigned
bounded fields. It rejects truncated, duplicate, unknown and positive
association/steering fields. Its output must be a fresh private file outside
the source tree. An empty report is not a successful vehicle observation.

These rows describe mode at clamp entry, before downstream ClientAction
consumption. They do not prove operator-seat authority, which resource is loaded,
input application or physical steering.

## Verification and next implementation edge

The source review covers native token handling, borrowing, forwarding and cleanup.
Offline state checks exercise class admission, mismatches, duplicates, reentry
and abnormal completion. The compiled gate checks normal-path original-call
counts and callee relocation identity in the actual x86 game/server objects.
It follows saved pointer values and flag effects through all 12 abnormal/failed/
entered cleanup cases, verifies observer-entry ESP restoration, and checks mounted
saved-stack bookends and `ret 4`. Actual instruction mutations include bypassed
abnormal cleanup, a clobbered saved TLS value, a wrong callee section, incorrect
stack repair, missing forwarding and changed flags. Incoming capture layouts,
argument origin, ownership admission and mounted interior stack effects remain
source-review claims; this finite gate is not a general ABI interpreter. No
native execution or SEH probe is inferred from these checks.

The next missing operation is a fresh native-owned ride → renderable → model
instance association. The verified getter chain is Sam2Game
`GetModelRenderable` (`0x83790`) → Engine `GetModelInstance` (`0x15b220`), but
neither getter is borrowed by this clamp extent. Cross-frame pointer equality
cannot substitute for that join. Installed resource identity and evaluated
Main/Seat transforms must then belong to that same instance before using the
inspected cockpit handles for one-/two-hand grabs. Fighter Loading prevents
treating its authored bind transforms as a fixed live control frame.

Preserve the native mode distinction in [IMMERSIVE_STEERING](IMMERSIVE_STEERING.md):
the inspected hovercraft's mode 2 X input requests lateral translation, whereas
the inspected wheeled mode 3 route selects native road-wheel limits by sign.
Neither fact establishes a driver's wheel or permits guessed pivots.
