# Paused project handoff — 2026-10-05

## Start here

The user explicitly paused implementation and requested this documentation
handoff. The goal is **paused, incomplete**. Do not resume implementation,
investigation, builds, packaging or deployment until the user resumes it.
Both outstanding read-only Astra reviews completed and their agents were closed;
their final findings are recorded below. No agent owns an outstanding write set.

Repository: `ss2-vr`, branch `master`. Source checkpoint at pause: **828b413**,
`Record saw receipt boundary and reopen roomscale query ownership`.
The documentation-only handoff commit follows that checkpoint.

Read this file and [AGENTS.md](../AGENTS.md) first, then the bounded references
below. [IMPLEMENTATION_STATUS.md](IMPLEMENTATION_STATUS.md) and
[MODLOG.md](../MODLOG.md) contain chronological checkpoints; older counts,
limitations and “active” statements describe their dates, not current acceptance.

## Intended result and constraints

Build a native Serious Sam 2 OpenXR VR mod comparable in interaction to Serious
Sam VR: The First Encounter: proper six-degree-of-freedom HMD and independently
tracked hands/guns, stereo/asymmetric eyes, independent dual wielding, convenient
per-hand weapon wheels and collision-terminated aiming lasers. The extended goal
includes roomscale body movement/collision, immersive sniper scopes,
mounted/vehicle controls, physical melee, complete VR menus/overlays and
multiplayer VR replication including remote weapons and head animation.

Teleport is explicitly excluded. Multiplayer is required. UI must have
comfortable physical size and follow only after a head-turn threshold. Native
guns retain authored animated hands/arms; articulated fingers/full-body IK are
not implemented.

Actual game/headset testing is excluded by the user. Do not launch the game,
native Windows host, Wine, OpenXR runtime or multiplayer session. Portable
checks, cross-compilation and owned static binary inspection are allowed when
work resumes. Those checks are not runtime verification.

Use **Astra for all investigation and review**, explicitly configured
`gpt-6-astra`, xhigh or higher (Max preferred for senior design decisions).
Verify effective settings on each current review turn: a resumed agent can lose
its override. Define bounded delegation contracts; final architecture and
integration stay with the main agent. Preserve native renderer, physics,
inventory, ammo, combat, RPCs, saves and AI. Prefer minimal adapters; do not add
guessed offsets, spare command bits, replacement controllers or duplicate state
machines.

Do not change installed game files/config/saves, deploy, publish, replace immutable
archives or export raw session telemetry. Full tool access does not authorize
destructive cleanup or history/config edits. The previously discussed daemon
workaround was not removed; the user withdrew that request.

## Source, package and worktree boundaries

| Layer | Current state |
|---|---|
| Accepted source | Scoped Astra approvals, including the five-site native primary join at `4baa4e8`, angular scope helper at `53909e6`, and observer pulse sender correction at `6acfdc7`. Later commits through `828b413` record investigation/gates. |
| Archive | Immutable `dist/ss2vr-0.2.11-dev.zip`. SHA256 `e72d51f0c57f3bc788dca969fd9e83c05ede1c4a4eeeee98509a8ed016e335ba`. Predates later accepted source and physical-melee work. |
| Unaccepted worktree | Physical-motion/logical-primary integration compiles and passes 25 portable groups, but native gesture/carry history is **NO-GO**. Not packaged. |
| Build outputs | May include unaccepted worktree code. Structural checks do not make them releasable. |

Uncommitted source at pause, deliberately preserved:

```text
 M CMakeLists.txt
 M src/common/controls.hpp
 M src/game/engine.cpp
?? src/common/gameplay_primary.hpp
?? src/common/physical_motion.hpp
?? tests/gameplay_primary_checks.cpp
?? tests/physical_motion_checks.cpp
```

Do not revert, discard, commit or package these changes merely to clean the tree.
They are separate from the documentation handoff.

## Implemented and remaining

