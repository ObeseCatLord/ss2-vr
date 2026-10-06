# Native input interval retirement and multiplayer presentation

Bounded incremental change from source12001a0. The existing native simulation,
input preparation, reliable receive slot, consumption/discard ACK, credit window,
weapon neutral gates and tracking epoch remain the behavioral reference. No
game, Windows executable, host, Wine, XR/runtime or network session is run.

## Verified source problem and chosen boundary

The previous simulation hook restores its stack-backed TLS interval only after
original simulation and normal completion. A native unwind may skip MinGW
cleanup. Extend the accepted native-finally boundary to own that interval;
preserve original simulation/entity/player callbacks once. Nested intervals
share root failure, while only the root native manager opens preparation.

Each changed preparation/MP critical scope explicitly releases its metadata lock
on native unwind, and contains its own direct GNU errors. This does not claim
all unrelated native callbacks, RPC resource cleanup or arbitrary crash recovery.
Abort tails invoke no native game callbacks, sends or allocation.

Interrupted local input advances the existing tracking epoch and retains a
sequence/time/session/reference/host-producer sample boundary. Existing update
and TriggerGate require a later eligible neutral; cached neutral cannot rearm.
The boundary survives update, player deletion and renderer snapshot resets.
Producer identity is sampled under the same existing IPC read lock.

Interrupted frozen peers keep their exact existing awaitingConsumption token.
Tracking and weapon-intent epochs invalidate stale held/release/pulse/zoom intent.
The next normal native tick stages that same inactive slot for discard, even
without avatar enumeration. No second ACK queue, scheduler or movement protocol.

## Verified review fixes and handoff ownership

Initial explicit Astra/xhigh review identified:
1. invalidatePlayer's lock-held allocating discard vector in the changed path;
2. definitely unsubmitted client credits and unsent completion ACKs losing
   ownership when codec allocation threw;
3. a cached local neutral sample rearming after snapshot invalidation.

Use fixed peer/recipient/completion arrays and explicit lock finally. A client
descriptor above finally observes actual native transport entry/return. Revoke
only definitely unsubmitted exact credits; ambiguous entry retains ACK ownership.
Codec errors are contained before transport, and the temporary carrier heap is
freed before native calls.

Completion peeks the actual slot, sends, then confirms only exact brain/
incarnation/nonces/sequence. Unsent or ambiguous ACKs remain in that inactive
awaiting slot for the existing next normal interval. Lifecycle invalidation
promotes a pending packet into that same slot for explicit discard; pending and
awaiting are mutually exclusive in the existing policy. No gameplay executes
during this promotion. Confirmed duplicate ACKs cannot extend client leases.

Actual compilation also exposed a relay Sample aggregate assigning the64-bit
presentation revision to liveIntentEpoch[0], leaving presentationRevision zero.
Remote rendering rejects zero revisions. Corrected the exact field position,
leaving authoritative intent epochs zero on relay samples, and enabled GNU
-Werror=narrowing on both x86 targets. Remote-head lifetime/enablement is separate.

## Offline evidence and limits

The complete x86 proxy/server and x64 host/official-loader build and all14 portable
groups pass. New production-policy traces cover interrupted shot/zoom discard,
exact token retirement, delayed old-epoch neutral, later fresh neutral, pre-call
credit revocation versus ambiguous handoff, peek/confirm ownership and lifecycle
pending discard. Core checks exercise cached neutral/held, later held before
release, later release, changed session/producer and future timestamps.

The new read-only sniper site verifier checks all8 pinned native windows and
pinned MinHook/HDE sources. Planning lengths are13/7/5/8/8/10/6/6. Its linear
direct-branch/HIGHLOW scan finds no interior reference. Direct scan completeness,
indirect incoming paths and actual HDE continuation acceptance remain explicitly
unknown; no production hooks or Windows relocation were executed.

Focused final Astra source verdict and disposition are recorded separately.
Native zoom input remains zero, and optic images are not integrated. Roomscale
body collision, physical swing melee, broader vehicles and complete remote-head
lifetime/enablement remain required. Full goal remains active; teleport excluded.
