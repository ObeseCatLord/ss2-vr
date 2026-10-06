# Native post-load task closure — Astra disposition

Fresh gpt-6-astra/xhigh; effective current-turn settings and Engine/Core/Sam pins
verified. Read-only static inspection. **UNPROVEN non-reentry.** Five concrete
task implementations are established; no actual nested public ray/triangle
invocation is demonstrated. Complete transitive callback closure remains absent.

Core46AA0 drains global queueBEF58/countBEF5C, not just the resource triggering
replacement. World/model replacement therefore cannot restrict task types.
AddPostLoadTask46880 is voidcdecl(resource,task): main-thread branch invokes
virtual8 immediately468CD; other branch queues469B3 and sets resource+4 bit0
for non-null resource469B9. Queued46A2D uses ECX=task with one resource stack
argument; concrete targets ret4; virtual0 destruction follows46A61.

| Concrete task | Registration / vtable / virtual8 |
| --- | --- |
| DeleteObject | Core392B7 nullresource /82EE8 /38990→389C3→mdDelete2E900 |
| VertexBufferUpload | Engine7C510/7C51F /20FD0C /7C950→7C95A→7C810 |
| TextureUpload | Engine85B03/85B15 /210804 /84110→resource58, then90 at84126 or7E3D0 |
| FixModelPost | EngineD2A5A, constructorD8BA0 /214708 /D8BC0→D8BC7→D57E0 |
| SoundUpload | Engine1A2643/1A2655/1A2878 /2204DC /1A2460→1A247B→19EEE0 |

The audit accounts for one direct Core and eight Engine IAT registration calls,
including allocation-failure branches; Sam2Game does not import this API.
This is not complete indirect/reflected callback closure.

Reflected loading3860E→mdPostRead2DF10→2DF2A/2D640 resolves inherited datatype
functions+1C and callsEAX at2DF36, cdecl with two-word(type,object) metapointer.
Member/container callbacks also dispatch through2DB20 at2DBB1/2DCB4/2DD69.
Concrete model registration D3C30→D3C3B→D2A00→D2A5A is reflected metadata
tag80 at32AC4C/targetD3C30 at32AC50, mapped to functions+1C byCore2417E.
Not all post-read callbacks are proved reachable inside the questioned ray.

The smallest remaining actual task descendant is deletion:
46A2D→38990→389C3→2E900→2EA19 calls a reflected destructor, cdecl
(type,object,-1), caller removes12bytes. It reads functions=[type+1C], then
target=[functions+4]. No bounded type/object set for queued deletion is proved.
Model repair reenters smart-object replacement D5847/D5872/D58A4; texture
upload and sound19EEFC also have indirect backend dispatch. Closing deletion
alone would not close these branches. None proves an actual nested ray.

## Main disposition

Adopt the concrete task set and global-queue scope. Reject an inference that
world/model resource identity excludes unrelated callbacks. No ready-state
exclusion, queue suppression or body-query activation is approved. Reopen the
ownership design before an expanding whole-program callback audit: compare a
proved native resource-readiness admission or an existing owned native query
extent against the fresh query candidate. Unknown callback reachability is not
evidence to replace physics/world traversal or add global serialization.
Full roomscale body movement/geometry/primitive accuracy/authoritative settlement
remain required. No native source, body placement or protocol changed here.
