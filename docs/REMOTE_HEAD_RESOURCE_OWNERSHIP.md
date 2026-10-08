# Remote head: finite native resource-lifetime gate

This record extends NATIVE_HEAD_ANIMATION_AUDIT.md and REMOTE_RENDER_UNWIND.md; it
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

## Historical disposition before the typed exclusion

| Candidate | Disposition |
| --- | --- |
| Native temporary palette with complete provenance and explicit cleanup | Retain existing implementation |
| Zero queued loading tasks as worker quiescence | Reject; dequeue precedes execution |
| Insert WaitUntilCompleted during rendering | Reject; it executes tasks and changes native scheduling |
| Config-only retain as instance/cache protection | Insufficient; do not add it as a substitute |
| Generic deletion queue as proof the selected model is invalidated | Not established; requires typed target association |
| Enable RemoteHeadTracking now | Deferred pending the finite target/reference exclusion below |

The next investigation was to determine whether the active instance/configuration/skeleton could satisfy the
Core391C0–391F8 selection, including the meaning/writers of both zero-count
arrays and the selected datatype destructor. The queue's deletion capability is
proven; association with the currently borrowed model is not. No new engine lock,
metadata registry, scheduler or lifetime workaround is justified by these facts.
Primary hardware/gameplay acceptance remains with the user.

## Completed typed exclusion and restricted source adapter

The subsequent bounded investigation resolved the selected load-cleanup edge.
Core's BED94 flag marks a resource-registration row; ordinary resource routes
retain that flag. BEDA4 marks an accepted serialized reference/root, rather than
being a reference count. Reader36754 marks the selected row at367D1 before
assigning the reference at367F8; returned roots are marked before cleanup3865A.
The accepted renderable's raw instance reference and the instance's configuration
and skeleton references therefore exclude these published objects from the
ordinary unaccepted-object selection at391C0–391F8. An instance allocated through
D3FD0 does not enter that loader row merely by allocation. The actual instance
datatype deletion still reaches D51A0→D3DF0 and retires its evaluation cache;
positive resource reference counts alone do not veto that queue.

This is a positive exclusion for the stock selected-model/load routes. It does
not certify every unpublished metadata row, arbitrary worker or other mod.
Admitted stock liveness, model-instance, local-role and client-index getters do
not pump replacement tasks. Their raw results remain borrowed; no new reference
scheme or engine lock has been added.

The source adapter now writes tracked head palettes only inside the existing
admitted **frozen VR stereo pair**. It rejects ordinary desktop passes before
binding reads and removes the old post-producer body-anchor capture. It uses the
anchor already frozen before native stereo production. Original palette
production remains once-only; scratch mapping, canonical non-aliasing, late
invalidation and native-finally retirement remain in place. Ordinary desktop
observers retain native animated heads. This restriction narrows the earlier
adapter rather than extending the native model lifetime.

The shipped option and missing-key fallback are enabled for that restricted
extent; an explicit `RemoteHeadTracking=0` still disables it. Actual multiplayer
head appearance, accessories and lifecycle acceptance belong to the user's
in-VR pass. Do not describe this as universal remote-avatar or desktop coverage.

The compiled UI-fault retirement audit also exposed a lazy TLS initializer in
the historical helper. Normal slot admission now publishes a same-thread pointer
to the already constructed frame; slot retirement clears it. Native fault
cleanup reads that pointer and performs only scalar field changes. It cannot
construct the frame, register its vector destructor, allocate or call native
code during unwind. The compiled verifier checks both entry and compiler clones,
alongside all seven existing remote cleanup extents.

## Final source review and verification

| Astra recommendation | Integration disposition |
| --- | --- |
| Restrict palette writes to the existing frozen native render extent | Adopted; removed ordinary desktop admission and its post-producer anchor getter |
| Reuse the already frozen body anchor; no new retain, worker lock or task pump | Adopted |
| Remove TLS construction from native UI-fault cleanup | Adopted with a constant-initialized same-thread admitted-frame pointer |
| Inspect fault implementation and compiler clones rather than trusting a name | Adopted in the compiled gate |
| Reject indirect or outgoing tail jumps from the scalar fault helper | Adopted; all branch targets must belong to its inspected body |
| Keep actual appearance and broader coverage separate from source GO | Adopted; user's multiplayer acceptance remains required |

Astra/xhigh approved the restricted source and actual compiled cleanup. Main
verified local current-turn routing tags; effective backend identity remains
uninspectable. Both x86 bridge objects are SHA256
`f911e02544e7b4a3a34c02b80baedebad0dbaca3a272ee43b9a042c62c5e6182`;
both remote-render objects are
`8614ccefa55039fc23bb53bf1449af91b27f22368825e21ac8c0901f1b3b49c2`.
All products rebuilt from source fingerprint
`6993876190e0beb3fc10e966a1edab6280eb5bcc8617e912bda686d4e589ac56`;
61 Debug groups, artifact/IPC10/wire6 and native-finally compiled checks pass.
Eight private modified-object controls inject calls, FP, indirect jumps and
outgoing jumps into each helper variant; all reject and originals pass. No native
exception, game, hardware or network runtime was executed for this change.
