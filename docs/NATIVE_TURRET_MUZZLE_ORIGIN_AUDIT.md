# Native turret muzzle origin: bounded investigation

Static inspection only. This records the native projectile-origin boundary for
`CTurretPuppetEntity`; it adds no hook or game call. Evidence is the installed
`../Bin/Sam2Game.dll`, SHA-256
`5628b4ed30a966f10c8e8ea46bf0789257a35ea80127bacacebe28b1ce5303df`,
matching `installed-build.json`, plus owned turret resources. Main spot-check
locates the stock turret resource and model in `All_PC_02.gro`; see correction below.

## Result

The selected `Gun`/`GunUD` body origin is **not proved** to be the native
projectile muzzle. It is the source of `CTurretPuppetEntity::GetShootDirection`
at `0x155CE0` only: its selected `CBody+0x2C` 28-byte `QuatVect` is used to
derive direction, not passed to a projectile call. The installed `Turret_Cannon.ep` instead declares the generic-shooting
`CSingleBlast` field `sb_idBarellAttachment` (engine spelling) and supplies the
nearby authored identifiers `Blast01` and `Barrel01`, alongside the cannon
model and turret projectile resource. This establishes a distinct authored
barrel-attachment concept, but does not establish its transform, its runtime
object offset, or a pure callable getter.

Consequently, there is no approved tip-origin sampler. A presentation laser may
use the already-audited `Gun`/`GunUD` body transform only as a body-origin
fallback if that product decision accepts it; it must not represent that as the
native projectile muzzle. Tip-accurate sampling remains suppressed.

## Actual turret firing route

`CTurretPuppetEntity::OnStep` is the inherited `CPuppetEntity::OnStep` jump at
RVA `0x156410 -> 0xA69F0`; there is no turret-specific `OnFire` override in the
export/vtable evidence. The turret resource is a puppet configuration, not a
`CCannonWeaponEntity` configuration: its resource-file list names the cannon
model and `CannonBall_Turret.ep`, and its metadata contains `CShootingProcess`,
`CShootBlast`, `CSingleBlast`, `sb_idBlast`, and
`sb_idBarellAttachment`.

The native animation event route is concrete. `CPuppetEntity::OnAnimEventShootingBlast`
at `0x875E0` resolves the current shooting-process data from the resource held
at `this+0x47C` (`0x875FA..0x87697`), records the event identifier at
`this+0x1C4` and increments the blast count at `+0x338`
(`0x876D0..0x876E6`), then calls the resolved shooting executor's virtual
`+0x1D0` with current shoot ID, blast ID, and fractional consecutive-shot value
(`0x87705..0x87727`). This is the original event/action path and is prohibited
as a sampler. It also confirms that sampling `GetShootingOriginAttachment`
cannot stand in for a turret blast attachment: that getter reads the current
action fields and copy-on-write resource at `+0x47C`.

At the projectile boundary, `CPuppetEntity::FireProjectileFrom` at `0x8AB50`
takes a by-value 28-byte `QuatVect` at stack `+0x0C`, and passes that same
placement through the puppet virtual projectile-properties/spawn sequence
(`+0x3D4` at `0x8AC44..0x8AC4E`). `SpawnProjectile` is `0x9DE30`. Thus the
placement supplied before `FireProjectileFrom` is the native projectile origin
subject to its wall correction; none of this path says that it is the
`Gun`/`GunUD` body origin.

## Why the weapon attachment getter cannot be borrowed

`CCannonWeaponEntity::OnFire` at `0x163C20` does obtain its own virtual
`GetShootingPlacement` (`vtable +0x1D0`, call `0x163D15..0x163D1D`) before
calling the owner's `ThrowProjectileFrom` slot `+0x3C8`
(`0x163D3F..0x163D41`). Base `GetShootingPlacement` at `0x4A6E0` resolves the
weapon model, calls its `GetShootingAttachment` slot `+0x1C8`, and invokes
`mdlGetAttachmentAbsolutePlacement(CModelInstance*, IDENT, Matrix34f&)` at
`0x4A81E`. That is a real model-attachment placement algorithm, not an offset.
It is not the turret route: `Turret_Cannon.ep` has no `CannonWeapon.ep` or
character-tool resource reference, and no static evidence ties its `Barrel01`
to a `CBaseWeaponEntity` instance. Calling it would also require an unproved
weapon instance and attachment selection.

## Exact remaining gate

One narrow static proof remains: decode this fingerprinted turret resource's
`CSingleBlast` instance to prove which `IDENT` populates
`sb_idBarellAttachment`, then locate the executor reached through
`OnAnimEventShootingBlast` virtual `+0x1D0` and prove whether it resolves that
ID as a model attachment, a mechanism body, or another placement. If it is a
model attachment, the required read-only seam is the documented
`mdlGetAttachmentAbsolutePlacement` ABI with the executor's actual model
instance and proved `IDENT`; if a mechanism body, the same evidence must prove
the body name and its `QuatVect` layout before `CBody+0x2C` is reused. The
referenced `Content/SeriousSam2/Models/Turrets/Controlable/Cannon/Cannon.mdl`
is present in `All_PC_02.gro` (main correction to the worker's initial absence
claim). Its authored local placement can be inspected as owned content but has
not yet been decoded/proven. Until the actual route and transform gate closes, do not call firing,
animation, COW/resource, RNG, RPC, or action code and do not infer an offset.

Main archive verification: stock `Turret_Cannon.ep` is11322 bytes,
SHA2564e1c39c04b1b0edb38954af66a3942f2f30039807f1562b97e748a69a4315607;
`Cannon.mdl` is15957 bytes,
SHA25647352e2041c84777edf01c19b5e5a98859e525426bcbeebb8c364f9f85019e41.
Both exact virtual paths occur in `All_PC_02.gro`, using case-insensitive slash
normalization. No proprietary resource is committed. Stock-vs-other-archive
field values and runtime precedence still need actual decoding.

Subsequent main follow-up locates the actual CShooter virtual1D0 executor and
purpose1 model-attachment route; see NATIVE_TURRET_EXECUTOR_FOLLOWUP.md. That
source-only record postdates the immutable0.2.7 package. Pure idle selection and
getter lifetime remain unproved; the initial missing-executor gate is superseded.
