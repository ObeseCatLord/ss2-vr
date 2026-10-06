Findings refer to the source snapshot before the disposition fixes; linked line numbers are historical.

Review found three static correctness bugs and one unresolved render-validity gate. No repository files changed.

- **High — aliased weapon handles cannot remain hand-independent.** `handOf` picks the first matching pointer and `fireQuery` picks the first matching handle, so identical/aliased native weapon entities bind both inputs to left hand. [engine.cpp](../src/game/engine.cpp#L87), [engine.cpp](../src/game/engine.cpp#L227).
  Minimal fix: before enabling per-hand fire, shooting-placement, or weapon-model overrides, require two nonzero, distinct native handles/pointers. For an alias, retain native input/aim behavior and suppress VR fire until the engine supplies distinct entities.

- **High — a failed completion lock can strand a rendering slot permanently.** `retireDeferred()` may fail to obtain the mutex; later in the same call `stereo()` can render another slot and overwrite `deferredSlot` if its completion lock also fails. The former slot remains `Rendering`, violating bounded two-slot ownership. [bridge.cpp](../src/game/bridge.cpp#L106), [bridge.cpp](../src/game/bridge.cpp#L350), [bridge.cpp](../src/game/bridge.cpp#L446).
  Minimal fix: if deferred retirement remains pending, do not claim/render another request; render desktop only until that exact slot is retired. Make `deferredSlot` single-assignment and assert it is clear before assigning it.

- **Medium — command overrides can expose retained movement/action state after tracking or focus loss.** The command readers gate on `vrSession` (up to one second stale), but return cached `values` populated by the last `poll`; they do not independently require fresh, focused tracking and current gameplay UI. [engine.cpp](../src/game/engine.cpp#L413), [engine.cpp](../src/game/engine.cpp#L444), [engine.cpp](../src/game/engine.cpp#L453). Direct weapon firing is better gated, but movement/use/sprint can remain latched until the next poll.
  Minimal fix: in every command read, require `fresh(s.input)`, `s.ui.gameplay`, and a current UI tick; otherwise call the original command query (or return a neutral value only for the confirmed player bindings).

- **Medium — cached raw entity pointers have no demonstrated lifetime contract across simulation and render hooks.** `Snapshot` stores player/weapon pointers; only player deletion conditionally clears it. Weapon deletion/reuse is not invalidated, while shooting and weapon-placement paths call engine code through cached `s.player`. [engine.cpp](../src/game/engine.cpp#L47), [engine.cpp](../src/game/engine.cpp#L213), [engine.cpp](../src/game/engine.cpp#L282). SRW locking prevents torn snapshots, not use-after-free.
  Minimal fix: retain handles plus an ownership/lifecycle generation, resolve immediately on the calling engine thread, and invalidate on both player and weapon deletion. Do not pass a snapshot raw pointer to native code without validation.

Static ABI result: the fingerprint-matching installed PE32 modules support the declared x86 boundaries: hidden-return camera/projection/shooting hooks, `GetWeaponAbsPlacement` reference arguments, and `Render(Matrix34f)`’s 48-byte by-value argument. I found no demonstrated calling-convention mismatch in these detours.

Unknown runtime limitation, not a proven bug: the stereo path only redirects `SetRenderTarget(0, capturedDesktopTarget)` and matching depth binds. [bridge.cpp](../src/game/bridge.cpp#L127), [bridge.cpp](../src/game/bridge.cpp#L393). A successful readback therefore proves only that the eye surface was copied—not that all final/postprocess/HUD output reached it. The missing target-routing/final-capture audit already identified in `ASTRA_REVIEW.md` remains the gate; do not replace the renderer.

scope_done: `src/game/*`, `src/common/*`, requested plans, and installed module export/disassembly validation only.
verification: source review; SHA-256 hashes matched the documented installed build; no runtime execution or edits.
assumptions/open risks: engine render/simulation threading and final target-routing behavior remain unverified without prohibited runtime testing.
follow-up: fix the four static issues, then perform the narrow native render target/final-image audit before claiming stereo output validity.


## Main-agent disposition

Effective CLI model gpt-5.6-terra/high was verified from the parent startup header. The review output file exists; its discarded final paragraph incorrectly said otherwise. No raw operational log is committed.

| Finding | Change |
|---|---|
| Aliased hand ownership | Alias detection blocks per-hand firing/pose overrides until distinct native entities exist. |
| Deferred slot overwrite | A pending retirement prevents another render request from being claimed. |
| Stale command cache | Every command read checks fresh focused tracking, current gameplay UI and live native ownership. |
| Raw pointer lifetime | Cache native handles; re-resolve on the calling engine thread, validate live hand ownership, and hook player/base/sniper deletion. Native cross-thread lifetime semantics remain an engine assumption. |
| Capture validity | Full-size WindowCanvas/format/non-MSAA gate; final owned RT/depth identity, failed-bind and MRT rejection. Complete postprocess coverage remains unverified and is stated in status. |

Additional integration fixes: use one snapshot/anchor for both eyes; preserve native shader state rather than applying a stale state block; derive authored model rotation from the actual supplied camera transform; map native Z- to forward using verified command-description associations.
