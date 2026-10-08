# Multiplayer local origin and retained hand capture

Astra/xhigh Source GO for the bounded local-owner route. Local configuration tags
were verified; effective backend routing cannot be independently inspected.
No multiplayer or headset gameplay acceptance follows from this source review.

## Movement ownership

Only the controlling local XR actor performs the existing checked post-simulation
roomscale move. Both the initial admission and callback revalidation require the
native local-player predicate and a single-player, native server or negotiated
local-client role. Remote actors and headless servers keep original native
ClientAction, discrepancy, correction and physics behavior. No second server
setter, payment ledger or packet counter is treated as a movement receipt.

The existing origin settlement still uses actual before/after native placement,
not requested distance or correction residuals. Native correction can pull the
body back; an already completed physical origin payment is retained, not reversed
or replayed. The design propagates body-relative articulation. It does not promise
identical historical world coordinates across machines, full remote-authority
collision acceptance or correction-free convergence.

The finite native investigation found that the checked setter's notification does
not supply a mandatory packet/baseline association. Native queue and correction
paths can mutate counters, clear residuals or return without placement. Therefore
an all-role payment adapter would risk duplicate payment without an actual native
ownership receipt. The local route reuses existing components.

## Pose capture ownership

The existing multiplayer Local and PendingIntents owners retain immutable raw
calibrated head/grip poses, captured before tracking-volume clamps. Each hand
carries its original avatar/producer/session/reference/tracking epoch/generation,
input sequence and timestamp. Future handoff recomposes retained poses with the
current origin and turn. It does not translate an already clamped old pose.

Tracking loss, owner replacement, stale capture and nonfinite or overflowing
intermediates revoke eligibility and require real neutral. An inactive hand is
independent of the other hand; its invalid pose cannot discard valid head/other
hand intent. Head-only overflow is checked before clamping. Invalid capture still
permits native capability Hello/expiry, avoiding the bootstrap dependency where
usable client tracking itself requires negotiation. No unavailable action creates
a release witness or ACK.

Wire6 and IPC10 remain unchanged. Existing intent identities, TTL, credits,
consumption ACK ownership and handed-off packet immutability remain authoritative.
There is no extra queue, pose registry or transport service.

## Review disposition

| Recommendation | Disposition |
| --- | --- |
| One native settlement owner | Adopted: positive local-player admission at entry and revalidation |
| Keep native network movement/correction | Adopted: no remote/server payment adapter |
| Recompose from raw per-hand captures | Adopted in existing Local/Pending owners |
| Preserve negotiation without tracking | Adopted: inactive coordinates do not block Hello |
| Independent inactive hands and head-only arithmetic | Adopted: focused regressions cover both |
| Infer placement from counters/residuals | Rejected: native counterexamples invalidate the inference |

`[Roomscale] Enabled=0` remains a development default. User-operated acceptance
must cover listen-host local, remote client and dedicated-server roles, physical
movement mixed with joystick input, walls/supports, native corrections, reconnect,
death/respawn and level changes. Another mod user must observe coherent body,
head and hands. Source helpers and offline packets cannot pass those checks.
