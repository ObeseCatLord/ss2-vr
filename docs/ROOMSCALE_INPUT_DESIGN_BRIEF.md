# Native-input roomscale adapter — decision brief

Solo-user native OpenXR port; full immersive goal remains active. User excludes
teleport and actual runtime testing, requires multiplayer and Astra for reviews
and investigations. No game/Windows/Wine/XR/server/network execution. Reuse
native collision, correction, simulation and RPC. No new physics solver.

## Verified environment and behavioral reference

Astra/xhigh read-only investigation establishes the following pinned native
facts; recorded module hashes are RESEARCH.md. Source ownership is current
Snapshot in engine.cpp151; initialization/recenter676; controls2805. Shared
tracking maps anchor * yaw(turn) * inverse(origin) * tracked in math.hpp83;
current horizontal tracking clamp0.75 is not body collision. nativeTrackingAnchor
(engine.cpp276) is a corrected view anchor, not isolated physics displacement.
Current head-obstruction query only fades (2053).

| Native seam | Verified evidence |
|---|---|
| Main simulation ownership | engine.cpp883/921 checks current simulation/world/manager, main thread, outermost interval and includes local player; full CSimulation::Step returns after physics |
| Ordinary local player input | Sam2GameF35CD/returnF35D3 calls brain virtual384 ProcessPlayerControls; thiscall void(byte,Vec3,Vec3), vectors by value, ret1C |
| Native action serialization | EE0F0 calls ClientActionEE260 through virtual388 atEE245; native56-byte argument block, ret38; remote branch uses original RPC |
| Movement basis | virtual50C ->legged60D50/base8D930; virtual518 ->GetOperatorMoveDir60D60 hiddenVec3 return+two by-valueVec3, ret1C |
| First-person desired motion | EnforcePuppetMoveLook8E370 calls SetDesiredTempoAndLookDir80690 at8E46B, state2; state3 drive/steer is not the ordinary walking branch |
| Native physics | legged699A0 uses SetFreedomAxes at6B7E1 and scalar SetVelocity6B7ED; Engine125C80/125D70 |
| Physics result | Engine Step1B70E0 manager1B739B then physics1B7406; mechanism GetAbsPlacement130B20 hiddenQuatVect ret4, missing-root fallback |

Reference behavior: real horizontal HMD travel drives the existing native body
through original collision/sliding; all hands/head use one coherent tracking
origin. Stick, platforms, impulses, corrections and server replication remain
native. No headset-as-mouse or fixed camera replacement.

## Architecture fork

Investigation's candidate rebase by accepted roomscale displacement cannot yet
attribute a contribution to roomscale: total motion also includes stick,
platforms, forces and native correction. Projecting total movement onto the HMD
request invents an allocation policy. A separate solver/direct placement/new ACK
would replace adjacent working native behavior without demonstrated need.

Proposed minimal alternative: explicitly choose **submitted-request settlement**.
For one raw horizontal HMD delta r, convert r to native walking intent and combine
it at the exact ordinary ProcessPlayerControls movement argument, before native
ClientAction serialization. Advance shared tracking origin by the submitted raw
horizontal delta exactly once under the same accepted input/lifecycle identity.
Do not claim this is accepted-displacement measurement. Native physics decides
actual combined displacement a. The shared camera subsequently follows native
anchor displacement a because the same r has been removed from local tracking;
collision, normalization and correction therefore constrain physical horizontal
motion through the native body rather than requiring decomposition of a.

Reasoning, not proven implementation: without collision/saturation, a=r+s yields
normal physical travel plus stick s; if a differs due collision or forces, the
world camera follows a while physical yaw/vertical pose and hand-relative offsets
remain tracked. At a wall, the native body blocks the attempted physical travel.
Mixed-input saturation compresses submitted physical travel under native policy.
This has a deliberate comfort/behavior tradeoff; it must not silently be labeled
accepted-roomscale settlement. Repeated/recentered/stale samples cannot rebase.

Reusable state: original Snapshot origin/turn/input/player generations and
simulation interval; existing local input/RPC/physics; shared rig/world-pose math.
Keep only previous raw tracking reference/request identity alongside existing
Snapshot if actual consumer requires it. No parallel movement ACK or correction
state machine, no independent tracking bank/physics actor.

## Open decisions and missing proofs

Current lean: submitted-request settlement is the smallest plausible adapter,
but do not implement it before review verifies/corrects the derivation and exact
producer/thread/action timing. [Unverified] whether equivalent to intended native
VR comfort under collision and correction. [Unknown] complete metre-to-intent
conversion, nominal speed/step/correction factors, and source-to-simulation action
identity. View anchor cannot substitute for physical placement results. Remote
movement must traverse original native action serialization once; avoid injecting
roomscale again on the server for input already combined by the client.

Review may reject the policy, simplify it or identify an existing narrower native
relative-motion adapter. It must not prescribe accepted contribution from total
displacement without proof. Human preference is only needed for an unavoidable
actual product behavior tradeoff; correctness/ABI questions belong to us.

Smallest offline production proof: one local tracking sample -> exact native
argument seam -> original serialized movement -> shared origin/head/hands/network
pose bookkeeping. Check duplicates, delayed simulation, mixed sticks, correction,
turn/recenter, loss/lifecycle changes, blocked/saturated requests and physical
vertical pose. Linux math/static ABI evidence is possible; native collision/visual
runtime equivalence remains explicitly untested under user's scope.

Review contract: one Astra/xhigh verify-then-critique, read-only/no delegation,
<=1200words prioritized GO/NO-GO on the architecture policy, exact prerequisites
for a bounded implementation, unnecessary state/deletion opportunities. Do not
re-review scope shaders, transport, offhand weapon FSM or roomscale-independent UI.
