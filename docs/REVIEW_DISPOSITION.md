# Astra review disposition

Reviewer: gpt-6-astra, xhigh, verified from the effective Codex CLI startup header. Ephemeral read-only CLI run; operational logs not committed. First tool reviewer returned no verdict because metadata was inaccessible; that response was not approval.

| Recommendation | Disposition |
|---|---|
| Reject Execute command-graph replay | Adopted. Native CPuppetEntity::Render3D prepares, collects, executes and frees commands for each eye. No copied graph or retained pointers. |
| Audit preparation and capture boundary | Adopted. Installed Render3D calls native projection, camera placement, renPrepareRender, local weapon injection and renFinishRender. Weapon rendering lives in native commands; no manual duplicate weapon render. Main local puppet checked by native IsLocal and single-player check. Nested views retain native matrices. |
| Restore engine bookkeeping, not only D3D state | Adapted. Restore target/depth/viewport/scissor, retain native shader/texture state to avoid cache mismatch, and finish with original desktop Render3D, rebuilding engine caches through original path. No renderer-global matrix edits. Eye projection override only in local main projection and identified weapon-render scope. |
| Equal weapons may couple fire inputs | Adopted. Override IsWeaponFiringPressed for actual current local weapon handles, per physical hand. Keep native state machines and ammo/cadence. |
| Base + Sniper pose/render hooks | Adopted. Both overrides resolved by decorated exports. Pose hook uses current ownership, not a global fire flag, so continuous-beam queries are covered by the same ownership check. Vehicle weapons are not treated as player hand weapons. |
| Verify hand-specific selection seam | Adopted. SetCurrentWeapon(WeaponIndex, PlayerHand, int) validates inventory and branches by hand in installed body. Selection only from native simulation hook. Left enum 0/right enum 1 verified from named CLeftAmmoHudElement/CRightAmmoHudElement accessors. |
| Single shared calibration transform | Adopted, with the later body-anchor refinement below. shared yaw/origin generation applies to head/eyes/hands/muzzle/model. Per-weapon model offset and shot-axis calibration derive from original native transforms, not an invented barrel axis. |
| x64 host with CPU transport | Adopted. One host, no parallel runtime paths. Explicit performance/Proton limits. |
| CPU readback is not latency-bounded | Adopted. Only IPC waits bounded; document synchronous readback stall risk. Match dimensions/formats and row pitch, resolve MSAA. |
| Publish input independently; precise frame ownership | Adopted. Separate tracking/input publication and rendering slots with Empty/Requested/Rendering/Ready ownership; cancellation never reclaims Rendering slot. Session/reference generations and frame identity checked; both eyes share frozen game origin/snap-turn and anchor. |
| No identical-image stereo fallback | Adopted. Without an eligible complete pair, zero projection layers are submitted; the final transport amendment permits conditional reuse of the previous real complete pair. Menu-only screen quad identified separately. |
| Unsupported renderer paths fail closed | Partially established. Unknown fingerprints and missing hooks retain desktop; bounds/formats/MRT/final-target checks reject unsupported stereo. Complete child-command/postprocess classification remains unverified (RENDER_AUDIT.md). Cinematics/vehicles and renderer effects remain unverified limitations, not advertised complete coverage. |

## Completion audit refinements

| Recommendation / verified defect | Disposition |
|---|---|
| Absolute cached model position contaminates muzzle after body movement | Fixed: camera-local calibration and retargetShot; translation/yaw regression. |
| Native dual-toggle/equip may change ownership after sampling | Fixed: refresh both handles after toggle and original OnStep; no double control update. |
| Camera animation leaks into tracking anchor | Astra follow-up adopted with adaptation: base GetViewOrigin purpose0 plus verified native height interpolation; original animated camera retained for authored offsets. Alive/first-person and unset-sentinel guards added. See ANCHOR_REVIEW.md. |
| Synthetic trigger/grip release during recenter | Fixed: preserve physical values and suppress controls while the chord is held; release-gated firing and no incidental wheel reopening. |
| Canvas copies bypass RT binding substitution | Fixed: route original-surface aliases for StretchRect/readback/fill, preserve exact native 32-bit color/depth formats. No native surface/texture caches replaced. |
| Individual eye translation clamp changes IPD | Fixed: one shared head-center correction preserves eye separation. |
| Native root occlusion history aliases both eyes | Fixed at the fingerprinted main-root Prepare call; distinct stable identifiers feed the original CRC/query manager, with nested callsites unchanged. |
| Strict 35ms cancellation starves slower complete captures | Astra transport review adopted: asynchronous existing-slot polling, one pending transaction including cancelled Rendering/Ready retries, provisional 150ms enqueue-age cutoff. |
| Async zero-layer gaps between complete updates | Astra amendment adopted: reuse only a complete released stereo pair with its original LOCAL poses/FOV while eligible; no relabeling or mono substitute. |
| One eye may be released before the other times out | Fixed: both waits before either write/release; cached pair invalid during replacement, reacquired cached image cannot be presented. |
| Late native/UI publishers can revive stale epoch | Fixed: process monotonic epoch, snapshot-lock serialization, early monotonic Interlocked publication, stale commit rejection and conjunctive UI/Request/header checks. Reserved terminal epoch prevents wrap/regression. |

No new rendering engine, simulation/inventory policy, worker thread, GPU-sharing service or image queue was introduced. Runtime quality/performance remains outside verification.

| Final Terra integration finding | Disposition |
|---|---|
| Ready check/commit race | Snapshot lock now held through the actual Ready transition; invalidation ordered before it rejects completion, afterward host epoch validation rejects delivery. |
| Abandoned writable XR images retained | Explicit no-write release outside IPC lock, cache invalidation before release, retry only existing timed-out acquisitions. |
| Follow-up STOPPING retains acquired images | Adopted: retire all existing stereo/UI acquisitions before EndSession with bounded retries, clear presentation state, fail closed on timeout. |
| Destroying an unwaited chain alleged to require release | Stronger prohibition not established by pinned spec. Attempt wait/release after GPU drain; destroy still-timed-out unwritten chains rather than illegally releasing. Retain objects on GPU drain failure. |
