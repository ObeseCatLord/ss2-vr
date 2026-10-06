Choose the exact post-DDE30 producer adapter. GO for its bounded implementation, with the integration gates below. Supersede the earlier dual-consumer direction; keep runtime/feature-completion signoff withheld. Astra/xhigh provenance is accepted from main's independent verification. No production edits, runtime, subagents, CPU-loop retrace, or helper changes were performed.

1. P1 — Adopt the producer seam; its wider write scope is justified by verified freshness.

Independently rechecked the Engine fingerprint and found precisely the two direct calls DA4E1 and E2E01. The render path calls evaluation E23F0 at E2DF9, then unconditionally calls DDE30 at E2E01. The cache-hit route E24FF→E25F8 still rejoins E2639→DBC90 before returning, so native world records, including the existing post-model adapter, are ready before copying. E2EE0/E2F0E then call E1570, which dispatches records 1..count-1 through E12B0; E14C5 precedes E14D3. CPU reachability uses the worker's completed consumer evidence.

DDE30 initializes its destination offset to zero and overwrites every active map slot, using canonical source [2EAB68]+20 or native fallback for bone -1. E2F48→DAD90 subsequently resets map/palette counts and the evaluated pointer. Thus the modified palette is disposable render output; evaluated animation-cache hits do not cause accumulated deltas. Direct palette-reference scanning corroborates the known producer and CPU/GPU consumers, though it does not exhaust indirect aliases.

Concrete specification:
- Capture the detour's incoming return address; run original DDE30 exactly once. Adapt only return E2E06. Re-read arrays after original returns because it may reallocate. DA4E6 and every other caller receive native behavior.
- Work exclusively on active mapCount slots, never unused capacity. Verify bounded spans, arithmetic, and non-overlap of the writable temporary range with canonical source matrices. Do not consult stale current-record/current-draw globals at this pre-dispatch seam.
- Prove each selected slot's chain: unique binding→body record→record mesh range→mesh draw range→draw palette interval→map.first matching that draw→map.second bone with the same record owner. Spot-checks additionally confirm mesh+0 stores its owning record (E1BF7), and draw+0 stores its mesh (E1CB3). Require those back-links and reject overlapping/ambiguous ownership, rather than trusting plausible indices alone.
- Validate Head uniqueness, parent traversal, sentinels and all selected results before the first write. Compute from the freshly copied native palette; write each selected slot once. Preserve unmatched, synthetic-root, unrelated and identity bytes. Cross-owner attachments remain excluded. Allocation/validation failure performs zero adapter writes; malformed selected data also marks the frozen pair invalid.

This retains native CPU/GPU branching, storage ownership and shader state. Separate consumer hooks add duplicate admission/mapping logic and a CPU pointer-swap window without demonstrated necessity. Do not add a rollback arena or additional coordinator.

2. P1 — Adopt the commit guard with explicit invalidation history.

IPC→snapshot→binding→multiplayer is consistent with the inspected paths: deletion releases remote/MP locks before snapshot work and originalDelete; disconnect/avatar/close release MP before native originals. This is a source-level spot-check, not proof of every indirect callback.

PresentationReadGuard must offer raw, already-locked lookup: never call presentation()/authority() while holding their non-recursive SRW lock. Freeze freshness once; guard lookups compare avatar/incarnation, capability, tracking generation and admitted validity without age expiry or selecting a newer sample. On clients, relay nonces are rewritten to the recipient's capability (multiplayer.cpp:569–571); validate against that current local capability as well as the stored relay.

Hold both guards through Ready and release in reverse order. Keep native restoration independent, and retain the pair fault until commit completes. Compatible observations update only the next bank; empty banks stay valid.

A remaining hole is invalid→valid recovery between freeze and commit. Current-state equality cannot detect it: PeerState::invalidateTracking preserves trackingGeneration (network.hpp:601), and client relay replacement accepts newer sequences without requiring changed generation (multiplayer.cpp:373–380). Use a per-binding/peer invalidation revision under its existing lock, captured at freeze and compared at commit, unless exact serialization proves no intervening writer. Advance it on incompatible transitions, not every pose or wall-clock ageing. Never acquire binding from inside the MP writer lock to propagate a sticky fault: that reverses the proposed order.

3. P1 gate — Establish native thread ownership; these locks protect plugin state only.

Current thread-local render flags do not establish a shared simulation/render thread. Pin the expected native owner at an existing simulation boundary; require it at freeze, producer and commit before object dereferences. Check relevant lifecycle-hook entries before invoking native mutation, not only observePlayer after originalStep. Unknown/mismatched ownership must refuse adaptation and invalidate the transaction.

These checks detect unsupported scheduling; they do not prove safety against unhooked concurrent engine mutation. That lifetime guarantee remains unverified here and must be established before native integration, rather than adding render-long locks around native callbacks.

Prioritized disposition:
| Item | Disposition |
|---|---|
| Exact post-producer seam | Adopt; replaces both proposed consumers |
| Provenance and scratch-first write phase | Adopt with owner back-links/non-alias gate |
| Pair guards | Adapt with raw lookup and invalidation revision/serialization proof |
| Native threading assumption | Gate integration on evidence/enforcement |
| Dual hooks/global swaps | Reject |

Required checks: cache-hit and successive-render refresh without accumulation; non-render caller passthrough; repeated Head mappings transformed once; malformed ownership causing zero writes; canonical/source and unselected byte preservation; raw guard non-recursion; incompatible recovery, age crossing, compatible sequence updates and invalidation around Ready. Confirm indirect callbacks cannot rebuild the renderer globals between this producer and normal consumers. Additional passes, CPU/GPU normals and actual stereo appearance remain coverage/acceptance gaps, not justification for duplicate hooks. Main retains final integration.
