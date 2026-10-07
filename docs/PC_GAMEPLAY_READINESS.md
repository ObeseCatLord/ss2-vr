# Native gameplay VR investigation — 2026-10-07

This record is active work, not completed VR acceptance. Genuine gameplay requires
separate native world images for both eyes, tracked head translation and rotation
in the native camera, correct asymmetric FOV and coherent hands/origin/body collision.
A menu quad, duplicated desktop gameplay, a synthetic image or successful xrEndFrame
alone does not meet that requirement. Teleport remains excluded; multiplayer remains
required. Preserve native D3D9 CPU readback into the separate D3D11 OpenXR host.

## Observed source issue and incremental fix

The admitted native additional swapchain replaces the unsuitable implicit16x16
buffer at the existing capture boundary. Menu color readback is independent of
stereo depth allocation. A new readiness cycle was found during integration:
tracking update requires rendererReady, but stereo allocation was gated by a
menu classification that depended on tracking gameplay readiness. Stereo capacity
is now attempted independently. menuVisible uses positive native menu presence;
inactive/stale tracking is no longer a reason to project desktop gameplay as a menu.
This is a confirmed draft-source bug, not a proven cause of the user's screenshot,
whose binary/run provenance has not been established.

The existing native Render3D eye loop, eye camera/FOV hooks and two-view OpenXR
projection layer remain in place. Source presence is not runtime proof. All three
backbuffer consumers use the admitted chain, with explicit native-finally owners
and reset generation retirement. Scope/reset/nested lifecycle recovery remains
unaccepted; do not generalize the ordinary pistol-test review to those routes.

## Actual observations

The actual proxy launched the actual x64 host under Proton Hotfix
hotfix-20260828-ptr-x86_64 against isolated simulated Monado25.1.0
v25.1.0-710-g735e29e4e. NVIDIA RTX4090, D3D11 via DXVK3.1, BGRA format87,
three swapchain images, FOCUSED and WMR interaction profiles were observed.
A prior native menu transport segment copied the actual1280x720 game menu through
D3D9 CPU readback; private CPU and exact game images were visually inspected.
That segment recorded4808 successful native menu-quad frames. It is menu transport
acceptance only, with no actual-device claim.

A later draft package (archive SHA256
92c9a6298e6f6b57289f9daa460c3b3fa7b20a42ac9142848cdf8f6c38483bff;
compiled source fingerprint
9a641a0455fb5153e7168e1b1701ab8158f065f2b3c41e88d7ee40b107defcd8)
initialized native stereo targets and both host eye swapchains1280x720. Exact-window
capture showed real first-person outdoor pistol gameplay. Native menu presence
cleared and gameplay readiness became1. However the host recorded zero completed
world pairs, with requests filling/retiring. A15-second read-only Ready-slot capture
found no pair. This is an unresolved gameplay VR result, not a stereo pass.

The exact game process was stopped at the boundary; the host subsequently exited.
Private logs and images were preserved. Bounded source diagnostics now distinguish
Render3D admission, request acquisition, target validation and completion. All60
Debug build/check groups passed both source snapshots. No equivalent current-source
Release, lifecycle fault, network or real-device acceptance is claimed.

Capture by compositor active-window selection proved unreliable on this desktop;
subsequent captures target the positively owned SS2 X window ID. Synthetic controller
rays can override desktop menu cursor navigation; bounded events directed only at
the owned Monado debug window changed the actual sampled controller pose. Those are
simulation input observations, not hardware controls or head/camera acceptance.

## Astra review disposition

An Astra Max attempt returned a capacity error and no review. The retry explicitly
selected gpt-6-astra/xhigh and inspected the fixed source diff. Configured current-turn
metadata reports those settings; independent effective-backend introspection was
unavailable. Its GO covers only ordinary single-player pistol world observation.

| Recommendation | Disposition |
| --- | --- |
| Keep narrow actual-chain owner and CPU transport | Adopted; existing render/IPC architecture retained. |
| Break stereo readiness/classification cycle | Adopted; capability allocation precedes tracking admission. |
| Observe existing per-eye camera/FOV/native-render route | Adopted; no render rewrite or flat fallback. |
| Require visible near/far parallax and translation/rotation response | Adopted; still unaccepted. |
| Exclude scopes, reentrant Reset and nested recovery from this test GO | Adopted; separate cleanup/runtime gates remain open. |

Next: resolve the exact blocking native render gate, then inspect actual left/right
world images and simulated lateral/vertical/forward head translation plus yaw,
pitch and roll. Keep hardware SteamVR/Steam Frame and Monado/Beyond acceptance,
Windows-native and Proton, multiplayer, melee and vehicles separate and unfinished.

## Direct-scene probe outcome and active gates

The no-menu +level route reached Jungle loading through the actual native stream.
The successful restored-position receipt was 10,562,049 bytes, SHA256
106f9e287110988720cc717aaaa22190cafcf90fb184fd5a83e9d467d93165df.
A fresh private prefix fixed the failed native display enumeration without modifying
the old prefix or global monitor settings. Private OpenVR bootstrap configuration
was necessary for Proton to process the existing xrizer override before wineopenxr.
The actual host reached FOCUSED, format87, both1280x720 swapchains and remote
simulated Index profiles; these differ from the earlier WMR simulation.

A later run reached local Jungle simulation with online interface0 and observed
the native null-interface shutdown wrapper. Thirty advancing actual input snapshots
matched the commanded neutral head pose. The first eye camera hook executed, but
world completion was0 despite admitted targets and UI ownership. No Ready world
pair was captured and xr_world remained0. The new bounded eye-stage diagnostics
will distinguish projection admission, target restoration and postlude failures.
A native press-key loading screen was observed; automatic dismissal remains under
investigation, so this is not yet a fully repeatable direct-gameplay acceptance.

The user reports trees and a waterfall visible through terrain in desktop output.
That output has not been established as stock, and VR remains affected until tested.
Exact location is unspecified. Compare an identical scene/camera in stock and
modded desktop and each eye; inspect depth size/format, clear/test/write state,
opaque versus alpha-tested/transparent ordering and restoration between passes.
Keep terrain, vegetation and waterfall enabled. The regression remains open.

The subsequent root-view trace found exact first failure: prepared depth0..1,
executed depth0..0.899999976, with callback, view, projection and near endpoint
checks passing. The current strict far-endpoint equality rejects the native
executed segment. Native partition evidence and the narrow correction are under
Astra review; no predicate has been relaxed. A modded desktop image was captured
automatically at the neutral baseline. No matching stock-renderer or accepted
eye occlusion comparison is available yet.

That diagnostic package archive SHA256 is
9748e1bba2677d3d46daa6396eab3ec149680a9e42f8b84d19605b22d82b0a94;
compiled source fingerprint
1ed2a5d581e2c19f2c1af0612b878acee1a3e71114669551629ed4718c1734c5.
All60 current offline Debug groups passed. Source review was bounded normal-path
Astra/xhigh; serving-backend/effective-setting introspection remains unavailable.
No full exception, reset, scope, hardware or network GO is claimed.
