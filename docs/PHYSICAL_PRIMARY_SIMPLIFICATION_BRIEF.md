# Physical primary: architecture simplification brief

Solo user/mod; proper swing melee, stock manual carry behavior and multiplayer
presentation are required. Actual game/headset/network testing is excluded.
No production physical-primary acceptance or package exists. Main owns final
architecture and source integration; Astra reviews/investigations are required.

## Verified environment and behavioral reference

| Fact | Evidence / status |
| --- | --- |
| OpenXR motion derives LOCAL grip XYZ plus aim quaternion; logical primary feeds existing Snapshot/packet/native five-site join | Verified current physical-primary worktree; build25 groups pass, not native acceptance |
| Physical gesture enabled only for exact stock saw vtable/weapon identity | Verified engine.cpp primaryIdentity; ordinary weapons remain manual |
| Existing native join captures immutable V per invocation and preserves native handheld history, mapping, blocking and callbacks | Accepted base4baa4e8; PRIMARY_JOIN_SOURCE_REVIEW.md |
| Gesture-only high persists in puppet348 and later manual-low carry can throw | Astra/max confirmed; production-helper portable counterexample recorded PHYSICAL_CARRY_MAX_DISPOSITION.md |
| Raw manual source is lost if original commands carry logical OR | Verified source value flow; prior manual-high and gesture-only-high collapse to combined1 |
| Command-cache prior rebasing does not repair native348 | Portable helper trace plus Astra/max native trace |
| Native constructor/copy/assignment carry348 but copied Authority18-row samples do not follow those lifetimes | Astra/max PHYSICAL_CARRY_LIFETIME_GATE.md; main source spot-check |
| Completed-store seam8E760 exists before epilogue | Astra/max exact static proof; not installed/linked verified |
| Original action and pose channels are independently ordered | Existing reliable snapshots/admission plus native action audit; no same-sample pairing proof |

Current repo is ss2-vr under the installed game; owned pinned modules are
../Bin/Sam2Game.dll, Engine.dll and Core.dll. All work stays in repo or /tmp
analysis scratch. Never launch native executables; cross-build/offline tests only.
Reviewer's effective gpt-6-astra at max must be verified using aggregate facts.

## Newly verified consumer/observer evidence

Fresh Astra/xhigh confirms OnStep order: operator atA6E03; position correction
90490 atA6E0D, healing9DF70 atA6E17, material damage9EC50 atA6E21; then carry
handler9B030 atA6E29. Stock operator down callback453D0 is ret4/no-op. Stock
pickup OnUseEA090 calls puppet slot290 atEA3E4→A2520, stores564 atA29E0 and
calls OnObjectGrab869E0. A stock pickup reentry from operator/intervening
virtuals was not established; healing macro-event closure is not complete.

Combined operator press resolves carry at8E6C0; carry logical0 prepares through
86C80, otherwise7FFC0→DoAttack101F00. Release resolves carry at8E6F6;
carry logical0 calculates ratio86AD0 then ThrowObject9AC50, otherwise
7FFE0→StopAttackFA770→weapon slot204. Blocking is native brain161, flip occurs
8E670..686. History store is8E75A. A late carry consumer tests348bit0 at9B207,
then ratio9B21D and can auto-throw1.0 at9B86C.

The candidate carry selectors are8E63F (blocked-release),8E697 (press before
timestamp),8E6EF (release), and9B207 (current manual-history bit0). These are
semantic boundaries; patch spans/eligibility/FP preservation are not approved.
Vetoing ThrowObject alone fails genuine manual release under logical-high.

Observer mismatch: native PreSend copies160→1E8 atE9579/7F, PostReceive
copies1E8→160 atEE04E/54. Field1E8 is NETUPDATE4000. Client presentation
replicas are excluded from knownVrAvatar, and remote_render grip retargeting
does not consume fireMask. Weapon B0/state,30/lastfire,38/firstshot,AC/count and
native animation queue are not NETUPDATE. Held calls4EF85/4F090→slot730→
8E480 read160. CallFireStart4CC40 and DoRecoil4D870 mutate native state and
animation; inspected base/saw range has no direct ProcessRPC import call.
Manual-only commands therefore leave gesture-only remote weapons idle in the
inspected replication path. Dynamic entity classification/projectiles do not
prove native firing-state presentation.

## Architecture decision reopened

The preferred minimal candidate from the first Max review was manual-only
original commands plus two raw-index history bits at native completed store,
with existing logical pose projection for handheld use. Evidence now expands
it to lifetime/copy/registration ownership, four carry selectors and a separate
observer presentation projection. This materially exceeds the earlier five-read
join estimate. Do not implement layers merely to make this candidate pass.

Current lean: preserve original commands/RPC, native combat/FSM and copy
semantics; solve the lost contribution at the narrowest demonstrated boundary.
Use existing ownership/provenance/admission. No new wire, action replay,
independently advanced history, generic renderer or global serialization.

Compare these concrete forks before selecting architecture:

1. Commit-coupled manual-history subrecord in existing Authority registry plus
   carry selectors and separate bound observer projection. All adjacent native
   systems remain; lifetime/capacity/registration and observer side effects are
   unresolved. Do not repackage these unknowns as an approved implementation.
2. Keep348 manual-only and avoid durable added history if the exact saw's
   native held/press/stop/FSM behavior can faithfully consume motion without
   combined prior history. This is UNKNOWN, not permission to synthesize edges
   or repeatedly schedule attacks. Verify native behavior before judging it.
3. Reuse native metadata that survives copy and replication only if an actual
   unused field/bit is proved end to end. Unused space has NOT been established;
   guessed bits/offsets and silently changing native protocol are rejected.

Direct DoAttack/DoRecoil invocation is not currently a valid alternate plan:
native state/continuation and remote authority side effects are unproved.
No branch is preferred merely because it is easier to mock or instrument.

## Review contract

One Astra/max reviewer, read-only. Verify before critique. Rank the forks by
actual behavioral correctness and necessity; actively seek deletion of added
storage/selectors/presentation policy. Give the smallest complete native saw
vertical and exact evidence for required seams, or identify the one remaining
fact that prevents a decision. Output <=1800words with prioritized decisions,
file/line/RVA evidence, GO/NO-GO for a concrete architecture, and clear unknowns.

Do not investigate constructor registration/handle lookup (a separate current
Astra agent owns that), scopes, roomscale, other weapons, vehicles or remote
heads. Do not redo the completed carry/observer traces broadly; audit only
load-bearing claims needed to select architecture. No source edits/delegation,
runtime tests, new native protocol or raw session telemetry. Missing evidence
must be reported without broadening. Final disposition/integration stays main.
