# Scope pre-collection native pose gate

Independent fresh Astra/xhigh investigation, effective routing verified;
owned DLL fingerprints match. No edits/runtime in review. Main disposition:
NO-GO for enabling a pre-collection scope camera today; conditional narrow
adapter remains preferable to a preview render or replacement evaluator.

| Verified finding | Disposition / remaining proof |
| --- | --- |
| Sam4BFF0 GetWeaponAbsPlacement is intthiscall, two Matrix34 references, ret8 | Reuse original authored calculation with frozen frame inputs; current nativeWeaponReference reacquires camera/body |
| EngineD8D60 bone getter intcdecl(model,IDENT32,outMatrix34) resets scratch through DAD90 | Exclusive native query ownership and unwind/partial-cache cleanup must precede use |
| Query result Q = Wquery × C × inverseRT(D); draw = Wdraw × C | Simple tracked-placement × Q is rejected; actual rigid definition D at bone-definition+48 requires right-side correction |
| DB020 returns first matching name across scratch records | Prove unique actual model/definition ownership, not a name alone |
| Native draw negates X at4CA0C, SetStretch4CA31, mdlRenderModel4CA48, restore4CA67 | Correct previous interpretation; preserve complete reflected/stretched affine and multiplication order |
| Query setup calls hooked DBC90, not draw palette DDE30 | Query purpose must preserve native evaluation but exclude unrelated remote adaptation and cover unwind |
| Main thread + nine idle counts do not prove exclusive ownership | Identify outer boundary excluding collection/evaluation/reentry; no mod-only mutex or pointer restoration after allocation |
| Native zoom u=F0+(EC−F0)×F4, projection uses theta×u | Use tan(theta/2)/tan(u×theta/2), not assumed1/u |
| scopeGpuEyeOwner admits only real retained eye targets | Extend existing frozen owner with explicit source purpose; no fake eye/second pose authority |

Smallest proof: one sniper with ordinary offhand, both assignments/two eyes;
query before collection; compare converted query affine with each actual native
draw through animation and warm/cold cache. Missing/duplicate bones, busy/nested
query, lifecycle/configuration change and correspondence mismatch reject image.
Cleanup after partial setup and allocation failure is a distinct required proof.
No source camera/auxiliary image enabled by this evidence. Existing rendered
palette observation remains the reference. Native runtime testing is excluded.

## Ownership / partial setup follow-up

Fresh independent Astra/xhigh, pinned Engine/Core/Sam static trace: NO-GO for
the public bone query at this boundary. This is not evidence that serialization
is impossible; no common exclusion witness was established for the concrete
record-count, bone-count, evaluated-pointer and palette consumers. RenderView
FEA84→Render3D establishes ordering before this collection, not a model-worker
barrier. Native task dispatcher10EE90 waits before returning, but the required
model/cache-writer-to-task-set/scheduler association is still missing.

DAD90 is not rollback. It clears counts/pointer and timing sentinels but keeps
capacity/backing/model references/cache lists. Concrete failure: DD550 publishes
capacity atDD58C before allocationDD593 and backingDD59C. An escaping allocation
callback can leave capacity nonzero/backing null; clearing count cannot repair
it. E2670 unwind1EA119 only destroys CProfileSample. Ordinary memory exhaustion
reports fatally and exits, rather than returning a recoverable getter result.
No adapter may catch partial setup, reset counts and continue native rendering.

Main adopts the reviewer's bounded alternative: pursue one completed ordinary
first-eye preview at the already-used originalRender boundary, then auxiliary
source render(s) and final eyes/desktop rebuild. It adds one native render with
unmeasured cost, avoiding a new public-query ownership/cache-recovery contract.
Copy derived optical parameters before endEye clears observations; keep them
under the existing frozen owner and verify preview/final draw correspondence.
Native cleanup must complete before auxiliary collection. This is the next
proof candidate, not scope-image enablement or removal of any admission gate.
