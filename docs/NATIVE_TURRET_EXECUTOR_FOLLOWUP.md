# Turret firing origin: main follow-up

Unreviewed bounded native/owned-content investigation after the0.2.7 package;
no production change or runtime execution. Same installed Sam2Game fingerprint
as NATIVE_TURRET_MUZZLE_ORIGIN_AUDIT.md. No proprietary bytes/content committed.

## Concrete executor and actual placement route

OnAnimEventShootingBlast875E0..8773F resolves the puppet's shooter handle1D4
at87705/8770C, then calls that object's virtual1D0 at87727. CShooterEntity's
vtable2B2398 is independently identified by virtual4=mdGetDataType160270;
virtual1D0=GenericOwnerShootOntoFoe160EE0,1CC=ClampShootOriginAngles160B20,
1D4=FireProjectile161EB0,1D8=FireBullets1621F0 and1DC=ApplyScattering160500.
This closes the earlier missing executor identity, rather than inferring it
from a name or a nearby code address.

GenericOwnerShootOntoFoe160EE0..161D96 is mutating, not a sampler: it performs
owner resource47C COW/refcount operations, changes shooter parameters/placement,
consumes RNG on its inspected branch, clamps and calls original fire. It resolves
the native shooting-process list at owner-resource294 via2ECC0, then the selected
blast via2EBF0. Those two bounded lookup bodies only search existing pointer
arrays for a matching first-field IDENT and return pointer/null (thiscall/ret4);
this does not establish which process/blast should be selected while idle.

Original shoot origin is obtained from owner virtualAC at160F95 with hidden
QuatVect output and purpose1. For the actual turret table2B39E8 (virtual4 is
mdGetDataType154DD0), virtualAC=CPuppet GetViewOrigin8EFF0. Its purpose1 branch
at8F1C2 resolves virtual550 GetShootingOriginAttachment at8F1D9, tests invalidID,
resolves the current model-renderable handle120, then calls original
CModelRenderable GetAttachmentAbsolutePlacement at8F20D with that actual IDENT.
The selected native attachment therefore precedes the shooter/projectile route;
the direction body's transform is not sufficient evidence for muzzle origin.
GetViewOrigin itself also performs native resource COW/refcount work and later
attachment/matrix calculations, so no new idle-purpose1 sampler is accepted.

FireProjectile161EB0..1621DE derives/scatters quaternion from shooter placement24,
writes that quaternion back, performs remote/authority checks and native wall
correction, constructs/spawns original projectile properties and cleans up.
It cannot be invoked for a laser. Its native placement originates from the
prepared shooter state; CPuppet FireProjectileFrom is a separate callable
boundary and alone did not establish the turret route.

## Direction-slot correction

The turret's actual table starts2B39E8, not the pointer16 bytes later that can be
misidentified by searching for a function address. Virtual594 is inherited
GetFrontDirection804B0; virtual5A4 is GetShootDirection155CE0; virtual5A8 is
GetExactShootTarget90260 with a float distance. The GenericOwner call160FCB
uses5A8, not an attachment identifier. Earlier investigation's wording that
DoConsecutive's virtual594 was GetShootDirection is corrected to GetFrontDirection.
No production binding depends on that mistaken label; mounted clamp59C/5A0
identity remains as previously proved.

## Owned model/resource evidence and remaining gate

Stock Cannon.mdl in All_PC_02 has IDNT at3C9:24 names through545; file ordinal5
is Barrel01 (also GunLR/GunUD/Cannon/Blast01 identifiers). Stock Turret_Cannon.ep
has IDNT at16A:13 names through256; ordinal11 is Barrel01,10 Blast01. Ordinals
are file-local translation inputs, never valid hardcoded runtime IDENTs. Merely
finding names does not decode the actual CSingleBlast member value.

Additional exact native selector proof: GetShootingOriginAttachment8EE70..8EF22
returns native selected-blast field4 through process294/2ECC0/2EBF0 when both
action IDs1BC/1C4 are valid. If either is invalid, branch8EF05..8EF22 invokes
original strConvertStringToID on the literal Barrel01 at29F9D4, without entering
the resource COW branch. Thus idle default selection is native, rather than an
invented hardcoded file ordinal. A separate idle per-button selector is not
automatically required for the original single origin; multi-barrel/per-button
coverage still needs its actual native policy.

Engine mdlGetAttachmentAbsolutePlacementD8FA0..D908F is cdecl int-return with
model-instance pointer, by-value runtime IDENT and Matrix34 output reference.
It reports0 when lookupE28B0 returns-1, otherwise copies the found matrix and
returns1. It performs model-config18 COW/refcounts and temporarily owns/restores
native child-traversal flag2C7F9C, invoking DAD90 cleanup. It is therefore not a pure
matrix peek; exact native phase/lifetime compatibility must precede new use.
The current handheld native shooting-placement reference already evaluates
original model attachments on the existing query phase, which is a reusable
behavioral reference, not evidence for replacing the model/record system.

Next design decision: compare borrowing the original virtual purpose1 origin
plus explicit successful attachment existence/identity admission with direct
attachment-matrix sampling. Reuse native selection/presentation/ray ownership
instead of adding idle process policy. Prove current ride/model/resource/frame
continuity and exact getter/native-record lifetime; preserve native wall-hit
queries and projectile/physics/RPC behavior. No direction-body, tracked-hand or
guessed offset fallback is accepted as a muzzle-tip laser. This follow-up closes
route and idle-default identity, not actual vehicle lasers or full equivalence.
