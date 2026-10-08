# Final implementation plan

Current authority: PC development and actual private/runtime testing are authorized.
Older no-runtime/no-launch statements below record the original plan and are
superseded by AGENTS.md and CLOUD_CONTINUATION.md. Full native dual wield and
multiplayer with mod users remain required; teleport is excluded.

Target: installed Steam Serious Sam 2, fingerprints in installed-build.json. Create source and compiled development package for proper OpenXR stereo, six-DOF head and hand tracking, independent native dual weapons, per-hand wheels, smooth movement and snap turn. No actual game/headset testing or launch. No claim of tested playability.

## Architecture
Preserve Serious Engine 2 and Sam2Game.dll. x86 d3d9 proxy acquires the actual device via pinned MinHook CreateDevice interception and loads game hooks. x64 ss2vr_host owns OpenXR+D3D11/actions and composition layers. Classic D3D9 CPU readback transports completed stereo images. This is the smallest compatible initial bridge; no renderer/game rewrite, custom inventory, gameplay state machine, OpenVR fallback or speculative D3D9Ex upgrade.

## Ordered implementation
1. Common fixed-width IPC, frame slots, math and input/wheel presentation. Versioned per-launch named mapping/mutex/events; stale/focus/tracking loss suppresses fire until physical release. No pointer/handle IPC. Input independent of rendered frames. Bounded wait and owned slots.
2. OpenXR host: extensions, runtime-selected adapter LUID, view sizes/formats, local reference space, session lifecycle, action set and bindings (Touch, Index, Vive, Microsoft motion), tracked aim poses, stick/thumbpad axes, triggers, wheel hold, use/jump/menu/recenter. One predicted snapshot per request. D3D11 swapchains with actual rendered pose/FOV submitted only for matching complete response. Separate stable controller wheel quads with legible labels and highlight, cancellation deadzone, no wheel-fire overlap.
3. Game input adapter: scope native input queries to player bindings, native simulation hook for inventory/selection intent; resolve current weapon handles on that thread. SetCurrentWeapon validates native inventory; enable existing dual mode via native ToggleDualWielding when weapons are idle. Override per-weapon fire queries to bypass identical-weapon coupling, retain all native weapon behavior. Physical hand enums verified: 0 left, 1 right.
4. Tracking/weapon adapter: base CPuppet GetViewOrigin purpose-0 body anchor with verified stance-height interpolation, original animated GetCameraPlacement as the authored-offset reference, shared origin/yaw calibration. Base+Sniper GetShootingPlacement hidden-return hooks supply per-hand tracked muzzle transforms. Native GetWeaponAbsPlacement supplies model-placement calculator seam; preserve native authored offset while replacing camera-bound grip pose. Engine quaternion/Matrix34 uses same native math conventions; explicit basis conversion calibrated from original camera view and projection. No remote-enemy or vehicle weapon mutation.
5. Stereo: positively identify local player and run native Render3D twice with immutable eye request. Each invocation builds and frees native collection, recomputing eye culling/LOD/billboards. Never replay CViewRenCmd::Execute or simulation. Local camera/projection overrides are scoped; weapon projection override scoped to actual local weapon rendering. Each eye has distinct final color/depth; native intermediate targets remain inside native invocation and are never retained. Route original backbuffer binds to active eye only; nested offscreen passes untouched. Resolve/copy final color with pitch handling. Restore target/depth/viewport/scissor; retain native shader/texture state and rebuild original desktop Render3D once to restore engine caches. No display of two copies of a mono image.
6. Menus/HUD: preserve desktop fallback, present native health/armor/per-hand ammo in a headset HUD; host menu screen quad for non-gameplay frames, with explicit classification. Runtime-loss and unknown builds degrade to desktop with diagnostic, never incomplete stereo advertised as working.
7. Build x86/x64 products and official x64 loader, package separate staging output, collision-refusing install/uninstall tool, Proton launch instructions and runtime caveats. Offline verification of ABI/fingerprint gates, math, wheel/trigger suppression, frame ownership and both PE architectures. Commit source, docs and final status; no proprietary assets or logs.

## Scope limits that must remain visible
Cross-compilation is not headset verification. CPU readback may perform poorly; GPU transport remains future optimization. Linux/Proton OpenXR runtime routing is unverified. Vehicle-specific weapon aiming, binocular/scoped sniper behavior, cinematics and exotic postprocess passes require later runtime verification and must not be described as tested. The baseline single-player exclusion is superseded by the required multiplayer extension in IMMERSIVE_PLAN.md and MULTIPLAYER_REVIEW_DISPOSITION.md. Physical camera translation is bounded; full roomscale body collision is not established.

