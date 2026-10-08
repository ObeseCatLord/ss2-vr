# Comprehensive user VR acceptance procedure

Preparation document: the full implementation is still in progress. This is not
a claim that every feature below is enabled or ready. The integration owner must
supply an exact build/package identity and list remaining disabled features before
asking for a final pass. Individual weapon-firing tests belong to the user in VR;
Codex must not continue those probes under the current instruction.

Use a separate test profile/save and a private multiplayer session. Keep original
saves and settings backed up. Record game version, mod package/source fingerprint,
platform, runtime version, headset and controller models. Use the same scene/save
and settings when comparing failures. Save captures and logs locally.

## Platform and device coverage

Record each available cell separately; simulation does not pass a hardware cell.

| Platform | SteamVR + Steam Frame | Envision/Monado + Bigscreen Beyond |
| --- | --- | --- |
| Native Windows | Actual headset/controller pass required | Record actual runtime availability; do not assume this Linux setup exists on Windows |
| Linux/Proton | Actual headset/controller pass required | Actual headset and selected controllers required |

Confirm the actual per-hand interaction profile in the host log. Frame has a
documented Touch fallback, but an Index profile is not proof of Frame bindings.
Beyond's controller model determines its bindings. Use the current package's
[control table](../README.md#controls) and [profile bindings](HOST_STATUS.md#controller-bindings).

## World rendering and tracking

1. Launch the supplied direct-gameplay test scene. World geometry must surround
   you at the headset's field of view. A rectangular menu panel is appropriate
   for menus; two small flat gameplay panels fail this test.
2. Look at a nearby solid object with a distant object behind it. Alternate
   closing each eye: depth and occlusion must agree, with binocular disparity.
   Inspect terrain, vegetation and the waterfall separately in both eyes.
3. Keep the controllers still. Move your head left/right, up/down and forward/back
   without turning; nearby and distant objects must show different parallax.
   Then test yaw, pitch and roll independently. The world must remain stable.
4. Turn with the right stick, move with the left stick, recenter, pause/resume,
   change level and load a test save. Check for stale frames, double movement,
   eye disagreement, wrong scale or an origin jump after each transition.
5. Approach a wall with physical head movement and ordinary joystick movement.
   Test the enabled body/head collision behavior, corners, stairs and low roofs.
   Do not count an intentionally disabled collision feature as a pass.

## Hands, aiming and native dual wield — user-operated firing

1. Check each weapon's physical grip, size and orientation against its controller.
   Move one hand while the other is stationary. Weapon identity, aiming and laser
   must remain independent. Verify whether the build actually includes hands;
   do not infer a hand model from a floating weapon or controller pointer.
2. Trace lasers across nearby walls and distant terrain. They should originate
   at the native muzzle and terminate on the closest eligible surface. They do
   not promise the final random projectile spread or penetration path.
3. For every combination the stock game actually permits, record left/right
   weapon identities and native eligibility. Unsupported combinations remain
   unsupported. A two-hand grip on one weapon is a separate test.
4. Fire left alone, right alone, alternating, and both simultaneously. Release
   each independently; no firing may persist after release. Include tap, hold
   and release for native charged weapons. Confirm actual impacts follow that
   hand's aim, not head gaze or the other hand.
5. Exercise native ammo, cooldown and reload only where applicable. Include the
   initial Zap's charge/release behavior and reload cycle; infinite ammo does
   not mean absence of reload. Compare questionable behavior against stock.
6. Switch either hand, equip/unequip, run out of applicable ammo, use an item,
   pause, lose/regain tracking, die and respawn while pressing and after releasing.
   A held control must not become a new press merely because tracking returns.
7. Test each eligible scoped hand: enter/release zoom, turn the other hand,
   fire, switch that hand and interrupt tracking. Magnified image, muzzle and
   zoom ownership must belong to the same weapon; surrounding world FOV stays normal.
8. Test physical melee only when the integration owner marks it enabled. Check
   intended contacts, misses, manual firing coexistence, held/released motion,
   switching, carried objects and lifecycle transitions. Record any unintended throw.

## Wheels, menus and comfort

1. Open each hand's wheel, select an owned weapon and release to equip. Center
   the stick before release to cancel. Open both wheels; labels, ammo and hover
   feedback must be readable and independently actionable.
2. Lose squeeze availability/tracking while a wheel is open, then restore it
   while still squeezing. It must cancel and require a real release/rearm.
3. Navigate the native menu using the pointer and buttons. The visible cursor
   must match the hit location; pause, back and confirmation must work without
   a desktop mouse. Holding a trigger across menu entry must not confirm an item.
4. Read HUD, messages, scores, player names, death screens and fades. At default
   settings, turn less than 35 degrees: the panel should stay anchored. Turn
   farther: it should follow smoothly, stop near an 8-degree residual and remain
   level. Check recenter, sitting/standing and physical position changes.
5. Adjust panel size/distance within supported settings. Record clipped text,
   unreadable labels, intrusive placement or uncomfortable following separately.

## Swimming, seats and vehicles

Test walking, sprint/jump/use and head-directed swimming. Test optional arm pulls
only if enabled. Enter/exit each supported vehicle and seat, aim, use native
throttle/steering fallback and check head/hand anchoring through seat changes.
For positively identified driver wheels with enabled physical steering, test left,
right and both hands, smooth transfer, release, tracking loss and joystick
fallback. Record vehicle/seat identity. Do not treat a road-wheel joint or a
weapon-wheel action as physical driver steering. Teleport is outside scope.

## Multiplayer with other mod users

Use matching client/server products and a private session. Test listen-host local,
remote client and dedicated-server roles. Observe both directions with another
mod user: head/hands/weapons, body motion, supported dual wield, scopes, melee,
vehicle seats and native hit results. Include physical head displacement mixed
with joystick movement, walls/support movement, packet delay where available,
death/respawn, reconnect and level changes. Remote body correction must not cause
detached hands or reset local physical travel. Pose packets or counters alone
do not pass this test: another player must see coherent native gameplay.

## Reporting and recovery

For every failure record build, scene/save, platform/runtime/controllers, role,
left/right eye, exact action sequence and expected/observed result. Keep the
original logs and a short capture locally; note whether stock reproduces it.
After normal quit, verify owned game/host processes exit and your original
profile/save/settings remain unchanged. Report startup, tracking-loss and runtime
restart behavior separately from successful gameplay. Mark each row passed,
failed, unavailable or not yet enabled; unavailable is not passed.
