# Implementation status — 2026-10-07

## Active PC implementation

The latest direct-scene run observed the expected native Jungle stream hash, local
gameplay with a null online interface, thirty neutral-head input observations and
normal native shutdown. It failed first-eye completion before publishing any world
pair; no native stereo/6DOF acceptance is claimed. The desktop terrain-occlusion
report and deterministic loading-screen continuation are active gates.

Latest rendering investigation: [PC_GAMEPLAY_READINESS.md](PC_GAMEPLAY_READINESS.md). Actual native menu CPU transport and host submission work in the private Proton/Monado lab. Actual outdoor first-person gameplay enables tracking and eye swapchains, but no native world pair was delivered in the observed segment. Distinct eye imagery/parallax and head translation/rotation remain unaccepted. The earlier startup summaries below precede this draft integration.

Development is active. Start with [CLOUD_CONTINUATION.md](CLOUD_CONTINUATION.md)
and [PLATFORM_SUPPORT.md](PLATFORM_SUPPORT.md); HANDOFF.md and the records below
are historical evidence, not the current pause state or feature count.

Windows and Linux via Proton are required. Native magnified scope source views
and ordered cap imagery are connected in source, alongside native zoom, mounted
controls/turret lasers, native flat/world overlays and multiplayer integration.
The current offline suite has60 groups. Native products cross-build. The user now
permits PC runtime testing and additional sessions before feature completion.
Initial Xvfb launches hit a stock-reproduced zero-refresh DXVK failure. A focused
real-display IPC10 proxy run now renders the native desktop menu. See
[PC_VALIDATION.md](PC_VALIDATION.md) for identities and observations. The actual x64 host reaches FOCUSED under Proton and native menu transport works.
Actual gameplay stereo remains unaccepted; see the newer direct-scene record above.
No headset or network runtime acceptance exists. Older coding-first/no-runtime
instructions are historical; actual observations remain separate from offline checks.
GitHub Actions is disabled at the owner's request; checks remain local.

User targets are now confirmed: SteamVR + Steam Frame and Envision/Monado +
Bigscreen Beyond, with Windows/Linux-Proton coverage. Simulation and brief SS2
focus are authorized. The private simulated Monado baseline and the actual x64
host under Proton both reach FOCUSED. A synthetic IPC fixture exercises the actual
host's D3D11 swapchains, WMR-profile hand/head input and successful menu-quad
xrEndFrame calls; it does not prove native game transport or actual hardware.
The game tracing checkpoint confirms recurring native additional-swapchain
presentation and actual1280x720 RT0 ownership. See
[PRESENTATION_OWNER.md](PRESENTATION_OWNER.md) for review disposition/native limits
and [PC_VALIDATION.md](PC_VALIDATION.md) for exact test identities. The draft capture adapter now routes all buffer consumers through the admitted native
chain; ordinary-path and reset retirement changes are implemented with limited reviews.
Full scope/nested/reset runtime acceptance remains open.

Current source connects default-off single-player roomscale body movement through
native whole-body queries, the checked native setter and actual-anchor origin
settlement. Default-off head-volume checks cover handheld and seated world queries,
with IPC10 head/eye clearance limits on cached frames; native exclusions can omit
the ridden vehicle itself. Optional immersive swimming preserves head-directed
joystick input as the default. Remote renderer native-unwind cleanup now explicitly
retires TLS, locks and owned scratch. Packaging checks matching compiled build-input
fingerprints and version/layout contracts instead of hardcoding an obsolete ABI.

Current PC source fixes squeeze-loss weapon-wheel cancellation using durable
per-hand epochs and separate producer admission. All60 Debug/Release groups,
UBSan and compiled/layout checks pass after correcting Release assertion coverage;
Astra/xhigh approved the bounded source follow-up. This is not physical vehicle
steering. See SQUEEZE_ADMISSION.md.