## Completion record
Record which real paths are implemented and built. Any unimplemented integration gate must be stated explicitly. Do not use scaffolding/test success as a substitute for proper VR integration. User's exclusion of actual testing does not authorize claiming runtime correctness.

Current optical source follow-up derives a proper rigid camera/physical radii
from the actual admitted animated Scope cap/full affine, under the existing
per-eye observation lifetime. Reflection remains in aperture geometry; centered
affine offsets preserve numerical stability. Astra/xhigh SOURCE GO covers the
math and experimental native-VS-preserving PS2a image shader/tool, not image
enablement. Owned UV0 planar correspondence is pinned; live UV/color-purpose,
pre-first-gun source capture, zoom mapping and cap substitution remain required.
See SCOPE_IMAGE_DESIGN_REVIEW.md and SCOPE_OPTICAL_FRAME_SOURCE_REVIEW.md. All18
offline groups/final builds/static ABI checks pass; no runtime has executed.

The current source observes actual ordinary tracked-sniper draw affines at the native palette producer, independently of optional remote-head tracking. Astra approved the read-only slice after thread-ownership, supersession and model-replacement fixes. Samples are per-eye, pointer-free and explicitly content-unverified. This adds no optic rendering or native zoom input and does not close the remaining immersion goal. See SCOPE_POSE_SOURCE_REVIEW.md and NATIVE_OPTICS_AUDIT.md; the immutable0.2.5 package predates this source follow-up.

## Final reviewed refinements

Astra's body-anchor and transport follow-up reviews are in ANCHOR_REVIEW.md, TRANSPORT_REVIEW.md and TRANSPORT_AMENDMENT.md; final dispositions are in REVIEW_DISPOSITION.md. The baseline used ABI3 with a request epoch; the current ABI7 retains it and early atomic shared invalidation. Epoch advancement/native publication is serialized by the existing snapshot lock; obsolete copied snapshots cannot publish. Death, third-person, deletion, renderer reset and control/calibration changes invalidate prior world images.

The host polls the existing slots without waiting for game capture, retains Ready through swapchain retries, and admits one pending native transaction including cancelled Rendering. Complete responses observed before the provisional 150ms enqueue-age cutoff replace the last pair. Both eye swapchain waits precede either upload/release; cached images are hidden if reacquired or partly replaced. Between captures, only the original complete released pair may be resubmitted with its actual original poses/FOV and current XR frame display time, within matching epoch/session/reference/dimensions/focus/tracking/gameplay guards. Invalid/expired output yields no world layer. This supersedes the initial zero-world-layer-on-every-missing-new-response wording; it never permits two copies of mono or mismatched eyes.

Canvas copies/readbacks/fills route only aliases of the original desktop surfaces. Native intermediate targets remain native. Root view identifiers distinguish per-eye occlusion history at a verified caller; no global culling patch or alternative query manager. One head-center translation bound preserves physical IPD. Recenter retains real triggers/grips while controls are suppressed.

Native completion holds the snapshot lock through the Ready commit, serialized against early invalidation. Copy pixels under IPC ownership before the short snapshot commit. If a Ready candidate is abandoned after XR waits, release existing waited acquisitions without writes and invalidate the cached pair; never release an unwaited image.

Normal session stop invalidates all presentation and retires existing acquisitions in every stereo/UI chain before EndSession, using bounded retries and fail-closed host exit on timeout. Resize/destruction first drains submitted GPU work and attempts no-write retirement; never release an unwaited image. A timed-out unwritten acquisition may be disposed of by destroying its drained swapchain, consistent with the pinned destruction contract.

## Current extension

OpenXR head and hand poses retain all three translation axes and pitch/yaw/roll. Native stereo, muzzle and weapon transforms consume those poses; pure yaw-only aiming is not used. Head horizontal/vertical bounds are separate, preserving deep crouches and one shared rig correction for both eyes/hands. The packaged ABI6 added presentation-correlated native menu pointers; source ABI7 adds independent logical action activity/generations and menu-pointer primary provenance. The immutable0.2.5 package uses IPC6/wire4; source0.2.6 uses IPC7/wire6 for explicit fresh neutral samples, per-hand ACK epochs and retained-shot zoom context. Both use native reliable snapshots/consumption ACKs, native authoritative weapons and identified remote model subtrees; dedicated loading uses the original module mechanism. An opt-in sampled head-lean fade uses the existing native ray phase and identical opaque stereo pixel processing, with no IPC or tracking change; Astra required default-disabled behavior until useful query availability and dynamic geometry freshness are established. Roomscale body collision, complete head protection, immersive scopes, vehicle controls, full overlays, physical melee and remote head bones remain required follow-up work (teleport excluded). No runtime testing is authorized.

