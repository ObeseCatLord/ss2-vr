# Native sniper optics audit — in progress

Read-only static evidence from the fingerprinted installed Sam2Game.dll. No game, headset or network session was executed. Immersive optics are not implemented by this audit.

The sniper vtable at RVA0x2D1F00 identifies OnAlternativeFirePressed+0x200→0x171430, OnAlternativeFireReleased+0x208→base0x49000, IsZooming+0x270→0x1714A0, ActivateZoomMode+0x280→0x171CF0 and DeactivateZoomMode+0x284→0x171DF0. OnAlternativeFirePressed calls the base callback, then invokes native activation or deactivation according to+0xD4. PutDown0x171470 invokes deactivation before the original base put-down path. A native zoom transaction therefore exists; raw flag writes would bypass its restoration and owner policy.

OnStep0x172820 runs the base weapon step first. While zooming and native resources/owner are available, it computes a native time interpolation using+0xE0/+0xE4,+0xE8 and+0xEC/+0xF0, records progress+0xF4, and calls owner virtual+0x770. The player implementation SetWeaponFOVMultiplier0xF8040 stores+0x858; player GetFOVMultiplier0xF8050 contributes to ordinary native projection. A magnified optic must retain the native zoom state/timing while keeping actual runtime HMD FOV for the surrounding world.

Sniper Render0x171F20 uses a flat orthographic overlay when zooming. The current XR wrapper retains the ordinary native gun/hand model instead; desktop rendering keeps the original overlay. This does not produce a magnified scope image. The Scope surface-name and loaded-resource association are established below, but complete live content/pose admission and a controller zoom-action route remain unresolved. Existing Fire/AltFire command values represent independent hands, so assigning zoom to AltFire without tracing the native owner/event dispatch could couple one hand's fire to the other's zoom.

The existing bridge copies both completed eye images to CPU memory before publishing them. A later independent native Render3D transaction could potentially reuse an eye render target without destroying those already copied pixels; a claim that a third target is inherently necessary would be too strong. That possible adapter still needs a proven native camera/zoom input, scope pose and lens mapping, root-query isolation, exact frame/image ownership and host layer budget. No speculative material replacement, second renderer, new scope IPC buffer or heuristic lens offset has been added.

## Damage and mesh closure — 2026-10-03

Native zoom changes gameplay. ActivateZoomMode0x171CF0 passes sniper parameters+0xB4 to shooter virtual+0x1BC at0x171DD5; DeactivateZoomMode passes parameters+0xB0 at0x171F14. The CShooterEntity vtable0x2B2398 resolves that slot to SetBulletDamage0x160340, which stores its integer argument at shooter+0x44 and returns with ret4. OnFire0x171C60 retains the base fire predicate and fires the existing shooter using weapon virtual GetShootingPlacement+0x1D0. A presentation-only zoom flag would therefore bypass the native damage transaction. No damage value or raw zoom flag will be invented.

The newest owned Sniper_Fp.mdl in Patch_02_068 references Sniper.skl and Sources/Meshes/Sniper.bmf. Its identifiers include Barrel01, Sniper, Muzzle and Muzzle_TP; there is no named lens attachment. The owned All_PC_01 Sniper.skl has one root bone named Sniper; Barrel01 is a model identifier and is not this skeleton bone. This corrects the earlier audit. The fingerprinted BMF (SHA2564eb829b83b7d15068ac07ea3f1a942cf49bcd9bce5141ebd89ab5e3d8c4179b9) has a Scope surface, Scope texture and Scope normal map. That surface has884 vertices and928 triangles. Native mdlGetVertexPosition0xD9160 uses a hidden Vec3 output followed by render-mesh reference, surface reference and vertex index; it locks the native vertex buffer for12 bytes and unlocks it. Per-vertex queries every frame would be inappropriate.

tools/audit_sniper_geometry.py checks this exact owned mesh without extracting it. It verifies serialized buffer framing, index bounds, finite positions, and byte-exact position/index slice round-trips. A rear planar cap consists of22 triangles with one24-vertex circular boundary, Euler characteristic1 and98.8616% disk coverage. Its model-space center is approximately(0,0.303,0.0486), radius0.017186. This establishes a geometric cap candidate, not a semantic lens or runtime placement. Material/UV meaning, actual animated model-to-world transform, native zoom input/authority and resource-modification compatibility still require closure. The script deliberately rejects other fingerprints and retains no proprietary vertex/index data.