| Area | Existing implementation | Remaining goal work |
|---|---|---|
| OpenXR / rendering | x86 D3D9 proxy/game/server hooks, x64 OpenXR D3D11 host and official loader; real HMD/hand XYZ and quaternion tracking, asymmetric independent eyes, shared correction, deep crouch; native authored gun models with tracked placement and native projection/view/depth. | Classified render/effect compatibility and final integration; no runtime verification. Bounded head translation is not body collision. |
| Weapons / lasers | Independent native weapons (including identical types when stock combo capability is enabled), per-hand selection/wheels, native damage/ammo/cadence and release gating; handheld collision lasers and exact audited mounted turret beam. | Physical melee reconciliation and remaining vehicle muzzle/laser families. |
| Multiplayer | Pose/authority, remote weapon presentation, per-hand epochs/credits, retained-shot zoom context, dedicated server source; accepted sender pulse correction below. | Receiver overwrite/transport loss and native observer consumption for short melee input; remote head lifetime/enablement. No session tested. |
| Roomscale | Bounded inactive model-triangle adapter with 446 helper checks and certified facing/order. Experimental head fade disabled by default. | Public query ownership/cleanup, primitive initial overlap, actual body geometry and finite movement, placement/replication, accepted-delta/origin settlement. No body-follow movement implemented. |
| Scopes | Independent native sniper zoom activation/timing/damage/cleanup; actual geometry/UV/color observations and angular magnification math. | Native magnified image capture/aperture replacement/color equivalence. No magnified lens image rendered. |
| Mounted controls | Native six-DOF seat anchor, right-hand look/clamp, movement/fire controls and replicated rider anchor. | Broader vehicle coverage beyond the audited turret and existing mounted path. |
| UI / overlays | Native once-only flat HUD/messages/death/scores/fades in both eyes, world markers and controller menus. Default panel 2.2m wide at 2.2m; follows after 35°, stops within 8°, capped at 90°/s. | Netricsa/cinematic and unsupported overlay/device/program paths, complete integration. Older status sections predate connected native UI. |
| Remote head | Native affine/palette adapter preserves native animation and tracked XYZ/quaternion about body eye. | Worker/model/palette lifetime proof and default enablement. Currently opt-in/off. |

References: [IMMERSIVE_PLAN.md](IMMERSIVE_PLAN.md), [FINAL_PLAN.md](FINAL_PLAN.md),
[HEAD_ANIMATION_REVIEW_DISPOSITION.md](HEAD_ANIMATION_REVIEW_DISPOSITION.md),
[SCOPE_IMAGE_DESIGN_REVIEW.md](SCOPE_IMAGE_DESIGN_REVIEW.md).

## Last accepted implementation: observer pulse sender

Commit **6acfdc7** fixes a relay loss in `src/common/network.hpp` and
`src/game/multiplayer.cpp`: `OrderedPosePolicy::freeze` expanded an accepted pulse
into the authoritative sample and cleared the active pulse; `freezeInput` then
relayed the cleared active state. A short pulse could disappear without transport
loss.

`freeze` now optionally returns the same frozen legal relay sample, separating
ordinary fire from historical pulse without inventing ordinary fire or
physical-down state. Accepted pulses bypass the existing ordinary 50ms throttle.
One-use consumption, same-tick cache, binding, mapping and nonces remain intact.
No wire/ACK/credit version change.

Production freeze-to-codec checks cover two-hand released pulses, no replay,
ordinary/pulse combinations, expiry, historical zoom and revoke between
receive/freeze. All 25 portable groups, network UBSan, cross-builds and structural
artifact checks passed; Astra/xhigh approved this bounded change. Latest-value
receiver overwrite, unreliable loss and native observer consumption remain
unfinished. See [OBSERVER_PULSE_SENDER_SOURCE_REVIEW.md](OBSERVER_PULSE_SENDER_SOURCE_REVIEW.md).

## Physical melee stopping point

Current WIP combines gesture/manual primary history. Gesture-only high can set
native player `+348`; later carry resolution at `+564` with manual low can reach
`ThrowObject`. Cached prior-input rebasing does not repair that interaction.
Native current `+160` and previous `+348` must remain manual-only. The accepted
five-site join predates gesture integration.

Astra rejected a B0/E8-only saw stop guard: a real short recoil pulse can require
canonical release while state8/sound3 makes that guard miss it. Proposed bits2/3/4
fail because bit2 is native grenade. AI hearing/dynamic replication do not supply
the missing observer transition on the audited paths.

