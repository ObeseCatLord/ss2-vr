# Native saw delivery and command-bit compatibility

Two fresh read-only Astra/xhigh investigations verified their effective routing
and the recorded Sam2Game/Engine/Core fingerprints. No native execution, source
hook, protocol change, installation or package change followed.

## Firing delivery: bounded absence proof

The apparent firing notification is AI hearing, not observer fire transport:
Saw OnFire1654B0 → base4CB70 → call4CBD6 → samEmanateThreatSound21CD40.
The helper is cdecl with source-brain handle, source-entity handle and float
radius. Its listener foe-manager slot1E8 at21CE38 resolves to
ThreatSoundHeard5A1F0 (thiscall, ret8), changing perception/foe/investigation
state. Adjacent owner slot340 resolves to MarkNonIdle7F0F0. The hearing call
precedes the base ammunition-acceptance result; it is not an accepted-shot ACK.

Dynamic creation and ordinary updates still use the reflected network
descriptor. Engine5E1B6/5E1BC/5E1BF include members only with flags4000;
104B30, FC460 and EC8E0 write/read/copy that list rather than the whole object.
Weapon last-fire30, first-in-row38, shot-countAC, stateB0 and model animation
queue0C are excluded. Last-fire-power3C is included, but is a scalar without
event identity or a transition callback. Saw PostReceiveUpdate is bareRET at
1C3240. It does not start/recoil/stop the client weapon.

Manual-zero160 → replicated1E8 → client160 therefore leaves the mapped held
query zero on these paths. This proof covers the AI-hearing helper and native
dynamic state delivery to an existing saw replica. It does not claim damage,
AI responses or every world effect fail to replicate. Attempted logical highs
that never fire have no firing notification on either path.

## Exact proposed command encoding: rejected

Hypothesis: logical low bits0/1; manual bits2/3; format bit4; preserve5..7.
Bit2 is already native tertiary fire, the hand-grenade command. Input producer
F3541/F355E/F356E sets04 atF3578. Operator8E580 iterates four lanes (limit4 at
8E737), not two. Lane2 rising dispatches slot524 at8E6E9 with index2;
7FFC0→DoAttack101F00 accepts it at101F9E and can drain grenade ammo at1020B2
before the projectile path10226E.

| Input transition | Proposed stored bytes | Native consequence |
| --- | --- | --- |
| Logical01/manual00 → logical01/manual01 | 11 → 15 | False tertiary rise, possible grenade/ammo use |
| Logical01 → logical01 without encoding | 01 → 01 | No tertiary rise |

ControlsEE0F0 (ret1C), ClientActionEE260 (ret38), RPC EE441 and current store
EE53A preserve the full UBYTE. PreSendE9579/E957F and PostReceiveEE04E/EE054
also preserve it. Full-byte preservation proves transport width, not spare
semantics. Native update dirty detection compares the whole byte atEngine104C68;
FB9B0/FB9D0 write/read8bits. Save/load transfers the full declared one-byte type.

The census additionally identifies a native348 zero reset at98B41, beyond
constructorA41FF. Copy/assignment D465/F29E and history8E75A preserve the byte.
Player103EA4 tests20, reinforcing preservation of existing upper bits.
Bits3/4 and all indirect consumers were not independently certified spare.

Main's Linux scratch linked the production `nativePrimaryRead` helper. With
both invocations projecting logical01, proposed bytes11→15 survive unchanged
and produce a raw lane2 rise under the audited four-lane edge expression.
The ordinary01→01 reference has none. Exhaustive upper-byte/primary-value
cases also confirm the current projection preserves native bits2..7. This is
a production-helper/data-flow check, not native grenade execution.

## Main disposition

| Finding | Decision |
| --- | --- |
| Notification is AI hearing | Adopt; do not rely on it for observer saw events |
| Dynamic replication excludes firing/animation state | Adopt bounded absence; observer consumption/delivery remains necessary |
| Proposed bit2 encoding triggers grenades | Reject the complete bits2/3/4 allocation; no production encoding |
| Operator handles four native lanes | Preserve tertiary and fourth-lane behavior in every primary adapter |
| Additional348 reset98B41 | Add to any durable native-history correspondence proof |
| Other bits or fields unexhausted | Do not infer a spare allocation or begin another guessing loop |

Current physical-primary source remains NO-GO. Next compare a weapon-bound
consumed logical-level record, leaving native manual160/348 untouched, against
the previously proposed manual-history registry. This comparison is unreviewed;
it does not authorize a new state machine, callback replay or observer queue.
