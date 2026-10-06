# Mounted adapter source review brief

Goal: source-level review of the minimal mounted VR adapter selected in MOUNTED_ADAPTER_REVIEW_DISPOSITION.md. Solo-user mod; preserve native controls, physics, RPC and render collection. No game, host, Wine, headset or network execution is authorized. Full vehicle parity is not claimed: actual vehicle-muzzle collision lasers remain a separate requirement.

## Environment and evidence

| Fact | Evidence/status |
|---|---|
| Isolated repository, native x86 hooks and x64 OpenXR host | [verified: AGENTS.md, CMakeLists.txt, installed-build.json] |
| Behavioral reference and exact native rider/body/view/clamp call chain | [verified: NATIVE_VEHICLE_CONTROL_AUDIT.md, NATIVE_MOUNTED_ADAPTER_BRIEF.md and native static audits cited there] |
| Minimal design conditional GO | [verified: Astra/xhigh MOUNTED_ADAPTER_DESIGN_REVIEW.md and disposition] |
| Current production write set | [verified: git diff: src/game/engine.cpp, src/game/remote_render.cpp; new common/rider.hpp and game/native_tracking.hpp] |
| Tracking anchor | [verified: nativeTrackingAnchor borrows existing purpose-0 view position/height; seated body quaternion comes from GetAbsPlacement; live rider checks surround getters; invalid native seat ID is loaded from the Core export] |
| Mode transitions | [verified: existing Snapshot/control cache now captures player/ride/state/seat, tracking generation, input and origin/turn; changes reset existing gates/wheels/calibration/origin/generations] |
| Commands and mounted aim | [verified: currentControls guards all cached values and edges; compatible newer input is allowed for mounted controls; mounted fire bypasses handheld ACK only; exact F3255 clamp caller uses captured right aim through native quaternion-to-Euler and original clamp] |
| Avatar and gun rendering | [verified: exact 943B6 root query returns first-person only in local seated XR eye; native FDA0B gun-injection query and desktop paths stay native] |
| Remote tracking | [verified: Binding includes rider identity, uses shared anchor, and suppresses hand subtree adaptation when mounted; IPC7/wire6 unchanged] |
| Authority/handheld isolation | [verified: non-handheld outgoing/frozen packets clear all weapon intents and equip requests; equip/dual/native fire/muzzle/calibration paths require fresh handheld rider identity, with post-native-getter checks] |
| Compilation/checks | [verified: final full x86/x64 cross-build and eleven offline groups pass; mounted-artifact-verification.json and both mounted compiled-ABI records match products. New full rotated-seat 6DOF/stereo and coherent cached-transaction regressions pass] |
| Runtime correctness | [unverified: testing explicitly excluded] |

## Decisions to audit

1. Is the two-hook mounted view/control implementation narrow and necessary, and does it preserve existing native rendering/physics/RPC? Lean yes. Rejected class registries, bit4 admission, seat attachment queries, new state machines and camera-field writes.
2. Does one copied native command/aim transaction survive coherent newer input while rejecting mount/seat/session/reference/generation changes at each use? Lean yes. Incompatible command queries now suppress both active and release edges. No new input queue or pose bank.
3. Are native mounted primary/secondary commands independent of handheld ownership/ACK, while handheld mutation and presentation are suppressed at every authoritative/local callback boundary? Audit lifecycle changes during native callbacks, copied request/fire intent, pointer validity and generation changes. Lean yes, pending review.
4. Does the borrowed native body pose remain safe/fresh for both local and remote rig reconstruction, including desktop restore and stereo commit? Audit unset/fallback handling and body/renderable lifetimes. Do not turn this slice into a new general lifetime framework.
5. Is clamp ABI/ownership/caller scoping correct? Native original clamp is always invoked, and bounded lifecycle fallback restores original input through current native limits only while brain remains live. Verify whether that fallback is necessary/correct rather than blessing it automatically.

## Review contract

Read-only production/source review, no edits or subdelegation. Main is freezing production during review and will work on compiled ABI verification and documentation independently. Audit the real implementation against the native evidence, then prioritize defects with file/line evidence and a concrete minimal fix. Challenge unnecessary architecture and identify deletion/simplification opportunities. Scope is the mounted diff and directly affected consumers; do not re-review the completed transport/physical-gun implementation, optics/roomscale/melee, or optional remote-head enablement except where this diff changes them. If evidence is missing, state that rather than broadening. Return at most 1500 words: GO/conditional GO/NO-GO, prioritized findings, unmet design conditions, verification actually performed, assumptions/open risks, and follow-up. No full immersive completion or runtime claim.