A narrower presentation option is an additional native Render3D capture before the two world eyes, followed by a depth-tested textured aperture drawn in each native eye. It could reuse an existing eye target and avoid new IPC images/OpenXR layers, unlike a foreground host quad. This is a design candidate pending Astra review, not implemented optics. It must hide owned gun/laser presentation during the optic capture, isolate root query history and restore native renderer caches through the final desktop render. Original native zoom remains authoritative.

CPlayerPuppetEntity::DoAttack0x101F00 invokes right-weapon alternative press+0x200 for button1 only when right+0x800 exists and left+0x804 is absent (0x101F45..0x101F85). Stock input dispatch therefore does not expose independent dual-sniper zoom. StopAttack0xFA770 selects left primary release when the left weapon exists, otherwise right alternative release+0x208. Sniper OnStep additionally requires its base alternative-held field+0x2C, and computes `F0+(EC-F0)*F4`; activating+0xD4 alone does not drive its complete timed behavior. The saved owner+0x558 is the native third-person flag: RendersIn3rdPerson0x941B0 reads it and OnToggle3rdPersonView0x7FFF0 toggles it at0x800E8..0x800F5. Dual zoom must preserve that shared owner restoration and actual native event lifecycle.

Zoomed sniper GetShootingPlacement0x172A70 returns owner purpose-0 view origin, whereas its unzoomed branch calls base0x4A6E0. The shared VR shooting seam now selects the base native attachment getter once for an accepted local/authoritative VR context. Rejected and nested contexts retain the incoming original getter with no VR retargeting. Laser queries use the same production dispatch with their explicit presentation-correlated snapshot; actual server shots retain negotiated-authority precedence and the same frozen accepted sample. Native zoom flags, damage and timing are unchanged. Astra caught and corrected weaker fallback acceptance and pointer-read ordering. The scope image/physical aperture remains unimplemented.

## Runtime surface-name translation — static closure

A bounded native metadata/loader investigation establishes CRenderMeshSurface+0 as `rms_idName`, reflected IDENT alias `23`, stride0x130. Metadata at Engine32C968/32C998..32C9AC records the type/name/alias/offset; constructorE7AE0 initializes the field from native invalidID and assignmentC7EE0 copies it. Core IDNT loader37010 calls runtime `strConvertStringToID` at37695 and stores ordinal→runtimeID at38CD0. IDENT reader277A0→35450 resolves the stored ordinal through38E70 and writes the translated ID at3547E; reflected member reader36700 supplies object+memberOffset. Thus stock BMF ordinal2=`Scope` is never a valid hardcoded runtime ID.

Runtime selection can resolve Core `strConvertStringToID("Scope")` (hidden32-bit IDENT output plus const char*, cdecl cleanup8) and compare the validated loaded surface name field. Engine `mdlGetSurfaceCount`E6890 returns section+0; `mdlGetSurface`E68A0 returns [section+4]+index*0x130 without validating pointers/index. Admission therefore requires a bounded live selected section/backing array and a unique matching surface. The pinned stock schema and header agree: alias23/IDENT, Scope884 vertices/928 triangles. Static metadata/schema checks passed. Native current weapon→resource→selected section identity is still a separate gate; a Scope name alone is insufficient to alter an instance cap. No extracted asset or disassembly is retained in the repository, and no runtime was executed.

## Loaded resource/section association — static closure

The owned weapon+24 handle resolves directly to CModelInstance, as proven by Sam native mdlRenderModel call4C9E3 and independently typed animation-queue use4B3A0. Engine configuration getterD2D70 follows instance+18; config mesh count/base are+34/+38 (entries8 bytes), and CMesh+30 is its render mesh. Native builderE1720 records instance in modelRecord+54, selected render mesh/LOD in meshRecord+18/+1C (stride20), and actual surface in drawRecord+18; the draw's mesh index+0 ties it to the owner model. Selected LOD uses section count/base+2C/+30 (stride8); sections contain surface count/base+0/+4 (stride130). Read exports E6670/E6680/E6860/E6870/E6890/E68A0 corroborate these layouts but perform no bounds validation.