Preferred conditional design: a consumed-logical-level subrecord per existing
native saw binding alongside existing local/Authority/Remote ownership, not a
new combat state machine or registry. Reconcile only missing canonical edges at
actual admitted native consumption; preserve bookkeeping and post-flip targets.
Known/unknown state, copy/replacement/retirement, capacity, save, nested commit
and observer eligibility still need proof. Whole-row `saveAuthority` replacement
does not preserve new metadata as-is. If accepted, remove gesture command/history
projection and carry-cache rebasing rather than retain parallel fixes.

Canonical press `7FFC0` -> `101F00`; release `7FFE0` -> `FA770`. Operator `8E580`
applies flip before dispatch. Release bookkeeping precedes saw stop. Saw release
`165490` calls base `48FF0` at `165493`, returns at **165498**, then sound call
`164F80` occurs at `16549C`. Narrow candidate: typed wrapper around original
`48FF0`, committing a prebound receipt only after successful return at `165498`.
No reconciliation hook is activated.

### Final Astra/xhigh result received before pause

The exact state4 -> state1 animation/get-idle path before B0 write is **proved
non-reentrant on the normal allocation-success path**. Unconditional non-reentry
is **UNPROVEN**: allocation failure reaches fatal exit callbacks whose targets
remain unclosed. No actual same-binding reentry was demonstrated.

Normal call family: `4A0D8` -> Engine `D1750` queue getter; `4A0E6` -> `F140`
clear-state/storage (count becomes zero); `4A0F7` saw slot218 -> `49040` literal
Idle identifier; `4A11E` -> `E7C0` play-animation append route; slot210 at `4A12B`
-> `1C3240` plain return; B0 write `4A134` or invalid-ID `4A3DB`.
Fixed storage/string/mutex paths did not reach same-saw `4EE20` or operator
`8E580`. Mutex contention is a wait, not a task pump. No resource/global-postload
edge was established in this normal closure.

Unresolved failure path: Engine `F140` -> `E8B0` -> `DC40/DC80` -> Core `211E0`
-> `20BD0` -> `209F0`; allocation-null `20A20` invokes default failure callback
`20920`, then `conFatalError` `F44F0`. Fatal exit calls `conExit(1)` via `457A`
-> `41E0`. Core **421A** invokes exit-list callbacks from **BB598** with registered
arguments. Targets/descendants were not closed against same-binding operator or
Step. Console listener work was not evidence of operator reentry.

**Next bounded proof after resume:** exact conExit registrations/fatal termination
behavior against this receipt boundary. Do not expand into generic postload
closure or claim unconditional approval. Then resolve lifecycle, manual carry
isolation and observer reconciliation before source integration.
See [PHYSICAL_SAW_DISPATCH_COMMIT_GATE.md](PHYSICAL_SAW_DISPATCH_COMMIT_GATE.md),
[PHYSICAL_SAW_DISPATCH_COMMIT_PROOF_BRIEF.md](PHYSICAL_SAW_DISPATCH_COMMIT_PROOF_BRIEF.md),
[PHYSICAL_SAW_CONSUMPTION_MAX_DISPOSITION.md](PHYSICAL_SAW_CONSUMPTION_MAX_DISPOSITION.md).

## Roomscale query stopping point

Readiness/refcount/count shortcuts were rejected: examined getters always return
one or do not clear/pin loaded resources. A whole-program postload callback audit
grew beyond the narrow ownership question without demonstrating actual nested
ray execution. Do not restart that broad audit by default.

### Final Astra/Max result received before pause

**NO-GO for proposed two-site cancellation coverage. Conditional GO for general
purpose-bound normal cancellation, subject to full cleanup proof.** Root world
and model-config sites have candidate cleanup exits; deeper model preparation
contains a concrete additional resource pump even with both root guards clear.

- World `29114` tests receiver `+4` bit0; `2911E` calls replacement slot0C.
  Mod-only cancellation could enter **29160**, with established frame and
  **EDI=0**, then native hull/visited cleanup `29170..291B9` and caller profile
  stop `29270`. This is new optional-query cancellation, not an existing
  resource-unavailable branch. Jump `29146` only skips replacement then consumes
  the resource: rejected.
- Model `DA280` config dispatch is `DA421`; **DA476** is early-miss epilogue before
  prep/mode work. Registered cleanup stays linked. Earlier hits can survive local
  misses: sticky whole-query **unavailable** must dominate publication; a local
  miss cannot become “no wall.”
