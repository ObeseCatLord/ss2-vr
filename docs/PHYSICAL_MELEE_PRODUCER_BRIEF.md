# Physical saw input producer — draft decision brief

Full immersion goal remains active; multiplayer required, teleport excluded,
native game/XR/Windows testing excluded. Main owns implementation/integration;
Astra/xhigh investigates and reviews. This draft does not approve production.

Verified native authority is in PHYSICAL_MELEE_NATIVE_AUDIT.md. One supported
stock family is CircularSaw ID0, actual shooting placement hook already feeds
its three ray queries; native start/state/cadence/damage/release must remain.
Held query alone is insufficient proof that native edge history releases the saw.
Astra's current follow-up checks whether existing command production supplies
that join before adding an ExecuteOperatorFiring adapter.

## Checkable current producer/consumer

| Fact | Current evidence |
|---|---|
| Same raw sample owns grip/aim/buttons/trigger | host.cpp Actions::sample; xrLocateSpace at predicted time, xrSyncActions, primary ActionStream |
| Input has no predicted XR timestamp or motion velocity | protocol.hpp Input; tickMs is GetTickCount64 at emptyInput, before action sample |
| Game input publication independent of rendering | host.cpp publish assigns shared.latest; render requests retain that Input |
| Host weapon classification could be stale/incomplete | Ui has currentWeapon IDs/gameplay/tick but no native handle or rider identity; publish reads UI after assigning latest |
| Actual per-hand weapon/rider/gates available game-side | engine.cpp update actual native handles/ID, RiderIdentity, ownership refresh, existing wheel/equip/TriggerGate |
| Neutral/eligible/raw-down witness must match logical fire | engine.cpp update network packet loop uses samplePrimaryNeutral/input generations and requires physicalDown for fireMask |
| Existing network policy already freezes taps/ACK/epochs | network.hpp OrderedPosePolicy, multiplayer.cpp simulation ownership |

## Minimal candidate and alternatives

Candidate: derive a contextual primary sample inside the existing game update,
only for an actual owned CircularSaw on a handheld, nonaliased hand. Use consecutive
finite LOCAL-space grip samples before origin, snap turn or avatar motion. Measure
translation speed in all axes and shortest-arc angular speed; a configurable
enter/exit hysteresis produces a held gesture. It is an input producer, not a
damage/contact/cooldown FSM. Original physical trigger remains available.

Merge trigger and gesture into one logical primary value and eligibility, then
feed the SAME value/eligibility through existing TriggerGate, command snapshot,
neutral witnesses, generations, pose packet and retained authority. Never change
only fireMask/Snapshot.fire. Raw menu/pointer trigger stays unchanged. Identity,
weapon/equip, focus, profile, tracking or recenter interruption must discard motion
history; a stale/repeated sample or unavailable pose is not a fresh neutral.
Require actual new neutral motion before gesture rearm, not zero synthesized on
history reset. Native press/hold/release join remains a required prerequisite.

Host UI-based rewriting needs no IPC field, but cannot alone prove exact current
weapon/rider ownership. A new IPC motion/timestamp field could preserve exact XR
sample timing without trusting UI, but raises version/packaging complexity and
must be justified by a demonstrated timing incompatibility. Game-side tickMs
derivation is minimal, but tickMs precedes XR sampling: speed accuracy under
variable prediction/SyncActions latency is UNKNOWN. Do not claim metre/second
fidelity without that proof. Request.predictedTime cannot be the sole producer
because requests depend on rendering and latest input does not.

A separate virtual-button/network transport, damage timer, hit-success debt,
gesture stretched until cooldown succeeds, or independent swept collision engine
would duplicate native behavior and is rejected. Native ray-based cutting stays
the reference unless a separate blade contact requirement is proved.

## Open review decisions

Determine the smallest coherent logical-input boundary and generation/neutral
contract, including source availability and manual-trigger coexistence. Verify
whether source Input copying/contextual flags can preserve render request and
menu provenance without extra state. Compare tickMs derivative with an exact
XR-time/motion sample addition; do not select a larger protocol merely because
it is easier to mock. Thresholds are tuning choices, not verified contact proof.

Smallest proof: one actual right-hand saw, neutral→qualifying motion→held→neutral,
local and delayed retained authority, reaching native start/cadence/placement/
release. Cover all-axis translation, quaternion sign equivalence, repeated/stale
samples, profile/reference/equip/wheel/tracking interruption, pulse expiry and
cooldown rejection. Prove production wiring and x86 ABI; no runtime launch.

No new producer, protocol, operator hook or gesture classifier is implemented
by this brief. It is input to Astra senior review after the native join result.