CMesh's CResource base is offset0, verified by constructorCE6D0. Matching a unique configured CMesh to the selected render pointer allows Core thiscall GetResourceFile3E660→GetFileName3F920 to provide its borrowed virtual filename. Require the exact owned Sniper.bmf virtual path plus current native weapon-instance equality, live array/selected-LOD/section/draw membership, unique resource/Scope association and validated content identity. Configuration getters can resolve proxies/AddRef/RemRef, so they are not mutation-free peeks or suitable for unguarded arbitrary rendering-thread calls. Path equality alone cannot prove unchanged geometry after a same-path override. Native metadata/byte/import/fingerprint checks passed, but live resource ownership and an in-memory/content fingerprint admission remain separate gates. This closes a concrete association route without installing guessed resource or surface hooks.

## Actual Scope skinning/animated pose evidence — follow-up

The exact Scope surface paths encode weights/index format0x80, buffer0 and offsets151520/155056; all884 vertices have weights(255,0,0,0) and local bone indices(0,0,0,0). Its single serialized weightmap name is Sniper. Native reflected surface metadata32CB50/32CB7C/32CC58 identifies weights+38, indices+40 and weightmap IDs+108; format stride table2C7FB0 gives4bytes for packed weights/indices. This binds the cap to one local draw palette slot, not global bone ordinal0.

Owned All_PC_01 Sniper.skl is1700bytes, SHA256f4474f087deab8bb04eb379446f4514ae2eaa5831898e7f56e67fde73b1d5061. Its IDNT has Sniper; SkeletonLOD STAR604 has one LOD, and bone SSAR614 has one bone named Sniper with empty parent. Bone reflected metadata32D988..32DB18 proves size78hex, name+0,parent+4 and sb_mInvAbsPlacement+48hex. The pinned stored inverse-bind matrix is identity. This corrects the earlier claim that Barrel01 was its skeleton bone; that name belongs to model metadata. Static byte/schema/fingerprint checks pass without executing native functions.

Actual skinned cap points therefore need the admitted loaded draw's evaluated palette matrix and model/world transform. A root-bone rotation/translation can move the lens; rigid controller model placement alone is insufficient. Native bone query DB140 multiplies evaluated P by inverse(definition+48), while mdlGetBoneAbsolutePlacementD8D60 builds with identity root, finds named bone, calls DB140 and combines the native model record world matrix. Query state cleanup, same-frame evaluation, native stretch/left-hand mirror and loaded resource/content identity still require closure before using that query for pre-world scope capture. No new pose or skeleton engine is justified.

The rear cap remains exactly triangles901..922, circular planeZ+.0486. Scope geometry extends along local Z to approximately-.309146, but a geometric cylinder axis is not yet a proven native reticle/ballistic alignment. Full magnified capture/reticle/color-stage and authoritative native zoom lifecycle remain integration gates; none is represented as implemented by this evidence.

## Native zoom lifecycle / actual handedness closure

The entity+BC guard is the right-hand-weapon flag, not the distinct sniper parameter+BC magnification field. Reflection names the property ep_bRightHandWeapon; CBaseWeaponProperties constructor495C0 defaults properties+4C to1. Entity OnCreate copies that property to weapon+BC at4E557..4E55A. CreateWeaponEntity102680 writes its second integer argument to properties+4C at1026DE. SetCurrentWeapon104310 creates hand1/right+800 with1 at10434C..104363 and the other/left+804 with0 at1043A5..1043BC. BringUp receives the separate animation boolean in both branches (calls10438A/1043DF); it does not assign handedness.

Therefore a stock left sniper's alternate press can set D4 and native zoom damage, while skipping owner third-person/time capture and native OnStep interpolation. Native parameter constructor171780 is CSniperWeaponParams; its BC=4.0 and reciprocal store172F49→weaponEC do not identify weaponBC. The investigation's preliminary conflation was caught by main and corrected before this record.

Sniper alternate release49000 only clears2C; it does not deactivateD4, restore damage or reset owner FOV. Held interpolation uses F0+(EC-F0)*F4 and shared owner858. PutDown171470 deactivates before base put-down; right sniper deletion1719D0 can also deactivate the owner's left weapon through native type/handle guards. Two enabled right-like zoom transactions can contend over saved owner558 and shared858. Stock left interpolation is absent; do not describe two stock snipers as both racing that setter.

