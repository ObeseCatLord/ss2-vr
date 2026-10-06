# Vehicle muzzle laser: minimal native adapter decision

Main mostly-worked design; not reviewed or implemented. Goal: actual collision-
terminated native vehicle muzzle laser, preserving mounted tracking/control,
native fire/physics/RPC and the existing handheld lasers. User excludes runtime
testing. Solo-user mod; no new protocol, graphics layer, firing or idle-action
state machine. Review this boundary only; main owns source/integration.

## Environment and verified facts

| Fact | Bounded evidence |
|---|---|
| Query/presentation seam already exists | [verified: source] engine.cpp trackedRayInit accepts only Sam21AE69 native LOS setup return, simulationThread, no active eye, exact Requested/fresh immutable input/frame. Handheld original shooting placement already evaluates native attachments here. freezeLasers freezes once for both eyes; existing root drawLasers retains native graphics/depth and sticky pair guards. |
| Mounted identity and rig already integrated | [verified: source/previous acceptance] current Snapshot.rider and nativeRiderCurrent, nativeTrackingAnchor, frame generation, native resolve/getMechanism helpers; mounted transitions advance/reset existing rig/cache. Native vehicle controls/firing and multiplayer stay native. |
| Actual original vehicle shot origin | [verified: native] OnAnimEvent875E0 resolves shooter1D4 then v1D0; identified CShooter table2B2398 v1D0=160EE0. Executor obtains owner vAC purpose1 at160F95 and v5A8 exact shoot target at160FCB, then native fire. Turret table2B39E8 vAC=GetViewOrigin8EFF0, v5A4=GetShootDirection155CE0. See NATIVE_TURRET_EXECUTOR_FOLLOWUP.md. |
| Native active and idle attachment selection | [verified: native] owner v550=GetShootingOriginAttachment8EE70. Valid action IDs1BC/1C4 select native resource294/process/blast field4; otherwise exact native8EF05..8EF22 resolves Barrel01 through original string-ID API. No file ordinal or invented idle selector is needed for original single origin. Active getter can COW47C; idle branch skips it. |
| Purpose1 incorporates original attachment | [verified: native] original GetViewOrigin vAC/purpose1 calls v550, resolves model-renderable handle120, calls original GetAttachmentAbsolutePlacement8F20D and reconstructs original QuatVect. It may COW resource/other caches. Sam whole body8EFF0..8F6F9 has ret8. Original native implementation remains the pose reference. |
| Explicit missing-attachment success test | [verified: native] Engine mdlGetAttachmentAbsolutePlacementD8FA0..D908F is cdecl int(model-instance*,runtimeIDENT,Matrix34&). Returns0 on missing lookup,1 after matrix copy. CModelRenderable GetModelInstance15B220 is the existing remote adapter API. No invented aggregate ABI. |
| Getter collateral state is real | [verified: native] mdl getter can COW model-config18 and temporarily owns/restores native child-traversal flag2C7F9C throughE28B0/DAD90. GetViewOrigin/selector are not arbitrary-thread pure peeks. Existing main-thread/query placement reference is reusable, not evidence for a replacement record/model subsystem. |
| Owned asset supports authored default | [verified: archive] stock Cannon.mdl and Turret_Cannon.ep exist in All_PC_02 with recorded hashes/Barrel01 name-table entries. Current scope does not assume name alone establishes presence in a loaded instance; actual native bool lookup must succeed. |

Runtime phase availability, appearance and moving-vehicle accuracy are [unverified].
Complete unhooked model-worker concurrency is [unknown] and the separate optional
remote head adapter remains disabled. Inspect enough exact native getter/record
ownership to decide this addition; do not use unknown alone to demand replacement.

## Compare designs

A (main lean). At the **existing simulation/query seam**, resolve the current ride
and original selected attachment through its native virtual550. Resolve its
current model-renderable120 and instance; explicit original mdl bool lookup
must succeed. Obtain the ride's **original virtualAC purpose1 QuatVect** and
**original virtual5A4 shoot direction**. Preserve native origin and reported shoot direction rather
than infer origin from tracked hands, Gun/GunUD body or a guessed tip offset.
Align the presentation orientation minimally with the returned direction,
retaining native roll; actual ray uses native direction. Revalidate live local
frame/rider/ride/seat/model/selected attachment around each native getter. No
Snapshot lock across calls. Borrow pointers only during the query; stage values
and publish only after final continuity checks. Exclude current player and
actual vehicle mechanism from the existing native bullet-category ray. Native
ray cleanup remains once on all supported exits. No damage/fire function called.

Freeze the same local LaserAim/LaserFrame pair once for two eyes. Extend those
existing private structs with explicit source kind/current rider/model identity;
no IPC/wire change or parallel cache. Rename their legacy weapon identity to
source identity within this module if needed. Handheld eligibility stays intact;
mounted source is the ride handle, not a stale handgun. One right-side vehicle
beam represents the native mounted aiming source; both buttons retain original
native firing. This is not invented dual vehicle weapon selection. If native
origin/attachment identity is unavailable or incompatible, no vehicle beam.

B. Directly convert the original attachment Matrix34 into a rigid pose, avoiding
the second placement evaluation. Fewer getter calls, but discards the actual
purpose1 reconstruction/clamp/offset policy unless equivalence is proven; no
evidence justifies replacing that adjacent working calculation. Ask reviewer
whether the matrix already fully matches and B is the smaller correct adapter.

C. Pure custom resource-table/idle selector plus body-tip offset. Reject: duplicates
original action selection, creates asset/version policy, and body origin is not
the proven muzzle. Calling native firing to discover it is also rejected.

Reusable: native origin/attachment/shoot-direction methods, existing query/ray
phase, mounted/frame identity, private laser sample bank, pair freeze/native draw
and native mechanics. Added state is only necessary sample provenance in the
existing structs. No other service, queue, transport, renderer or gameplay policy.

## Smallest proof and depth budget

Expected100–160 native/helper lines plus meaningful portable regressions and
bounded compiled caller checks. Smallest source proof is live mounted query ->
successful real attachment/native origin/direction -> native collision -> staged
identity-matched sample -> existing frozen two-eye native line draw. Tests must
exercise actual production continuity/pose helpers, seat/model/ref changes and
failure after native callbacks; cross-build all products and audit new cdecl/
hidden-return thiscall calls. No runtime claim, partial fallback or counters as
proof. Source review follows implementation.

Astra/xhigh read-only contract: independently verify load-bearing claims and
prioritize actual bugs/necessary architecture; seek deletion/simplification.
Choose A/B or concrete narrower fix. In particular decide exact query/getter
record ownership and native origin/direction fidelity; label unknowns. Scope:
above source laser/mounted helpers and exact fingerprinted native APIs; do not
re-review transport, markers, optics, roomscale, flat HUD or remote head policy.
No edits/delegation/game/host/Wine/XR/network execution. Return full final critique
<=1100words with file/native-RVA evidence, verdict and narrow conditions. Missing
evidence means a bounded missing proof, not a broader redesign/human permission.
