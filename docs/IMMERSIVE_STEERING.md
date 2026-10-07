# Immersive steering: requirement and native evidence

Owner requirement, 2026-10-07: vehicles with steering wheels must support grabbing
and turning the wheel with either hand or both. Hand transfer should not cause a
steering jump. This feature is not implemented yet.

## Existing input route to preserve

The mounted adapter already disables handheld weapon wheels while seated, keeps
seated movement in the vehicle's native input axes, and supplies controller aim
through the original look clamp. Steering should join the existing local input
producer and native ClientAction transport, not add a vehicle physics controller
or another network steering command. Throttle, braking, aim and firing remain
independent of the hand-grab calculation.

The pinned Sam2Game wheeled class retains EnforcePuppetMoveLook at1563D0 and
SetDriveSteerRatioAndLookDir at802C0. The former forwards raw move and look; the
latter stores move at8C and look atA4, preserving pitch adjustment and mode3.
The native wheeled step consumes X for steering and Z for driving.

There is an important limit: the inspected mode3 wheeled steering path selects
zero or a native joint limit based on the input sign and each road-wheel's flags.
It does not prove proportional steering from the input magnitude. A floating
input named "ratio" is insufficient evidence that a90-degree hand turn produces
a proportional front-wheel angle. Do not replace native physics or silently
advertise proportional handling. `tools/verify_vehicle_steering_native.py` checks
36 instructions, exports, class slots and the native joint target import against
the installed binary fingerprint. It installs no adapter.

## Geometry and control admission still needed

A road-wheel physics joint is not the driver's steering wheel. Wheeled class
identity alone does not prove a visible steering wheel, its center, rotation
axis, radius, driver seat, grip locations or current visual angle. Native model,
mechanism, skeleton and attachment metadata must establish those separately.
The required asset inventory is pending transfer from the user's offline PC.
No generic wheel placement or guessed model offset has been installed.

Croteam's [game description](https://www.croteam.com/serious-sam-2/) covers varied
vehicles and mountable animals. The publisher's [original manual, pages21–23](https://www.mogelpower.de/manuals/Serious_Sam_2_Handbuch.pdf)
also lists very different vehicle types, including a rollerball and aircraft.
These are background references, not evidence that a specific installed model
has a steering wheel. Match controls to the actual vehicle rather than applying
a car wheel to every ride.

The existing squeeze actions already supply weapon-wheel buttons while on foot.
Their current IPC bit does not distinguish an inactive action from a released
grip. Immersive grabbing needs positive action availability/generation and fresh
release-to-rearm evidence; do not use tracking recovery as an automatic regrab.
The recenter chord must interrupt/release any steering grasp safely.

## Bounded interaction behavior

- Acquire only near the positively identified wheel rim, with a fresh squeeze
  press after a known release. Either hand may acquire first.
- Compute hand motion in the wheel's non-spinning vehicle-relative frame, so
  vehicle translation/rotation is not mistaken for turning the wheel.
- With one hand, use that hand's angular motion around the wheel axis. With two,
  combine their incremental turn without cancellation from opposite grip points.
- Joining, leaving or exchanging hands must preserve the current commanded angle.
  New grips establish their own baseline instead of snapping to an absolute angle.
- Native limits remain authoritative. Reject nonfinite poses, singular radial
  projections, impossible jumps, stale samples and superseded ride/seat/rig owners.
- Loss of one tracked hand releases that grasp; a valid remaining hand can keep
  control. Losing both, leaving the seat, pausing or recentering retires the grasp
  and requires fresh release before reacquisition.
- Ordinary joystick steering remains usable when there is no admitted grasp.
  No input helper, fixture or compilation result constitutes native integration.

No game, Wine, Windows executable, headset or driving session has been executed.
