# Mounted VR adapter: incremental design brief

Goal: restore six-DOF XR view and controller input while natively seated, retaining the vehicle's controls, camera offsets, physics, operator ownership, turret geometry and native RPC. No runtime/game/network testing is authorized. No vehicle controller rewrite, physics setter, new transport or cached vehicle pointer is proposed. This is an unreviewed main-thread proposal; source is not changed by this brief.

| Fact | Evidence/status |
|---|---|
| Native module | Fingerprinted Sam2Game.dll in installed-build.json |
| Existing VR gate | engine.cpp trackingEligible excludes native thirdPerson; mounted players are excluded |
| Native third-person | 941B0..941F1 returns true for a resolved rider544 after alive/558 check |
| Stable native mount | Rider544 current ride handle,548 state3,54C seat IDENT; fresh handle resolution |
| Native seated eye offset | GetRelativeViewOriginForPose67C20..67D2E chooses data98 for state3, retaining native pose/scale |
| Native base anchor | GetViewOrigin8EFF0..8F6F9 purpose0 composes relative offset with v54 absolute body pose; then substitutes v54C operator-look quaternion |
| Body pose | Player v54=GetAbsPlacement8E8E0..8E964; mechanism114 getter or model120 pose2C, no attachment query |
| Existing seat simulation | LerpRiderOntoSeat62E00..639E6 gets vehicle seat matrix, separately computes native operator look, writes native rider mechanism pose via SetAbsPlacement639DA |
| Unsafe alternative | Model attachment query D8FA0..D908F clears/restores shared model-query context and calls cleanup; never add this query during rendering |
| Native input seam | Input consumer F2DF0..F3AE6 calls CPlayerBrain ClampLookDirEul at F3250 (return F3255), reference Vector3 argument |
| Clamp ABI | ED2E0..ED313, thiscall Vec3&, ret4; dispatches current puppet v5A0, retaining ride v59C/camera pitch limits |
| Euler conversion | Core mthQuaternionToEuler17050, native hidden output + const quaternion reference |
| Vehicle movement | Native mode2D8&4 selects EnforceMoveLook→SetDrive802C0; live bit checked, class-wide initialization not inferred |
| Native turret fire | GetShootDirection155CE0 retains mechanism Gun/GunUD; handheld muzzle adapter is not the vehicle firing path |

## Minimal adapter compared with alternatives

Preferred: reuse the native rider body pose already computed by the seat simulator. Preserve baseViewOrigin's native eye position/view-height interpolation, replacing only its operator-look orientation with freshly read native rider absolute orientation while seated. Head/eyes/hands continue through the same shared XR rig transforms. This adds a pose getter and a narrow mounted branch, not a parallel anchor cache or model query. [Verified: bounded native bodies] Body setter receives a quaternion independent of the separately computed operator look; SetLookDir gets the distinct look quaternion and zero bank at63960..63985. [Unknown] Visual comfort/native seat animation under headset; cannot be tested in this task.

Alternative: observe every native seat getter and retain a seat pose bank. Rejected as unnecessary state/lifetime complexity while the native rider body pose remains reusable. Calling the getter anew is rejected because it runs destructive model queries. Replacing native movement/vehicle simulation is rejected: no demonstrated incompatibility.

## Intended end-to-end wiring and open decisions

1. Resolve the current local rider and seat afresh; support only seated state3 with a live ride and known native vehicle/control seams. Treat mount/dismount/seat changes as the existing snapshot/rig generation boundary. No raw-pointer cache. Decide whether mounted native558 should exclude VR or be ignored only for the supported mounted view; it also reflects native camera mode.
2. Split XR view eligibility from handheld weapon eligibility. Permit the mounted camera, but suppress handheld wheels/equip/dual toggles/retargeting while seated. Keep native vehicle rendering and firing. Head/hands remain replicated as tracking; mounted player-weapon primary/zoom intent must be zero.
3. Feed right aim orientation through the exact F3255-origin brain clamp hook before calling the original clamp. Convert the existing world rig orientation with native QuaternionToEuler; retain the original clamp and subsequent native ClientAction/SetDrive/RPC. Fallback when aim/tracking/seat identity is invalid. Left axes feed native steer/throttle directly, without the FPS snap-yaw rotation. Use/jump/sprint remain existing native commands. No vehicle weapon placement patch.
4. Preserve native collection policy. A mounted eye camera may otherwise be inside the native third-person rider body. Decide the narrow native first-person collection seam/caller that can exclude the local rider mesh while retaining vehicle geometry; do not write558 or globally change third-person behavior. Desktop stays native.
5. Verification target: cross-build; offline rig retains independent eye offsets and XYZ/pitch/yaw/roll while seat rotates; mount identity changes reject prior pairs/input; native hook ABI/static exact-origin checks. Source review must trace actual camera/input/handheld suppression paths. No runtime playability claim.

