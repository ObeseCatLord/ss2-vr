# Vehicle laser muzzle boundary: investigation

Bounded static inspection only; no vehicle-laser implementation, game/host/Wine/headset/network run, or installed-game change. Evidence is for `../Bin/Sam2Game.dll`, SHA-256 `5628b4ed30a966f10c8e8ea46bf0789257a35ea80127bacacebe28b1ce5303df`.

## Read-only barrel-body candidate

For `CTurretPuppetEntity`, the native direction path supplies a narrow presentation-only source without entering generic fire:

- `CTurretPuppetEntity::GetShootDirection` at RVA `0x155CE0` is x86 `thiscall`, with `ECX=this`, a hidden `Vector3f*` at `[ESP+4]`, and `ret 4`. It resolves the mechanism handle at `this+0x114`; when absent it forwards to CPuppet's implementation at `0x805E0` (`0x155CF0..0x155D12`).
- With a mechanism, it converts the native IDs `Gun` and `GunUD`, calls imported `CMechanism::GetBody(IDENT)` (`0x155D33..0x155D78`), prefers a live `Gun` body and otherwise uses `GunUD` (`0x155D81..0x155E59`). The selected `CBody` has a 28-byte QuatVect-shaped payload at `+0x2C`, copied by the original code (`0x155D88..0x155D96` and `0x155DFA..0x155E08`). The original derives its returned forward vector from the copied quaternion (`0x155D98..0x155DE1`, `0x155E0A..0x155E53`).

The bounded candidate is therefore: on the existing simulation/query phase, while the local mounted identity is stable, resolve the same mechanism and selected body, copy that 28-byte transform into local storage as the barrel-body origin/rotation, and call the original `GetShootDirection` for the native direction. This invokes no generic shooting, projectile, animation, action, sound, or RNG path. It is only established for turret bodies and only for the barrel body's transform origin. The inspected code does not prove a muzzle-tip attachment/forward offset beyond that origin; no laser may be emitted if a tip rather than the body origin is required.

`CMechanism::GetBody` is imported with its declared `thiscall` ABI, `ECX=CMechanism*`, one by-value `IDENT` argument, and `CBody*` in `EAX`. The handle/body resolves must be treated as transient: validate the same local rider/ride/seat and mechanism handle before and after copying/calling, on the game thread. Any null, changed, or unrecognized body is a no-laser result. Do not retain native pointers across the phase or use this in stereo collection.

## Generic shooting and attachment boundary

`CPuppetEntity::DoGenericShooting` at RVA `0x891D0` is `thiscall`, receives `IDENT` plus a 32-bit argument, and `ret 8`. It serializes/executes the host RPC path (`0x891F6..0x89329`), writes generic-shoot fields `+0x1C0/+0x1C8/+0x1CC`, records simulation time at `+0x358/+0x35C`, and dispatches virtual `+0x330` with action `6` (`0x8933A..0x8937A`). It is prohibited for laser sampling.

`DoConsecutiveGenericShooting` at `0x89E30` is also not a sampler: it evaluates active action/animation state, may clone/remove/play model animation and sound, and uses virtual `+0x594` for direction at `0x8A05D` (actual inherited GetFrontDirection, not the separate turret GetShootDirection slot5A4; main table correction in NATIVE_TURRET_EXECUTOR_FOLLOWUP.md). Its examined path does not obtain `GetShootingOriginAttachment` or a turret-body origin.

`GetShootingOriginAttachment` at `0x8EE70` is a hidden-return `IDENT` `thiscall` (`ret 4`), not a placement getter. It depends on current fields `+0x1BC/+0x1C4` and uses a copy-on-write smart-resource sequence that assigns `this+0x47C` and adjusts references (`0x8EE8E..0x8EEC1`), then returns configuration data (`0x8EEC3..0x8EEF4`). It must not be treated as a pure substitute for the barrel transform or called merely to sample a laser.

`ExecuteOperatorFiring` at `0x8E580` loops four fire bits. It calls virtual `+0x520` for held input (`0x8E68C..0x8E691`), `+0x524` for a press transition (`0x8E6E4..0x8E6E9`), and `+0x528` for release (`0x8E728..0x8E72D`). When its virtual `+0x530` selector is nonzero, buttons 0 and 1 are remapped to the opposite boolean callback value (`0x8E670..0x8E686`); buttons 2/3 remain their loop index. A presentation sampler must observe no such callback and must not infer attachment context from a physical action button.

## Minimal remaining gate

Static evidence supports the turret barrel-body origin plus native direction pair above, but does not establish that the body origin is the visible muzzle tip, nor an attachment offset applicable to every mounted vehicle type. The next gate, if tip accuracy is required, is a separate bounded proof of the selected turret body's authored muzzle offset without invoking fire/action/resource-COW paths. Until then, accept only the copied barrel-body origin or suppress the vehicle laser; do not fall back to `baseWeapon4A6E0`, tracked-hand pose, generic shooting, or guessed offsets.
