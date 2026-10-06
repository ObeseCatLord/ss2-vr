# Multiplayer plan and Astra review disposition

Astra was explicitly selected at xhigh for the design review. Main retained the native transport and reviewed the load-bearing address/header claims against the installed binaries. Terra follow-up resolved native object lifetimes, slot routing and client handle mapping. No game or network session was executed.

| Review finding | Final disposition |
|---|---|
| Proposed H/J header reversed | Corrected: H=0; target is the bound player brain; J is read from the current native chat descriptor `[Sam2Game+0x40694c]+0x54`. Never invent a VM selector. |
| Message/stream ownership and queue behavior unknown | Native stack CStream (8 bytes) and RPC (0x28 bytes) constructors, copying SetData, cloning send and destructors verified. Submit a bounded latest pose at most20Hz; reliable physical edges use the same immutable packet. Sequence acknowledgement limits outstanding submissions; never inspect or mutate native queues. |
| Receive slot and RPC target not authenticated by chat handler | Intercept server ExecuteRRPC/URPC before VM reflection. Require current receive slot0..17, its bound brain target and brain+0x28 avatar equality. Consume tagged malformed and wrong-owner carriers before reflection. Nonces correlate the negotiated session; they are not cryptographic identity. |
| Private sends might be rejected by relevance | Static closure: player-brain category virtual+0x6c returns3; native relevance accepts category3 immediately. Enforce recipient slot/brain equality in the adapter. |
| Client packet handles differ from server handles | Consume client packets after native target mapping in CClientInterface::ExecuteRPC. Map relay subject handles through the native entity mapper; never treat server IDs as local pointers. |
| Capability lifetime conflated with weapon/death lifetime | Slot+avatar+server incarnation binds capability. Clear before disconnect/avatar replacement/close. Tracking or weapon replacement disarms firing without silently falling back to native keyboard fire for a negotiated VR peer. |
| Pose and native fire commands can arrive in different orders | One validated body-relative pose includes physical trigger state, physical release serials and per-hand fire intent. Freeze it before the native entity/weapon loop and retain it through the complete entity/script/physics interval; both firing query and muzzle hooks consume that exact frozen sample. Retain one bounded press sample for a short reliable tap. Native cadence/ammo/damage remain authoritative. |
| Equipment retries could repeatedly replace a weapon | Validate native inventory and CanChangeWeapon on authority. Compare actual native ID before SetCurrentWeapon; maintain a bounded intent and disarm on handle replacement. |
| Dedicated server incorrectly depends on rendering | Use the native plugin-module loader and a fingerprinted authority bootstrap. It must work without IPC, a local headset player or GfxD3D. Do not inject a new transport or start another server. |
| Pose coordinate meaning and muzzle offsets ambiguous | Wire calibrated physical grip position with runtime aim orientation in body coordinates. Server derives authored model-to-shot offset through verified native getters with owned temporary arguments; rendering must not be a prerequisite for authority. |
| Listen host bypasses network receive slot17 | Feed local tracking through the same validation/freeze path, bound directly to its current native brain/avatar. No unproved loopback. |

## Ordered implementation

1. Bounded versioned ASCII codec and local-tick freshness/release validation. Portable checks reject malformed, replayed, old-generation, impossible-fire and out-of-range packets.
2. Native server receive/client mapped receive hooks; stack message transport; capability/sequence acknowledgement and lifecycle reset. Preserve ordinary native RPCs and chat.
3. Split local headset presentation from server weapon authority. Freeze tracking/fire/equipment at simulation boundaries. Retarget original muzzle getters; preserve native weapon execution.
4. Retarget only identified dynamic weapon children in the existing puppet model tree for remote presentation. Reuse the same authored assets and transforms, avoiding duplicate models or body animation replacement.
5. Native dedicated module bootstrap, current fingerprints and installation instructions. Cross-compile both architectures/server module, run portable checks, verify exports/layouts and package a separate development artifact.

This plan supersedes the baseline single-player exclusion. Multiplayer remains unfinished until producer, routing, authority and remote presentation are connected; codec checks or compilation alone do not establish it. Actual headset/network testing remains excluded by the user.