## Remote head follow-up

Astra approved one exact post-DDE30 temporary palette adapter shared by native CPU/GPU skinning. Source integrates full XYZ/quaternion Head/owned descendant delta while retaining native animation and affine deformation, with atomic ownership/provenance/non-alias checks and no canonical pose writes. It is explicitly opt-in/default-disabled until complete unhooked worker/model/deletion lifetime is established. Pair admission now freezes remote eligibility once and holds raw MP/binding guards through Ready; incompatible recovery history cannot revive older pairs. Astra source/follow-up approved four corrected defects. Nine offline groups and final cross-build/static checks pass. This is a bounded checkpoint, not completion of the full immersive goal.

## Physical gun execution plan refinement

Fresh Astra/xhigh review verified native clip-distance changes between root Prepare and ActivateExecution. The minimal adapter therefore captures installed raw P/full view/depth at initial root execution and shares them through an exact owned gun invocation, including finite tracked model placement. Original by-value camera remains the authored placement reference; native shaders/backend projection/animation/stretch/weapon gameplay remain in use. Only Core inverse and native Gfx depth hooks are added. Native restore is observed, failed/partial transactions receive bounded native cleanup and sticky whole-pair rejection. This closes a physical gun presentation boundary; it does not complete scopes, roomscale, vehicles, native overlays or swing melee. No runtime testing is authorized.

## Native zoom adapter preparation

Astra selected field-free exact native read predicates and direct native Activate/Deactivate/base alternative-held callbacks, preserving true handedness through every native callback. Six predicate sites and before-toggle/post-base candidate boundaries are statically audited; the single shared MinGW x86 entry macro is compile-only, not linked or installed. Native state/timing/damage/audio remain the behavioral reference. The wire5 retention component and exact no-left stock alternative-route correction have bounded Astra source GO. They do not enable zoom: current producer emits zero zoom fields. Local pre-weapon preparation reuses the existing current-world entity-manager seam and native roster, independently of server consumption ownership; its separate review record captures final disposition.

Remaining native zoom gates include actual ownership/lifecycle admission, input neutral and profile collision provenance, native held reassertion after base step, managed-left cleanup through sound-stop/deletion, pre-toggle shared-owner cleanup and authority/prediction behavior. Loaded scope content, actual cap draw/index provenance and scope color/target/HDR ownership must precede optical rendering. Vehicle evidence now confirms the existing first-person gate excludes mounted players; mounted input/camera/seat lifetime remains an explicit separate native adapter requirement. See SNIPER_ZOOM_PREDICATE_REVIEW.md, SNIPER_PREDICATE_ENTRY_REVIEW.md, SNIPER_ZOOM_LOCAL_PREPARATION.md and NATIVE_VEHICLE_CONTROL_AUDIT.md. No feature completion or runtime correctness is inferred from these preparations.


## Action and weapon-intent integration

Source now carries OpenXR action activity and logical stream identity independently of hand tracking. Fresh raw primary release is explicit; a clear hysteresis latch or old cumulative serial cannot rearm an acknowledged replacement. The existing ACK/pose/retention policy supplies per-hand epochs and post-ACK sample exclusion. Client submit, cached native commands, weapon fire and authoritative muzzle use borrow the existing admission state at consumption; no parallel transport or scheduler is introduced. Native sniper desired zoom remains zero. See INPUT_INTENT_INTEGRATION_BRIEF.md and the source review/follow-up records for exact dispositions. Full equivalence remains active; no runtime was launched.

## Mounted tracking/control checkpoint

Astra/xhigh final source GO closes the minimal mounted adapter after stale-selection, normal-release/history, hand-loss, getter-lifecycle and handgun-deletion fixes. Local/native authority/remote presentation share the actual rider/seat body anchor, preserving full XYZ/quaternion tracking and independent eye views. Right-hand aim passes through original clamp/ClientAction/vehicle dispatch; native movement/fire/physics/RPC stay intact. Root943B6-only avatar selection leaves handheld injectionFDA0B and desktop paths native. IPC7/wire6 unchanged. Final full cross-build, all11 offline groups and refreshed artifact/compiled ABI checks pass. See MOUNTED_ADAPTER_SOURCE_GO.md and mounted-source-checks.json.

Vehicle-muzzle lasers remain separate required work; mounted integration does not imply complete vehicle parity. Existing handheld lasers and comfort-following HUD/menu panels remain. Full native overlays, roomscale body collision, immersive optics/native zoom, physical melee and complete remote head support still require implementation. No runtime was launched.