Complete BaseWeaponOnStep4EE20..4F0BD contains no direct BC reads. SniperOnStep's direct BC reads gate interpolation17284E, zoom-sound start172979..172981 and stop172A36..172A3E. A temporary guard adapter is a candidate for preserving native timing/damage, but forcing BC around the entire step could affect indirect base-step callbacks that inspect handedness. Native zoom shared-owner restoration, deletion coupling and a narrowly scoped post-base-step guard still require Astra design review before implementation. No raw D4/timestamp/progress/damage writes are justified.

Existing engine authorityStep stages equipment and accepted native firing samples; multiplayer beginTick freezes before any entity/weapon step and completeTick retires after the full interval. An additional per-hand desired-zoom field could use that existing ordered native carrier and dispatch original weapon alternative events in the same simulation phase, without a new RPC/protocol engine. No such wire field, controller action or authoritative zoom dispatcher is currently implemented.

## Bone-query cleanup and native color-stage evidence

The exact mdlGetBoneAbsolutePlacement body is D8D60..D8F97. Its missing-bone branch copies identity, calls DAD90 at D8E1A and returns0; success copies the combined native model-world/bone result, calls DAD90 at D8F87 and returns1 at D8F8E..D8F97. DAD90 resets the shared model/mesh/draw/bone/palette counts and evaluation pointer, so this query is destructive to an in-progress native model transaction. It cannot be called inside weapon draw or collection merely because its public signature resembles a getter. Exception-path cleanup has not been established.

Setup E2670..E28A6 creates an identity sentinel with E1640, builds the requested instance with E1720, and uses native animation-cache eligibility (including aniGetAnimQueueAge at E2771). A cache hit calls DBC90 at E27BA. A miss executes the native evaluation path E2060/D8100/DDE00/DB2D0 and calls DBC90 at E2879. No alternate skeleton evaluation is needed. DBC90 already has the remote-weapon adapter hook; a future pre-world query must deliberately suppress that presentation adapter for this public-query caller without suppressing original native evaluation.

DBC90..DC50C incorporates instance stretch into model-record world matrices: the root-child branch DBCDC..DBD6C multiplies the parent matrix columns by instance+0/+4/+8 and preserves translation. Attachment branches retain their native transform and multiply columns by instance stretch at DC44F..DC4DD. Consequently the public query returns an animated, stretched model-space bone placement when given its identity root. The actual left-hand temporary stretch/mirror order and the query-to-render same-frame/lifetime correspondence still require proof; applying a second guessed scale would be wrong.

renFinishRender14BFA0..14C0DB enables gfxSubPixelHDR at14BFDC before root Execute14BFF7. After that Execute returns, it destroys the native root and frees collection storage, performs lighting cleanup, disables/resolves HDR at14C06E, and conditionally applies the native final fade at14C082..14C0B1. Therefore the current post-root laser phase is before those final effects. Copying a fully finished Render3D image and inserting it at that phase would process the image again. A narrower candidate is retaining the actual root output before HDR resolve/fade and inserting at the matching post-root phase in the world eyes. Matching source target/format/dimensions and native root postprocessing remain prerequisites; this audit does not yet prove that candidate end to end.

### Zoom dispatch follow-up

Pinned native AltPress171430 calls base48FE0, toggles original virtual Activate/Deactivate, and returns1. Base48FE0 has only the native alternative-held store and returns0; Release49000 only clears held and returns. A hold-to-target adapter can therefore call original native Activate/Deactivate and native base held methods, avoiding toggle-to-target event replay while retaining native damage and time initialization. This is a proposed seam, not integrated zoom.

Native DoAttack101F00 button1 invokes right alternative press at101F85(return101F8B) only when the left weapon handle resolves null. StopAttackFA770 button1 releases left primary when available; otherwise releases right alternative. Existing VR left-trigger/plcmdAltFire routing can accidentally toggle right sniper during no-left transitions. Any managed zoom integration must isolate that stock dispatch. Native BaseWeaponStep4EE20 directly invokes owner firing queryv730 and weapon fire callbacks and can invoke deletionv1AC for absent owner; temporary handedness must exclude the entire base execution and be restored only while exact native lifetime remains valid. SniperStep base call return172830 is statically verified. SetBulletDamage160340 writes only shooter44, ret4; both zoom callbacks use the shooter virtual1BC with native paramsB4/B0, without direct RPC at these setter/callback bodies. This does not by itself establish all client-side shooter behavior.

