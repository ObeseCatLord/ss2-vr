# Serious Sam 2 OpenXR VR

## Current publication authority — explicit renewed user approval
The user explicitly approved publishing SS2 VR source and sanitized reverse-engineering notes to public ObeseCatLord/ss2-vr, with strictly empty author/committer emails and no game assets, binaries or credentials. This delivered approval supersedes the prior publication hold below. Regular bounded source checkpoints are authorized; releases/binary publication are outside this renewed source-only scope. Keep Actions disabled, workflows absent, full-history privacy verification and all private runtime data local.

## Historical PUBLICATION HOLD — superseded by delivered approval
This historical hold blocked public publication pending renewed confirmation. It superseded earlier authority at that time; the explicit delivered approval above now supersedes it. During that hold, local development/tests continued and pushes, public PRs/releases and external publication were prohibited. Local commits may be preserved with strictly empty author/committer emails; Actions remains disabled and workflows absent. Explicit renewed user confirmation has now lifted the hold for source and sanitized notes only. Private game assets/captures, original checkout/WIP, installation and user saves/settings remain protected.

## Current PC continuation — start here
The latest user instruction delegates individual weapon-firing tests to the user
in VR. Do not run more individual weapon-firing probes. Continue remaining
implementation and other authorized tests, then supply a comprehensive in-VR
acceptance procedure. Existing firing-probe source and historical records are
supporting tools/evidence, not instructions to keep executing those probes.

The user's 2026-10-07 authority permits PC development, additional sessions and runtime testing where feasible. It supersedes older root-only, transfer-only, pause and no-runtime instructions. Read docs/CLOUD_CONTINUATION.md and docs/IMPLEMENTATION_STATUS.md; docs/HANDOFF.md is historical evidence, not a pause instruction. Use this isolated sanitized latest-source checkout. Preserve the old installed-game checkout, rejected melee patch, user changes and old ancestry; never merge/reset it into this checkout. Cloud source edits have stopped. One main integration owner coordinates disjoint workers and publication.

## Scope and authorization
Multiplayer with other users of this mod is an explicit requirement. The user also explicitly authorizes development and testing for online play. Use controlled/private multiplayer tests and preserve existing user saves/settings/profiles. The opt-in skip-online-init adapter applies only to the isolated single-player fixture; it is not the production multiplayer policy.
Build native OpenXR VR for Windows and Linux/Proton with independent 6DOF hands/HMD, dual wielding, per-hand weapon wheels, lasers, magnified scopes, comfortable threshold-follow UI and multiplayer. Teleport is excluded. Runtime testing on actual hardware is authorized where feasible; startup logs/fixtures are not headset/network acceptance. Wheeled vehicles with positively verified driver-wheel geometry require one-/two-hand grab-and-turn steering and smooth hand transfer. Preserve joystick fallback, native physics/throttle/aim/fire and ClientAction transport. Squeeze availability requires explicit release/rearm; inactive is not released. Tools may be installed as needed.

## Architecture discipline
Preserve Serious Engine 2's rendering, simulation, inventory, ammunition, cooldowns, damage, projectile, save and AI behavior. Prefer a narrow hook/adapter over replacement. Compare incremental designs to rewrites before changing architecture. No invented offsets, fake stereo, headset-as-mouse aiming, shared weapon aim, or duplicate gameplay state machines. Record ABI evidence and binary fingerprints. Unknown compatibility must be stated, not disguised as support. Pause architecture changes when they duplicate existing policy or exceed the reviewed estimate.

## Repository and safety
Never commit proprietary binaries/assets/disassembly, telemetry or credentials. Private analysis stays outside the repo. Stage runtime files reversibly, back up files/settings/saves before mutation and refuse unrelated collisions. Prefer isolated game/prefix copies. Regular bounded anonymous source publication is authorized; Actions stays disabled and workflow files absent. Use exact PIDs for process operations. No security/credential changes, irrecoverable cleanup or destructive history/config edits without specific approval.

## Evidence and completion
Native built-in dual wield compatibility is required, preserving actual eligibility and mechanics. Verify supported combinations in direct-launch actual gameplay: per-hand identity/pose/aim, independent release and simultaneous fire, equip changes, native ammo/reload where applicable, per-hand scope ownership, lifecycle interruption/death/respawn and multiplayer. See docs/RUNTIME_ACCEPTANCE_MATRIX.md.

Gameplay acceptance requires genuinely distinct per-eye native world rendering, tracked head translation and rotation driving the native camera, correct FOV/parallax and coherent hands/origin/body collision. Menu quads, duplicate flat gameplay images and synthetic/submit-only probes are intermediate results. See docs/PC_GAMEPLAY_READINESS.md for the urgent active rendering gate.

Read current continuation/status, relevant exact gate docs, FINAL_PLAN.md, RESEARCH.md and MODLOG.md before substantive changes. Distinguish written, built, static and runtime verification. Build x86 game/server and x64 host/loader separately. Rebuild all products after any src/cmake/CMakeLists edit before packaging; check compiled fingerprints and IPC10/wire6 layouts. Inactive helpers, counters and mocks are not completed features. Fail closed on unknown binaries. Gameplay mutation stays on the simulation thread; rendering must not advance it. Never activate handoff/unaccepted-melee.patch: it can throw carried objects. Native160/348 remain manual-only.

## Ownership and review
Additional sessions/delegation are authorized. Before spawning define objective, exact scope, read-only or disjoint write ownership, bounded output, evidence/verification and failure path. Workers must not revert others' edits. Keep one integration owner and no competing publishers. Use Astra explicitly at xhigh or higher for investigations/reviews and verify effective current-turn model/effort; resumed overrides can change. Keep final architecture/integration in the main session. Close completed agents. Historical reviews certify only their original scope. Record bounded design, evidence, regressions and unresolved gates; reopen designs contradicted by evidence.

## Local validation and runtime safety
Local/private runtime testing may precede full feature completion. Preserve original user data and reversible backups. Inspect hardware/runtime and input/foreground availability before driving; do not interfere with active user input. Do not blindly change runtime registration, permissions or security settings. Actions remains disabled; checks run locally.

The user explicitly approved brief SS2 game focus and simulated Monado testing on 2026-10-07; those questions are resolved. Required device targets are SteamVR + Steam Frame and Envision/Monado + Bigscreen Beyond, on Windows and Linux/Proton. Simulation acceptance is separate from each actual device/platform. Beyond controller profile depends on the user's controllers. Use isolated/per-process runtime setup; never pass a native Linux runtime manifest to the Windows loader. Preserve CPU D3D9 readback into the separate D3D11 host. Do not change global runtime defaults to perform these tests.

## Build conventions
C++20, explicit x86 calling conventions at the engine boundary, fixed-width pointer-free IPC structures and bounded waits. Use pinned upstream dependencies with license notices. Use rg for searches. Test behavior and protocol invariants, not copies of implementation. No TODO, stub or guessed ABI may be represented as a complete feature.

## Publication metadata privacy
Public commits must not expose personal email addresses. Use an anonymous project
author/committer name with empty email fields, and inspect the actual outgoing
commit metadata before publishing. Do not rely on a publishing API's default
author identity. Source trees may be transferred independently of commit metadata.
Never merge old email-bearing history back into a sanitized public branch.
Before any push, run `python tools/verify_publication_history.py <outgoing-ref>`
in the isolated publication checkout for every outgoing ref. It checks complete
raw ancestry with replacement objects disabled, not just the tip; it rejects
shallow histories and annotated tags. Inspect source contents and messages
separately. Never bypass a failure by limiting the gate to a revision range.
