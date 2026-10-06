# Astra Max: smallest saw consumption-history boundary

## Goal and scale

Solo-user native OpenXR mod. Full physical saw firing and multiplayer replica
behavior are required; no runtime testing. Main owns final design/integration.
Decide the smallest sufficient native boundary after three smaller shortcuts
were refuted. Read-only senior review; do not implement or broaden to optics,
roomscale, remote head or vehicles. Seek deletion, not just local correctness.

## Facts and environment

| Fact | Evidence / classification |
| --- | --- |
| Repo is ss2-vr under installed Serious Sam 2; x86 game hooks, x64 OpenXR host | [verified: AGENTS.md, CMakeLists.txt] |
| Physical worktree is unaccepted; five-site join still projects combined native348 | [verified: engine.cpp operatorFiring/ss2vrPrimaryPredicate and PHYSICAL_CARRY_MAX_DISPOSITION.md] |
| Current commands/history can conflate gesture-only and manual-high; later carry can falsely throw | [verified: PHYSICAL_CARRY_MAX_DISPOSITION.md] |
| Scalar primary helpers cannot allocate, resolve, lock, call native callbacks or throw | [verified: engine.cpp1190 and PRIMARY_JOIN_SOURCE_REVIEW.md] |
| Native saw starts via held query; primary0 DoAttack does not start it | [verified: PHYSICAL_SAW_LEVEL_REVIEW.md] |
| State4 can fire before held-low; B0/E8 alone misses a second logical release in recoil | [verified: PHYSICAL_SAW_STOP_GATE.md, PHYSICAL_SAW_SCHEDULE_PROOF.md] |
| Native canonical StopAttack has first-shot/sound and player bookkeeping effects | [verified: PHYSICAL_SAW_LEVEL_REVIEW.md, PHYSICAL_SAW_STOP_GATE.md] |
| Proposed native bit packing triggers grenade lane2 | [verified: PHYSICAL_NATIVE_DELIVERY_AND_BITS_GATE.md] |
| Observer exact-weapon held binding has conditional design GO, but not stop/delivery GO | [verified: PHYSICAL_OBSERVER_CONSUMER_REVIEW.md] |
| AI firing notification and ordinary dynamic state updates don't deliver observer saw transitions | [verified bounded: PHYSICAL_NATIVE_DELIVERY_AND_BITS_GATE.md] |
| Existing Source producer, Authority and baseWeaponStep hook exist; prepared intervals are ephemeral | [verified: engine.cpp334,894,1570; gameplay_primary.hpp] |
| Durable command-history requires native constructors, reset98B41, copy and registration coverage | [verified: PHYSICAL_CARRY_LIFETIME_GATE.md, PHYSICAL_CARRY_REGISTRATION_GATE.md and new gate] |
| Model/effort current-turn verification is mandatory; use explicitly gpt-6-astra/max | [verified: AGENTS.md] |

Relevant worktree files: src/game/engine.cpp, src/common/gameplay_primary.hpp,
src/common/physical_motion.hpp, src/common/native_primary_projection.hpp,
src/game/multiplayer.cpp. Native owned
binaries may be inspected statically; pins are in docs/RESEARCH.md. Do not
export disassembly or game assets. No game/Windows/Wine/XR/network launches.

## Main lean: exact-weapon consumed-level metadata

[unverified candidate] Keep original native command160 and committed348 manual,
including all four lanes. Continue existing handheld logical held projection.
For the exact equipped tracked saw, keep one previous **consumed logical level**
at the weapon binding, not previous manual command at the player lifetime.
On admitted native consumption, a logical high→low invokes the original
canonical release before baseWeaponStep can fire. A manual release while
logical-high must not prematurely stop that exact saw. Other weapons, carry,
tertiary commands and vehicle behavior remain native/manual references.

The record is not a cooldown/attack/recoil state machine: native B0/E8 and
callbacks continue owning those. It must not acquire native348's constructor/
copy lifetime merely to remember a weapon input edge. Prefer a narrow subrecord
in existing local producer/Authority binding ownership if that can cover native
consumption; no second entity registry by default. Source-produced previous
level is NOT assumed to equal native-consumed previous level. Manager/prepared
interval values alone are not durable metadata.

[unknown] Exact minimal interception for canonical release, suppression of a
coincident manual release, order relative to operator/baseStep, native callbacks
and reentry. Native copy/equip/bring-down may already terminate the weapon, but
that is not assumed. Metadata must be published at the real successful consumer
boundary; missing/unknown cannot be treated as prior0. Multiple nested native
consumptions must commit in actual order, including partial native unwind.

In particular, the40ms high in the established recoil counterexample is not
necessarily queried by native held code. Recording only successful held-query
results would miss it and still lose the60ms canonical release. Candidate
consumption therefore means admitting the immutable logical input at a native
step/operator boundary, even if cooldown suppresses the later held query; the
exact boundary and commit semantics require review. Preserving rising-edge
player bookkeeping (MarkNonIdle) also matters when manual0/gesture1, even though
primary0 DoAttack does not start the saw. Do not interpret level-start capability
as proof all native press effects can be deleted.

[unknown] How to distinguish logical sources at already-accepted manual and
gesture consumers without duplicate bookkeeping, and whether canonical full
StopAttack can be invoked with exact native raw→semantic mapping. The proposal
does not license direct damage/recoil calls or repeated StopAttack on each low.

## Alternatives and concrete tradeoffs

1. Weapon consumed-level candidate (main lean): stores only information now
   demonstrated absent from B0/E8. Avoids conflated command history and carry
   selectors. Must prove native manual-release suppression/order and lifecycle.
2. Existing Authority registry with separate manual-history subrecord: retain
   five-site logical operator projection; four carry selectors restore manual
   edges. Solves real information loss but adds registration/copy/reset/capacity,
   stable-row/stale-save and late-carry complexity. Current copied18rows don't
   already provide the necessary lifetime.
3. Native field reuse: exact bits2/3/4 rejected; no spare native location proven.
   Don't request another broad spare-bit hunt merely to avoid a bounded record.
4. Held/state-only saw adaptation rejected by reachable release counterexample.
   Trigger-only melee or dropping canonical bookkeeping misses user fidelity.

Observer stop history may be the same boundary as authoritative/local saw
history; merging their mechanism is in scope, while their eligibility remains
different. Existing50ms unreliable latest pose loses short holds. Investigate
only enough to decide whether accepted observer pulseMask can be consumed in
the existing input-retention policy, before proposing another queue/protocol.
No causality between native action/pose/create arrival is assumed.

## Requested output and limits

Verify actual source and load-bearing native evidence before critique. Rank the
forks and let that ranking select the one deep specification. Return ≤1800words:
GO/CONDITIONAL GO/NO-GO; prioritized findings with exact file/line or RVA;
smallest ownership/dataflow/native boundaries; reuse/deletion list; one precise
remaining proof if the evidence is insufficient; minimal connected offline
vertical trace and final compiled ABI requirements. Include effective current
model/effort confirmation. Read-only, no delegation. Do not rediscover completed
scope/collision rendering investigations or implement speculative helpers. If
the candidate genuinely requires a full native-history layer, explain why with
a counterexample, rather than expanding silently.
