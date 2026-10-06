# Swimming control choice

## User-facing behavior implemented in source

`config/SS2VR.ini` contains `[Swimming] Immersive=0` by default. In water,
head orientation steers the ordinary joystick movement vector. Hand gestures
are unnecessary. Head roll does not turn strafing into vertical movement.
The game retains its native surface-swimming pitch rule, speed, water physics,
buoyancy, collision, control messages and authority.

Set `Immersive=1` and restart the game/host to add forward arm-pull propulsion.
With the joystick idle, pulling either tracked controller backward relative to
the head adds bounded forward input. Both controller grip poses must be tracked.
Recovery strokes do not propel backward; use the joystick to reverse. Any
nonzero joystick/jump input takes priority, so head-and-joystick swimming remains
available while immersive mode is enabled. Menus, weapon wheels, tracking loss,
recenter, identity/reference changes and water-mode transitions reset strokes.
The optional heuristic uses a0.2m/s pull threshold and reaches full contribution
at1.2m/s per hand; these are uncalibrated defaults, not measured comfort values.

The same setting and x86 adapter apply on Windows and under Proton. This is not
a claim of verified OpenXR routing or headset behavior on either platform.
No game, Windows executable, Wine, headset or network session was run.

## Native implementation boundary

`waterPlayerControls` wraps CPlayerBrainEntity::ProcessPlayerControls (EE0F0)
only for the pinned local main-thread input call returning to F35D3. Other
callers pass through without touching stroke history. It retains the native
fire byte and look vector, changes only the movement vector and invokes the
original function exactly once. Dedicated servers do not install this hook.

The native player vtable29E878, +518 GetOperatorMoveDir60D60 and +57C
MovingIn3DArea61030 are pinned. Native enum records establish pose3 PP_DIVE and
pose4 PP_SWIM. Water input requires the native movement flag4 and excludes
flight flag2; land, mounted players and unknown/derived layouts pass through.
Player/brain handles, current rider, tracked input and water mode are checked
before and after preparation. No pose is written to a native entity.

The existing sampled turn is removed once from the native movement input;
worldHeadTracking already includes it. Native quaternion-to-Euler and four
native GetOperatorMoveDir calls establish the three current input-basis columns
and desired head-directed vector. The native function supplies its own surface
pitch clamp. A finite, near-orthonormal, positive-determinant inverse maps that
desired direction back into the native input frame. Failure preserves the
original input. No custom swimming physics or alternate transport is added.

Native ProcessPlayerControls forwards all three movement components through
ClientAction virtual388. ClientAction EE260 serializes them and stores them in
brain+144. ProcessOperatorInput8D930 consumes that vector with brain+138 look
through the native GetOperatorMoveDir virtual518. Input therefore follows the
same native local/remote route. Network delivery and actual locomotion remain
runtime-unverified; static copying is not a multiplayer gameplay test.

## Verification and limits

`tools/verify_swimming_input_abi.py --game <owned-input-root>` checks the exact
owned Sam2Game fingerprint, exports, vtables, pose-name records, native axis
quantization, control/RPC stores and native direction consumer. Optional
`--object <compiled-engine.cpp.obj>` checks the actual GNU x86 production
wrapper's incoming anchor, forwarded argument slots, callee cleanup, single
native call/tail paths and four direction-query calls. Both normal Python and
Python -O keep the checks enabled. Proprietary files/disassembly stay private.

Portable tests cover mode admission, arbitrary three-axis rotation round trips,
degenerate/reflected/sheared/nonfinite bases, exact joystick priority, default
stroke disablement, arm pull/recovery, translated tracking rigs, duplicates,
replays, reference/owner/rig/water changes, stale/future input, tracking loss,
invalid orientation, wheels and discontinuities. Native body movement and its
separate ownership/settlement gates remain unfinished elsewhere in the project.

This checkpoint passes42 portable groups in Debug and Release, Linux ASan/UBSan
for the production swimming helpers, x86 proxy/server and x64 host builds,
normal/-O native/compiled ABI checks, and the full artifact/export/IPC check.
A deliberately damaged compiled wrapper with ret24 instead of ret28 is rejected
by the ABI verifier. These checks do not establish actual water behavior.
