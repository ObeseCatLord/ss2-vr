# Serious Sam 2 OpenXR VR

## Current cloud continuation — start here
On 2026-10-05 the user resumed development and asked dot to perform it itself in
its cloud workspace. The user requires root-owned development and now requests a thorough self-review
after implementation and research into VR mods, VR implementations and SS2.
Do not delegate development, investigation or review. The local task may fetch
needed SS2 inputs and publish regular source checkpoints under the user's
2026-10-05 authorization; proprietary game inputs remain private.
Read [docs/HANDOFF.md](docs/HANDOFF.md) as the historical checkpoint, then
[docs/CLOUD_CONTINUATION.md](docs/CLOUD_CONTINUATION.md) for current work. The
original pause and Astra-review process are superseded for this continuation;
native safety, no-runtime-testing and evidence constraints remain. Preserve the
unaccepted WIP patch and original PC files.

## Scope and authorization
Build a native VR mod for the user's installed Serious Sam 2 with independent tracked hands, dual wielding and per-hand weapon wheels, following Serious Sam VR: The First Encounter's interaction model. The current extension requires multiplayer VR replication and immersive features, plus comfortable UI that follows only after a head-turn threshold. Teleport movement is explicitly out of scope. OpenXR is the required runtime API when feasible. The user expressly excludes actual game/headset testing; do not launch the game, drive input, or claim runtime verification. Compilation and offline checks remain in scope. Tools may be installed as needed.

## Architecture discipline
Preserve Serious Engine 2's rendering, simulation, inventory, ammunition, cooldowns, damage, projectile, save and AI behavior. Prefer a narrow hook/adapter over replacement. Compare incremental designs to rewrites before changing architecture. No invented offsets, fake stereo, headset-as-mouse aiming, shared weapon aim, or duplicate gameplay state machines. Record ABI evidence and binary fingerprints. Unknown compatibility must be stated, not disguised as support. Pause architecture changes when they duplicate existing policy or exceed the reviewed estimate.

## Repository and safety
Keep all work in this repository. Never commit proprietary binaries, extracted assets, disassembly dumps, session telemetry or credentials. Analysis scratch files belong outside the repository. Do not overwrite installed game files, config, saves or other mods. Packaging creates a separate staging directory. Deployment scripts must refuse file collisions. Nothing may be published without explicit instruction. Use exact PIDs for any process operation. Never use destructive cleanup or history/config edits without explicit intent.

## Evidence and completion
Read docs/FINAL_PLAN.md, docs/RESEARCH.md and MODLOG.md before substantive changes. Maintain a factual implementation status; distinguish code written, built, statically verified, and runtime verified. User's no-game-testing instruction overrides skill workflows requiring a launch or video. Build x86 hooks and x64 OpenXR host separately. Wire real paths end to end; counters, mock poses and empty callbacks do not implement VR. Fail closed on unknown binary builds. Gameplay mutation stays on the game simulation thread; render hooks must not advance simulation.

## Ownership and self-verification
The user requires root-owned work without outside reviews. Keep architecture,
implementation, inspection and conclusions with the main assistant. Do not
start coding/review workers or ask again for an independent review unless the
user changes this instruction. Transfer-only local tasks do not grant local
coding or game-execution authority.

Record the bounded design, evidence, regression results and unresolved gates.
Historical Astra decisions remain evidence only for their original scopes; they
do not certify new code. Do not infer native correctness from portable tests,
compilation or an inactive helper. Reopen designs when evidence contradicts
them, and preserve engine/lifetime/ABI requirements even without an outside
review step.

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
