# Native vehicle seat camera/body anchor — findings

Binary examined: `Sam2Game.dll`, SHA-256 `5628b4ed30a966f10c8e8ea46bf0789257a35ea80127bacacebe28b1ce5303df`.

## Proven narrow pose seam

The local rider carries the current ride handle at `+0x544`, ride state at `+0x548`, and seat IDENT at `+0x54C`. `RegisterForRide` (`0x9AA30..0x9AAD8`) stores handle/IDENT/index at `+544/+54C/+550`; `DoStartRiding` (`0x7EE70..0x7EE7A`) sets state `3`; `UnregisterFromRide` (`0xA95C0..0xA9625`) clears the handle, resets state to `0`, and invalidates seat/index. Resolve afresh each simulation/render transaction; do not retain a vehicle pointer.

All target vehicle vtables retain `+0x224 = CPuppetEntity::GetSeatAbsPlacement`, RVA `0x84D10..0x84F4F`:

| class vtable | `+0x224` | control slots |
| --- | --- | --- |
| Wheeled `0x2A7EA0` | `0x84D10` | `+514=0x1563D0`, `+58C=0x802C0` |
| Aircraft `0x2B79B0` | `0x84D10` | `+514=0x1563D0`, `+58C=0x802C0` |
| Rolling ball `0x2B9240` | `0x84D10` | `+514=0x1563D0`, `+58C=0x802C0` |

ABI: `Matrix34f * __thiscall GetSeatAbsPlacement(void *ride, Matrix34f *out, IDENT seat)`; hidden `out`, `ret 8`. It first dispatches vehicle `+0x214`, then resolves the ride model `+0x120` and calls native `GetAttachmentAbsolutePlacement` at `0x84D25..0x84D45`; it returns the resulting world matrix at `0x84EEC..0x84F4F`. This direct callee does not read the rider or rider `+0xA4` look field. `CLeggedPuppetEntity::LerpRiderOntoSeat` independently calls its ride's `+0x224` at `0x62E1B..0x62E32`, confirming it is the native seat-placement route.

Use only while resolved rider state is `3` and ride/seat are current: convert this returned world `Matrix34f` to the existing `Pose` and use it as the body/eye anchor. Keep the existing head pose composition above that pose. This supplies native seat translation and vehicle orientation while the view is no longer derived from operator aim. The existing camera detour remains the narrow view seam: `CPuppetEntity::GetCameraPlacement` is `0x808A0..0x808D7`, hidden `QuatVect*`, `ret 4`; it can retain its original call and replace only the local XR result.

Do **not** use the current full `baseViewOrigin(player, ..., 0)` for a mounted anchor. `CPlayerPuppetEntity::GetViewOrigin` (`0x103300..0x103963`, hidden output plus purpose, `ret 8`) normally calls base `CPuppetEntity::GetViewOrigin` at `0x103901`, then `AnimateViewOrigin`. Base view origin (`0x8EFF0..0x8F6F9`) calls the object's `+0x54C` look getter at `0x8F19A`; player vtable `0x29E878 +0x54C` is `CPlayerPuppetEntity::GetLookDirEul` (`0x80550..0x80571`), a direct copy of player `+0xA4`. Its quaternion therefore follows native operator aim. `GetSeatAbsPlacement` is the proven replacement for the mounted body anchor, not a controller replacement.

`ClampLookDirEulForCamera` (`0x80580..0x805CF`, `ret 8`) only clamps `Vector3f.y` against camera-control `+0x3C/+0x40`; `ClampLookDirEulAsRide` reaches it through vehicle `+0x59C` (`0x901B0..0x901D4`). It is a view-angle limit, not a seat/world placement getter.

## Controls and unresolved `+0x2D8 & 4` admission

`ProcessOperatorInput` (`0x8D930..0x8E1A8`) tests `+0x2D8 & 4` at `0x8DC37`/`0x8E0F9`; when set it dispatches the vehicle `+0x514`. The three vtables above all select `0x1563D0..0x15640A`, which forwards raw move and look to `+0x58C`; base `SetDriveSteerRatioAndLookDir` is `0x802C0` (`thiscall`, two by-value `Vector3f`, `ret 0x18`) and preserves local/RPC physics behavior.

Class coverage for the bit is **not statically proven**. Wheeled/aircraft/rolling-ball constructors (`0x15B910`, `0x156300`, `0x159880`) call the base constructor but contain no `+0x2D8` write, and no target-class initialize/OnCreate body writes that field. The observed bit-4 writer is base `CPuppetEntity::OnEndCGS` (`0x893A0..0x894E4`, OR at `0x894CF..0x894D9`); it is not vehicle-specific. Do not admit vehicle controls merely from class/vtable identity. The missing bounded proof is the target vehicle lifecycle path that invokes this base writer (or another runtime/data-driven initialization) and leaves bit 4 set.

This establishes a camera/body anchor only. A later input adapter may feed tracked controller aim through the proven native setter, but this investigation does not authorize a replacement controller, vehicle firing path, RPC, or physics implementation.

Investigation evidence only; no Astra design/source verdict or vehicle adapter is implied.

## Narrower native body-pose reuse candidate

Main spot-check confirms `CLeggedPuppetEntity::LerpRiderOntoSeat` is62E00..639E6 (next export639F0). It retains separate body and operator-look quaternions: body output atstack-A8 comes from the native seat/interpolation branch, while native `SetLookDir` uses another quaternion atstack-70 through `QuaternionToEuler`63960/virtual5B063985, zeroing bank. The final body placement is passed to mechanism `SetAbsPlacement`639DA. This supports using the existing body pose rather than adding a seat-query cache.

Player v54 is `CPuppet GetAbsPlacement`8E8E0..8E964. It reads current mechanism114 via the native mechanism getter, or model120 pose2C. `Engine CMechanism GetAbsPlacement`130B20..130B6C performs only native handle resolution and seven-float pose copy from root body2C, with native unset fallback. No attachment/model query or renderer cleanup is called. Base `GetViewOrigin(0)` already composes its eye offset using this exact absolute pose, then replaces orientation with the operator-look quaternion. `GetRelativeViewOriginForPose`67C20..67D2E specifically uses native seated view-offset data98 in state3. Thus retaining base eye position/height while substituting the native body orientation is the smaller adapter candidate. It requires fresh mount/handle/pose validity and Astra review; no source adapter is installed.

Input ownership is also bounded: F3250→ED2E0 is the original brain reference-vector clamp, returnF3255. The same consumer copies the resulting stack-4C look and stack-38 movement and fire byte into brain virtual384 atF35CD (returnF35D3); actual vtable29BE90+384 resolvesEE0F0 `ProcessPlayerControls`. This preserves the original native ClientAction/RPC route. Player InjectFDA00..FDA7D checks virtual5F0 third-person then only injects native hand weapons; the separate local body collection treatment is still to prove. See NATIVE_MOUNTED_ADAPTER_BRIEF.md, an unreviewed main proposal.