Open gates before implementation: exact first-person collection/body treatment; body-pose native getter callback/lifetime guard; mounted input sampling order versus the existing game snapshot; right controller look relative to native bank/pitch offset; native operator fire command delivery. Obtain an Astra design review after these narrow facts are verified. Do not fill unknowns with a custom controller or infer full vehicle parity from a camera-only slice.

## Resolved minimal plan for Astra review

[verified: native bodies] `CPuppet Render3D`94380 queries native third-person at943B0 (return943B6), and passes either null or its own entity to `renSetAvatar`943BF. A return-zero only at this exact root call while drawing the admitted mounted local XR eye selects native first-person avatar collection. Every other third-person query remains native, including player gun injectionFDA05 (returnFDA0B), which naturally remains disabled while mounted. No558 writes, extra injection hook, body pose cache, or explicit body-mesh hide are needed. The existing desktop rebuild has no active eye and remains native. Base CPuppet injection1C3240 is a shared empty stub, not a body-rendering implementation; no inference from its alias export is used.

[verified: native body getter] Engine `CMechanism GetAbsPlacement`130B20..130B6C is a root-body handle resolve and seven-float pose copy with native fallback, no model query/callback/render cleanup. Caller CPuppet8E8E0 resolves that mechanism or copies native model120 pose2C. The borrowed native player getter is reused; compare live ride/seat/handle before and after, reject native unset/nonfinite pose. Existing base eye position retains seated offset and height smoothing; only anchor orientation is replaced. Capturing a native seat matrix bank was considered and is unnecessary.

[verified: control dispatch] F35CD→brain384 resolvesEE0F0, forwarding the F3250-clamped stack-4C Euler look, stack-38 native movement and native fire byte. Hook only the F3255-origin native brain clamp, before original clamp, for the exact current local seated rider/brain. Convert the existing world right-aim quaternion through native QuaternionToEuler; set bank zero as the existing native control vector does, then preserve native camera/ride pitch clamp and drive pitch-offset/wrap. Do not patch ProcessPlayerControls, SetDrive, turret muzzle or physics. Unsupported ride identity/mode/invalid tracking falls back to native input.

Proposed exact write scope: engine.cpp (narrow view/control/handheld branches and two exported hooks), one portable common mounted-rig identity/anchor helper plus meaningful core checks, verify_artifacts symbol/count recognition if needed, and status/review documents. IPC7/wire6, host, bridge, native physics and transport remain reusable and unchanged.

The existing Snapshot will carry the current ride handle/state/seat identity. Changes use its existing generation/origin reset; pre-boundary stereo cannot publish. Camera tracking eligibility admits known seated vehicle/control instances while handheld eligibility remains excluded. Wheels/equip/dual toggles/calibration/handheld muzzle/laser adaptation and player-weapon wire intent are suppressed while seated. Keep native hand/head tracking relative to the same seated rig for replication; actual native vehicle controls/firing continue through the game's normal control RPC, not the VR handheld-weapon protocol.

Primary command split is necessary: the new IPC7/wire6 player-weapon admission must not suppress native vehicle control fire merely because seated player-weapon intent is zero. Use the same local raw TriggerGate/activity/stream neutral policy for native vehicle fire commands and captured ride/input provenance at command use; retain native operator ownership/cadence/ammunition. Native player weapon fireQuery returns zero while seated; actual vehicle-owned firing queries remain native. This is a mode branch at the demonstrated boundary, not another multiplayer weapon protocol or vehicle state machine. Mounted steer/throttle axes bypass only the FPS snap-yaw rotation; native clamps/RPC consume them unchanged.

Open review decisions: whether the root avatar gate sufficiently preserves native first-person collection without any mesh rewrite; whether native rider body orientation is the correct seat anchor at steady state and transitions; whether the mounted command split/lifetime guards cover input polling before snapshot update. Appearance and actual vehicle control behavior remain unverified by user instruction. Native collision lasers for vehicle-mounted guns are a separate extension of the existing query phase, not claimed by this adapter. No human preference question is needed; choose the smallest evidence-supported adapter or name the exact bounded missing proof. Review may seek deletions/simplification, but must not replace native systems or silently count a camera-only slice as complete vehicle parity.
