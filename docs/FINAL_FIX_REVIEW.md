# Focused final-fix review

Effective GPT-5.6 Terra/high settings verified. Read-only, no runtime execution. Historical source references describe the reviewed pre-session-fix snapshot.

Result: the completion race is fixed; the post-wait abandonment bug is fixed, but a separate normal-session abandonment leak remains.

- Fixed — epoch/Ready serialization: [`commitStereo`](../src/game/engine.cpp#L420) takes `snapshotLock` shared through `commitNativeFrame`’s `Ready` transition (lines 423–428). Invalidators take it exclusive and publish the epoch first (e.g. [`deleted`](../src/game/engine.cpp#L316), [`invalidateRenderer`](../src/game/engine.cpp#L338)). The pixel copy remains IPC-protected before this narrow snapshot-lock section ([`bridge.cpp`](../src/game/bridge.cpp#L592)). I found no invalidator that waits for IPC while holding `snapshotLock`; `publishSnapshot` releases it before acquiring IPC ([`engine.cpp`](../src/game/engine.cpp#L180)).

- Fixed — reported post-wait stranded acquisition: [`pollEyes`](../src/host/host.cpp#L1329) waits both eyes without IPC. If the post-wait IPC recheck rejects the Ready pair, it calls [`discardEyeAcquisitions`](../src/host/host.cpp#L1361), which retries only acquired images, performs no write, invalidates `cachedValid` before release, and calls `releaseWaited` ([`host.cpp`](../src/host/host.cpp#L1305)). Timeout retains ownership for retry ([`host.cpp`](../src/host/host.cpp#L1330)).

- New P1 ownership gap: normal `XR_SESSION_STATE_STOPPING` calls `xrEndSession` without first discarding acquired eye images ([`host.cpp`](../src/host/host.cpp#L1115)). The following `!running` loop only publishes empty input and sleeps ([`host.cpp`](../src/host/host.cpp#L1570)); it never invokes `discardEyeAcquisitions`. Thus a timed-out or waited acquisition can remain owned across session stop, contrary to the required abandonment behavior. Resize/destruction similarly destroys chains without an explicit no-write release ([`host.cpp`](../src/host/host.cpp#L1242), [`host.cpp`](../src/host/host.cpp#L245)). Khronos requires a successful wait before release and specifies that an application releases an image when finished with it. [OpenXR specification](https://registry.khronos.org/OpenXR/specs/1.1-khr/html/xrspec.html)

Tests: [`frame_checks.cpp`](../tests/frame_checks.cpp#L99) is correctly an offline protocol/state-transition test, not a fake OpenXR API test. It models timeout retention and no-write release (lines 109–124), but does not cover STOPPING/resize abandonment. `g++ -std=c++20 -Isrc -fsyntax-only tests/frame_checks.cpp` passed. No built CTest tree was present, so no test executable was run.

Scope_done: exact requested files and supplied review inspected; no edits, runtime, game, host, Wine, or headset execution.

Assumptions/risks: conclusion is static and protocol-spec based; no runtime scheduling/OpenXR implementation behavior was exercised.
## Main-agent disposition

The reviewer confirmed both original fixes and the absence of the reported lock-order cycle. Its STOPPING ownership observation was adopted: `finishSessionAcquisitions` invalidates presentation and retires existing acquisitions in all eye, wheel, menu and HUD chains before EndSession. It never acquires an additional image. Wait retries are bounded to 500 ms; failure exits the host through normal fail-closed teardown rather than carrying ownership into a restarted session. Menu/HUD/wheel upload state is invalidated before retirement.

Resize/destruction now also attempts a successful wait and no-write release of existing acquisitions after the caller’s GPU drain. A still-timed-out unwritten image is not illegally released; the drained swapchain is destroyed. The stronger suggestion that destruction itself always requires prior release is not established by the pinned specification: [xrDestroySwapchain](https://github.com/KhronosGroup/OpenXR-Docs/blob/release-1.1.53/specification/sources/chapters/rendering.adoc#L256-L272) requires submitted graphics work to have finished and does not state that an unwaited acquisition must first be released. [Wait/release semantics](https://github.com/KhronosGroup/OpenXR-Docs/blob/release-1.1.53/specification/sources/chapters/rendering.adoc#L417-L467) prohibit releasing before a successful wait. Teardown retains graphics/runtime objects until OS process exit if its GPU drain fails.

The session-stop/destruction amendment was integrated and statically checked by the main agent and cross-compiled with host warnings treated as errors. It was not exercised against a runtime. The portable frame checks establish ownership predicates/transitions, not injected OpenXR call-failure behavior.