Full implementation is NOT complete: multiplayer roomscale body/origin settlement,
physical melee consumption/observer delivery, broader vehicle coverage, complete
remote-head lifetime/enablement and the final full-project review remain open.
See the current roomscale, head-volume and swimming records linked from the
continuation. Do not substitute test counts for these integrations.

## Historical checkpoint record

The following sections retain their original scope and conclusions. References
to “current”, “paused”, old ABI/test counts or absent scope images apply to those
historical checkpoints only.

## Unaccepted physical-melee worktree

The current working tree additionally contains physical-motion production and
logical-primary integration. It compiles and passes25 portable check groups,
but its native gesture/carry history is **NO-GO** and is not in a development
archive. A gesture-only native history high can become a false carry release;
passing producer checks do not repair that native interaction. The accepted
five-site join described below predates this physical extension.

Astra has rejected both a B0/E8-only stop guard (a reachable short recoil pulse
loses canonical release) and proposed bits2/3/4 metadata (bit2 is the grenade
command). Native AI-hearing notification/dynamic state replication also does
not supply the missing saw observer transition on the investigated paths.
Canonical native consumption, carry isolation and short-input observer delivery
must be resolved before source acceptance or packaging. See
PHYSICAL_NATIVE_DELIVERY_AND_BITS_GATE.md and
PHYSICAL_SAW_CONSUMPTION_REVIEW_BRIEF.md. The full immersive goal is paused;
roomscale body movement, scope images, broader vehicles and complete remote-head
lifetime/enablement remain unfinished. No runtime testing was performed.

## Latest accepted native query and primary source

The accepted source includes the Astra-approved bounded, inactive native model
triangle query adapter and the independently source-approved five-site native
firing join. Desktop startup and carried-object commands remain native; known
VR handheld consumers preserve the native operator/history/mapping path.
Client/server/host/loader builds, all23 offline groups
and actual linked primary-entry/ordinary-wrapper checks pass. This does not
implement body-follow movement or scope images. Public query lifecycle,
primitive contact, body geometry/placement and multiplayer settlement remain
roomscale gates. See ROOMSCALE_MODEL_QUERY_SOURCE_REVIEW.md and
ROOMSCALE_QUERY_OWNERSHIP_AUDIT.md. The immutable0.2.11 package is unchanged.

Latest source adds Astra-approved optional matching pre-draw opaque scope
observations through actual PS2.0/device/retained eye reads, preserving independent
geometry/UV and original native forwarding. All21 offline groups/full cross-build
and compiled ABI/cap/fixture checks pass. No image transaction is enabled and no
runtime is verified. See SCOPE_COLOR_READER_DESIGN_REVIEW.md and
scope-color-source-checks.json. The immutable0.2.11 package remains older.

The single-player baseline is packaged as0.1.0-dev. Current source implements multiplayer input/authority/remote weapon presentation, native menu ray input, comfortable UI, grip alignment, haptics, collision lasers, an opt-in native remote-head palette adapter and physical native gun P/view/depth adaptation. All products cross-compile and ten offline groups pass; Astra source/follow-up reviews approved the physical-gun adapter; the separate0.2.5-dev package records this checkpoint. Full immersive equivalence remains incomplete. No game, Windows host, Wine, OpenXR runtime or headset was executed. No installed game files/settings were modified.

## Real integration paths

The x86 proxy loads the system D3D9 implementation and intercepts CreateDevice, Reset and Present. It pins its owning DLL so detours remain mapped during graphics backend reloads. Native game hooks attach only after five exact SHA256 matches (four native dependencies/executable checks on the headless path). The x64 host owns the actual OpenXR session, predicted poses, action spaces and swapchains. IPC ABI 6 is fixed-width, pointer-free and identical on x86/x64 (83,887,512 bytes including two stereo slots and one classified mono menu buffer).