## Native admission trace follow-up

Bounded Terra investigation (analysis, not review) and main spot-checks establish base weapon OnStep4EE20..4F0BD checks its native owner type and CanFireFromOwnWeapons8E4F0..8E577 before state-machine work. Sniper OnStep172820..172A61 then independently gates continuation on D4/held2C/BC, so base-policy denial is not an implicit native zoom exit. Activate171CF0..171DE1 has no state/policy check and resets native activation/damage whenever called. SetState4A070..4A3E6 writes native stateB0; PutDown4B270..4B333 requests literal2, BringUp4B380..4B4BA requests3; native fire queries admit1/7 and native OnStep has its own4 path. These facts do not justify activation merely whenever own-weapons policy passes: that would permit reactivation after PutDown/BringUp. Keep native callback/lifetime and state admission distinct; active native zoom must not restart on cooldown4. No integer semantic is guessed beyond these native control paths.

Sniper OnFire171C60..171CED calls original base OnFire, obtains original shooting placement and dispatches shooter FireBulletsFromPlacement1604D0..1604F1 -> FireBullets1621F0. Zoom chooses native shooter SetBulletDamage160340 via v1BC. Main confirms DoTheFiring4CDC0..4CE3C gates owner v414 (actual IsAlive7F470) and ammo but contains no role check. These bounded methods prove synchronous native simulation effects; they do not establish client/server ownership. Stock DoAttack/StopAttack caller and native authority/prediction evidence remain required before direct local-client zoom admission. No zoom is enabled by this investigation.

The bounded stock caller investigation confirms Player OnStep103970..103F45 has a server branch103A54..103A84 for native current-puppet bookkeeping which rejoins103A86. Both continue through Legged OnStep6D1A0 (base call6D24A), Puppet OnStepA69F0 operator validation/ExecuteOperatorFiring8E580, native fire-edge dispatch7FFC0/7FFE0 and DoAttack101F00/StopAttackFA770. Sniper press171430..171461 updates base held2C then native Activate/Deactivate, while release only clears held. No client/server rejection is established on this bounded callback chain. It proves same-instance native implementation, not upstream prediction-instance selection or downstream exclusive authoritative damage.

A smaller candidate avoids arbitrary local-client mutation at roster preparation: stage input at the existing interval boundary, and reconcile only inside actual original sniper OnStep/OnFire callbacks for their exact live native-owned weapon. This preserves the native caller-selected simulation object and all original fire/damage execution; lifecycle/input admission still requires review. It is a proposal, not an implemented callback adapter.


## Bounded finish-copy clarification

Owned Engine inspection confirms gfxSubPixelHDR disable8CC2B..8CC78 calls gfxCopyContent8B6E0 at8CC4F, then restores the original drawport at8CC5B. gfxCopyContent8B6E0..8B7F7 dispatches CCanvas::CopyContent60E80..60EB2. The already-recorded GfxD3D startup binding selects1280..13B1, which resolves color surfaces and calls device StretchRect at1373. No shader/bloom dispatch appears in these bounded copy bodies; the function name HDR alone proves no spatial postprocessing kernel. Native child render/effect commands may contain separate effects, and this audit does not classify them.

The final fade remains a separate native gfuFill after root destruction/free/lighting cleanup and copy, at14C082..14C0B1. A finished scope image inserted before that fade would receive fade twice unless captured/composed at a matching stage. Native color-format transfer/filtering/rounding still requires exact applicability; equal final surface identity proves neither color correctness nor pixel ownership.

GfxD3D CurrentRenderTarget2970..2BF4 has a special native depth-formatED path: it may borrow the common depth surface when dimensions fit, or allocate a different depth when they do not. Bindings at2BA9/2BBB select RT/depth independently. The finish copy resolves color and has no depth-copy call. Consequently final eye depth identity cannot prove it contains every root's world/gun depth under all native scale/canvas paths.

The current bridge allocates eye depth with Discard=TRUE. Microsoft documents depth contents invalid after rebinding a different depth surface; simply retaining COM identity does not retain usable depth. Depth copy additionally requires non-discardable full equal-sized/equal-format surfaces outside BeginScene/EndScene. Sources: [CreateDepthStencilSurface](https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-createdepthstencilsurface), [StretchRect](https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-stretchrect).