- Concrete deeper path: `DA4DC` -> `E23F0` -> `E246D` -> `E1720` -> **E1810**.
  Config can replace at `E17AB`; config `+30` skeleton is tested at `E1806`, then
  dispatched at `E1810`. Skeleton vtable `214688` slot0C -> `1BF786` -> Core
  `3E560`, clears flag at `3E574`, then **3E578 -> 46AA0** pumps global tasks.
  Extra mesh/child-config replacement sites: `E1A51`, `E1F60`, `E2003`.
- During prep `DA499..DA4CB` changed three model-mode globals; `E1640` populated
  scratch counts. Normal completion calls `DAD90` at `DA4F4`, restores modes at
  `DA503..DA511`, and closes profiling. Early `DA476` is not reusable here.
  Existing `E1720` minus-one return is not certified cancellation: caller
  `E2472` proceeds using global counts.
- Lexical TLS alone cannot distinguish unrelated nested triangle work (already
  documented by the adapter). Detection after shared scratch corruption is too
  late. Prove invocation provenance and preserve ordinary stock calls unchanged.

**Next bounded proof after resume:** normal purpose-bound cancellation through
actual model-prep cleanup, starting at skeleton `E1810`; prevent only the extra
mod-owned query's pump and unwind prep/model/world ownership normally. Seek
deletion/simplification. Reopen architecture if independent cleanup/state
machinery grows; compare existing native query-owned extent as fallback.
Do not fabricate old pointers, skip replacement then continue, or publish earlier
hits after cancellation. Full arbitrary callback closure, global serialization
and replacement world/controller remain rejected directions.

After proof, smallest source slice is an **inactive** adapter with distinct
hit/miss/unavailable, not body movement. Checks must cover earlier hit then
rejection, stock dispatch arguments, pending skeleton/partial prep, compiled x86
stack/register/x87/SEH/profile/mode cleanup and subsequent normal-query baseline.
This grants no body/primitive/settlement acceptance.
See [ROOMSCALE_QUERY_REJECTION_REVIEW_BRIEF.md](ROOMSCALE_QUERY_REJECTION_REVIEW_BRIEF.md),
[ROOMSCALE_RESOURCE_ADMISSION_GATE.md](ROOMSCALE_RESOURCE_ADMISSION_GATE.md),
[ROOMSCALE_POSTLOAD_TASK_GATE.md](ROOMSCALE_POSTLOAD_TASK_GATE.md).

## Other remaining integration

Scopes require an actual magnified image, not optics helpers or observations.
Public bone queries were rejected for scratch/worker lifetime. Prior lean
direction: observe a completed ordinary first eye before auxiliary rendering;
not implemented. Native source capture must exclude guns and flare through real
caller context, preserve pre-bloom HDR/color and replace only the verified scope
aperture. Experimental image shaders remain disabled. Read scope design/cap/GPU/
color review documents before editing.

Roomscale also needs primitive initial-overlap and actual body-shape evidence;
finite-sweep/accepted placement cannot be inferred from full-volume overlap or
small-delta rays. Remote head needs worker/model lifetime closure. Vehicle/UI
coverage must reach actual native owners. Finish source reviews, integration and
docs before creating a new separate package. Portable checks alone do not prove
full equivalence.

## Verification instructions for resumption

Use explicit repository working directories. Known commands:

```sh
python3 tools/build.py --minhook-source deps/vendor/minhook --openxr-source deps/vendor/openxr --jobs 4
cmake --build build-core -j4
ctest --test-dir build-core --output-on-failure
PYTHONPATH=deps/python PYTHONDONTWRITEBYTECODE=1 python3 tools/verify_artifacts.py --game '..'
```

Focused static ABI scripts under `tools`: `verify_primary_entry_abi.py`,
`verify_zoom_entry_abi.py`, `verify_native_finally_abi.py`, `verify_scope_gpu_abi.py`,
with `--game '..'`. Accepted network follow-up also passed UBSan:

```sh
c++ -std=c++20 -O1 -g -fsanitize=undefined -fno-sanitize-recover=all -Isrc tests/network_checks.cpp -o /tmp/ss2vr-observer-relay-network-ubsan
/tmp/ss2vr-observer-relay-network-ubsan
```

These are instructions for resumption, not checks to run while paused. No game,
headset or network runtime test or installed-game deployment was performed.
Private temporary scratch/log files are not authoritative history; use committed
briefs, dispositions and source evidence.
