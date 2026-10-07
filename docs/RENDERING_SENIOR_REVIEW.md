# Rendering investigation: Astra senior review and disposition

2026-10-07, public base b654b74a with a fixed 15-file development snapshot.
Explicit gpt-6-astra/max selection was confirmed from the reviewer's local
current-turn model/effort tags. Serving-backend introspection was unavailable.
The reviewer verified snapshot hashes, inspected source and bounded owned native
evidence, and made no changes or runtime launches. This is a rendering/test design
review, not full-mod, hardware, network or lifecycle acceptance.

| Recommendation | Main disposition and current state |
| --- | --- |
| Repair the actual native input mapping before changing UI admission | Adopted. Position-missing is demonstrated. The actual bound builtin program declares TEXCOORD0 into input register0. A fresh Astra/xhigh investigation independently verified the common backend route for all six builtin variants and matching binary fingerprints. The minimal gate admits only the unique stream0/offset0/FLOAT3/default/TEXCOORD0 position element under existing builtin program identity. No global semantic alias, replacement shader or UI bypass. |
| Correlate camera, pixels and successful projection by request identity | Adopted. Camera diagnostics now include request/session/reference/tracking generation for held poses. Accepted native pairs and successful host projection submissions emit matching bounded receipts. The observer records that identity, UI request and completion. Assessment requires exact identity, not another frame at the same pose. |
| Require sustained complete rendering, distinct from fresh input or cached reuse | Adopted. Assessment requires 30 distinct neutral world/UI pairs with both native camera observations and successful matching projection submissions, plus the seven actual pose captures. Counters remain diagnostics; visible near/far parallax remains required. Runtime acceptance is open. |
| Build a stock-renderer comparator before changing depth/state restoration | Adopted and implemented as the private comparison slice; see STOCK_RENDERER_COMPARISON.md for actual probes and limits. Select the private fixture mode before workers/device hooks, retain scene and online-isolation startup hooks, verify five native binaries and completed installation, and reject the unvalidated 9Ex route. Readiness/capture must work without a host/channel. Match scene, settings and camera; call it stock renderer with fixture hooks. |
| Retain the exact root-depth partition adapter | Adopted. Main confirmed the narrow callsite and strict generic predicate. Matching native parent writes15EF50/15EF60, child endpoints15F23D/15F244, application155DCB and restoration1560B2 support only the exact float32 partition. This does not prove terrain occlusion or restoration. |
| Move expensive pixel diagnostics outside IPC ownership | Adopted and implemented. Existing owned CPU copies are hashed/compared and logged after the shared-channel lock is released. Pixel transport/commit and once-only native rendering remain unchanged. |

The review changed the testing contract: the former assessor joined cameras by
pose and accepted any successful world submission in the run. That could combine
unrelated requests. Exact correlation now rejects missing, stale, different-session
or different-generation evidence. Producer diagnostics changed with the assessor,
because changed-pose-only logging could not prove later held-pose captures.

The minimal implementation remains an adapter to the original shaders, buffers,
native draw arguments and once-only overlay callbacks. Original desktop drawing
occurs exactly once, with its HRESULT and query order preserved; partial eye replay
rejects publication and follows existing cleanup. Keep native topology, stream,
clipping, viewport, program identity, owner and generation admission. No new
renderer, transport, shader substitution or gameplay state machine is justified.

The user-reported desktop terrain leak remains a separate open defect. UI input
rejection does not establish its cause. Compare stock-renderer and modded desktop
at the same scene/camera, then both native eyes. Keep terrain, trees and waterfall
enabled; inspect attachment compatibility and native depth/state/cache/ordering
only against a demonstrated difference. Exact reported location is unknown.

Preservation evidence is narrow: the two inventoried original cloud payload hashes
remain unchanged; Steam launcher remotecache metadata differs. Private settings,
scene/save assets, shader code, captures and operational telemetry remain local.


## Follow-up: right-eye terrain occlusion

The user localized the defect to the right eye. Actual forwarded draw observations
revealed early opaque/alpha-tested right-eye geometry at0..1 and later transparent
effects at0..0.9, while the left eye stayed0..0.9. Astra/max verified the native
bitwise endpoint cache early-return and native geometry setup reference. Main
spot-checked that evidence and adopted preservation of actual pre-bind endpoints
while changing eye/scope/desktop viewport geometry. No cache mutation, native
physics/rendering replacement, force-enable or hidden effects.

Baseline → fix → old-build counterexample reproduced the predicted state/visual
change, including independent user observations. The fixed product is restored;
full seven-pose follow-up is running. The bounded reproduction and full-mod
acceptance remain distinct. See STOCK_RENDERER_COMPARISON.md.
