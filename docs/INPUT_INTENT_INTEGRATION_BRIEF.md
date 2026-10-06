# Action provenance and weapon-intent source review brief

Goal: narrow integration of the approved per-hand intent epoch/neutral design. Preserve native transport, weapon scheduler, inventory, damage and the existing pending-intent/credit policy. Main owns integration; Terra implemented only network.hpp/network_checks.cpp and has no review verdict. User requires Astra reviews and prohibits game/host/Wine/headset/network launches. Offline builds/checks are allowed. Scope is this source change, not roomscale/scopes/vehicles/head rendering.

| Environment | Verified source/artifact |
|---|---|
| Parent commit | 3a3f90c, copied-authority invalidation fix |
| Prior design | ZOOM_CAUSALITY_REVIEW.md; conditional epoch/raw-action design GO |
| Native products | x86 proxy and dedicated module; x64 host + pinned official OpenXR loader |
| Current IPC | ABI7, Input264/Request392/Shared83887640 bytes; native layout probe verified |
| Current wire | 6, all declared action/epoch fields serialized/parsed |
| Immutable package | 0.2.5 remains IPC6/wire4, unchanged |
| Native zoom | Desired zoom producer remains zero; raw zoom provenance is preparatory only |

[verified: source] Host samples primary float isActive/finite independently of hand pose tracking. Sprint[h] is the contextual zoom stream except Vive right Jump[right], chosen by opaque profile handle equality, with unknown-profile Sprint fallback. Same synchronized Sprint/Jump reads feed button state and raw zoom; raw capture precedes recenter/wheel/gameplay filtering. Per-action/per-hand ActionStream generations advance on inactivity/profile change; profile events have no affected-hand ID, so same-profile rebind events conservatively invalidate both logical streams. Ordinary profile comparison is per-hand. Exhaustion is permanent unavailable, not wrap. No bound-source/physical-button inference.

[verified: source] Local trigger gates reset on primary generation change; inactive triggers cannot fire or increment raw release serials. Actual fresh eligible raw-up samples advance serials. Input carries both independent streams through IPC; menus correlate presented pointers with the primary stream generation and reset native click gates across changes. Ray choice prefers an action-eligible hand. No zoom callbacks or movement behavior replaced.

[verified: source] Existing Local records per-hand IntentInputBoundary. Only validated current-capability consumption credit installs ACK epochs. Duplicate/retired credit cannot refresh lease or admission. Changed-hand install cancels cached/history intent without restamping raw snapshots. Echo requires both later input sequence and tickMs strictly after ACK receipt (GetTickCount64 common host/game clock), with no future timestamp. Listen host applies the same boundary at actual native invalidation. No new IPC lock on ACK path, queue or message type.

[verified: source] PeerState advances epochs on every affected invalidation, including repeated weaponGeneration; exhaustion remains zero until new capability. Epoch mismatch cannot touch release baselines. Primary/zoom activity and stream generations are independent. Retired streams cannot regress witnesses. Admission is rechecked at receive/freeze/use; native authoritativeFire also compares copied/current epochs and both admitted fire masks. Muzzle epoch admission preserves tracking independent of firing.

[verified: source] Retained pulses preserve grip/type/epoch/stream and historical zoom including zero. Main corrected premature initial filtering (new real neutral may establish admission before filtering), already-composed zoom rejection retaining fire, and PendingIntents neutral clearing from epoch-zero cached samples. Meaningful first-release/retired-stream/late-use tests added; no history relabelled on failed send.

[verified: checks] Initial 11 offline groups and first full cross-build passed; final source includes subsequent pending-history regressions requiring final rebuild. Source/artifact ABI checks will follow final review. [unknown] Runtime appearance, latency and network behavior; no launch permitted.

