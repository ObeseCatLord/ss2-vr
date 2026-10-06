# Native turret laser: source review brief

Main owns all writes. Baseline ec3deac plus current uncommitted implementation.
Design/disposition in NATIVE_VEHICLE_LASER_DESIGN_REVIEW.md. Goal remains full
immersion; this bounded slice is original turret aiming laser only. No runtime
testing authorized. Exact native table gate is2B39E8, not universal vehicle
support or projectile-scatter prediction.

Read-only Astra/xhigh SOURCE contract: review src/common/lasers.hpp,
src/game/engine.cpp new laser helpers/trackedRayInit/freezeLasers/bindings,
src/game/native_memory.hpp and unchanged moved helper in remote_render.cpp,
tests/vehicle_laser_checks.cpp, existing tests/ui_checks.cpp and compiled x86
caller evidence. No edits/delegation/game/host/Wine/XR/network. Main integrates.
Output <=1200words prioritized defects, exact file/native evidence and explicit
bounded GO/NO-GO. Missing proof should be bounded, not a redesign of other
features. Challenge the220-line native diff against smaller reuse; no duplicated
model/idle/fire/transport policy is intended.

## Connected source

- Existing native query return21AE69/no-eye/simulation thread now additionally
  requires actual native main thread. DAD90's nine counts/evaluated pointer must
  be idle before/after model evaluation. 2C7F9C is never treated as idle/lock.
- Mounted branch precedes stale handgun/selection requirements. Source captures
  rider/seat/ride, model/instance/resource/config, action/blast/attachment and
  mechanism. Exact table gate and bounded current process/blast presence protect
  the original selector's unchecked lookup; native550 still supplies actualID.
- Explicit bool mdl attachment must succeed; original virtualAC purpose1
  supplies actual world origin; original5A4 supplies reported aim. Direct native
  normalized direction is the collision ray, shortest-arc orientation only
  supports native line/hit presentation. No native fire/gameplay/pose mutation.
- Every getter/collision boundary checks current source, scratch, fresh compatible
  local input and exact uncancelled Requested request. Changed COW views reject
  and are reacquired on the next query. Original native ray lifecycle clears
  intermediate hits and always leaves a clean baseline for the native caller.
- Existing LaserAim bank carries local source kind/provenance/request sequence;
  handheld cache retains original bounded policy, vehicle cache reuse omitted.
  Final publication rechecks; existing two-eye freeze checks actual current
  vehicle source and latest hand tracking. Both eyes draw the same frozen beam.
- Collisions exclude player and actual ride mechanism. One right-side beam;
  both native fire commands remain unchanged. No IPC7/wire6/renderer change.

## Checks and limits

First full cross-build passed; all13 offline groups passed including pose/roll/
near-opposite axes, all outstanding scratch fields, source COW/model/seat/action/
attachment/mechanism/request changes and identical freeze. Final small native
capacity and owner-link checks are awaiting refreshed build. Compiled callers
will be independently inspected in both x86 products. Portable fixtures exercise
production admission/pose helpers, not native invocation availability/concurrency.
Idle count checks are explicitly not a cross-thread lock. Complete unhooked
worker concurrency, normal mounted query opportunity, moving freshness and
appearance/performance remain unknown/unverified. Other immersion gates are out
of this review and remain active. No source acceptance is inferred from tests.