## Native world-marker refinement

Implemented Astra's minimal post-Render3D design in the existing admitted eye loop. Exact parent returnFEA8A supplies an optional callback; fallback/recursive/desktop calls retain original rendering. Original marker helpers receive copied actual executed view/full asymmetric P and validated dimensions/native fade after final native color, before eye readback. Actual target/depth/viewport and live frame/rider/world-info/drawport checks surround the native phase; partial failure rejects the pair. No root reactivation, parent/listener replay, additional hook, quest model, IPC or wire change. Bounded Astra/xhigh SOURCE GO closes the slice after fixing a cross-eye fixture and recording manual compiled argument provenance for exact accepted artifacts. Full final cross-build/all12 offline groups and artifact/ABI checks pass;0.2.7 is the separate development package.

Next native flat UI work must retain its original once-only owner: RenderOverlayFadings mutates timing, and the HUD container invokes element callbacks before drawing. The current opaque image transport does not supply proven transparent native alpha. Compare bounded capture around the original invocation with immediate GPU-draw duplication, preserving native desktop/quest policy and reusing the existing image channel/comfort HUD layer. Prove alpha/blend coverage, full-field fade ownership and coherent pair/image admission before architecture/source work. See NATIVE_FLAT_OVERLAY_AUDIT.md. Actual vehicle blast attachment/executor placement remains a separate narrow gate; main corrected the model-absence investigation error using the owned All_PC_02 archive. Roomscale/scopes/melee/complete remote head remain active requirements. No runtime testing authorized.


## Current0.2.8 turret laser checkpoint

Implemented the Astra/xhigh-reviewed minimal native turret aiming laser in the existing query/sample/frozen-pair/draw path. The exact audited turret table2B39E8 supplies original selected attachment, explicit existence, original purpose1 world origin and reported shoot direction. Native main thread plus actual idle model scratch guards protect the destructive query boundary; active resource/config COW, model, rider/seat, action, attachment, mechanism and request changes reject stale sampling/publication/freezing. Collision excludes the actual vehicle mechanism; no vehicle sample reuse, gameplay/fire rewrite or protocol change. One right-side native mounted beam; both native fire commands stay original.

Astra design/source GO, full x86/x64 cross-build, all13 offline groups and refreshed artifact/compiled caller checks pass. Meaningful portable checks cover actual production pose/scratch/frozen-source helpers; they do not execute native callbacks, COW, missing-attachment lookup or native gameplay. Manual receiver/buffer provenance is recorded for actual compiled hashes. See NATIVE_VEHICLE_LASER_SOURCE_REVIEW.md and vehicle-laser-source-checks.json. New separate0.2.8 package; older archives immutable.

Full equivalence remains active: broader vehicle implementations, roomscale body collision, native zoom/optic images, complete flat overlays/full-field fades, physical swing melee and complete remote-head lifetime/enablement remain unfinished. Exact source acceptance does not prove query opportunity, moving freshness, appearance/performance or unknown unhooked-worker concurrency. No runtime/headset/network session or installed-game changes. Additional native UI inspection proves conversation Render mutates texts/time, and the current Ready commit precedes original flat overlay; any capture must preserve once-only native ownership and immutable correlated images. NATIVE_FLAT_UI_ONCE_OWNER_FOLLOWUP.md records evidence, not an implemented UI adapter.

## Complete flat UI architecture decision

Explicit/effective Astra xhigh selected once-only native overlay plus immediate
DrawPrimitive/DrawIndexedPrimitive duplication and an extended existing owner/
pair transaction as the smallest full-blend route. Conditional design only;
matrix-only projection plus fresh stencil depth is not source-approved. Main
proved native physical viewport/scissor mapping and actual builtin GPU object
records, but found glyph effects transform XYZ and re-enable depth testing.
Original depth enable does not force ALWAYS; default comparison is LESSEQUAL.
Resolve source clipping/depth before flattening. Preserve actual adjusted
device constants, original RGB/alpha/assets, post-original state side effects,
pre-UI world dimming order, exact native fade scopes and existing comfort owner.
No native callback replay, new HUD queue, shader rewrite or multipass RGB bank.
See NATIVE_FLAT_UI_DESIGN_REVIEW.md and NATIVE_FLAT_UI_CLIP_STATE_AUDIT.md.
The0.2.8 package is unchanged; complete flat UI remains unfinished.


## Accepted flat native UI geometry (2026-10-04)