Native simulation calls identify the local puppet and inventory, maintain controller-origin/snap-turn calibration, and call the original hand-specific selection/dual-wield methods. A pending selection intent waits for native changeability rather than replacing native weapon policy. Weapon handles are resolved on the calling engine thread; player and base/sniper weapon deletion invalidate ownership. Aliased hands suppress VR firing rather than coupling triggers. Native fire queries for distinct current hand weapons receive independent states. Base/sniper shooting-placement and model-placement hooks use the same tracking transform and preserve native authored rotation; muzzle displacement is derived from camera-relative native flat-model/shot placement and bounded, so calibration does not retain an old absolute world position. The tracking body anchor bypasses native view/weapon animation but retains verified stance-height interpolation; the original animated camera is used only to derive authored model/shot offsets. Death and third-person rendering disable VR gameplay. This calculation still needs visual/gameplay verification.

The per-eye path invokes the native Render3D prepare/collect/execute/free transaction twice with one frozen game snapshot/body anchor and native reference camera and one immutable XR request. Its local weapon injection remains native. It substitutes the original WindowCanvas color/depth only; intermediate canvas targets remain native. A final desktop invocation restores native camera/render bookkeeping. An old D3D state block is deliberately not applied before that invocation, because it could desynchronize native shader/texture caches from the actual device.

Only full-size, non-MSAA 32-bit WindowCanvas views with the matching native depth format are classified. Each eye's final RT/depth identity must match its owned surfaces; failed binds and active non-null MRT slots reject the pair. Readback copies the native non-MSAA A8R8G8B8 or X8R8G8B8 color format to equal-format system memory, honoring row pitch and supplying opaque alpha. Native desktop color/depth aliases are redirected at bind/copy/readback/fill seams. Root-only view IDs separate per-eye native occlusion-query history; nested IDs remain native. The host accepts only complete same-request pairs and submits their actual eye poses/FOVs. Timeout/cancellation never frees a game-owned Rendering slot. A deferred completion must retire before another slot can be claimed. Input publication remains independent. Requests carry the native tracking epoch; early atomic invalidation and a snapshot-serialized Ready commit reject obsolete frames.

The host draws independent frozen controller-wheel quads with verified weapon names/ammo and a separate health/armor/weapon-ammo HUD. Explicit mono menu/loading frames come from the native backbuffer at Present and go to a screen quad, never a projection. Foreground-only menu input preserves keyboard navigation and dispatches controller ray motion/trigger through the native virtual640x480 mouse cursor. Pointer clicks carry their own input sample and exact displayed-frame identity. BGRA8 UNORM is preferred; RGBA8 UNORM is supported through an explicit red/blue conversion. No sRGB color conversion is assumed.

## Limits and open evidence

