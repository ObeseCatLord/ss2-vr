# Astra mounted source review: initial NO-GO

Reviewer Averroes used explicit/effective gpt-6-astra/xhigh, independently verified from minimal turn metadata. Read-only source review; no edits, delegation or runtime. Main record of returned critique; exact load-bearing source findings were spot-checked before changes.

1. P1: rider transitions cleared primary/zoom masks but retained copied and underlying equip requests. Entering handheld mode could execute an old request after dismount. Clear affected requests on every boundary and require the live existing intent epoch before mutation.
2. P2: currentControls required current fire for release queries, suppressing ordinary trigger release in mounted/single-player modes. Separate context/stream validity from active-fire admission; keep client ACK requirements and suppress incompatible edges.
3. P2: mounted clamp checked captured right hand only. A newer known tracking loss must invalidate its use without replacing the captured pose.
4. P1: local shooting and weapon-placement calibration paths still lacked post-getter rider/player/weapon/generation guards. Stage output/calibration until final validation.
5. P2: deletion of retained handgun ownership while mounted advanced tracking generation and cleared fire, allowing another native press while physically held. Clear obsolete ownership/calibration while preserving mounted command generation/fire latch.

Architecture verdict: retain the minimal two-hook/shared-anchor design; no replacement controller, seat cache, class registry, transport extension or physics/RPC rewrite needed. Root caller943B6 and gun callerFDA0B remain correctly separated; native body/view-height and remote mounted binding approach are appropriate.

The fallback second clamp call was a deletion candidate: keep only if native clamp chain can mutate lifecycle and that retry has focused evidence. Main subsequently audited exact BrainED2E0 -> puppet901E0 -> ride901B0 -> camera80580. Every exported puppet vtable's ride clamp59C resolves901B0; that function forwards the native camera324 to80580, which reads pitch limits and only writes the supplied look vector. The chain has no lifecycle/mutation/RPC operation. Main therefore removed the second clamp call rather than adding recovery policy. Audits contain no committed native bytes or disassembly.

Reviewer inspected scoped source/diff and compiled/artifact records and ran diff whitespace checks. Full build/eleven offline checks were accepted as reported, not rerun by reviewer. Runtime remains explicitly unverified, and actual native vehicle-muzzle collision lasers remain follow-up. Source enablement requires a bounded follow-up on the five fixes.
