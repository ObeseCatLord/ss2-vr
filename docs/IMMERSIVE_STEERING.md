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
The local stock inventory is now complete:14 root archives,12,342 entries and
4,385 model/mesh/skeleton/mechanism/parameter entries with CRC, identifier-table
round-trip and serialized object-count checks. Astra/xhigh found no positively
identified driver-wheel geometry. This does not prove an unnamed wheel absent.
Center, axis, rim radius, wheel-to-seat transform and visual angle remain unknown.
No generic wheel placement or guessed model offset is installed.

Notable candidates are Sirius_Car_05/Car_Blue, Sirius_Car_11/transport_vehicle_open,
HoverFighter/Fighter_Final, FlyingSaucer/FlyingSaucer, RollerBall/RollerBall_Vehicle
and Helicopter/Helicopter_Player under the stock Models/Vehicles tree. Named
Seat/Console/CockPit metadata is not a proven attachment/transform. Car_Blue's
referencing parameters serialize aircraft parameters; its filename does not
establish a steerable wheeled ride. Eight serialized wheel-joint templates occur
in turret/shooter models, so template type alone does not identify road or driver
wheels. Catapult's Wheel/Wheels_Front/Wheels_Back names remain geometry candidates,
not driver controls. Runtime precedence, loose overrides, world-embedded content
and mod-subdirectory archives were outside this bounded inventory.

Next content proof: identify an actual visible driver wheel in a fingerprinted
loaded model, its surface/bone/attachment ownership, rim and non-spinning parent
frame relative to the driver seat. Then reuse the guarded native attachment
query with explicit success/current rider/model/seat ownership. The existing
turret-specific gate does not authorize other classes. Private inventory and
asset hashes remain outside public source; no assets/disassembly are published.

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

The inventory/review used no runtime. Subsequent PC startup tests are recorded
in PC_VALIDATION.md; no steering drive or headset/controller acceptance exists.

## Additional bounded installed-content coverage

Astra/xhigh subsequently indexed17 nested archives with8614 entries, inspected
14 additional metadata objects (eight models, six puppets) and exactly three
world dictionaries/type/reference collections. Loose Content filenames provided
no standalone candidate control geometry. This did not reopen the14 root archives.
No positive driver wheel was identified; this remains a bounded result, not proof
that unnamed or unopened geometry is absent.

InSamnity2's rideable SiriusCar_Ridable_Blue parameters identify aircraft puppet
and aircraft player controls, with a Seat referring to the SiriusCar_Blue model.
That model has Barrel01, Barrel02, EnterPoint, Engine and Seat child references,
an aircraft joint template and a seat-local transform. This establishes a concrete
seat/model chain for an aircraft-class rideable car; it does not identify a driver
wheel or authorize grabbing at the seat transform. Its City PartII world refers
to this puppet and model. The Car.bmf mesh was not reopened in this scope.

Cart wheel/hinge metadata remained unlinked to a driver seat. Renovation's portal
wheel had Open/Close animation and kinematic-body metadata, with no driver
association. FlyingSaucer Console and dropship Cockpit names did not establish a
seat-linked independent control. Scooter_Air was hovercraft parameters and
RollingBall_Player was rolling-ball parameters.

The three collections were InSamnity2 Sirius City PartII, Renovation Sirius City
PartI and Renovation Kronor Compound, inspected at dictionary/type/reference level
without a world entity census. Remaining unopened nested entries include2178
models,588 puppets,46 mechanisms,127 controllers,160 BMFs,44 skeletons and33 worlds;
counts include duplicate logical paths. Runtime resource precedence, the actual
seated instance and baked unnamed control geometry remain unresolved. A positively
identified control mesh/bone and its physical pivot/axis relative to the seat are
still required. No steering geometry or offset has been guessed or installed.


## Completed nested metadata follow-up — 2026-10-08

The subsequent collector searched all2,997 selected nested metadata entries:
2,186 models,594 puppets,46 mechanisms,127 controllers and44 skeletons. It read
177,186,181 bytes through CRC-checked archive reads and retained40,172 names and
167,017 type declaration headers. This supersedes the earlier unopened *metadata*
counts; it does not establish geometry or exhaustive serialized type coverage.
The collector excludes pointer/template and external type declarations and does
not resolve numeric object/member transforms. No driver control was positively
identified. Numeric geometry in160 BMFs and36 worlds remains uninspected; three
of those worlds previously received only dictionary/type/reference inspection.
Counts include duplicate paths;12 noncontributing nested archives were not
independently manifested in the collected JSON.

Astra's bounded review identified one concrete next content gate: the rideable
Sirius car model's referenced `Sirius_Car_05/Meshes/Car.bmf`. A dedicated read-only
investigation now examines that mesh and its immediate model/seat chain. Runtime
resource precedence, a visible driver's control and its pivot/axis/rim remain
unproved. Physical steering remains unimplemented until positive geometry is
established. Vehicle gameplay testing belongs to the user.
