# Astra remote head architecture brief — 2026-10-03

Goal: wire received full XYZ/quaternion head tracking to native remote-avatar head skinning, preserving native animation, affine model transforms, weapon/body behavior and native multiplayer. Solo mod; no game/headset/runtime testing authorized. CPU-skinning coverage remains required for full completion; its exact consumer is investigated independently by Atlas in scratch CPU/. Review this bounded hardware vertical slice and pair coherence; do not settle the whole goal from GPU-only progress.

## Facts and environment

| Fact | Evidence/status |
|---|---|
| Existing remote binding reaches exact current player handle/incarnation/body CModelInstance | [verified: src/game/remote_render.cpp observePlayer/currentBinding; no guessed global avatar] |
| Current record global2EAB50, instance+54, world+24, native records stride68 | [verified: Engine E12B0; existing remote adapter] |
| Draw-entry global2EAB48 is a pointer; start/count at entry+4/+8 | [verified: Engine DC510/DCD10] |
| Map pairs pointer2EAC74,count2EAC78; each mapping's+4 is native bone index | [verified: E1DE5..E1E39 and DDE30] |
| Bones pointer2EAC64,count2EAC68,capacity2EAC60; stride28, owner+0,parent index+4,definition pointer+24 | [verified: E18D2 captures new model index; E19E3..E19F5 stores fields; native DB140 independently accesses+24. Main corrected the initial+4C off-by-one.] |
| Runtime Head ID must use Core strConvertStringToID; serialized ordinal17 cannot be reused | [verified: exact Core export and Engine1BA0 hidden-output/string/caller-cleanup8] |
| P is model-local skin palette; W is applied separately; B=P*inverse(S) | [verified: DDE30, E12B0 and DB140; audit has RVAs] |
| shaBoneMatrices77590 retains pointer until draw/clear, not immediate upload | [verified: exact setter body, DCD10 DD0D2..DD106; main corrected unsafe lifetime assumption] |
| shaBindBoneMatrices775B0 actually submits pointer,count*3 through native gfx callback | [verified: exact body; caller645A6 return645AB, register17] |
| GfxD3D6330 callback invokes D3D9 SetVertexShaderConstantF vslot94 | [verified: startup binding reference4BEF, callback636C; API documentation linked in audit] |
| Head has no stock descendants; runtime parent indices can cover changed descendant layout | [verified: owned stock SKL and native builder; no custom-resource support assumption] |
| Received head Q is a tracking delta in body-eye frame, including origin/snap-turn correction | [verified: math.hpp bodyHeadTracking/worldHeadTracking and network production; Q is not an absolute skull pose] |
| CPU branch reaches other native evaluation and bypasses setter | [verified conditional; exact CPU consumer unknown, Atlas owns it] |

Repo ss2-vr, native files ../Bin. Installed fingerprints in docs/installed-build.json. Main audit docs/NATIVE_HEAD_ANIMATION_AUDIT.md. Scratch /tmp/ss2-vr-mp/head-animation-investigation has bounded helper/dumps; matrix helper affineMultiply/affineInverse already in common/model_tree.hpp. New source baseline a45c67b, IPC6/wire4, package0.2.3 retained. There are no production head edits yet.

## Minimal design versus replacement

Reuse remote_render's existing binding bank and frozen eye-pair bank. Add a hook on exported shaBindBoneMatrices, not the retained setter. Only accept the exact shader caller645AB, native retained pointer matching paletteBase + start*48, matching count, bounded map/bone/record capacities and current record within native record array. Resolve that exact record.instance to one live current remote binding. Find a unique runtime Head bone owned by that record index. Use native parent indices to mark mapped Head/descendant slots; validate graph indices/cycles atomically before changing copied matrices. Never call mdlGetBoneAbsolutePlacement inside draw: it rebuilds shared render globals.

For accepted tracking delta Q and frozen native body-eye E:
`Dworld=E*Q*inverse(E)`; `Dmodel=inverse(W)*Dworld*W`; selected copy `P'=Dmodel*P`.
Use existing affine functions, retaining native stretch/shear/mirroring and skin correction/animation. Identity Q takes byte-exact passthrough. Nonselected matrices are byte-identical. No head value becomes body physics; full XYZ may stretch blended neck weights and is not full-body IK.

Create a bounded copied span only at the synchronous consumer. Invoke the same native gfx callback `(firstRegister, count*3, copiedFloatData)` for accepted adaptation; otherwise call original shaBindBoneMatrices once. Native deferred pointer, palette and shader lifecycle remain untouched. Prefer this over a setter-owned arena/window or replacing animation: it deletes lifecycle state instead of adding it. Freeze sample/body/binding eligibility before the left eye; no age expiry between eyes. Recheck handle/incarnation/body-instance lifecycle without substituting a newer sample.