Astra/xhigh gave bounded source GO for the flat panel transform and tests. The
adapter retains original source clipping via six transported halfspaces and
synthetic output clip-Z, while preserving flat eye X/Y/W. Added VR UI draws will
disable destination depth tests/writes for legibility; source fog or depth-dependent
shading is not admitted. Original program objects, UV/color, blending and native
callbacks remain the integration reference. Physical thickness, replacement
shaders and scratch masks were rejected.

Final production-helper checks pass 1658 boundary witnesses, all14 offline
groups and Windows x86/x64 compile-only checks. These verify geometry, not native
GPU draws, raster appearance, callback coverage or frame completion. The helper
is not yet connected; the next step is the once-only owner/draw/pair adapter,
actual compiled position semantics and pre-UI dimming/fade ordering. Full goal
remains active. No runtime or installed-game changes; immutable0.2.8 unchanged.
See NATIVE_FLAT_UI_CLIP_SOURCE_REVIEW.md and native-ui-clip-source-checks.json.


## Native UI cleanup and admission preparation (2026-10-04)

Astra/xhigh approved the bounded native-finally boundary and actual program,
adjusted-eye-projection and creation-thread/device gates. Main caught a mistaken
HLSL-route claim; the verified native assembly route let us delete a proposed
shader parser. Native SEH differs from MinGW DWARF2, so one small MSVC-target C
finally object isolates cleanup while retaining the surrounding toolchain. GNU
callbacks realign locally and contain direct errors to a failurebool; they never
rethrow across the foreign boundary. Native exceptions remain native. Future
resource ownership/reentry and reset-only failure policy still require integration.

Full cross-build/all14 offline groups and compiled ABI/artifact checks pass.
The actual final PE code, scope/funclet/IAT identity, four HIGHLOW relocations and
GNU callback alignment are checked; no exception, Windows code or game was run.
The installed ProtonHotfix32-bit API-set/schema/provider export chain was inspected
read-only. Runtime selection, unwinding, GPU appearance and performance remain
unverified. IPC7/wire6/version0.2.8 and immutable archives remain unchanged.

Complete UI is still not connected. Next extend the existing Rendering pair
through the once-only original brain/overlay owner, add closed-domain immediate
GPU duplication with explicit cleanup/state restoration, pre-UI alpha-preserving
dimming and frozen existing comfort geometry, then final presented-pair metadata.
Keep existing world-only behavior where the complete-UI owner is not admitted.
No new queue/renderer/protocol service. Full immersion goal remains active;
roomscale collision, scopes/native zoom, physical melee and remote-head lifetime/
enablement remain required. See NATIVE_FLAT_UI_NATIVE_BOUNDARY_REVIEW.md.


## Current0.2.9 connected native UI checkpoint

Astra/xhigh SOURCE GO closes the once-only native UI integration after four
corrected defects: raster point/line and point/wireframe-fill admission, retained
failed-unlock ownership/reset quarantine, and session loss during moved fallback
HUD upload. Original brain/overlay/fade callbacks run once, immediate admitted
filled triangle draws preserve native programs/resources/blend/alpha/order in
both existing eye RTs. Exact native fade calls cover whole eyes. Actual adjusted
P/source constants and viewport/scissor clipping supply flat physical panels;
source fog/stencil/userplanes and unsupported color output reject the pair.
Nonlockable/actual swapchain identity, creation/main thread, current hooked device
and raw-resource/frame generations gate compatibility. No new renderer, shader
parser, HUD state model, frame queue or XR layer.

Existing Rendering ownership now extends through original overlay AND enclosing
owner return; Ready publishes only both final images with matching immutable
geometry. Pre-UI world dimming preserves alpha via existing pitched readback/
UpdateSurface, then final export only makes alpha opaque. The existing35°/8°/
90°s comfort anchor updates once and freezes full-canvas panel size/aspect; stats
fallback is suppressed only for a final surviving completed UI pair, including
reuse and post-wait revalidation. Final loss/quit clears EVERY layer. Failed raw
restore/unlock halts adaptation until successful reset/new creation. Nativefinally
cleanup covers explicit resource and commit-lock ownership, not arbitrary crashes.

Full x86 proxy/server+x64host/official-loader cross-build/all14 portable groups,
refreshed41-export/5-internal-seam artifacts and native-finally ABI checks pass.
IPC8/wire6/version0.2.9 products must match; previous archives remain immutable.
NATIVE_FLAT_UI_CONNECTED_REVIEW.md and native-ui-connected-source-checks.json
record exact source/artifact evidence. Compiled/offline acceptance does not prove
native callbacks/GPU/foreign unwind/provider behavior, stock callback/device/thread
opportunity, appearance/deadlines/performance or runtime/headset compatibility.
No game, host, Wine, XR/server/network session, installed-game/config changes or
publication. Roomscale body collision, native zoom/optic images, physical swing
melee, broader vehicles and complete remote-head lifetime/enablement remain
required; full goal is active, teleport excluded.