This supports the existing Astra visibility/color gate rather than completing it. Foreground blending without depth writes cannot be reconstructed from retained depth after final color replacement. No optics rendering, depth-copy workaround or second postprocessing pipeline is added.

## Native shader-module reference resolved (source follow-up after0.2.11)

Read-only owned-file investigation, followed by main static spot-checks, resolves
the BMF external shader reference. `Bin/Shaders.module` is a virtual module
resource path, not a required on-disk `.module` file. Core's LoadModule105D0,
RegisterModuleResource42E00 and resObtainResourceFromModule_t43DA0 provide the
native module-resource route. No file needs obtaining or installing.

Pinned Shaders.dll SHA256
`de7652d61f5af2ec8b218b902d09820b3ef76c1d796f23d66a31ea35374fd642` declares
module Shaders at6330..637A. Its6CED..6CF9 registration associates resource41A
with Poly Bump. The pinned Sniper.bmf EXOB table119C supplies41A at11AC and
references the virtual Shaders module. The material graph uses CPixelightShaderArgs
and the Scope color/normal textures. The corresponding native CPolyBumpShader
GetState7B40 and Exec7CA0..8D14 provide the next bounded implementation evidence;
Exec includes native lighting preparation. This Shader DLL is an audit fingerprint,
not a new production support gate or an enabled optic hook.

A preliminary missing-file conclusion was rejected and corrected by tracing the
native module registry. A module identifier is not a missing asset. The bounded
material investigation below closes the serialized arguments/color recipe;
actual render-purpose selection and the root-command suffix remain required.
No scope image or shader replacement is enabled by this evidence.

## Stock Scope material and actual GPU input route

Owned-file inspection and main spot-checks establish Scope surface name2 at1320
and preset5 at1414. Its one STAR config39C8A/count39C8E selects shader7/args8,
platform3, null attributes and flags255 at39C92..39CA2. Args8 is serialized
CPixelightShaderArgs version10; native Shaders metadata aliases that type to
CPolyBumpShaderArgs. DCON39CE2/count39CE6 is empty. The pinned Python audit now
checks that graph and all25 scalar fields after the complete stock BMF hash.
This is serialized evidence, not proof that live overrides remain absent.

The native Exec is LONG thiscall(self,CShaderArgs*) with ret4, not a void
multi-argument callback. GetState7B40/common helper68C0 and Exec7CA0/common
helper6FC0 select stock blend500 (disabled), depth42 (LESS_EQUAL) with writes,
single-sided rasterization, alpha-test disabled and full-bright disabled.
Exec8CDD invokes helper70B0, whose7106 calls shaRender once;8CE2 runs cleanup.
Partial native fade takes blend501 (SRC_ALPHA/INV_SRC_ALPHA), modulates alpha
and disables depth writes in6E10. Stencil remains inherited. These facts do not
classify separate native depth/shadow passes.

Native lighting calls at7E13/7E1E/7E30 and light-modulated RGB82A4..82E1 mean
substituting a scene texture in the stock lit program would relight that image.
A future bounded unlit cap-color substitution must preserve the original
geometry/VS, raster/depth/stencil and partial-fade semantics at that actual pass.
The original gun command uses sort key8FF00 and Execute4BDE0, which dispatches
weapon Render throughv1F4. CBaseWeaponRender4C740 temporarily changes native gun
view/projection/depth, renders the actual model, then restores them. Retaining
final eye depth or appending finished lens color cannot substitute for this pass.

The hardware-input path DCD10 binds original position/weights/localindices via
shaSetChannel76FB0/shaBindChannel772C0. Engine2C7F88 != -1 substitutes positions
and must decline the proposed static-input geometry gate. GfxD3D A844..A8AD
binds actual managed COM VBs with unchanged offsets;91C0 creates VB usage0,
FVF0/pool1 and IB usage0/INDEX16/pool1. Upload90F0..91B5 copies bytes unchanged.
The dynamic/write-only ring paths208/218 are a different route and decline.