## Implementation review and architecture reopening — 2026-10-03

Astra's implementation review ran with verified `gpt-6-astra` / `xhigh`. Main spot-checked native ReverseMapEntityHandle at Engine RVA0xF10B0 and its VM caller0x17B29B. The output is a server wire handle, which must not be resolved in the client's local handle table. Main also confirmed the partial-activation behavior in MinHook's queued apply implementation.

| Finding | Disposition |
|---|---|
| Outbound target not reverse mapped | Adopted: validate the local brain, call the native client mapper immediately before SetData, require nonzero mapped output. Never locally resolve that server wire handle. |
| ACK age permanently stalls an otherwise idle stream | Adopted: explicit bounded recovery, capability rotation and real neutral rearming; identity and liveness are separate. |
| Short taps lost before reliable submission; peer-wide pending press cancels the wrong hand | Adopted: bounded immutable per-hand retained taps, expiry and hand-specific cancellation. Submission reports acceptance/retention to the native input producer. |
| Newer unreliable pose can suppress an older reliable edge; Pose and Edge can also fire the same press twice | Architecture reopened. The attempted split delivery introduced interacting consumption policies. Astra recommends the smaller ordered full-snapshot reference before adding cross-stream press identity and credits. |
| Queued hook activation can fail after enabling some detours | Adopted: directly disable every owned hook, retain all trampolines until every disable succeeds, then remove. Check failures and refuse retries after incomplete rollback. Dedicated bootstrap logs failure and does not advertise VR capability. |
| Pre-OnStep camera/model mixed with later native shot | Adopted: keep only frozen wire input and native identity in authority state. Evaluate native body/camera/model references at the muzzle boundary; no render calibration dependency on the server. |
| Dedicated path still requires input/raster symbols | Adopted: resolve those only in graphical mode. Pin the exact Core/Engine/Sam2Game mappings that own cached native addresses. |

### Revised input delivery decision

Keep the game's existing reliable RPC machinery and native weapon state machines. The next implementation uses one outstanding ordered full snapshot, a reliable consumption ACK, latest unsent state on the client, and bounded retained taps. A short already-released tap must be a one-interval pulse; an ordinary held level must remain available to native automatic-fire cadence. An ACK means input was processed/discarded, not that a bullet fired. No application retransmission or alternate UDP transport is added.

Credit is returned only after the native consumption interval, not receipt. Outstanding native submissions remain bounded across capability recovery; changing a nonce does not cancel the native queue. Old capabilities and ACKs cannot authorize new input. Strict capture-age expiry is not inferred from arrival-time freshness.

This simpler path limits authoritative aiming cadence to approximately `min(20 Hz, 1/(RTT + simulation wait))`. It is not evidence of adequate high-latency responsiveness. Separate replaceable aiming and reliable press streams will be reconsidered only with demonstrated latency requirements and one shared consumption identity. Offline checks must exercise the actual adapter policy under delayed reception/consumption, overlapping hands, held/released triggers, cancellation and recovery. No runtime/network session has been executed.

## Final integration correction

Main found that aggregate capacity alone allowed several current-capability snapshots despite the reviewed one-outstanding rule. The shared production credit policy now rejects a second token for the same capability; aggregate capacity4 bounds old queued tokens across recovery. Server pending/freeze/finish/discard behavior is implemented once in OrderedPosePolicy and used by the native adapter and portable scenario checks. Per-hand retained taps preserve the original grip and weapon identity, cancel only their own hand on replacement/wheel activity, and expire after200ms. Consumed/discarded packets return their exact token; replayed ACKs cannot refresh liveness.

The native scheduler seam is CSimulation::Step Engine0x1B70E0, with input preparation at its entity manager0x1B3B60 after world changes/time advance. That manager dispatches independent entity OnStep functions and RunTick; outer simulation subsequently steps physics before returning. Main therefore pairs beginTick with post-simulation completeTick even if lifecycle state changes inside the interval. Player OnStep alone is not the weapon consumption boundary. Native muzzle/model references are sampled at the firing getter rather than retained before the tick.
