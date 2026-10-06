Scope_done: read-only audit complete; no files changed, no runtime execution.

1. **High — muzzle calibration caches an absolute model position, so body translation contaminates aim.**
   `weaponAbs()` stores `flatPose` from `originalWeaponAbs(w, matrix(anchor), ...)` as `calibration[h].nativeModel` ([engine.cpp](../src/game/engine.cpp#L404)). `shooting()` later treats `out->p - c.nativeModel.p` as a local muzzle vector ([engine.cpp](../src/game/engine.cpp#L363)). Both are world-space positions. Any anchor/body movement between calibration and a shot becomes extra muzzle displacement, then is merely clamped to 0.5.

   Fix: derive and cache an attachment/muzzle offset in the native model’s local frame from samples using the same canonical anchor, then reconstruct from the current target model pose. Do not cache an absolute `nativeModel.p` as an offset source.

   Verified native seam: `CBaseWeaponEntity::GetWeaponAbsPlacement` is Sam2Game RVA `0x4BFF0`; it accepts caller `Matrix34` and builds a model placement. Installed body begins by resolving weapon owner and querying weapon placement at `0x1004BFF0–0x1004C052`.

2. **Medium — dual-toggle can leave the snapshot’s handles/calibration stale for a tick.**
   After `toggleDual(p, 0)`, `update()` neither rereads both native handles nor clears/rekeys both calibrations ([engine.cpp](../src/game/engine.cpp#L227)). `handOf()` requires both the cached and live handle to match, so a native transition yields no hand association until the next `OnStep` ([engine.cpp](../src/game/engine.cpp#L101)).

   Installed `ToggleDualWielding` at RVA `0xFB090` changes dual state and invokes native transition calls; it does not directly write `+0x800/+0x804` in its own body. Whether its invoked virtual transition changes handles synchronously is uncertain. Make this robust: immediately reread both handles after toggle, reset gates/calibration for each changed handle, recompute aliasing, and publish that snapshot.

3. **Verified correct — per-hand selection mapping itself matches native ABI.**
   The source maps hand 0 to player `+0x804` and hand 1 to `+0x800` ([engine.cpp](../src/game/engine.cpp#L89)). Installed `CanChangeWeapon` RVA `0xFBAB0` uses exactly `0x804` for enum 0 and `0x800` for enum 1. Installed `SetCurrentWeapon` RVA `0x104310` writes only the selected side: hand 1 → `+0x800`; otherwise → `+0x804`. The post-selection reread of that hand ([engine.cpp](../src/game/engine.cpp#L209)) is therefore correct. No concrete cross-hand selection failure found.

4. **Shot orientation / camera-hook check — no direct camera-hook inclusion verified; indirect inclusion remains uncertain.**
   Base `GetShootingPlacement` is RVA `0x4A6E0`; it resolves weapon `+0x28` then calls virtual `+0xAC`, and invokes weapon virtual `+0x1C8`. It does not directly call `CPuppetEntity::GetCameraPlacement` (RVA `0x808A0`; documented virtual slot `+0x5F4`). Sniper RVA `0x172A70` either uses its own owner virtual `+0xAC` or delegates to base. Thus there is no static proof that `out->q` already contains the detoured head pose, and no verified double-head rotation defect in `shotOffset` ([engine.cpp](../src/game/engine.cpp#L358)). The unresolved dynamic type behind the `+0xAC` dispatch prevents a stronger claim without runtime/type evidence.

Verification: inspected installed PE32 Sam2Game/Core DLLs and supplied assembly; fingerprint matches documented Sam2Game hash. No tests/run performed.
Assumptions/open risks: model-setting verification was not visible here; no runtime proof of attachment meaning, timing, or virtual target type.
Follow-up: fix item 1 first; then make toggle post-state refresh atomic before evaluating any remaining transform behavior.


Parent provenance: effective CLI settings verified; ephemeral read-only review. Line references describe the pre-fix snapshot. No runtime execution.

## Disposition

The absolute-position defect is fixed: model calibration is stored relative to the original native camera, and shot retargeting transforms current native shot placement into that same camera frame. Offline regression covers body translation/yaw between calibration and firing. Handle/UI ownership is refreshed after dual-toggle and after the original native OnStep, without resampling controls twice. Main-thread vtable tracing resolves the previously uncertain owner `+0xac` dispatch to `CPlayer::GetViewOrigin` (RVA0x103300), rather than the HMD camera hook; its normal chain uses base GetViewOrigin and native animation/weapon modifiers. No direct double-HMD camera path was found. Physical alignment remains untested.
