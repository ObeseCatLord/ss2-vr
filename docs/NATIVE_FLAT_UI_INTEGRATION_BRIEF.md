# Complete flat native UI: concrete integration amendment

Baseline9a76324. Main owns architecture/integration; one local game mod, no
runtime testing or installation. Preserve accepted geometry and native renderer,
callbacks/assets/shaders/order/protocol ownership. Review only the refinements
below, not accepted geometry or prior turret/world-marker work.

## Verified environment and reusable boundaries

| Boundary | Evidence |
|---|---|
| Once owner | [verified: owned native binary] Brain RenderView export EB7B0, thiscall no arguments; resolves brain+28 before virtual610 callEB833 and overlay608 callEB857 (returnEB85D). Alternate branch skips ordinaryworld and passes overlayflag1. Native player overlay export105480, thiscall oneint/ret4. |
| World | [verified: source] engine.cpp render requires markerParentReturnFEA8A; bridge.cpp stereo477+ currently owns Rendering/readback/desktoprestore/Ready. Reuse that transaction, no new queue. |
| Actual eye P | [verified: owned source/binary] root native depth callback155DD1 after original graphics setup captures executedWeaponWorld. Add copy of actual adjustedP exported2E6438 at this existing capture, before marker ortho/retirement. Actual sourceUI constantsc1..4 queried perdraw; builtinUseSimpleShader combines adjustedP/currentview. |
| Programs | [verified: binary] both VS/PS recorddeviceobject offset0;12byte recordsboundedcount/data; builtin6pairs2E6F2C/30. Currenthandle fields exported. [verified: corrected native assembly route] compiled builtin position semantics established below; GetFunction/token parser deleted. |
| Resources/order | [verified: source] owned matching non-MSAA eye RT/defaultpool and offscreenreadback/systemmem alreadyexist. Current head dimmingCPUafterreadback means GPUeye colorsremainundimmed. |
| Comfort | [verified: source] host hudAnchor is existing35deg enter/8deg stop90deg/s owner with lifecycle reset. Requests currentlycontain392bytes, IPC7; no panel/capabilityfields. |
| Device | [verified: API/source] GetCreationParameters exposes PURE/MULTITHREADED flags; pure blocksstategetters. CurrentproxyCreateDevice callback can record exact creationthread. TLS alone does not serialize valid multithreaddevice clients. [unknown] complete stockdeviceflags domain; runtime admission must query. |

## Worked minimal amendment and open decisions

1. Pre-UI world dimming: retain exact existing CPUquantization, but only when
visibility<1 read into existing readback, lockwrite/dim/unlock, then UpdateSurface
back into ownedeyeRT BEFOREoriginalflatUI. Finalreadback occurs afterUI. No dimmer
shader/state manager/newtarget. [verified: Microsoft UpdateSurface] source
SYSTEMMEM offscreenplain→DEFAULT RT supported with matchingformats/dimensions,
nonMSAA/unlocked/nondepth. Extra transfer cost unverified; rejectpaironfailure.
Rejected replacementshader and finalCPUdim because larger or wrongnativeblendorder.

2. Whole draw transaction: admit only non-PURE, non-MULTITHREADED device on
recorded creationthread AND nativeMainThread. This uses native singlethread API
ownership contract; never infer atomic transactions merely from MULTITHREADED.
Rejectunsupporteddevice forcompleteUI; do not replace every D3D method with a
parallel locking layer. Reviewer should challenge adequacy/cost. Nativeactual
creationflags are queried rather than forced or changed.

3. Onceowner integration: originalBrainRenderView executesonce under stackowned
scope with currentresolvedlocalplayer/nativeepoch/thread. Worldcapture stores
existingRendering slot identity plus twoexecutedadjustedP and retaineddesktop
surface references, and defersreadback/Ready. ExactoverlaycallerEB85D withflag0,
samecurrentresolvedplayer/owner enters immediateUI scope. Originaloverlay executes
once; then finalreadback+existingcommitStereo locks validate allcurrentnative
snapshot/rider/epoch/request guards. Missingoverlay/alternate/nesting/failure
retiresRendering; emptycompletedUIvalid. No cachedborrowedcommanddata or newqueue.
Enclosingexit must retire abandonedslot even on nativeexception.