Review priorities: causal holes across native ACK/cached input, partial invalidation and consumption; action inactivity/rebind coalescing; historical pulse eligibility and zero zoom semantics; native reentrant copy/use; IPC/menu correlation. Challenge necessary state and simplify. Do not re-review approved native callback architecture or future optic/vehicle integration. Read real source and test coverage, not just this brief. Output <=1000 words, prioritized findings, GO/conditional/NO-GO and exact file/line evidence. Report only owns docs/INPUT_INTENT_SOURCE_REVIEW.md; code read-only. Await main verification of effective gpt-6-astra/xhigh before verdict.

## Bounded source fixes for Astra follow-up

[verified: source] The unpublished wire6 now carries explicit `primaryNeutralSampleMask` alongside activity and serials. `samplePrimaryNeutral` is the actual producer helper extracted from existing hysteresis: only fresh eligible finite trigger `<.2` increments release serial and publishes current neutral; `[.2,.65]` preserves the latch and contributes no neutral. Codec rejects neutral/held overlap or neutral from inactive actions. Pending/Peer admission requires this evidence. The ACK/IPC boundary and native cadence are unchanged.

[verified: source] Existing `Sample` returns live per-hand validation epochs as separate non-wire metadata; raw frozen snapshots retain their original echo. `currentIntentSample` requires both exact captured/returned sample identity and each echo matching the live epoch. Engine `currentWeaponSample` retains avatar/incarnation/age/validity. Fire and muzzle recheck native handle/owner plus this same current admission after native reference callbacks. No fire-mask requirement is introduced for nonfiring muzzle tracking.

[verified: checks] Full x86 proxy/server and x64 host cross-build completed; all eleven offline check groups passed. Four combined producer→ACK→pending→server traces cover both hands across epoch and action-stream changes: `.4→.8` rejects despite cumulative pre-boundary neutral, new `.1→.8` restores intent. Epoch-zero cached neutral and matching stale muzzle echoes with a newer live epoch also regress. Codec checks explicit neutral round-trip and malformed overlap/inactivity. Refreshed artifact/compiled ABI checks preserve IPC7 and native hook counts. No runtime launched.

Follow-up is limited to the original P1/P2/reentrant findings and these concrete fixes. Main has frozen production source. Sole report write for the follow-up: `docs/INPUT_INTENT_SOURCE_FOLLOWUP.md`, <=650 words; original review is preserved. No new binary investigation, agent fanout, or neighboring scope work is required.

## Complete client P1 fix for final bounded follow-up

[verified: source] Reused Pending's existing neutral bitset via `filterPrimaryIntents` and `currentPrimaryIntent`, with no new policy state. Submit now returns the actual stamped/admitted packet by reference whether sent or retained; it filters held primary before saving latest, deriving press edges, retention, or sending. Native Snapshot captures epochs and intersects client fire with that result. The same MP lock guards `localPrimaryAllowed` (current net/avatar/capability/lease/epoch/input sequence/primary generation/latest held fire/Pending admission).

[verified: source] Native `fireQuery` performs the fresh MP admission check. Poll filters fire before taking controlsLock, and existing command getters now copy only sampled fire provenance with cached values under controlsLock and recheck admission after releasing it. Both ordinary float and Down/Pressed/Released/Repeated routes are covered. This closes ACK arrival after submit OR after poll; stale cached command values cannot be relabelled with a later snapshot. No MP/native call is made under controlsLock, and no shared lock spans native callbacks. Non-primary commands are unchanged.

[verified: checks] The rebuilt complete source passes both Windows architectures/server plus eleven offline groups. Combined traces additionally use the exact submit filter/use helpers to reject .4→.8 local held intent, accept new neutral/press, then reject the captured held snapshot after an intervening requireNeutral ACK. Artifact/compiled-ABI records are refreshed for these binaries. Production is now frozen again.

Final follow-up sole write: `docs/INPUT_INTENT_CLIENT_FOLLOWUP.md`, <=650 words. Validate complete P1 client use, per-hand independence, lock order and existing P2 closure; do not broaden to native prediction consequences or vehicle/optic work. Main will spot-check and persist disposition. Source consumers cannot be executed here; tests exercise the real portable helpers, not a replica Windows adapter.
