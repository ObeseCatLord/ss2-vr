# Physical primary carry boundary — reopened architecture

Main pauses new native architecture pending Astra senior review. The current
physical producer is not source-accepted or packaged. Repeated carry fixes show
that preserving the combined scalar alone does not preserve manual history.

## Evidence and reference

| Claim | Status / evidence |
| --- | --- |
| Original native operator reads brain160 current and puppet348 prior; store remains native | [verified: PRIMARY_JOIN_SOURCE_REVIEW.md, primary-load-boundaries.json] |
| Accepted five-site adapter replaces current bits; positive carry has mask0 passthrough | [verified: engine.cpp primaryAdmission/native_primary_projection.hpp] |
| Gesture-only high followed by carry-only manual0 contaminates command release history | [verified: two independent Astra source counterexamples; poll/query source] |
| Ordinary manual-only carry entry/exit currently causes unnecessary source reset | [verified: Astra follow-up, gameplay_primary.hpp context equality] |
| Pure producer corrections now preserve manual-only carry transitions and rebase poll prior command to previous manual contribution | [verified: current worktree; tests pending extension] |
| Rebased poll history alone also repairs native operator348 prior | [unknown; it appears insufficient because native348 is committed combined history, not priorValues] |
| Server retains a native raw manual previous bit independently | [unknown; investigate exact native ABI/caller ordering] |

Reference is base4baa4e8 manual/native carry behavior. No game, Windows, XR or
network session may run. Owned modules pinned; exact RVAs in existing docs.
Linux C++20/offline and Windows cross-builds only. Solo operator; no new service.

## Open decision

First verify whether carry consumes a removed gesture's native348 prior high,
and whether an existing native manual/history seam is sufficient. No new offsets
or hypothetical unused native bits. Do not re-review whole VR or weapon damage.

Lean: preserve original command/native manual history at the narrowest boundary,
while existing frozen logical intent feeds the accepted native handheld join.
Consider first a minimal existing native-history adapter. Only if disproved,
compare a two-bit manual contribution recorded at the SAME actual native history
commit in existing player/authority lifetime metadata (not new queue/ACK/combat/
independent history state machine). That alternative would also need original
command bits to represent manual contribution, rather than merged gesture, so
server raw native RPC supplies manual state without a new wire protocol. This is
a proposal, not implemented or verified. Identify every native caller affected
by such separation before approving it. Model the carry-start/carry-end lifetime
and local/remote native commit paths explicitly.

Rejected shortcuts: infer neutral from missing gesture, keep gesture high during
carry, modify throw/damage FSM, assume poll priorValues equals native348, reuse
unknown native upper bits, or introduce new movement/action replication.

Depth budget1800 words. Objective one decision: minimal correct manual/native
history boundary needed for gesture-primary plus carry. Cite exact source/RVAs,
GO/NO-GO, counterexample and smallest source/portable vertical proof. Failure:
report missing ABI/lifetime evidence instead of expanding the architecture.
