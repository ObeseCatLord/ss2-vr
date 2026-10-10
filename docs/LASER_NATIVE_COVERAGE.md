# Non-scoped handheld laser aiming

## Current calibration coherence — 2026-10-10

Calibrated native muzzle reach now remains native for ordinary weapons too.
All16used native slots retain and recheck current model/configuration/resource/
stretch identity; authored grip corrections remain IDs1/13. Earlier audit limits
below are historical. No new other-weapon firing or headless acceptance is claimed.
See IMPLEMENTATION_STATUS.md for exact build and remaining native model availability.

Lasers are enabled by default. Both hands independently sample their current
native weapon's shooting placement, validate ownership/equipment/tracking, and
query a zero-radius native bullet ray. Its endpoint is the nearest eligible hit
or the configured maximum distance. Both eye renders consume one immutable pair.
Opening a wheel, selecting, replacing a weapon, losing a hand, or receiving stale
calibration suppresses that hand's invalid beam rather than borrowing the other.
Scope zoom is not required for a non-scoped weapon's laser.

This retains native ammo, charge, damage, recoil and projectile handling. A laser
shows the native central aim ray; random spread, arcing projectiles, penetration
and explosion areas are not represented by a straight beam.

## Installed native shooting dispatch

Astra/xhigh inspected the pinned owned binaries; the integration owner separately
verified all three hashes, both shooting exports, all16 listed slots and the four
ray export addresses. Local model/effort tags verified; backend unattested. Exact
binary identities are in [installed-build.json](installed-build.json).

Sam2Game exports two `GetShootingPlacement` implementations: base RVA `0x4a6e0`
and sniper `0x172a70`. The inspected weapon vtable slot is `+0x1d0`:

| Weapon | Vtable RVA | Shooting getter RVA |
| --- | --- | --- |
| Circular saw | 0x2cdd10 | 0x4a6e0 |
| Zap gun | 0x2d2ca0 | 0x4a6e0 |
| Auto shotgun | 0x2ccc78 | 0x4a6e0 |
| Double shotgun | 0x2cec60 | 0x4a6e0 |
| Uzi | 0x2d26a0 | 0x4a6e0 |
| Minigun | 0x2d0590 | 0x4a6e0 |
| Rocket launcher | 0x2d1258 | 0x4a6e0 |
| Grenade launcher | 0x2cf918 | 0x4a6e0 |
| Plasma rifle | 0x2d0be8 | 0x4a6e0 |
| Klodovik | 0x2cff28 | 0x4a6e0 |
| Cannon | 0x2cd960 | 0x4a6e0 |
| Serious bomb | 0x2d15d0 | 0x4a6e0 |
| Colt | 0x2ce340 | 0x4a6e0 |
| Beam gun | 0x2cd268 | 0x4a6e0 |
| Flamer | 0x2cf250 | 0x4a6e0 |
| Sniper | 0x2d1f00 | 0x172a70 |

Native Uzi/Zap firing invokes this weapon-receiver slot. Calling the original
base getter preserves virtual attachment and weapon-in-view matrix subdispatch;
Flamer's attachment override therefore remains native. The native matrix route
reads both owner weapon handles and selects dual parameters when both are present.
No extra non-sniper shooting hook or invented dual-weapon eligibility is needed.

## Native collision result

Engine `cldCheckRay` (`0x29200`) enters the collision traversal through `0x29100`.
The traversal at `0x2f8a0` filters category/avatar/mechanism and iterates candidates
and children against a shrinking current best distance. The inspected primitive
and model hull paths reduce that distance; zero-radius model traversal continues
through eligible triangles after accepting a closer hit.

`raySetMaxDistance` (`0x1b3530`) initializes maximum and current-hit distance.
`rayIsHit` (`0x1b35a0`) requires a distance below maximum;
`rayGetHitDistance` (`0x1b35c0`) returns that scalar. The adapter's minimum .01,
zero radius, native category/exclusions and query cleanup retain this selection.
No continue-ray adapter or change to native collision physics is indicated.

## Evidence limits and user acceptance

Native getter coverage and nearest-hit semantics are established for the pinned
routes. Whole-binary fingerprint checks enforce those installed bytes. Portable
checks cover immutable eye pairs, per-hand wheel/selection/loss, invalid endpoints,
age, replacement requests and identity. They do not prove every weapon's visual
muzzle calibration or grasp alignment. Verified alignment remains IDs1/13-only;
no reach tolerance or model correction was widened based on this audit.

The [user test](USER_VR_TEST.md) must check every available non-scoped weapon,
both hands, supported native dual combinations, nearby and distant surfaces,
actual muzzle origin, switching and tracking interruptions, and ordinary scope
ownership separately. No firing or gameplay was launched for this investigation.
