# Physical primary producer — executable design

Current base4baa4e8 has the source-reviewed native primary operator/held join.
Earlier producer/native-join drafts are historical. Main implements; Astra/xhigh
reviews. Multiplayer mandatory, teleport excluded, native runtime testing excluded.

## Verified environment and open decision

| Fact | Status / evidence |
| --- | --- |
| Native primary consumer integration | [verified: `PRIMARY_JOIN_SOURCE_REVIEW.md`, base4baa4e8] |
| Raw menu/render input ownership | [verified: `controls.hpp` menu predicates, `engine.cpp` Snapshot/Input] |
| Packet current-neutral witness and ACK policy | [verified: `network.hpp`, `multiplayer.cpp`; audit actual source] |
| Native saw identity | [verified: `PHYSICAL_MELEE_NATIVE_AUDIT.md`, ID0/vtable2CDD10] |
| Gesture tuning / player comfort | [unverified: runtime deliberately excluded] |
| Host ticks approximate actual XR pose intervals | [unknown: no calibrated pose timestamp in existing Input] |
| Platforms | [verified: Linux offline C++20 + Windows x86/x64 cross-builds] |

Open decision: admit a game-side logical sample versus rewriting host primary
input. Lean to the bounded game adapter because only it has actual native weapon
ownership; host rewriting would add classification/transport without evidence
of incompatibility. Neutral and generation policy may be the same decision in
two forms: merging/simplifying them is in scope. Solo operator; no new service,
protocol or framework. Review at most1600 words, no whole-VR re-review.

## Reference and smallest boundary

Original input, menu pointers and frozen XR request provenance remain raw. Native
CircularSaw ID0/vtable2CDD10 is the audited melee family; its original start,
held cadence, three tracked muzzle rays, ammo/damage and release stay unchanged.
No hit timer, attack debt, cooldown extension or swept-collision replacement.

Add a small game-side `GameplayPrimary` value/provenance per hand to the existing
Snapshot/ControlSample. It contains only logical value, eligibility, generation
and current-sample neutral evidence; no pose copy or command queue. The existing
local update derives it from current raw input plus verified actual weapon/rider/
noncarrying identity. Manual trigger remains available for every existing mode.
Gesture is enabled only for the actual owned native saw, with both hands supported
under existing nonalias/native dual policy. Host UI cannot classify ownership.

Do not rewrite host Input or serialize another protocol. The SAME logical value,
eligibility and generation feed existing TriggerGate, raw-down/neutral sampling,
native command history/admission, packet primary fields and local ACK checks.
Raw Input remains the render/menu/pose source. Existing pose/intent transport,
freezing/retained pulses/ACKs/native five reads remain reusable.

## Logical motion and neutral contract

Two local per-hand producers retain only a previous finite LOCAL grip/aim pose,
source sequence/time, context key, motion hysteresis and one monotonic logical
ActionStream generation. Sample before origin/snap-turn/native avatar transforms.
Use `weaponTracking` (grip position, aim orientation), all XYZ displacement and
shortest quaternion arc, treating q/−q as the same orientation.

Use bounded consecutive new samples: strictly increasing sequence/tick, current
host age≤200ms, gap≤100ms, finite valid unit poses and bounded discontinuity.
Normalize displacement/arc to a30ms host-tick reference as a sampling heuristic;
initial enter/exit distances .04/.016m and arcs .18/.07rad are tuning choices.
These are not calibrated physical velocities: host tick precedes XR sampling.
Translation OR angular enter starts held motion; BOTH below exit ends it.

Unavailable, stale, profile/session/reference/producer/equip/rider/recenter/wheel
context changes discard pose history. First seeding/repeated input is not new
neutral. Reuse InputSampleBoundary for context changes; actual fresh eligible
quiet motion plus manual trigger<.2 can rearm the existing TriggerGate. Do not add
a second trigger-arm FSM. A held gesture cannot bypass unknown manual neutrality.
Merge gesture down as1 with the original manual value otherwise; a neutral
witness requires ALL enabled sources currently known neutral in the same fresh
sample. Missing pose/reset is unavailable, not a synthetic release witness.

Logical generation advances on source/context/availability changes, never normal
press/release edges. It is monotonic across invalidation, including Snapshot
reset; exhaustion fails closed. Preserve physical trigger hysteresis. Manual-only
native carry/vehicle/ordinary gun behavior stays under existing controls.

## Wiring and proof

Derive after current hand refresh; late equip/dual/ownership changes invalidate
the existing hand result and require a later eligible neutral. Native getters
must be followed by identity/source rechecks. Network physicalDown/neutral/fire
must all describe this logical source, including retained gesture-only pulses.
PrimarySourceHands/currentControls/poll/localPrimaryAllowed must use logical
generations consistently; zoom/menu/render use raw provenance unchanged.

Portable tests exercise the actual producer, gates and existing intent/retained
freeze policy: independent dual gestures; translation/rotation/sign; duplicates,
gaps/loss/context and fresh neutral; manual OR motion; cooldown does not extend
gesture; new ACK after quiet then nonneutral hysteresis cannot launder old release;
retained high→low reaches the already integrated native consumers. Compiled
client/server/host and actual linked five-site checks remain required. Source
trace must prove real update→commands/packet→frozen native reads, not mocks alone.

Compare to host rewriting or new XR timestamp/velocity fields: both add stale
classification or transport complexity without a demonstrated incompatibility.
Stop/reopen if implementation duplicates pose state, source admission, command
history, ACK scheduling or native combat. Full collision/scopes/head objective
remains intact beyond this producer. No runtime/contact/feel guarantee is inferred
from portable checks.