## Current0.2.10 input and multiplayer checkpoint

Astra/xhigh SOURCE GO accepts native input interval/TLS retirement and exact
credit/discard ownership after three review fixes: allocating lifecycle lock
scope, transport handoff credit/ACK retention, and cached local neutral rearming.
The original simulation/entity/player callbacks and existing scheduler, token,
neutral gates and reliable protocol remain. Root interruption restores TLS,
invalidates existing tracking/intent epochs, retains actual inactive awaiting
slots for later normal discard, and excludes pre-interruption local samples by
sequence/time/session/reference/producer. Snapshot resets preserve that boundary.
No native send, game callback, allocation, new ACK queue or protocol on abort.

A related actual compiler warning exposed the relay Sample revision landing in
liveIntentEpoch[0], leaving presentationRevision0 and preventing admitted remote
VR presentation. Corrected exact aggregate positioning and made narrowing a
build error for both x86 products. Full remote-head lifetime/enablement is separate.

Final x86 proxy/server+x64 host/official-loader build, all14 portable groups,
artifact/native-finally ABI and pinned8-window static planning checks pass.
Direct scan completeness, indirect incoming paths and actual HDE continuation/
installed-hook behavior remain unknown. Exact evidence/dispositions:
NATIVE_INPUT_INTERVAL_REVIEW.md and native-input-source-checks.json. No game,
Windows program, host, Wine, XR/server/network session, installed-game/config/save
changes or publication. IPC8/wire6/version0.2.10; prior archives immutable.

This is a real input/replication source fix, not native zoom or optical image
integration. Native zoom producer remains zero. Next connect callback-owned
native zoom/predicates only after actual borrowed cleanup lifetime, operation
reentry and production-entry admission close; preserve native damage/timing/
scheduler and historical shot context. Full roomscale collision, optic images,
physical swing melee, broader vehicles and complete remote-head support remain
required. Full goal active; teleport excluded.


## Current0.2.11 native per-hand zoom checkpoint

Native zoom input is connected end to end: contextual per-hand OpenXR action,
later neutral admission, local snapshot and existing wire6 intent, actual
caller-selected sniper Step/Fire, original native held/Activate/Deactivate,
field-free conditional reads and original PutDown/Delete. Native flags, timers,
progress, damage, ammo, recoil and sound remain native. Existing active zoom
survives native cooldown4/recoil8; holster/bringup cannot activate. Right-only
selection leaves left intent intact. Exact right-delete cross-call preservation
uses current ephemeral owner/BC routing independently of older cleanup claims.

Astra/xhigh source GO follows six interaction fixes plus the reassociated-owner
routing correction. One bounded per-source provenance store remains outside
copied snapshots/authority, with cleanup-only lifetime and no live eviction.
Revoked effects cannot restart, but same-source native Stop remains available;
source deletion/reuse prevents stale outer accesses. External native activation
retains stock ownership and restoration, conservatively declining another
adapter takeover until source deletion. 128-record exhaustion declines new zoom.
No deferred pointers, raw native state writes, scheduler or protocol change.

Final x86 proxy/server and x64 host/official loader cross-build and all15 offline
groups pass. Actual linked entries/callback return ABI, native-finally compiled
shape,47 exported/11 internal seams and matching IPC8 layouts pass. Unchanged
pinned HDE32 host decoding accepts18 stolen+8 continuation instructions. These
checks do not execute native callbacks, MinHook installation, Windows or VR.
Exact review/evidence: SNIPER_BORROWED_ZOOM_SOURCE_REVIEW.md and
native-zoom-source-checks.json. No installed Bin/config/save changes.

This completes native zoom controls, not optical rendering. Native owner FOV
is shared; the surrounding XR world preserves runtime eye FOV. Magnified optic
images, roomscale body collision, physical swing melee, broader vehicles and
complete remote-head lifetime/enablement remain required. Teleport stays excluded.

## Scope cap stage refinement after0.2.11

Astra rejected late replacement of completed cap pixels because it cannot retain
transparent foreground/effect ordering. The next bounded image proof is the
native cap's actual color pass and relevant root suffix, with matching-stage
capture and original depth/stencil/material pass behavior. Pre-HDR capture alone
is insufficient. Reuse native resource retention/read boundaries for any later
four-slice copy; neither readableMemory nor native lock counts pin allocations.

