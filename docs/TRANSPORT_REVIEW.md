**Adopt B for transport liveness, with the corrections below.** Preserve the existing slots, native rendering transaction and OpenXR host. A larger synchronous wait retains the cadence problem; no evidence justifies GPU transport replacement.

[Verified from source] [requestEyes](../src/host/host.cpp#L1308) starts the 35 ms deadline; `reapLocked` expires Rendering and discards expired Ready. [Native completion](../src/game/bridge.cpp#L575) follows both eye captures and desktop restoration. Input publication precedes the synchronous wait. Consequently, transactions consistently exceeding the deadline cannot deliver projection images. **Actual transaction duration remains unknown.**

1. **P1 — Epoch validation is necessary, but adding the field alone is insufficient.** Session/reference/dimensions do not identify native snap, recenter or ownership changes. Add `Request.trackingGeneration`, include it in immutable identity comparison, and reject mismatches against the frozen native snapshot before either eye renders. Validate snapshot initialization, player identity, gameplay eligibility and input session/reference alongside the epoch.

   Use a process-lifetime monotonic epoch allocator; resetting a `Snapshot` must not reset its identity. Fail closed on counter exhaustion. The existing change triggers remain authoritative.

   Crucially, [player deletion and weapon invalidation](../src/game/engine.cpp#L286) change native state without publishing corresponding shared-UI invalidation. `publishSnapshot` also updates native and shared snapshots separately. Therefore “current shared UI matches” can mean *stale shared UI matches*. Make invalidation publication coherent with transport acceptance; check for native epoch changes before publishing Ready as well. UI freshness is an additional guard, not a substitute. Do not claim this race closed merely because the new field compares equal.

2. **P1 — Count cancelled Rendering against the one-transaction limit.** “One unexpired request” is weaker than “one pending native transaction.” Retain its outstanding identity until native retirement; do not enqueue behind a cancelled Rendering slot using the second slot. This avoids creating a backlog behind a slow or stuck capture.

   Under the mutex: validate channel ownership, reap, select an eligible complete pair, copy both images **and its Request**, then release that Ready slot. Admission follows only when no Requested/Rendering transaction remains. With this invariant, “newest complete” needs no queue or ordering subsystem. Cancellation and completion serialize through the existing mutex; never reclaim Rendering, including after timeout or deferred retirement.

3. **P1 — Zero projection deliberately creates gaps in world presentation.** Khronos OpenXR **1.1.53**, matching the pinned SDK checkout, permits submitted views/FOVs differing from `xrLocateViews` and describes reprojection in stable LOCAL space. My inference is that delayed images can be submitted with their actual original eye poses/FOVs; this establishes API compatibility, not acceptable visual quality. Use the **current XR frame’s** predicted display time for `xrEndFrame`; retain the old request’s prediction timestamp as image provenance, never as replacement current-frame timing. [Pinned rendering specification](https://raw.githubusercontent.com/KhronosGroup/OpenXR-Docs/release-1.1.53/specification/sources/chapters/rendering.adoc)

   The same specification requires omitted layers not to be drawn, and zero layers to clear the display. Thus the no-reuse requirement means no world projection between completed pairs, even if HUD layers remain. B fixes starvation; it does **not** establish continuous world presentation. Preserve that explicit requirement and document this limitation rather than assuming the compositor retains the previous projection.

4. **P2 — Define age and diagnostics precisely.** Treat 150 ms as a provisional transport cutoff, not a verified acceptable latency. Measure monotonically from enqueue; do not subtract `XrTime` from Windows ticks. Require eligibility when Ready is consumed: matching session/reference/epoch/dimensions, current gameplay, renderer availability and existing focus/tracking/channel guards. Retain cancellation for lifecycle invalidation as well as age expiry.

   A capture finishing before the cutoff can still expire before the next poll; promise acceptance only when **observed eligible before expiry**. Count expiration once per request. Distinguish age expiry, epoch rejection, native retirement/failure, mutex deferral, IPC acceptance and actual projection submission. A stuck Rendering slot should produce a bounded, rate-limited diagnostic while input publication continues; it never authorizes reclamation.

| Recommendation | Disposition |
|---|---|
| Async polling using existing slots and buffers | **Adopt**; no capture wait, new thread, queue or simulation policy. |
| Native epoch gate | **Adopt with corrections** to invalidation publication and monotonic lifetime. |
| ABI change | **Adopt**; update protocol assertions, both compiler layout records, frozen verifier values and core layout checks. |
| Larger synchronous deadline / GPU rewrite | **Reject as the solution**; neither is necessary for this boundary repair. |
| Claimed continuous VR presentation | **Not established** under the required zero-projection policy. |

Required offline regressions should exercise production predicates/transitions: Ready observed at 50 ms under a 150 ms cutoff accepted exactly once; boundary-expired and generation-mismatched replies rejected; respawn cannot reuse an epoch; invalidation after begin rejects completion; cancelled Rendering blocks admission until retirement; cancellation/Ready race orders remain safe; both eyes and submitted metadata come from the same request.

Read-only review only. No edits, subagents, tests or runtime execution; the separately approved body-camera accessor was not re-reviewed.

Parent provenance: effective gpt-6-astra/xhigh CLI settings verified; ephemeral read-only review. References describe the pre-fix snapshot. No runtime execution.