- Full child-command/postprocess classification is incomplete. The verified WindowCanvas binding and final-target checks support the narrow capture path; they do not prove every shadow/reflection/effect/HDR pass is correct. See RENDER_AUDIT.md. Only a complete, currently eligible pair can produce a world projection. An existing valid pair may be reused between captures.
- World-to-meter scale follows native camera/weapon transforms with scale 1; no headset calibration of physical size was performed. Horizontal head motion is bounded to0.75m, independently of vertical motion[-1.8,+1.0]m; both eyes and hands share the correction, preserving physical IPD and normal controller reach. Exceptional hand reach is bounded to1.3m from the physical head. These bounds are not wall collision or roomscale body physics.
- Experimental sampled head-lean fade is implemented and disabled by default (`HeadComfort.Enabled=0`). It uses native thick bullet-category queries, excludes the player/mechanism, and dims both world eye images from one exactly matched request/body/head result; host HUD/wheels remain visible. Missing/stale results do not dim. First-person/client query availability, moving-collider freshness, overlap and complete eye enclosure are unproved, so this is not continuous head protection. See HEAD_COMFORT_REVIEW_DISPOSITION.md.
- Continuous weapons share the verified native shooting-placement virtual family; this is static hook coverage, not proof of every weapon's firing path or accuracy. Native melee behavior is retained, not replaced with physical swings. Vehicle aim and immersive sniper scopes are outside this implementation.
- Native mission/conversation/boss/fade/zoom overlays are not all ported to headset layers. The small headset HUD exposes health, armor and current-hand ammo; the desktop retains native overlays.
- Stock first-person rendering and native multiplayer sessions are the current integration targets. Other custom native modules, cinematics, oversized drawports, MSAA and MRT rendering are unsupported. Multiplayer source is unverified in a session; teleport is excluded.
- CPU readback/upload may stall. The host no longer blocks XR frames waiting for game capture; it polls one pending transaction and may reuse a complete pair until the provisional 150 ms enqueue-age cutoff. Both image waits precede either write/release, and reacquiring a cached image hides that pair. Abandonment releases waited images without writes; a release failure leaves the pair invalid. Normal session stop retires existing acquisitions in all stereo/UI chains before EndSession, with bounded retries and host exit on failure. Logs distinguish accepted, submitted/reused, retired, expired, invalidated and image-wait counts. No performance or latency guarantee exists.
- Valve's wineopenxr bridge is a real Proton route, but headset runtime/container/prefix integration is not validated here. No runtime configuration was modified.

## Validation performed

- x86 proxy/adapter and x64 host/official loader compile and link using pinned MinHook/OpenXR dependencies.
- Offline checks cover independent release-gated triggers, tracking-loss/wheel cancellation, native forward-motion basis after snap turns, rigid transforms, stance-height sentinel rejection, body-motion-invariant muzzle offsets, head-bound IPD preservation, asymmetric projection and BGRA/RGBA channel conversion. Frame checks cover immutable pair identity, delayed Ready, cancellation ownership, epoch ordering/exhaustion, cached-index acquisitions and wait retries.
- Both cross-compilers emit identical IPC layout bytes; required decorated exports exist in the fingerprinted game. PE machine types and all proxy entry exports are checked. Static GCC/winpthreads linking eliminates additional MinGW runtime DLL dependencies.
- Terra reviewed native ABI/input/ownership; findings and dispositions are recorded in NATIVE_REVIEW.md. A second focused native canvas audit records binding facts in RENDER_AUDIT.md. Astra's xhigh design critique preceded implementation; focused anchor/transport amendments and final Terra integration reviews are recorded with dispositions. See COMPLETION_AUDIT.md.
- Package/installer checks operate on hashes and synthetic fixtures or a read-only preflight. They do not run the game. Artifact details: artifact-verification.json.

## Current multiplayer interval

Wire version4 uses native reliable full snapshots, consumption ACKs and bounded credit. The verified CSimulation/entity-manager boundary freezes every bound player before any weapon entity steps, and retires consumption after entities, scripts and physics. Original muzzle references are evaluated at native firing time; native inventory/ammo/cadence/damage remain authoritative. Remote tool subtrees are identified by native instance, filename ID and body ancestry, with affine deformation preserved and one frozen stereo snapshot. Head poses replicate as data; remote head-bone retargeting is implemented but opt-in/default-disabled pending complete native lifetime evidence. Relay presentation is lossy and stale data suppresses only affected hands. Final credit/tap corrections are integrated and checked offline.

## Native optic/head follow-up

Astra signed off on the shared muzzle dispatch: accepted VR snipers use the base native gun attachment once; rejected/nested contexts stay native and unretargeted. One validated local/authority context is retained through getter and retargeting, including post-getter handle/lifecycle guards. Explicit local laser snapshots use the same dispatch. Native zoom damage/callbacks and IPC6/wire4 are unchanged; helper tests do not establish runtime aiming equivalence.