Source now shares exact model/mesh/draw/palette selection and records selected
surface layouts in the existing pointer-free scope sample. The owned cap parser
and private pinned-asset check preserve full affine/reflection and real winding.
Astra gave bounded SOURCE GO; all16 portable groups/full cross-build pass.
No live buffer or optical image path was enabled, and0.2.11 archive is unchanged.
See SCOPE_CAP_STAGE_REVIEW.md. Full equivalence remains active; no runtime testing.

## Actual Scope GPU geometry boundary

Astra/xhigh accepted the minimal actual-DIP COM reader after requiring complete
draw-hook coverage and same-invocation rejection retirement. Native input uses
the original static managed source, per-stream layout and actual evaluated full
affine; four exact hashes precede cap extraction. Owned copies are hashed only
after all locks and references are retired. Failed/indeterminate Unlock stops
all intercepted drawing for the process, preserving one uncertain reference;
Reset is not resource retirement for managed buffers. No new hooks, registry,
simulation, protocol or resource recovery layer. All17 offline groups, full
cross-build and compiled ABI/artifact/finally checks pass. Actual runtime remains
excluded;0.2.11 package unchanged. See SCOPE_GPU_GEOMETRY_REVIEW.md.

Next optics work must classify the exact native cap color-purpose and relevant
root suffix, then replace cap color with an unlit sample at that same draw while
preserving original VS/geometry/depth/stencil and native partial-fade behavior.
Magnified source views still need their own matching capture stage, camera/view
identity and per-hand ownership. Loaded geometry alone does not authorize image
insertion or complete content verification. Roomscale collision, physical melee,
broader vehicles and complete remote head support remain required.

## Live UV input extension after optical preparation

The actual-DIP observation now admits five exact copied hashes, including raw
stream3 FLOAT2/TEXCOORD3 UV. Mandatory UV narrows diagnostic coverage; it does
not classify native color purpose or suppress ordinary rejected draws. Existing
invocation retirement/no-revival and lock ownership remain authoritative.
Centered fitted image coordinates have bounded conditioning, local-fit and CPU
float-reference checks; they are derived values and remain unconnected to live
image substitution. All19 offline groups/full cross-build and owned five-slice/
compiled ABI checks pass, Astra/xhigh SOURCE GO. No runtime testing performed.
See SCOPE_LIVE_UV_REVIEW.md. The immutable0.2.11 package predates these sources.

Native stock vtable inspection confirms all17 gun Render implementations target
the existing base/sniper hooks. Unknown classes still cannot inherit that proof.
Actual source-cut content, color-purpose/program/c8/c9, transfer and auxiliary
view identity/animation remain gates before magnified image integration. Full
roomscale collision/melee/broader vehicles/remote-head work remains required.
All further investigations and reviews use Astra under the user's latest rule.

## Further native admission and roomscale policy

Astra traced actual ordinary model/preset/final-material routes and generated
shader/declaration mapping. Current program/constants must come from successful
device reads; native caches can update after failed setters. A strict UV-interface
recognizer can close the c8/c9 relation without invented native bytecode hashes;
complete skin/position/alpha identity and root/source COLOR still need proof.
See SCOPE_NATIVE_COLOR_ADMISSION.md. Native magnified imagery remains unfinished.

Roomscale submitted-request settlement earned only conditional prototype GO,
with production NO-GO for unresolved action duration, units and publication.
Main does not promote this speed/acceleration-constrained horizontal policy as
proper direct6DOF. Preserve original native collision and replication; seek an
existing relative-collision move with attributable accepted displacement before
replacing any adjacent engine system. No production roomscale change yet.
See ROOMSCALE_INPUT_DESIGN_REVIEW.md. Full immersive objective stays active;
physical melee, broader vehicles and complete remote head also remain required.

## Actual scope UV program checkpoint

Current source admits actual bounded immutable VS1.1 UV tokens and finite c8/c9
rows through successful device getters, retaining the existing copy/lock owner.
Fresh post-DIP raster validation derives image coordinates in the existing bank.
Astra/xhigh bounded SOURCE GO; all20 portable groups and x86/x64/static checks
pass. This is UV admission only; magnified image integration remains required.
See SCOPE_PROGRAM_ADMISSION_REVIEW.md/scope-program-source-checks.json.

Native roomscale checked-placement, kinematic, manipulator and generic motor
paths do not yet establish proper1:1 collision-aware player displacement. No
unsafe body-mode/solver replacement is added; full goal remains active. Evidence
and narrow follow-up are in ROOMSCALE_COLLISION_API_AUDIT.md.

## Actual opaque scope observations and native input join

