# Native roomscale collision API audit

Two Astra/xhigh read-only investigations examined native checked placement and
the solver's candidate-pose producer. Effective profiles were verified locally;
no edits, delegation or native execution. Findings are scoped to the pinned
Engine/Sam2Game modules in RESEARCH.md. Addresses below are RVAs.

## Checked placement does not slide or certify small moves

Engine CMechanism::SetAbsPlacement 134010 is int thiscall(const QuatVect&,ulong),
ret8. Flag1 invokes part checks; a fraction below1 at1346FC branches to failure
135C62 before commit. Success commits134D7B and returns1 at135D35. It discards
the fraction and supplies no clipped displacement or sliding result. The
no-parts/model-aspect fallback13407C commits directly.

CMechanismPart::CheckMove1315D0 returns float in x87 ST0, ret4; it invokes aspect
CheckMove13185E without movement. CPrimitiveHull::CheckMove52030 takes old/new
poses, fraction and hit-aspect references, ret10. Translations below0.9 times
InnerRadius skip the query52091→52180. Larger moves cast a centre ray52123 with
an allowance; this is not a swept avatar volume. A returned fraction1 therefore
cannot certify collision-free roomscale travel.

Puppet relocation8EA90/legged67A90 and ordinary correction90706 use unchecked
placement or mutate orientation/brain state; group relocation8EB20 has no result.
DoLocalMove9EE30 copies action into state4 and returns literal1 at9F2C5, not
accepted movement. Model GetAbsPlacement130B20 is distinct from raw root-body
placement via133B60/26FF0. Borrowed mechanism/part pointers cannot survive
replacement; parts are destroyed by131F80.

## Solver integration and pose targets

Simulation1B7406 enters physics10D7C0. Solver seed13CA60 writes six velocity
scalars/body into owner+BC and seven-float poses into+A8. Candidate routine
1402D0, called1406CC inside140690, invokes integrator13EA10 at1405EC.
13ED83–13EDB0 integrates position += finalVelocity*substepDuration;13EEB8 writes
the result. UpdatePlacementsAndVelocities13CB60 at1406D4 reduces CheckMove
fractions and commits SetRelPlacement13D237. There is no isolated per-request
accepted displacement; using internal scratch would reconstruct the solver.

Kinematic flag body+4C/80000 (IsKinematic283D0), constructor110ED0 flags90000,
sixDOF. Hybrid constructor110410 uses threeDOF. EvaluateAnimation136D50 computes
(desired-current)/step1376CC–13771D and sets velocity1379C1/1379CF; setter10FE60
is void and activates the body. Kinematic bodies have zero inverse mass/inertia
and cleared forces at13D550, unlike the player's ordinary hybrid collision
response. Static model OnStep14B860 calls animation14B894; this does not establish
safe player roomscale movement.

Manipulator SetDesiredPlacements126070, void thiscall(two pose references),ret8,
copies persistent targets127C60. GetJik1264F0 converts error to capped velocity
and force/torque-limited constraint rows, not acceptance. Puppet carrying86480
creates a joint for the carried object's body86606, stores handle+374 and submits
targets8692C/9B83A. It is not a proven player locomotion adapter.

Generic motor X setters11C590/11C5A0/11C5B0 are void float force/velocity/position
commands. Sentinel FF61B1E6 at11C5C0 selects velocity mode. AddLinearLimit124DC0
requires positive force; position mode applies error*phy_fLimitSp/substepDuration,
clamped speed and bounded force. Game wrappers116B21/116B51 activate bodies;
PostReceiveUpdate1170C0 establishes motor-entity replication, not player request
replication. Relative velocity11E100 reports combined motion only.

Solver storage is worker-owned:1406F0 selects2ECEE0+index*1E0, index<16;
10EE90 starts workers, participates on the main thread and joins them. Main-thread
submission does not make solver scratch main-thread-only.

## Main disposition

NO-GO for these APIs as default proper1:1 roomscale adapters. Keep existing
bodies, collision contacts, simulation and replication. No body-mode change,
independent physics solver or unchecked relocation is justified. No prototype
is added. The input-settlement alternative's limitations remain documented in
ROOMSCALE_INPUT_DESIGN_REVIEW.md; it does not close body collision.

Astra identified one possible bounded follow-up: GenericMotorEntity::LinkTargets
116EE0 and concrete callers, to establish whether a native motor owns a player
root. Even a positive ownership finding would not prove attributable acceptance.
This audit neither claims all native APIs exhausted nor changes the active goal.
