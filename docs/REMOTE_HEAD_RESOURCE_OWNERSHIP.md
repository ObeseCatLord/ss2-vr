# Remote head: finite native resource-lifetime gate

The existing post-DDE30 disposable palette adapter remains default-off. This
record extends NATIVE_HEAD_ANIMATION_AUDIT.md and REMOTE_RENDER_UNWIND.md; it
does not replace native animation, storage or scheduling. Astra/xhigh conducted
bounded read-only inspection of the pinned Engine/Core/Sam2Game. Main verified
local current-turn model/effort tags; backend routing introspection is unavailable.
No game, native callback or network session was executed for this investigation.

## Verified ownership and callback edges

CModelRenderable GetModelInstance15B220 returns its instance at+5C. RenderObject
15C7ED/15C7F5 passes that borrowed pointer to mdlRenderModelE2B10. BuilderE1928
stores it in record+54; E19F5 stores borrowed bone definitions in bone+24. The
renderable destructor15B7A7/15B7B0 calls mdlDeleteModelInstanceD56A0, which destroys
and frees the instance atD56BF/D56C5. No draw-held instance reference is proved.

Configuration replacementD2D40/D2D6A retains instance+18's new configuration and
releases the old one. Skeleton replacementD32F0/D331A does the same for
configuration+30. Core CSmartObject AddRef48500/RemRef48530 serialize count+8
with their existing mutex; last release calls virtual+8. This ABI cannot be
applied to CModelInstance: instance+0/+4/+8 are stretch floats. Reference-count
serialization alone is not safe acquisition from an unprotected borrowed pointer.

EvaluationE248F→D86A0 links instance+28 and evaluated-object+18, then publishes
the evaluated pointer2EAB68. Instance destructionD3EC8→D8110 clears these links
and evaluation counts atD8148/D8181. Final renderE2F48→DAD90 resets renderer
counts/pointers without a matching object retain/release. Retaining only config
and skeleton resources cannot establish instance/cache protection.

After the palette producer, EngineE2E19/E2E4A invokes destruction-holder resource
replacement through configuration+44. CDestructionHolder vtable209750,+0C points
through1BF786 to Core AcquireReplacement3E560. On the main thread,3E578→46AA0
drains the global post-load queue. Queue mutexBEF3C serializes enqueue/execution,
not the borrowed render objects. Main-thread enqueue468CD can execute immediately.

One resolved configuration task is enqueued atD2A5A; vtable214708,+8 targets
D8BC0→D57E0, mdlUpdateFadeAndCullRanges. Verified writes affect config+74/+78/+7C
and recurse child configurations; this is not itself an instance retirement proof.
A separate real deletion task can reach the same queue: Core390F0 selects non-null
BED74 entries with BED94/BEDA4 both zero at391C0–391F8, constructs CDeleteObjectTask
vtable82EE8 at39281 and enqueues392B7→46880. Its callback38990 resolves its meta
handle then invokes mdDelete2E900, whose2EA19 dispatches the datatype destructor.

## Disposition and precise next edge

| Candidate | Disposition |
| --- | --- |
| Native temporary palette with complete provenance and explicit cleanup | Retain existing implementation |
| Zero queued loading tasks as worker quiescence | Reject; dequeue precedes execution |
| Insert WaitUntilCompleted during rendering | Reject; it executes tasks and changes native scheduling |
| Config-only retain as instance/cache protection | Insufficient; do not add it as a substitute |
| Generic deletion queue as proof the selected model is invalidated | Not established; requires typed target association |
| Enable RemoteHeadTracking now | Deferred pending the finite target/reference exclusion below |

Determine whether the active instance/configuration/skeleton can satisfy the
Core391C0–391F8 selection, including the meaning/writers of both zero-count
arrays and the selected datatype destructor. The queue's deletion capability is
proven; association with the currently borrowed model is not. No new engine lock,
metadata registry, scheduler or lifetime workaround is justified by these facts.
Primary hardware/gameplay acceptance remains with the user.