The optional scope reader now observes actual immutable PS2.0 properties,
retained eye RT/depth, captured full viewport/depth and supported opaque raster
state before native buffer locks and again after unlocks. It adds no material
hooks, shader-bank traversal or module policy. Failed color observations leave
independent geometry/UV admission intact. Matching pre-draw observations do not
prove uninterrupted state through the eventual DIP or authorize an image pass.
Astra/xhigh final SOURCE GO, full cross-build/all21 offline groups and compiled
ABI/cap/fixture checks pass; SCOPE_COLOR_READER_DESIGN_REVIEW.md records limits.

Native firing investigation rejects brain borrowing and approves a five-site
read-only scalar join preserving native history, mapping, blocking and weapon
FSM. The executable brief reuses existing simulation preparation/frozen intent.
Actual consumer integration remains required before physical motion production.
Stock USE COMBO WEAPONS=YES plus actual native capability/dual state is required
for uncoupled identical guns; no raw dual-state write or new toggle loop.
See PHYSICAL_MELEE_JOIN_DESIGN_REVIEW.md/PRIMARY_JOIN_IMPLEMENTATION_BRIEF.md and
NATIVE_COMBO_CAPABILITY_AUDIT.md. Full immersive goal remains active.

The actual optional color reader now also requires the exact ordinary gun-command
origin and fresh native query index-1 in both snapshots. Astra/xhigh bounded
SOURCE GO/full builds21 groups; SCOPE_QUERY_NATIVE_AUDIT.md distinguishes native
command-order evidence from unproved actual-GPU failure-total query absence.
Source capture completeness/transfer and ordered image execution remain gates.

ClientAction replay is not a primary-only alternative: extra RPC/correction can
change unrelated native state. PRIMARY_NATIVE_ACTION_AUDIT.md and
PRIMARY_JOIN_REFERENCE_COMPARISON.md reopen the narrow read-substitution decision.
Full-volume overlap is not sweep acceptance; a native thick-triangle edge miss
is a demonstrated narrow kernel incompatibility, not permission to replace the
collision world/solver. ROOMSCALE_QUERY_ADAPTER_BRIEF.md and
ROOMSCALE_KERNEL_ADAPTER_REVIEW_BRIEF.md compare incremental adaptation; no
roomscale mutation/replication is added before that review and lifetime proof.

## Narrow native query and primary source verticals

The completed senior comparison selects immutable five-site primary A and
rejects accepted-action B's unrelated correction/RPC effects. Source replaces
the high-level fireQuery with original operator/held consumers and actual native
mapping. Initial all-pawn neutral reservation was corrected: unclaimed wrappers
retain mask0 through admission reentry; known VR reserves mask03 neutral before
gameplay getters. One captured value persists through same-pawn nested native
history commits. Existing simulation preparation/intent/transport remain the
owners; no brain action/history stores or second command scheduler are added.
Full cross-build/all23 offline groups and actual linked entry/return/finally
checks pass. Independent Astra/xhigh SOURCE GO follows corrections for desktop
startup ownership and native carrying. Local recognition requires existing
initialized tracking plus matching player/handle; a local nonce alone cannot
claim it. Live native carry uses mask0 and revokes prepared handgun high without
changing immutable ancestors or native throw/history behavior.
See PRIMARY_JOIN_REFERENCE_COMPARISON.md and primary-load-boundaries.json.

Astra/xhigh SOURCE GO accepts the bounded model-triangle adapter after a
native-order facing certificate fixes a cancellation counterexample. Original
TOI/world traversal remain; one lexical scope adds finite contact-depth bounds.
446 portable checks, UBSan/x87 variants and actual compiled detour ABI pass.
No hook is activated and no body movement is implemented. Native public query
scratch includes cleanup callbacks and visited-hull flags; scalar save/restore
is not a lifecycle transaction. The NativeSolver join is verified, but a fresh
main-thread query extent, primitive contact, native TOI accuracy, body dimensions,
checked placement and authoritative settlement remain gates. Do not replace the
solver/world or add global serialization to hide those unknowns.

Scope first-gun capture has a concrete flare omission. Native collection can
omit guns via the existing contextual third-person predicate atFDA0B for an
explicit source purpose, leaving the full original world/suffix/cleanup path.
No global show-weapon write or extra collection hook is selected. A completed
source would already contain bloom/HDR/fade; insertion must solve color-stage
equivalence before any image is enabled. See SCOPE_SCENE_SOURCE_AUDIT.md.
All investigations/reviews are Astra/xhigh. No runtime testing was performed;
full immersive equivalence remains unfinished.