The pinned owned-mesh audit proves a circular geometric cap candidate, not authored lens semantics. Astra selected native capture/textured apertures as the minimal next scope design, with explicit placement/cap, weapon-depth, color-stage and per-hand zoom-authority gates. Full scopes remain unimplemented. The remote head audit identifies native model-local skinning and instance/bone mapping, corrects file ordinal/runtime ID and bone-record indexing, and records retained-pointer lifetime plus conditional GPU/CPU consumption requirements. The subsequent0.2.4 checkpoint implements opt-in Head animation under the Astra-reviewed producer/lifetime guards; complete remote-head equivalence is still gated.

##0.2.4 head and remote-pair checkpoint

Added an explicitly opt-in/default-disabled native remote Head/owned-descendant renderer palette adapter. The producer refreshes temporary matrices before both CPU and GPU skinning; canonical animation matrices remain unchanged. Full XYZ/quaternion delta preserves native animation/stretch/shear. This is not anatomical IK or complete remote-avatar equivalence. Core main-thread/owner gates detect unsupported scheduling; unhooked native worker/model/deletion lifetime remains unproven, so shipped RemoteHeadTracking=0.

Remote hand presentation now shares immutable per-pair admission, local invalidation history, and binding/MP guards held through Ready. Astra caught and approved fixes for native-only fallback after remote threading failure, stale server revision composites, unrelated unskinned draw rejection and producer exception/reentrancy handling. Nine offline groups pass; x86 proxy/server and x64 host/loader build; IPC6/wire4 and four static internal entries are verified. No runtime/network session or installed-file mutation occurred. Other full-immersion requirements remain unfinished.


## Physical gun follow-up

Native owned XR gun rendering now uses the actual world execution raw projection, full view matrix and depth range, preserving the original by-value camera for authored model placement. This removes the stock gun-specific near/far and foreground depth compression from the admitted XR gun transaction. Successful tracked hand placement shares the same admission; lost/nonfinite placement suppresses draw. Exact caller gates and scoped nested suppression preserve native calls outside that transaction. Missing native restore is repaired through native callbacks/cache invalidation, and a sticky fault prevents stereo Ready. CPU cache and offline clip-depth checks do not prove actual GPU wall occlusion, near-face clipping or playability. Astra source/follow-up GO and packaging are recorded separately. Scope resource/section association and native overlay boundaries are statically traced, but immersive scopes and complete overlays remain unimplemented.

## Current0.2.6 checkpoint (supersedes historical status above)

Matching IPC7/wire6 host/game/server products include independent action activity/stream generations, fresh release/ACK admission, passive scope observation, corrected native alternative route, and Astra-approved mounted tracking/control integration. Seated full-6DOF rig uses native body/eye anchor, independent eyes and captured right-hand aim through original native controls. Native vehicle physics/firing/replication are preserved; remote riders share the native anchor. Wheels and handheld mutation/presentation are suppressed while mounted. Final cross-build, all11 offline groups and artifact/ABI verification pass;38 exported/5 internal seams. Source acceptance/details: MOUNTED_ADAPTER_SOURCE_GO.md.

Handheld lasers are implemented; actual native vehicle-muzzle lasers are unfinished. Roomscale body collision, native zoom/optic images, full native overlays, physical swing melee and default-enabled remote-head lifetime/support remain unfinished. All runtime/headset/network behavior remains unverified by user instruction. Earlier0.2.5 and other archives remain immutable.

## Current0.2.7 world-marker checkpoint

Native navigation beacons and world objective markers now draw inside each actual eye's completed Render3D/readback interval, using copied executed native view/full projection, original fade and original draw helpers. No retired-root activation, parent replay, listener mutation, new hook or protocol change. Native thread/frame/rider/world-info/drawport and actual surface/depth/viewport checks reject incomplete phases/pairs. Desktop/fallback remain native. Astra/xhigh bounded source GO, final full cross-build and all12 offline groups pass;38 exported/5 internal seams, IPC7/wire6 unchanged. See NATIVE_WORLD_OVERLAY_SOURCE_REVIEW.md and world-marker-source-checks.json. The new0.2.7 package includes this integration;0.2.6 predates it and remains immutable.

