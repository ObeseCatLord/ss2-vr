# Final integration review

Read-only GPT-5.6 Terra review at high effort. Historical line numbers refer to the reviewed pre-fix source. No runtime was executed.

## Findings

1. **P1 — Ready completion is not serialized against epoch invalidation.**
   `bridge.cpp:595` calls `stereoCurrent()`, but `engine.cpp:420-424` only copies the snapshot and checks the atomic epoch; neither lock remains held through the Ready commit at `bridge.cpp:598-601`. An invalidator can advance/publish the epoch between that check and `s.state = Ready`.

   The host should discard that Ready later via `eligibleFrame`, so this is not proof of stale projection submission. It does violate the stated requirement that native completion reject a superseded epoch at commit, and can transiently create stale Ready/backpressure. The completion path needs a snapshot-serialized final validity/epoch check tied to the slot commit.

2. **P2 — Acquired OpenXR images can remain stranded when the post-wait recheck aborts.**
   `host.cpp:1315-1321` acquires/waits both images; if the second mutex acquisition fails or `reapLocked()` expires/invalidates the Ready candidate, `host.cpp:1325-1333` returns without releasing either image. `Swapchain` releases only through `uploadWaited()` at `host.cpp:329-331`.

   Retaining acquisition for a wait timeout is intentional and correct; this issue is the later abandonment path. It can reduce swapchain availability (especially a one-image chain) until a later successful completion or teardown. Add an explicit no-write release/abandon path when the candidate is no longer usable after both waits.

## Verified correct within scope

- ABI3 is pointer-free, packed, and internally coherent: `Request` is 288 bytes and `Shared.latest` offset is 48 in [`protocol.hpp`](../src/common/protocol.hpp#L6). Existing layout checks cover those values.
- Epoch publication is monotonic and reserves `UINT32_MAX`; exhaustion fails closed. Snapshot publication rejects captured epochs superseded by either shared or native state ([`frame_policy.hpp`](../src/common/frame_policy.hpp#L12), [`engine.cpp`](../src/game/engine.cpp#L180)).
- Deletion, weapon loss, and renderer/device loss publish the epoch before mutating dependent native state ([`engine.cpp`](../src/game/engine.cpp#L313), [`engine.cpp`](../src/game/engine.cpp#L337), [`bridge.cpp`](../src/game/bridge.cpp#L201)).
- The cache design otherwise meets the stated transport rules: Ready survives wait timeouts; both waits finish before either upload/release; cache becomes valid only after both releases; original request LOCAL poses/FOV are retained; reacquiring a previously released index suppresses reuse; final epoch/age/focus check occurs before `EndFrame`.
- `CViewRenCmd::Prepare` hook ABI/discriminator are supported by the pinned binary:
  - installed `Engine.dll` and `Sam2Game.dll` SHA-256 values match the exact source fingerprints;
  - actual Engine call is `0x1014c512 -> 0x10156370`, returning at `0x1014c517`;
  - native `Prepare` reads four stack arguments and ends `ret 0x10` at `0x10156599`;
  - the `__fastcall` detour correctly supplies `ECX=this`, dummy `EDX`, and preserves 16 stack bytes for the four explicit arguments;
  - native code seeds/updates the render-command CRC from argument 4 and combines parent CRC, so the two root-only XOR variants produce distinct per-eye IDs. Nested calls remain untouched.

## Test coverage gaps

[`frame_checks.cpp`](../tests/frame_checks.cpp#L1) covers request aging, invalidation, cancelled Rendering, immutable requests, and epoch exhaustion. It does not cover:

- cached-index reacquisition and `ImageOwnership` transitions;
- timeout-retained Ready followed by invalidation/age after both waits;
- first/second release failure;
- the native completion-versus-invalidator interleaving above;
- Prepare ABI/callsite evidence.

## Scope done / verification

Read-only review completed for the exact requested files and supplied disassembly. I additionally hash-checked the installed Engine/Game DLLs and disassembled the specified Prepare/callsite bytes. No files were edited; no tests, game, host, Wine, or headset runtime were launched.

Assumption/open risk: no runtime scheduling or OpenXR-runtime behavior was exercised. The stated post-final-check invalidation race remains acknowledged by design.
## Main-agent disposition

Both findings addressed: `commitStereo` holds the native snapshot lock through `commitNativeFrame` setting Ready; invalidators take the exclusive side before epoch publication. Pixels copy under slot ownership before that short commit. `releaseWaited` and `discardEyeAcquisitions` release abandoned writable images without a write and invalidate cache before changing the runtime’s latest released image. Ready stays available across wait timeout; existing timed-out acquisitions are retried without acquiring additional images. Runtime behavior remains unverified.
