# Coupled zoom causality review

## Four copied-Authority calls: source GO

Effective reviewer model/effort: `gpt-6-astra` / `xhigh`, independently confirmed by main in this conversation; telemetry not inspected.

**GO for these four substitutions only.** `src/game/engine.cpp:280`, `:310`, `:330`, and `:740` now call `network::invalidateWeaponIntents` on the copied Authority pose, with the same affected-hand mask passed immediately beforehand to `multiplayer::invalidateWeapons`. The copy originates before cancellation (`src/game/engine.cpp:264`, `:266`, `:318`, `:732`); mutating peer records alone cannot clear that copy.

The shared helper clears affected fire, pulse, desired zoom, and historical pulse zoom (`src/common/network.hpp:61`), preserving physicalDownMask, releasedSerial, and opposite-hand fields. Peer frozen/pending/active records already use that helper (`src/game/multiplayer.cpp:747`). These substitutions close the local-copy cancellation gap without adding state, changing consumption tokens, or fabricating release evidence. Cancellation intentionally removes invalidated shot history; valid opposite-hand history remains intact.

Scope assumptions: the existing callers identify the affected hands correctly; native callback/lifecycle integration remains main's separate gate. Both x86 builds and helper regressions are reported by the supplied brief (`docs/AUTHORITY_INTENT_INVALIDATION_BRIEF.md:7`), not rerun here. No runtime verification or zoom-enablement approval is implied. Current zero-zoom production and immutable archive 0.2.5/wire4 are supplied constraints.

## Neutral/replacement design: conditional GO; current enablement NO-GO

**Choose B plus a per-hand, server-owned intent epoch in the existing consumption ACK and echoed Pose.** Raw zoom/release serials alone are NO-GO for replacement causality. No additional queue, transport, native handle, or weapon registry is needed.

Evidence: Pose and Ack carry no weapon epoch (`src/common/network.hpp:28`, `:35`); weaponGeneration only resets server release admission (`:638`). A newer-sequence neutral packet sampled before a same-type replacement can subsequently satisfy `:646`–`:672`. Canonical weapon ID cannot distinguish those instances. Existing cancellation only affects already-stored records, not that delayed packet.

Concrete rule:

1. Within each capability, maintain nonzero `intentEpoch[2]` in PeerState. Advance only affected hands at actual weapon invalidations; revoke their primary/zoom admission and cancel frozen/pending/active/copied intents. Each boundary must advance despite repeated/saturated weaponGeneration; exhaustion fails closed and requires a new capability. Do not reinterpret trackingGeneration as server weapon identity.

2. Piggyback current epochs on existing Hello/consumption/discard ACKs. Preserve their exact nonce/sequence credit token (`src/common/network.hpp:692`, `:727`). In `sendPeerAck`, snapshot epochs under the existing lock only for the matching live capability/incarnation (`src/game/multiplayer.cpp:206`); retired-capability discards only retire credit. Install epochs only after current-capability credit/lease validation (`:354`; `src/common/network.hpp:289`), never regress them or let duplicate ACKs refresh admission.

3. A changed acknowledged hand cancels its PendingIntents and requires new primary/zoom neutral. Stamp subsequent genuinely fresh input with that epoch. **Never relabel a cached sample, pre-ACK release witness, held intent, or retained pulse as new-epoch admissible input.** Fresh raw-down observations may echo the epoch, with intents suppressed until neutral. Record an input-sample boundary at ACK receipt; only a later focused/tracked/action-active sample can establish neutral. Keep the opposite hand untouched. PendingIntents must retain/check the capture epoch alongside its existing grip, type, generation, and historical zoom (`src/common/network.hpp:197`, `:207`).

4. After ordinary packet validation, compare each echoed epoch before advancing that hand's release witnesses. A mismatch cancels that hand's four intent fields and cannot arm or poison its serial baseline; continue admitting the unaffected hand and complete/discard through existing credit policy. Recheck at freeze/use: a receive-time match can become stale before native consumption. An old-epoch pose still receives the current epoch in its normal eventual ACK, so discovery needs no unsolicited messages.

5. Add raw zoom down and monotonically advancing neutral/release evidence to the existing admission state. Qualifying neutral requires current-epoch, fresh, focused, tracked, action-active, unblocked raw-up input; desired zero never qualifies. Preserve raw witnesses through suppression. Carry one two-bit zoom-sample-eligibility mask: existing validMask identifies head/grips, not action activity (`src/common/network.hpp:19`). Producer `isActive` and sample validity must survive this boundary; inactivity revokes zoom admission without fabricating release. Logical-stream changes require fresh neutral for affected hands. Keep primary and zoom release admission independent.

6. Apply eligibility before composing fire from admitted pulses (`src/common/network.hpp:705`). A zoomed historical pulse lacking admission is rejected as a pulse, never converted into an unzoomed shot. Conversely, an admitted pulse's captured zoom—including zero—remains immutable for its consumed interval, even when today's raw zoom is up. Use `intervalZoomMask` after pulse admission (`:57`); rejected pulses cannot override latest desired zoom (`:717`). Cancel history only for its affected invalidation/expiry.

| Recommendation | Disposition |
|---|---|
| Desired-zero admission (A), or raw evidence without replacement barrier | Rejected: no causal link to actual replacement. |
| Raw evidence plus per-hand echoed epoch | Adopted; retain existing PeerState/OrderedPosePolicy/PendingIntents. |
| Global reset or new zoom scheduler | Rejected: unnecessary disruption/duplicated policy. |

Gates: offline traces must cover delayed same-type replacement, replacement between receive/freeze/use and ACK delivery, repeated invalidation, wrong/retired/duplicate ACKs, epoch exhaustion, cached-neutral restamping, inactivity/stream changes, unaffected-hand retention, and zoomed/unzoomed retained pulses after release. Verify no credit leak or historical relabelling. Verification here was source inspection; no builds/tests or runtime launches were performed.

Assumptions supplied by main: source36 exported/5 internal, IPC6/wire5 unpackaged, zero zoom producer, prior Astra approval of SAME logical Sprint[h] stream (Vive right Jump[right]), logical provenance without bound-source inference. Preserve that input decision. Implementation requires explicit wire/IPC version changes where semantics change; archive 0.2.5/wire4 remains immutable. Native owned-handle/lifecycle admission and fresh-sample eligibility are main's independent enablement gates; this review does not establish them.
