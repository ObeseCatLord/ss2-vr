# Development-gated local roomscale controller

The controlling-local-XR controller is connected for single-player and negotiated
multiplayer roles. `[Roomscale] Enabled=0` remains the development default. Source
review accepts one local native settlement owner with original multiplayer
propagation, not a second server payment adapter. This supersedes the historical
single-player exclusion. See [MP_LOCAL_ORIGIN_CAPTURE.md](MP_LOCAL_ORIGIN_CAPTURE.md).
Gameplay/collision/convergence acceptance remains user-operated.

## Native extent and actor admission

The adapter runs after the original full CSimulation::Step returns, only from
the pinned normal game-loop caller (Sam2Game258DE). The original entity-manager
preparation records the world and manager. Admission checks the same current
simulation/world/manager, completed simulation flag, inactive joined physics
pool, exact main thread, no nested simulation, healthy input and no active native
ray, weapon, zoom, UI, scope or stereo transaction. A per-thread simulation
revision rejects any reentered step while an attempt is in progress. Stereo/UI
admission spans the whole pair, including gaps between eyes and deferred UI.

Only the existing prepared, recognized local player, its exact player/brain
classes and current bidirectional brain ownership are admitted. The player must
be alive, handheld, outside menus/recenter/wheels, with a fresh usable tracking
sample. The exact WorldInfo object and its current camera handle are checked:
Brain::RenderView gives an external camera precedence over the player view, so
a nonzero camera handle blocks roomscale. This does not take control of a
cinematic camera or change native controller policy.

The body capture still admits only an unattached, single-part hybrid root with
one or two complete primitive sphere/capsule hulls and active propagation. Carry,
ride, unknown classes, unsupported shapes and incomplete trees are refused.
Every extra ray requires the inspected cleanup-list shape, empty retired visited
array and idle model scratch. Native cleanup remains native; interrupted
traversal is not repaired by copying pointers or manually clearing flags.

These are bounded admission checks in the pinned native lifecycle, not a new
engine-wide lock or a universal proof about external callbacks/modifications.

## Query, native commit and origin publication

A horizontal head offset proposes a model translation. Per-step distance is
bounded by the smaller of 0.25 native units and half the smallest captured hull
radius. The actual root candidate still comes from the original checked native
setter; the mod does not substitute a guessed model-to-root transform.

Every captured hull is covered and swept through the existing native query
pipeline. The query-radius allowance is 4% of hull radius, the contact-depth
budget is 0.2%, and native rounding deviation has a separate small displacement
allowance. Unsupported/pending geometry rejects the entire attempt. The original
mechanism precheck remains in place; its zero-radius centre-ray safeguard is not
mistaken for a full-body capsule sweep.

After every sphere is clear and owner/body state is revalidated, the snapshot
lock guards an odd rig-revision claim. The lock is released before the original
native body write and its normal callbacks. Reentered mod input updates are
suppressed during that transition. The original setter owns body, joint, model
and broadphase changes. The isolated math FP frame has already restored native
caller state before the first native write.

On normal completion, fresh native anchor/body reads must match the actual root
candidate and original body identities/descriptors. The origin helper uses the
actual anchor displacement and an explicit small head-position error budget.
Only the origin and rig revision are published, preserving unrelated snapshot
changes, manual command history and tracking/network intent generations.

A clean pre-write refusal retires any claimed revision without consuming an
origin displacement. A reset/superseding owner never receives the old origin.
An uncertain mutation or failed post-read quarantines further roomscale and
invalidates presentation through the existing interrupted-input epoch path.
Native unwind cleanup performs only metadata/lock operations; it does not call
native getters, roll a body back, or retry a possibly consumed displacement.

## Bounded readability cache

Repeated field validation uses a fixed 24-region readability cache only inside
the separately admitted lifetime extent. It is sealed after initial capture,
cleared before native commit callbacks and rebuilt for post-commit reads. It is
not a reference, lifetime pin or substitute for handle/owner checks. This avoids
repeating a VirtualQuery call for every field of every sphere's repeated owner
check. A full cache, new unreadable range, overflow or inconsistent query result
rejects access rather than widening the bound.

## Offline evidence

51 portable groups pass Debug/Release. Both x86 game products and the x64 host
build. The controller verifier pins the native phase/camera/cleanup evidence and
checks actual compiled post-simulation ordering, root-target math containment,
query-before-publication ordering and FP-free cleanup. It passes normally and
with Python optimization; a deliberately inserted FP reset in the cleanup is
rejected. Existing query-observer and swimming argument checks remain valid.
New publication and readability helpers have production-code regression tests;
these do not execute or emulate the native physics engine.

Default enablement, actual multiplayer convergence and the rest of the full
VR implementation remain unfinished. These checks are not runtime verification
or a claim that the final mod is ready.
