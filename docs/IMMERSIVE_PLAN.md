# Immersive feature extension plan (in progress)

The user requires multiplayer VR replication and a UI comfort/size pass. Teleport is explicitly excluded. The earlier single-player development build remains the behavioral starting point. No game/headset execution is authorized; source integration, static ABI investigation, cross-compilation and meaningful offline checks remain in scope. Do not advertise tested equivalence.

## Preserve vs replace

Preserve native rendering, physics, player commands, network session/RPC transport, inventory and weapon state machines. Add the narrow tracking, presentation and protocol adapters needed by the requirements. A second renderer, custom UDP server, alternate damage simulation or rewritten menus would duplicate working architecture and is rejected. Native network schema extension is not proven safe; use an authenticated/versioned carrier within original transport only after sender/routing/capability semantics are established.

## Current facts and remaining work

- Original FP weapon assets include authored hand/arm components; the existing native model transform moves them with the gun. See WEAPON_HANDS.md. Independently articulated fingers/full-body IK are not established; grip/model calibration should preserve assets/animations.
- HUD/menu quads now use level LOCAL-space comfort anchors: 35° entry, 8° residual stop, bounded yaw/position following, lifecycle/reset guards and explicit viewing-distance guards. A shared binocular-FOV-aware layout reserves both frozen weapon-wheel panels. Rectangular HUD textures expose separate health/left/right rows. Astra reviewed the design; see UI_REVIEW_DISPOSITION.md. Comfort and grip settings are in config/SS2VR.ini. Headset readability remains unmeasured.
- Native collision laser aiming is implemented through the audited native ray-init phase and root stereo draw context. Both eye samples freeze together; invalid, stale, wheel and equip hands are suppressed. See LASER_REVIEW_DISPOSITION.md for Astra findings and dispositions. Runtime visuals/availability remain unverified.
- Native menu keyboard navigation is already foreground-only and release-gated. Ray-to-panel pointing now dispatches through the audited native menu poll/cursor/button functions. Pointer coordinates, trigger level and exact displayed frame correlate in ABI6; keyboard navigation and the original menu hierarchy remain intact.
- Native RPC sender ownership, target mapping and capability routing are audited and implemented. Ordered snapshots freeze at the verified full native simulation interval, with consumption ACKs; one current-capability credit and immutable per-hand tap policy are integrated and checked offline. Local input, authority aiming and remote rendering have separate native ownership gates. Unknown routing is not authorization to invent identifiers or expose stock peers to arbitrary VM RPCs.
- Physical roomscale movement/collision requires an actual native movement-request/result seam and correct origin consumption; native physics must remain authoritative. Do not use direct position writes or claim collision from head clamping.
- An opt-in sampled head-lean fade now reuses the audited native query phase, exact request/body/head matching and one immutable stereo value. Astra required default-disabled behavior because first-person/client query availability and moving-collider freshness remain unknown. See HEAD_COMFORT_REVIEW_DISPOSITION.md. Full continuous head protection remains unfinished.
- Native firing feedback is hooked at DoTheFiring(float), preserving its OnFire return value (base RVA0x4cdc0, ret4). Native health decreases produce damage feedback; optional OpenXR vibration failures do not disable VR. Physical grip position, runtime aim direction and per-weapon alignment are shared by model/muzzle placement. Immersive sniper optics, vehicle controls and complete native overlays still require implementation through verified native seams.

## Completion bar

Wire real features through native producer/consumer paths. Record concrete implemented behavior and unknown ABI or feature gaps. Build both products, exercise portable state/math/protocol invariants, verify native exports/layouts/artifacts and package current source/products separately. Preserve old artifacts and existing installed files. Compilation or scaffolding alone does not establish runtime playability or multiplayer correctness.

## Current source checkpoint — 2026-10-07

Current source uses IPC9/wire6 and passes59 portable groups; x86 proxy/server and
x64 OpenXR host/official loader build. Native zoom and magnified per-hand scope
captures/ordered cap imagery are connected for their admitted renderer subset.
Default-off single-player roomscale uses native body queries, checked placement
and actual-anchor origin settlement. Default-off handheld/seated head-volume
queries and cached-frame receipts are connected, with native own-vehicle exclusions.
Optional immersive swimming retains head-directed joystick control by default.

