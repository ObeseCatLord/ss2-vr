# Roomscale native motor candidate — Astra decision

Astra/xhigh read-only investigation of one candidate, no broader re-exploration,
edits, delegation or native execution. Main adopts NO-GO for GenericMotorEntity
LinkTargets as direct1:1 roomscale. Existing native physics remains unchanged.
Full immersive goal remains active; this closes one candidate only.

## Exact ownership and native path

Sam2Game116EE0 LinkTargets is void thiscall(entity), no explicit args/ret. Entity
24 owns CGenericMotorJoint; target handles2C/34, part identifiers30/38, initialization
placement3C. Targets dispatch virtualB4 at116F23/116F50; puppet/player vtables use
GetAspectForAttaching83AE0, CAspect* thiscall(IDENT),ret4. Invalid identifier returns
resolved puppet120; explicit identifier searches mechanism114 via Engine133B80
GetBody then falls back120. It does not implicitly select GetRootBody. LinkTargets
requires CBody116F6F–116FCB, unlinks117007 and initializes11702F. No existing
player-owned motor/root identifier or durable player-body owner is established.

Entity creation1179A0 copies a joint template and invokes virtual8 at117AE1;
Engine11CAC0 allocates15C bytes for a motor. Configure117520 updates target values
and relinks117627; boot117640 tailjumps LinkTargets. Engine123CB0 Initialize is
void thiscall(CEntity*,CBody*,CBody*,const QuatVect&),ret10; joint owner4 is the motor
entity, bodiesC/14 attach to joint lists/frames. Unlink123AF0/internal123A60 detaches
and clears pointers. Delete117650/destructor117420 remove owned joint/renderable/
template. Body attachment references are not durable player identity evidence.

## Why it cannot settle a relative roomscale request

Engine setters X force/velocity/position11C590/11C5A0/11C5B0 and Z equivalents
11C610/11C620/11C630 are void thiscall(float),ret4. They persist values in limit
records: X24/Z6C, force10/velocity14/position18 within each. Position is an
attachment-axis coordinate, not a one-shot relative body request. Sam wrappers
116B10/116B40 and116C50/116C80 activate bodies but return no acceptance/duration.

Engine GetJik11CCE0, long thiscall(CClusterData&),ret4, projects attachment
separation and calls AddLinearLimit124DC0 for X/Y/Z11DFF2/11E065/11E0D8. Return is
row count, not movement acceptance. AddLinearLimit motor branch requires force>0,
and124FE5–125025 computes:

`targetVelocity = clamp((desired-current) * phy_fLimitSp/substepDt, -speed,+speed)`

Default limit factor0.2 is configurable; force bounds124F9B–124FB0 still apply.
Thus even ideal unsaturated enforcement corrects a fraction per physics substep,
not direct1:1 displacement. Solver13C7E2–13C7F0 owns simGetStep()/solverSubsteps at
ownerCC/clusterB8. Native units per simulation time have no XR-metre conversion.

Joint traversal13DE70/13DEAB dispatches GetJik; shared solve1405A4→1405AB→1405CB
then integrates1405EC→13EA10. Integration13ED83–13EDB0 uses final velocity times
native dt;1406D4→13CB60 publishes combined placement/velocity. Contacts, ordinary
motion and all constraints have interacted. Solver1406F0 selects worker-owned
indexed1E0-byte storage, not simulation-thread request settlement storage.

GetValues11BF10 returns configured limits; GetPlacement11C4A0 attachment frames;
GetRelVelocity11E100 combined velocity. None returns request identity or accepted
roomscale displacement. Post-minus-pre movement and subtracting requested ordinary
locomotion cannot isolate nonlinear contact/acceleration effects. Motor impulses
also are not accepted roomscale distance.

PostReceiveUpdate1170C0 relinks/restores motor values: motor entity replication,
not exactly-once player roomscale/ack/client reconciliation/listen-server proof.
Existing engine simulation interval spans native physics but has no attributable
motor acceptance. Current head/grip packets are produced before that settlement;
speculative origin rebase would be unjustified.

Missing: proven player-root motor owner/lifetime, finite source-attributable
post-collision displacement and authoritative application/settlement. This
candidate directly contradicts the displacement requirement. No limits/body-mode/
solver replacement or persistent motor policy is added. Verified module
fingerprints remain those in RESEARCH.md; no proprietary dump is committed.