Pair coherence amendment: if the frozen remote bank/lifecycle changes during a stereo transaction, reject the whole world pair at commit through the existing commit path. A per-eye skip after earlier writes is insufficient. Preserve desktop native restoration and existing ready/epoch/cancellation ownership. This should be a narrow remote_render pair-validity query and commitStereo predicate, not another frame coordinator.

## Open decisions/current lean

1. Actual constant consumer versus retained setter: lean consumer, no engine-global pointer mutation and no retained plugin scratch arena. Validate the callback lifetime and whether native wrapper does anything omitted.
2. Full delta versus skeletal replacement: lean delta about body eye, preserving animated palette; identity exact. Explicitly assess anatomical head origin/large crouch translation limitation; do not quietly remove XYZ or advertise IK.
3. Descendants/ownership: lean native resolved parent-index traversal with unique Head/record owner, bounds and cycles checked. Treat unknown/ambiguous graphs as native fallback; don't infer universal ID17 or duplicate skeleton state.
4. Stereo/lifecycle: lean one frozen bank plus commit rejection if bank becomes invalid; challenge exact invalidation/epoch requirements without revisiting unrelated transport. Remote existing tool presentation should benefit from coherent rejection too.
5. CPU coverage: this review approves at most a hardware implementation slice. CPU consumer remains a required extension. Don't redesign CPU skinning or advise claiming completion.

## Review contract

Astra gpt-6-astra/xhigh; read-only source/native review, no subagents/production writes/runtime. Scratch only /tmp/ss2-vr-equivalence/astra-head. Main owns implementation, Atlas exclusively investigates the CPU consumer. Verify cheap load-bearing claims first; rank findings, choose highest-leverage deep spec, give go/no-go for this bounded implementation and explicit evidence gaps. <=1400 words. Don't re-review muzzle/UI/lasers/scopes/full movement/network. Return final; missing evidence must be labeled and bounded.

## Reopened decision: single producer adapter

New verified evidence is in NATIVE_HEAD_ANIMATION_AUDIT, last section; CPU worker findings scratch CPU/FINDINGS.md. Replace two consumer adapters with a post-DDE30 adapter ONLY at exact normal-render return E2E06; non-render DA4E6 remains unchanged. Original producer executes exactly once first and fully refreshes palette. Plan validates bounded record/mesh/draw/map/bone/palette spans, unique body record per admitted binding, unique owned runtime Head, all native parent graph indices, native sentinels and draw-entry provenance. Compute all selected matrices into invocation-local copied palette before any native palette write. Write only selected same-owner Head/descendant slots; unmatched/fallback bytes remain exact. Canonical evaluated source matrices, native pointer, cache, animation and CPU/GPU state remain unchanged. No setter hook, shader callback bypass, CPU pointer swap or new skeleton state is needed. Cross-owner head attachments excluded until separately proven; stock Head has no children. Full XYZ remains.

Minimal alternative is your approved GPU synchronous consumer plus separate CPU pointer swap, which duplicates adaptation and exposes global pointer lifetime. Producer changes temporary renderer matrices (like existing post-model world records) rather than canonical pose. Challenge whether that change is justified by exact freshness/order proof and whether additional consumers need coverage. Main lean: producer. No source edits yet.

Pair plan: acquire binding shared lock and a narrow multiplayer PresentationReadGuard through commitNativeFrame/Ready under existing snapshot lock. Order is IPC ownership→snapshot→binding→multiplayer. Existing writers release multiplayer lock before remote/snapshot calls; deletion obtains remote lock before native originalDelete. Freeze bank under binding lock plus MP guard, with freshness only at admission; frozen eligibility/sample/body anchor never changes between eyes. Revalidate handle remote/alive/exact body instance and MP avatar/incarnation/nonce/trackingGeneration/valid mask (ignore wall age and newer compatible sequence). Sticky pair fault survives desktop restore. Relevant binding writers lock exclusively and invalidate admitted bank on incompatible changes; compatible observation updates next bank only. MP guard blocks relay replacement/invalidations through Ready. Native simulation/model-object mutation is assumed same native game thread, existing render/simulation boundary; if this cannot be statically established, refuse concurrent thread rendering instead of pretending locks protect all engine objects. Assess smallest enforcement needed. Empty banks native-only valid. Ordinary absent draw/head mapping native fallback; malformed selected mapping invalidates pair, not partial adaptation.

Re-review contract: Astra/xhigh, read-only, no extra subagents, no runtime, same scratch. <=1000 words. Verify new load-bearing producer/order/provenance claims, choose producer vs dual-consumers, identify concrete implementation gates. Do not redo full CPU loops, prior affine proof or unrelated features. Main owns integration.