Actual Poly Bump declaration uses stream0 FLOAT3 at byte0/stride12; stream5
UBYTE4N localindices at155056/stride4; stream6 UBYTE4N weights at151520/stride4
only when the selected weight count exceeds1. Relevant elements have offset0,
method0, TEXCOORD usage5 and UsageIndex equal to the stream. One palette bone
does not establish that shader weight count. Weights remain in the shared VB
even when stream6 is absent. Static Scope draws TRIANGLELIST/base0/min0/884
vertices/start0/928 primitives; cap901..922 contains66 indices. Managed VB size
is212128; INDEX16 IB size19038. Four exact source slice hashes still gate content.

GetStreamSource and GetIndices return owned references; those references pin
object lifetime, not arbitrary writer immutability. READONLY is incompatible
with WRITEONLY, and successful locks require matching unlock before a draw.
Sources: [GetStreamSource](https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-getstreamsource),
[GetIndices](https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-getindices),
[buffer access](https://learn.microsoft.com/en-us/windows/win32/direct3d9/accessing-the-contents-of-a-vertex-buffer),
[locking resources](https://learn.microsoft.com/en-us/windows/win32/direct3d9/locking-resources).
No live geometry or image admission follows from metadata alone. The actual DIP
ownership adapter is a separate Astra decision; no native CPU reader is revived.

## Scope image interface and pre-gun capture refinement

The subsequent actual DIP reader is implemented and accepted separately in
SCOPE_GPU_GEOMETRY_REVIEW.md. It does not establish color-pass/image admission.
Current follow-up verifies the installed GPU-Programs.asm PolyBump VS is1.1:
position uses effective MVP c1..c4, diffuse UV emits oT3.xy from dp4(v3,c8/c9),
and normal UV emits oT0.xy from v4/c10. Tangent/sign uses v2; offset90912 is its
FLOAT4 data, not diffuse UV. Native oD0.w is effective faded color alpha multiplied
by translucent fog. A replacement must multiply original diffuse texture alpha
by that VS output, not incoming args+48 alone.

Pinned Scope UV0 is format84/buffer0/offset181824/stride8, named3; its884-coordinate
slice SHA256 is199e21cfca88d708d605db3ee54f35bba76191bf698c725baf4a39034480b2fd.
The actual24-vertex cap UV map is invertible and planar; the Python owned-file
audit now checks that relation and outward+Z winding. Live UV stream/declaration,
selected mapping and post-c8/c9 transformation still require admission. No UV
copy was added to the live four-slice reader yet.

Helper70B0's stock single-sided path calls7106 once, returning7108. Double-sided
args+38 with disabled writes calls70DC and70EB, then returns70EE; it does not
fall through for a third draw. Double-sided with writes uses the one-call branch.
This corrects preliminary investigation claims; none of these flags alone prove
default-color purpose.

Engine77F00 force replacement uses2E49F0/2E49F4 before modifiers and virtual
Exec. Preset config at+34, strideC supplies shader+0/args+4;75E20 may replace live
args before dispatch. Explicit depth14C760 forces2EEF98/2DA124 at14C80F and clears
at14CC74. This excludes that depth route, not every possible shadow path.

Main's further bounded gun inspection proves both native mdlRenderModel calls
4C9E3/4CA48 push zero for the fourth H argument. EngineE2B10's ordinary model
path callsE1570 atE2F0E, then tests that fourth argument atE2F13; zero skips optional
E54C0. E1570 iterates evaluated model records viaE12B0. A narrow actual-model
callsite purpose adapter may therefore close more than a global shadow flag
search; complete applicability/material lineage remains unaccepted.

Astra conditionally selected source color capture at the FIRST actual local gun
boundary, latched before admission, before gun drawing and later finish/fade.
Failure there cannot retry at the other hand. Universal override coverage and
the intended world-content cut are still required. Source views must use explicit
purpose under the existing frozen frame owner, not fake eye indices or a second
animation/calibration bank. SCOPE_IMAGE_DESIGN_REVIEW.md records the disposition.

The experimental mod-owned lens shader uses TEXCOORD3/COLOR0 and PS2-extended,
with original VS/geometry intact. Linux vkd3d1.17 emitted151 single-slot executable
instructions/6temporaries; baseline PS2's64 arithmetic limit is insufficient.
Before any native use, require PS2a capabilities and the reported register/slot
budget. No shader is installed or enabled. References: [baseline PS2 limits](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx9-graphics-reference-asm-ps-2-0),
[extended capability flags/budgets](https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dps20caps).
PS3/VS1.1 pairing is still unknown and is not assumed by this route.
