# Fresh native collision-query extent — Astra disposition

Fresh gpt-6-astra/xhigh; effective current-turn routing and Engine/Core pins
verified. Static inspection only. Result: **UNPROVEN exclusive ownership**.
Native initialization and normal cleanup are established, not body movement.

## Established ordinary lifecycle

Stock callers initialize, configure, check, copy results and return. Cleanup is
usually deferred to the next initialization:

| Stock caller | Exact sequence |
| --- | --- |
| Primitive CheckMove52030 | init520AB, check52123, consume, clear excluded avatar/mechanism52171/52178, return52185 |
| Hovercraft GetDistToGround121880 | init121943, check121972, consume121985, return12198E/121998; caller copies first result12207C before next fresh query1220AF |
| Rendering caller | initE33F8, checkE3469, copy position/normalE347A/E3483, returnE3889 |

rayInit1B35E0 is voidcdecl. It calls registered cleanup vtable+4 with ECX=this
at1B3601, unlinks the node1B3607 and resets defaults through1B36A4. Collision
cleanup2D9750/vtable209260/28B50 is registered at2924E. Model cleanup
2EAB28/vtable214700/D8880 is registered atDA404 and resets the triangle index
and five model-hit pointers. A linked cleanup node after normal return is
expected residue, not necessarily unfinished traversal. cldCheckRay29200 is
intcdecl; normal result is EAX.

## Joined boundary and one missing edge

Simulation1B7406 calls physics10D7C0. Body callbacks10D80F, collision10D822,
contact reduction10D829 and joint creation precede dispatch10D930. Its joined
continuation10D935 precedes contact callbacks10D954, pending-array release and
joint destruction. The routine receives this in ECX, retains ESI, plainret at
10D9E6. **10D935 is an interior continuation**, reading pending count[ESI+74];
callback storage[ESI+70] and joints remain live. There is no ray cleanup between
the dispatch return and callbacks. The demonstrated solver workers are joined;
this does not prove universal ray ownership.

Engine traversal2911E and model preparationDA421 can call smart-object slot0C.
CWorld vtable207410 maps it to thunk1BF786 → Core AcquireReplacement3E560.
On the main thread,3E578 enters mlExecuteAllPostLoadTasks46AA0 →46ADF→469E0.
At46A2D, native task virtual+8 receives ECX=task and one resource argument;
task destruction follows46A61. Proxy AcquireReplacement47200 can also wait
through46D80 and pump tasks46E25. These conditional paths are demonstrated;
an actual nested ray is **not** demonstrated. The unresolved proposition is
whether their reachable task targets can initiate public rays/unrelated
triangle work, or whether the path is unreachable during an admitted query.

## Main disposition

| Finding / recommendation | Decision |
| --- | --- |
| Normal fresh init/check/consume with deferred cleanup | Adopt exact native lifecycle reference |
| Both collision and model registered cleanup matter | Adopt; no filter-only restoration |
| Joined10D935 excludes that solver dispatch but has live contact state | Adopt narrow worker proof; not callable exported entry or full ownership |
| Resource replacement can pump queued callbacks | Adopt as the single next ownership proof; no guessed readiness flag |
| Conditional normal extent init→configure→check→copy→init | Candidate only after callback closure; no preservation of prior transaction |
| Native query unwind handler1E28A9/228830 only destroys profile sample1E28A0 | Preserve native exception propagation/mod TLS cleanup; never claim rayInit repairs interrupted visited-hull traversal |
| Source activation/placement/replication | Still gated; no hook or movement added |

The next bounded investigation is the native post-load-task registration/target
set reachable from the resource replacement path, including its proxy variant.
It must establish non-reentry or produce a concrete counterexample; another
global query lock or scalar save/restore layer is not the default solution.