4. GPU boundary: desktop originalDrawPrimitive/DrawIndexedPrimitiveonce first;
retain originalHRESULT. Postoriginal captureD3DSBT_ALL plus explicitallRT/depth/
vp/scissor. Nativeactualbuiltinboundobjects/currenthandles/actualcompiled
positionsemantics gate; queryactualconstants/viewport/scissorenable/fog/stencil/
userplanes. Sourcefog/stencil/userplanes reject. Duplicateimmediatelybothownedeye
colors with acceptedhelper, originalprograms/streams/textures/colors/blends,
disabledepth/writeandnativepixel-scissor, install6clipplanes. Restoretargets/depth
FIRSTthenstateblock, viewportANDscissorLAST. Fail anysetter/draw/restore =>pair
rejected. Multithreadunsupported; no oldblockacrossnativecallbacks. Offscreen
nativeoperationsremainoriginal; no claim thatDrawUP/custompathscovered.

5. Fades: hook originalgfuFill export6CFD0 scoped only at two originalcallreturns
FE90A/FEA07 under admittedoverlay. Require actualfullcanvasphysicalviewport,
scissoroffandoriginalbuiltin/absentfogstencilplanes. Originalfillquadconstants
remainunchanged forfull-eye draws becauseeyedimensionsmatchcanvas. Preserveactual
blend/order; nocoloraccumulator/replay/rectangleheuristic. Paneldrawselsewhere.

6. Host/IPC: freeze existinghudAnchor duringrequestprepareonce, fullcanvasaspect
and pose inrequest; relative panelInEye = inverse(requesteye)*frozenLOCALpanel.
Shared rig/turn/reference corrections cancel; actualnativeeye/worldadmission
remains existing gate. Use existingmenuWidth/menuDistance forcompletecanvas
panel instead of statsstripwidth/4, centerverticaloffset0. Generatedstats still
usesexistinghudWidth/distance and sameanchor. ResponseflagNativeUiComplete belongs
toactualcompletedpair, copiedtocachedpairafterbothuploads/releases. Suppressstats
ONLYwhenactuallypresentedworldpairflagcomplete; not request/drawcount/menuMono.
ABI8 needed; nativewire6unchanged. No extra imagechannel/XRlayer/comfortscheduler.

Estimate reopened: acceptedgeometry150lines; remainingtightdevice/owner/IPC
adapter roughly500–700lines pluschecks. Largest statecapturenecessary because
nativeRGBblendingmustremainoriginal. Seekdeletion before renderer-likegrowth.
Verticalproofoffline: actualhelper→nativewrappercompiledcallshape/restoreorder,
portableowner/completion/cachedcapability boundarytraces; no mocks are claimed as
nativeappearance proof. Fullstockcoverage/actualnativeGPUbehavior/throughputunknown.

## Reviewer contract

Verify actualsource/docs before critique. Readonly, no edits/commits/delegation,
no game/host/Wine/XR/server/network execution. Prioritize whether amendment is
necessary, smaller reuse opportunities, ownership/state/dimming risks. Return
max1800words with conditionalGO/NO-GO, prioritized findings, exactfile/line or
native-evidence refs, concrete disposition suggestions, no proprietary dumps.
Astra selectedexplicitlyxhigh; main verifies effective settings. These are
technical choices main can make within currentauthorization; no userpermission
question. Fullgoal remains active, teleportexcluded.

[UpdateSurface](https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-updatesurface),
[D3DCREATE](https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dcreate).


## Main simplification after bounded compiler investigation

Main caught and corrected the initial investigation's unconditional HLSL/profile
claim. Gfx7220 markerchecks72A7/72C1/72DB compare vs/ps/xps and branch76FA to
assemblyhelper6CC0;6DAA calls thunkAFA2 D3DXAssembleShader. CompileShader7698 is
only the nonassembly fallthrough. Engine parser removesCT, may upgrade1.1→2.0,
but retains bounded builtin m4x4oPos,v0,c1 through genericline append. Thus the
native assembler contract plus exactlivebuiltinobject/fingerprint is main's
selected minimal semantic admission; proposed GetFunction/tokenparser is deleted.
Both versions retain the same position behavior. Actual declaration/streamv0 and
c1..4 remain queried. This correction reopens only that former unknown gate.
No native device/program/runtime executed. Worker closed; main owns conclusion.
