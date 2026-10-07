# Native remote head-animation evidence — 2026-10-03

Read-only static investigation of the fingerprinted Engine/Sam2Game and owned model resources. No runtime was executed. This audit is not a head-animation implementation or review signoff.

The owned SeriousSam.mdl references Sources/SeriousSam.skl. The installed Patch_02_090_593613 skeleton metadata has24 bones: Head's serialized IDNT ordinal17 has Neck ordinal12 as parent, and no bone has Head as parent. These ordinals are file-table references, **not runtime IDENT values**. Resolve runtime `Head` using Core strConvertStringToID (hidden IDENT output, string argument, caller cleanup8). The Engine wrapper0x1BA0 demonstrates that ABI.

The remote player/renderable chain already resolves its exact current CModelInstance through GetModelInstance0x15B220. Native dispatcher0xE12B0 sets the current record global0x2EAB50 from the0x68-byte render-record array0x2EAC24; record+0x54 is the instance and+0x24 the model/world matrix. Native evaluated skin matrices remain separate from that model/world matrix.

The current draw entry is the pointer stored at0x2EAB48. Its+4/+8 are mapping start/count. Mapping pairs at pointer0x2EAC74 have stride8 and store a resolved bone index in+4; count is at0x2EAC78. Bone records at pointer0x2EAC64 have stride0x28, count at0x2EAC68, and definition pointer at**record+0x24**. The definition's first dword is runtime IDENT. Main corrected an investigation off-by-one: native lookup starts at index1 and base+0x4C, so that address is `base+1*0x28+0x24`; it is not a+0x4C field in every record. Native0xDB140 independently accesses the selected record+0x24.

Native0xDDE30 copies evaluated model-local skin matrices from the current evaluated object (pointer0x2EAB68, matrix pointer+0x20) to temporary palette pointer0x2EAC94. Getter0xDB140 yields model-relative animated bone placement `B=P*inverse(S)` from palette P and serialized correction S; renderable absolute-placement wrapper0x15C060 then applies the native model/world transform. A received body-relative head delta Q about body eye E can therefore use `Dworld=E*Q*inverse(E)`, `Dmodel=inverse(W)*Dworld*W`, and `Phead'=Dmodel*Phead`. This preserves the native animation/correction and affine world transform. Identity Q should bypass multiplication and retain exact native bytes. Full XYZ affects Head-weighted vertices and can stretch the blended neck boundary; it does not establish full-body IK.

**The palette setter retains a pointer.** Export shaBoneMatrices0x77590 stores its three arguments in globals0x2E49FC(pointer),0x2E4A00(count),0x2C372C(third argument), then returns. It is not a synchronous copying upload. Main verified this exact body. DCD10 calls it at0xDD0D2, performs native draw work, then clears it with `(nullptr,0,2)` at0xDD106. A scratch span owned only by the setter detour would dangle. Any copied-palette adapter needs storage lasting through actual native draw consumption and explicit cleanup/nesting handling.

The0xDD0D2 setter call is conditional on the native GPU-skinning branch. CPU-skinned consumption, exact retained-pointer draw lifetime, runtime hierarchy/changed-resource coverage and bounded palette/record/pointer checks are still being closed. The narrow candidate remains an instance-specific copied-matrix adapter at native draw consumption; no global skeleton replacement, instance pose mutation, alternative animation engine or hardcoded local palette slot is justified. Astra design review is required before implementation.

## Narrower hardware consumer

