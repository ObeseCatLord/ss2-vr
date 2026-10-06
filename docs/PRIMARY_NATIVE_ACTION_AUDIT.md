# Primary-only native action replay — Astra disposition

Astra/xhigh read-only investigation, effective routing verified locally in
aggregate. Main adopts NO-GO for peroperator ProcessPlayerControls/ClientAction
replay across multiplayer. It has demonstrated nonprimary RPC/correction effects;
no brain lease, partial function jump, action replay or new policy is implemented.

All RVAs Sam2Game unless marked Engine. Exact x86 entries:

| Function | ABI |
|---|---|
| ProcessPlayerControlsEE0F0 | void thiscall(Brain*,uint8 buttons,Vec3 look,Vec3 movement), vectors by value/byte four-byte stack slot,ret1C; vtable29BE90+384 |
| ClientActionEE260 | void thiscall(Brain*,uint8,Vec3 look,Vec3 movement,Vec3 reportedPosition,uint32 base,uint32 mover,uint8 seqCC,uint8 seq150),ret38; vtable388 |
| GetLookDirEulE9590 | 12-byte copy of138 through hidden output/EAX,ret4 |
| GetGroupMoverEDD10 | borrowed CPuppet* thiscall(), controlled-puppet enumeration |

Principal exports:
`?ProcessPlayerControls@CPlayerBrainEntity@SeriousEngine@@UAEXEVVector3f@2@0@Z`
and
`?ClientAction@CPlayerBrainEntity@SeriousEngine@@UAEXEVVector3f@2@00V?$Handle@VCBaseEntity@SeriousEngine@@@2@V?$Handle@VCPuppetEntity@SeriousEngine@@@2@EE@Z`.
No duration, timestamp, expiry or buttons-only parameter exists here. No complete
accepted-action/movement-record getter was established.

Controls obtains current mover placement/support-relative position and sequences
1CC/150, then calls ClientActionEE245. It builds a fresh positional report. Accepted
action stores complete fire byte160 atEE53A, look138 atEE551–EE565 and movement144
atEE568–EE57C; blocking161 and prior puppet348 remain. StartFireE9AD0/StopFireE9B10
dispatch directly to puppet attack/release, not current byte or coherent history.
VM receiver28D860 invokes the same complete action, not a smaller setter.

## Demonstrated larger effects

Engine LocalInterface216210 is remote0/server0; Client215690 remote1/server0;
Server209C08 remote0/server1; DemoPlayback215DB0 remote1/server0. Actual interface
gettersF8260/F8190 matter; single-player label alone is insufficient.

EE286 remote branch serializes and calls Engine ProcessRPC_t17B370 atEE441, reaching
SendRPC_t17B1A0/netSendMessageF8640. Local acceptance subsequently executes too.
Extra client invocation adds an action submission; it is not a local-only button
change. Native initialization158/15C/1E4 atEE4EC–EE534 also can run.

Server acceptance updates authoritative position/base/mover1D0/1DC/1E0 at
EE649–EE724, compares supplied positional report, stores correction1C0 at
EE806–EE811 or advances sequence1CC atEE829–EE836. Synthetic controls with current
position can replace an outstanding nonzero correction with zero even when
look/movement are copied exactly. Direct ClientAction still has these effects;
copying movement144 does not reconstruct positional/base/mover/sequence contract.
Fabricating handles/mismatched sequences would introduce acceptance policy.

Byte persists until subsequent native writers. Another ClientAction replaces it;
PostReceiveUpdateEDE60 nonlocally-operated branch copies1E8→160 atEE054; boot/reset
also write it. PreSendUpdateE9540 copies accepted look/movement/buttons to
1A8/1B4/1E8, so injection participates in ordinary replication. Operator caller
A6E09 does not own all later writes or guarantee every weapon step follows it.
Existing interval freeze before any weapon remains reusable and must not be
acknowledged/frozen again inside an operator wrapper.

## Main disposition

LocalInterface can mechanically change accepted high/low through original action,
but that is not whole-interval/multiplayer approval. Peroperator replay, direct
StartFire/StopFire and jumping into EE53A are rejected. Preserve genuine native
commands/action/RPC, existing frozen intents/generations and weapon FSM/history.
No public button-only acceptance operation was established by this bounded family.
Compare immutable read substitution against these demonstrated broader effects
before adding layers; see PRIMARY_JOIN_REFERENCE_COMPARISON.md.

Verified Sam SHA2565628b4ed30a966f10c8e8ea46bf0789257a35ea80127bacacebe28b1ce5303df;
ClientActionEE260,length5EF,
9e4c6a99ddaf92606452cdf482c874f4957d3db5d97c9fb9c48b975674517134.
No proprietary action payload, dump or source is committed.
