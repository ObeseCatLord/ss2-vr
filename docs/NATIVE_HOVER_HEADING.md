# Hover steering heading — bounded native evidence

Read-only Astra/xhigh investigation, 2026-10-10. Local routing verified; no
independent backend attestation or native execution. REA instructions were
available but binary-analysis tools were not callable; existing Capstone/pefile
were used, with no registration repair, downloads or game changes.

Owned modules: Core.dll SHA256
7a1bd56b9bfa3edfbb23f4d3c96490e40a7c0b031b85e797e1af91b2ba3cf207;
Sam2Game.dll5628b4ed30a966f10c8e8ea46bf0789257a35ea80127bacacebe28b1ce5303df;
Engine.dllda6efc9f72637eb3b6f48eadca2107be89b09c00618b6e72d5d3632938a7d851.
These are source/static evidence identities, not package or hardware acceptance.

## Conversion and native target

The existing cdecl hidden-output-pointer Core QuaternionToEuler export at
17050–171B3 returns heading as atan2(2(wy+xz),1-2(x²+y²)). There is no degree
conversion or heading offset. A unit pure-yaw quaternion yields
atan2(sin(delta),cos(delta)), in radians, numerical zero for identity and the
same sign for small positive/negative yaw. Pure yaw avoids pitch singularity.
Across ±pi the representative wraps; whole-turn winding is not retained.

The existing positive resource coverage is three Fighter variants and Saucer;
this investigation did not expand authored geometry or lifecycle coverage.
Their vtables2A8558/2B8420 share control514→158810; horizontal-strafe admission
selects processed movement/look through5B4→80690. It stores heading unchanged
at actorA4 and uses mode2; its angle adjustment concerns pitch. Rotation-ratio
5D8→930A0 wraps heading differences. Mode2 actuator5D4→158BE0 passes actorA4
to Engine SetHeadingAngleDesiredPosition at159319. Original ClientAction
transport remains F35CD→EE0F0→EE245/virtual388→EE260; no replacement physics.

Engine heading setter121570 forwards to joint target1248D0. The angular solver
124920–124DB8, particularly124CC8–124D3F, reduces desired-minus-current error
with constants P=3.1415927410125732 and T=6.2831854820251465:

    r = fmod(desired - current + P, T)
    error = (r < 0 ? r + T : r) - P

It scales by approximately0.2/timestep, clamps native angular speed, and applies
native low-speed suppression. Adding the native-converted relative yaw to the
returned look heading is therefore a valid angular target modulo a turn.
The exact antipode reduces to-P. This supports the incremental adapter; it does
not establish physical alignment, actual grab behavior or runtime comfort.

## Limits

Conversion uses mathematical2pi while the solver uses rounded T, so periodicity
is not universally bit exact. The investigator's offline binary32 reconstruction
covered4617 combinations with maximum circular discrepancy about9.54e-7 radians;
it did not execute game code. Very large scalar baselines can lose small additions.

Joint target1248D3–124915 compares unwrapped old/new targets against approximately
one degree. Equivalent targets separated by a turn can change wake/change flags,
consumed by121480. Full-state periodicity is disproved. The adapter retains this
native behavior; it must not claim identical wake bookkeeping. User vehicle
acceptance must check wrap crossings and transfers. CRT/IAT identity, x87 mode,
loaded runtime behavior and device acceptance remain unattested.