Multiplayer body/origin settlement, physical melee consumption/observer delivery,
remaining vehicle/head coverage and the final full-project review remain open.
No game, Windows/Wine, OpenXR, headset or network session has been executed. New
packaging verifies matching compiled source/version/layout contracts; the old
0.2.11 archive is an unchanged historical checkpoint. See CLOUD_CONTINUATION.md
and IMPLEMENTATION_STATUS.md for current boundaries rather than historical counts.

## Historical offline checkpoints

The x86 game proxy/server module and x64 OpenXR host/loader compile. Seven offline check groups cover math/IPC, frame ownership, UI/recovery/grip/lasers, network codec/policy, partial hook rollback, native affine subtree retargeting, and installer fixtures. ABI6 retains Input224/Request352/Ui352 and appends a56-byte menu pointer; Shared is83,887,512 bytes. Static verification covers exported hooks and the menu/entity-manager/model-pass internal entries. No runtime was executed. The0.2.2 follow-up includes coherent crouching/head/hand tracking and matching wire4 products; older archives are retained checkpoints.

Native server receive-slot/avatar mapping, lifecycle invalidation and a dedicated plugin bootstrap are connected. Remote model presentation preserves native assets, animation, culling and deformation; each eligible hand is independent. Full feature equivalence remains unfinished and must not be inferred from these checks.

The0.2.4 follow-up adds default-disabled source for native remote head palette adaptation shared by CPU/GPU skinning, with full XYZ/quaternion delta and preserved native animation. Complete worker/model lifetime, normals/extra-pass appearance and anatomical IK remain unproven. Remote pair consistency now freezes age admission and protects publication against lifecycle/history changes. Astra reviewed architecture, source and targeted fixes. Nine offline groups pass, both architectures build, and static verification includes the optional palette entry. See HEAD_ANIMATION_REVIEW_DISPOSITION.md.

The early source0.2.6 follow-ups retained IPC6 and used wire5 for immutable per-shot zoom context. At that checkpoint the native zoom producer was zero and scope optics/native zoom were unfinished; the current native zoom adapter supersedes that limitation. Exact stock alternative-route suppression and17-slot getter admission were Astra-approved native fixes. A shared compile-only predicate entry and incremental all-mode pre-weapon local preparation received separate Astra source reviews. Eleven offline groups passed without runtime execution. The0.2.5 archive remains immutable, wire4 and predates these source changes.

## Swimming control choice — owner requirement, 2026-10-06

Immersive arm-stroke swimming must be optional. Head-directed joystick swimming
is the baseline and must remain usable without arm gestures; enabling immersive
input must not remove that fallback. Keep this input choice independent of the
native swimming/diving collision shapes, water physics, buoyancy and authority.
Default to the non-gesture mode unless the owner selects immersive swimming.

Source now connects this choice through the local native control producer; see
[SWIMMING_CONTROLS.md](SWIMMING_CONTROLS.md). `[Swimming] Immersive=0` defaults to
head-directed joystick input. Setting1 adds bounded forward arm-pull input only
while the joystick/jump vector is idle. Both modes retain the native three-axis
ClientAction RPC and movement consumer, with no direct body writes or new wire
fields. This has source/offline/static verification only; water transitions,
comfort and Windows/Proton runtime behavior still need the owner's later tests.

## Immersive steering-wheel control — owner requirement, 2026-10-07

For vehicles that have steering wheels, add tracked grab-and-turn steering with
either hand alone or both hands together. Support releasing one hand and passing
the wheel to the other without a steering discontinuity. Resolve the actual
vehicle wheel placement and steering axis rather than guessing a generic wheel
in front of every seat. Feed the existing native steering input and preserve
native vehicle physics, speed, constraints, networking and mount ownership.
This requirement is not implemented yet. Controller grab routing, wheel geometry,
vehicle-family admission and interruption/release behavior remain to be connected.