World markers do not implement native flat conversation/boss/death/score/fade panels. Investigation proves RenderOverlayFadings updates native timing and that transparent HUD capture needs a separately verified alpha path; per-eye native overlay replay is unsupported. Vehicle origin investigation is corrected: owned Cannon.mdl exists in All_PC_02.gro, but actual authored blast attachment/executor placement is not yet decoded/proven. These are evidence gates for remaining work, not implemented lasers/UI. Full immersive goal remains active and incomplete; no runtime launched.


## Current0.2.8 turret laser checkpoint

Implemented the Astra/xhigh-reviewed minimal native turret aiming laser in the existing query/sample/frozen-pair/draw path. The exact audited turret table2B39E8 supplies original selected attachment, explicit existence, original purpose1 world origin and reported shoot direction. Native main thread plus actual idle model scratch guards protect the destructive query boundary; active resource/config COW, model, rider/seat, action, attachment, mechanism and request changes reject stale sampling/publication/freezing. Collision excludes the actual vehicle mechanism; no vehicle sample reuse, gameplay/fire rewrite or protocol change. One right-side native mounted beam; both native fire commands stay original.

Astra design/source GO, full x86/x64 cross-build, all13 offline groups and refreshed artifact/compiled caller checks pass. Meaningful portable checks cover actual production pose/scratch/frozen-source helpers; they do not execute native callbacks, COW, missing-attachment lookup or native gameplay. Manual receiver/buffer provenance is recorded for actual compiled hashes. See NATIVE_VEHICLE_LASER_SOURCE_REVIEW.md and vehicle-laser-source-checks.json. New separate0.2.8 package; older archives immutable.

Full equivalence remains active: broader vehicle implementations, roomscale body collision, native zoom/optic images, complete flat overlays/full-field fades, physical swing melee and complete remote-head lifetime/enablement remain unfinished. Exact source acceptance does not prove query opportunity, moving freshness, appearance/performance or unknown unhooked-worker concurrency. No runtime/headset/network session or installed-game changes. Additional native UI inspection proves conversation Render mutates texts/time, and the current Ready commit precedes original flat overlay; any capture must preserve once-only native ownership and immutable correlated images. NATIVE_FLAT_UI_ONCE_OWNER_FOLLOWUP.md records evidence, not an implemented UI adapter.


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

## Scope source follow-up after0.2.11

Astra/xhigh SOURCE GO: existing native scope observer now uses one exact draw
selection to copy selected surface counts/channel descriptors alongside the
tracked full affine. A portable cap parser and private stock-asset verifier
preserve actual lens triangles and mirrored/stretch geometry. All16 offline
groups and full cross-build pass. This is diagnostic source/owned-byte evidence;
no live buffer reader or magnified lens renderer is enabled. `contentVerified`
remains false. The0.2.11 package is unchanged. See SCOPE_CAP_STAGE_REVIEW.md and
scope-geometry-source-checks.json. Full immersive equivalence remains unfinished.

The subsequent Astra/xhigh source follow-up admits actual live cap geometry at
the existing exact native DIP. It requires complete draw interception, current
physical raster/affine, exact static managed COM bindings and four owned-slice
hashes. Explicit native-finally lock ownership contains failed/indeterminate
unlock by stopping draws until process termination; managed-buffer uncertainty
is never reset-cleared. Recognized rejected Scope passes retire geometry for
that invocation; unrelated surfaces preserve it. `contentVerified` remains false.
All17 offline groups, full cross-build, owned cap, artifact and compiled ABI/
finally checks pass. This integrates geometry admission, not magnified images.
See SCOPE_GPU_GEOMETRY_REVIEW.md and scope-gpu-source-checks.json. No runtime
launched or installed files changed; immutable0.2.11 archive remains unchanged.
