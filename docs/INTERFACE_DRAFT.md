# Historical adapter contract draft

Superseded by FINAL_PLAN.md and IMPLEMENTATION_STATUS.md after Astra review.

The game remains the only owner of inventory, ammunition, fire cadence, projectiles and weapon-slot validity. The mod owns tracking transforms, controller bindings and transient wheel UI.

## Runtime boundary
An x64 host process creates an OpenXR D3D11 session, gets adapter LUID from xrGetD3D11GraphicsRequirementsKHR, creates device on that adapter, and enumerates RGBA/BGRA formats and stereo view sizes. CPU transport is a compatibility path, not a performance claim. The x86 plugin reads predicted poses and independent action values; no OpenVR code path.

Per-launch named shared memory, mutex and request/response events. Every payload uses fixed-width integers and floats, not handles/pointers. Header includes magic, ABI version and mapped size. Request frame identity and input generation separate; render pose snapshot stays immutable during both eye renders. Request contains two XrView-equivalent poses/FOV, head + left/right aim poses, buttons/axes/focus, prediction time and local timestamp. Response echoes exact frame identity and dimensions. Host submits only same-request stereo pairs; a timeout produces zero layers. No infinite waits in game render/simulation.

Pixels BGRA8, bounded dimensions, tightly packed rows independent of D3D9 pitch. Two complete eyes or no response. Reset/loss releases resources and invalidates output. Render source and target/depth/viewport states restored. Game retains desktop render on unavailable runtime or unknown game build.

## Input boundary
Simulation hook observes local player, copies current focused tracking input within stale timeout, updates presentation wheels and routes requests through native per-hand weapon selection. Independent trigger holds are released for tracking loss, focus loss, menu/wheel opening and stale host. Trigger suppression requires a physical release before re-arming after interruption. Wheels select on release only when hover is outside deadzone and inventory still validates selection; neutral release cancels. Freeze wheel presentation origin on open. Stick locomotion and snap turning feed the existing player action object, not player position writes. Recenter resets tracked origin and body transform; it does not teleport game physics.

Tracking transform must be shared by head, eyes, controller muzzle and model placements; one owner, one scale. Convert relative tracking pose to engine camera/body anchor; physical translation remains bounded to limit uncompensated physical motion. Native collision/occlusion needs explicit evidence before claiming roomscale body collision. Triggers never use headset reticle aim.

## Offline acceptance
Build x86 proxy/hooks and x64 host/loader. Inspect PE machine types, exported symbols and dependency sets. Offline tests may check math, wheel cancellation and suppression, bounds/frame identity. These do not establish runtime or headset behavior. No game launch in this task.
