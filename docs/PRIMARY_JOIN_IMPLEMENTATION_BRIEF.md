# Five-site primary join — executable integration brief

Main implementation proposal after PHYSICAL_MELEE_JOIN_DESIGN_REVIEW.md. Not
source approval. Baseline5c4fe50 plus independent scope-reader work. Astra/xhigh
reviews all investigations and source changes; no game/Windows/XR execution.

## Behavioral reference and minimal change

Native operator8E580 and held8E480 own button mapping/blocking/history and the
weapon FSM. The demonstrated incompatibility is current F6 held override versus
native edge/history and retained VR taps in intervals without a new native action.
The read-only five-site adapter changes only loaded selected scalar bits before
the original instructions. It replaces the existing held override's authority;
it does not replace operator, command/RPC, brain, weapons or multiplayer policy.

Extend the existing SimulationInterval prepared-player entry with a small
primitive projection rather than add a global pawn registry, persistent lease,
new transport or duplicate frozen pose sample. Keep the original handle and
adapt its existing zoom preparation lookup mechanically. The projection stores
exact prepared subject/brain/weapon identity values, managed native bit mask,
captured logical high bits and validity/revocation metadata. It carries no
native object ownership and cannot introduce high after interval preparation.

After existing local update/authoritative authorityStep has frozen input and
finished equip/dual changes, bounded preparation obtains native mapping and
brain/avatar/rider/carry binding using verified getters. Revalidate identities
and captured source after getters. Exact getter/export contract remains needed;
do not guess function signatures or reconstruct native mapping in the adapter.
Uncoupled identical dual must use the native configuration route identified by
the separate effective-resource investigation; never force player9BC.

## Actual consumers

Wrap original operator and held query with caller/main-thread/root simulation/
prepared manager/world checks and NativeFinally TLS restoration. The scalar
helper itself only compares subject pointer values, wrapper kind and bounded
mod-owned primitive records, then merges the mask. It must not dereference the
subject, call native getters, acquire a native handle, allocate or throw. Each
wrapper refreshes revocation/expiry once before invoking native code; predicate
sites reuse the resulting same interval projection. Native callbacks may only
suppress that high by lifecycle revocation. Nested recognized VR invocation is
neutral for managed primary; other callers preserve original values.

Player and weapon deletion revoke prepared projection metadata before original
callbacks, by pointer-value comparison against prepared identities. Cleanup
restores mod TLS only. Recognized invalid VR ownership retains a neutral mask;
dropping ownership must not expose a high desktop/brain value. Current F6 detour
becomes ordinary forwarding through F6F00→FB590→8E480 after the join is installed,
or is removed if no other responsibility remains. Delete authoritativeFire if
its only consumer is removed; do not keep parallel firing authority.

## Verification target and estimate

One tightly coupled engine/native-boundary change: existing interval preparation,
two ordinary wrappers, five macro entries, two lifecycle revocation seams and
hook installation/static verifier metadata. No additional service/protocol or
policy layer. The shared macro needs only EDI result slot support; its compiled
fixture checks selected register, stack, flags and FX preservation without
executing Windows code. Source review must trace actual production calls from
existing frozen local/remote inputs to native reads/history/release, not accept
pure scalar tests as end-to-end proof.

Required cases: unclaimed desktop; local and remote retained high→low without
new ClientAction; single left/right; native uncoupled dual/flip mapping; blocked
release; interrupted/expired/nested/deleted/replaced ownership; unrelated and
upper register bits; original native history store and native callback ordering.
Cross-build both x86 client and server and inspect compiled entries/trampolines.
Runtime behavior is deliberately unobserved under the user's exclusion.

Stop and reopen design if a durable brain lease, extra history/ACK machine,
runtime primary resampling, custom hand mapper or duplicated combat policy
becomes necessary. Physical motion production follows coherent native join;
it cannot substitute for this production consumer integration.

## Astra review disposition — reopen before implementation

Native ABIs and EDI source/artifact receive GO. The unrestricted callback and
nested-neutral guarantee above receives NO-GO; no production join is installed.
Operator caches flip8E58D–8E593, computes edge flags before callbacks
8E691/8E6E9/8E72D, then commits history8E75A. Revocation cannot retract a delivered
callback; neutralizing the final commit can erase high history needed for release.
Nested original operators can dispatch release/overwrite history, so TLS-only
restoration does not prove observational neutrality. These are demonstrated
ordering limitations, not a reason to invent an edge FSM or restore native history.

Recognition must reserve neutral metadata before reentrant update/authorityStep,
remain separate from admission and survive nested intervals. Refresh validates
and revokes only, never remaps captured high. Compare reciprocal brain/avatar,
weapon/owner, rider/carry, actual combo and flip after native getters. Deletion
alone misses nondeleting swaps/configuration changes; revoke before ordinary and
sniper-specific deletion processing. Carry field564 is native release routing.

Stable recognized managed mask is03 for the two existing VR command lanes;
preserve bits2–7 and upper register bits. One valid weapon projects its hand via
the actual native mapping index. A lone left weapon is not always index0:
combo/9BC can map it to1. Uncoupled combo maps both hands through captured getter
indices; aliased/fallback-identical and unsupported dual topology are recognized
neutral until native edge/held agreement is proved. Do not OR independent intent
into a coupled mode and call it independence.

Exact native exports, all Sam2Game:

| Function/RVA | Export/ABI |
|---|---|
| GetWeaponFiringButton FB590 | ?GetWeaponFiringButton@CPlayerPuppetEntity@SeriousEngine@@UAEJV?$Handle@VCBaseWeaponEntity@SeriousEngine@@@2@@Z; int32 thiscall(Player*,uint32 handle),ret4; returns index0/1 |
| IsFlippingFireButtons1022B0 | ?IsFlippingFireButtons@CPlayerPuppetEntity@SeriousEngine@@UAEHXZ; int thiscall(Player*),ret |
| ExecuteOperatorFiring8E580 | ?ExecuteOperatorFiring@CPuppetEntity@SeriousEngine@@UAEXXZ; void thiscall(Puppet*),ret; prologue558BEC83EC18 |
| IsFireButtonPressed8E480 | ?IsFireButtonPressed@CPuppetEntity@SeriousEngine@@UAEHJ@Z; int thiscall(Puppet*,int32 rawIndex),ret4; prologue558BEC568BF1 |

FB590 may dereference the resolved weapon without a null check: validated current
handles and post-getter revalidation are mandatory. F6F00 uses virtual72C then51C,
returnF6F1F; player vtable29E878 selects FB590/8E480. Operator callerA6E09 remains.
Reciprocal puppetbrain38C/brainavatar28 and rider544/548/54C are verified. Native
DoYouOperateMeEDB00 enumerates controlled puppets, not merely reciprocal equality;
it is not a scalar-helper query. CanFireFromOwnWeapons is not noncarrying proof.

Bounded follow-up checks primary callback closure/same-pawn reentry and considers
an immutable operator projection preserving native delivered-high history. This
is an open decision, not approved behavior. No adapter or physical motion producer
is promoted by exact ABI checks alone.