Export shaBindBoneMatrices0x775B0 reads the retained pointer/count and directly calls native function-pointer `_gfxVertexProgramConstantsF` at0x2E62FC with `(firstRegister, count*3, data)`. The identified native shader caller is0x645A6 (return0x645AB), first register0x11. GfxD3D startup binds that callback to0x6330; its0x636C device call is vtable+0x178, IDirect3DDevice9::SetVertexShaderConstantF. This is the actual hardware constant consumption boundary. A copied-argument adapter there can leave the deferred pointer, native matrices and clear/reset lifecycle untouched, rather than allocating setter-lifetime storage. Local copy lifetime through the device setter follows its input-array contract; see [Microsoft's setter documentation](https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-setvertexshaderconstantf).

Native bone records also carry model-record owner index at+0 and resolved parent bone index at+4. Builder0xE18D2 captures the new model-record index;0xE19E3..0xE19F5 stores owner, resolved parent and definition pointer. Its local parent lookup uses serialized definitions' parent runtime ID, producing an index in the same native bone array. Descendants can therefore be identified from native indices without rebuilding a skeleton or assuming the stock Head has none. Export sklGetBoneName0xE8A30 directly returns definition+0; sklGetBoneParent0xE8A70 returns definition+4. Neither evaluates a model or rebuilds render globals. These are static ABI facts; bounds/current-owner/lifetime and copied-argument adaptation still require Astra review.

## Shared temporary palette producer and CPU coverage

Static CPU investigation confirms E14C0 DC510 establishes the current draw, E14C5 E1200 calls DF5B0 at E1227, and E14D3 DCD10 draws afterward. DF5B0 reads renderer palette2EAC94 at `(draw.start + vertexBoneByte)*48` for one through four influences; output vertices2EAD54 are published to2EABC8. No canonical skeleton matrix is written. CPU normals and material behavior remain a separate limitation.

Main verified DDE30 has only two direct calls: DA4E1 (non-render query) and E2E01 (normal rendering). DDE30 copies every map slot from evaluated source2EAB68+20 to renderer palette2EAC94 starting slot0; map -1 copies native fallback2C8008. Every slot is overwritten, including cache-hit evaluations, before drawing: E2E01 → E2F0E or E2EE0 → E1570 → E12B0 → CPU E14C5 / GPU E14D3. E1570 dispatches records1..count-1. Thus post-producer changes to temporary mapped matrices do not accumulate across render invocations and reach both skinning paths without pointer replacement or retained plugin storage. This is a proposed alternative, not implemented or approved yet.

The producer runs before current-record dispatch, so ownership must come from bone.owner and mapping.drawEntry. Native record+8/+C indexes mesh entries2EAC34 (stride20 hex); each mesh+4/+8 indexes draw entries2EAC54 (stride20 hex), whose +4/+8 are palette start/count. Native E1C75/E1E24–E1E2D stores the draw-entry index into map.first. Adaptation must validate this full provenance before writes; drawing visibility is not lifecycle invalidity. Synthetic bone0 has owner0,parent-1,null definition and must remain native.

## Native thread boundary and remaining lifetime gate

CSimulation::Step1B70E0 is called synchronously through Sam IAT294948 at258D8. CPuppet Render3D94380 is a synchronous virtual+600 call atSamA0D86/FEA84; their shared scheduler is not proven. Core thrIsThisMainThread63BA0 compares native thread identity with its recorded main thread. Production pins before original simulation, requires that predicate, and refuses mismatches before native adapter reads.

EngineStep calls physics10D7C0 at1B7406. Dispatcher10EE90 schedules registered Core ExecuteThread calls at10EEB8 and waits each WaitOnThread at10EEDE before returningret4; Step returns1B7418 afterward. This proves completion of those registered physics tasks, not coverage of all model/skeleton/deletion workers or permission for concurrent rendering. Default-disabled head integration remains conditional until that lifetime coverage is established. No runtime thread IDs were observed and no game was executed.

## CPU direction channels and loading-worker counter — 2026-10-07

Root rechecked the pinned producer/consumer instructions. DDE30 copies all active
48-byte palette slots from canonical evaluation (or the native sentinel fallback).
CPU DF5B0 uses the same temporary palette2EAC94 for its position stream and two
direction streams. The position transform adds matrix translation; the checked
three-component direction transform uses the nine linear coefficients without
translation. The four-component direction path preserves its fourth scalar.
Outputs replace2EABC8/2EABCC/2EABD0 respectively. This narrows the previous CPU
coverage uncertainty; it does not certify every material/shader normal convention,
extra rendering pass, concurrent lifetime or actual appearance. The GPU setter
still retains a pointer and its binder sends three constant registers per matrix;
no consumer substitution or pointer swap was added.

The native loading task pool is a separate lifetime concern from joined physics
workers. Core CountPendingTasks63090 reads the queued count at pool implementation
+3C. PopTask62F00 decrements it before worker OnExecute63140 invokes the task's
virtual callback. Zero pending tasks therefore does not establish worker
quiescence. WaitUntilCompleted632F0 itself pops and executes tasks on the caller
before joining workers; it is not a passive rendering-side observation and must
not be inserted as one. No new task-pool lock, scheduler call or worker pause was
introduced. Complete model/resource lifetime remains unproved.

`tools/verify_head_palette_native.py` reproduces these bounded facts against the
exact Engine/Core fingerprints, normally and with Python optimization. It does
not execute native code or enable RemoteHeadTracking. The earlier GPU/CPU
architecture remains; native-unwind cleanup was separately corrected in
REMOTE_RENDER_UNWIND.md.
