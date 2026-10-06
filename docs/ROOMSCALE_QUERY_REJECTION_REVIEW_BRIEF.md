# Astra Max: reject only an unavailable mod-owned query

## Goal / scope

Solo-user SS2 OpenXR mod; full roomscale body collision/multiplayer required,
teleport excluded, native runtime testing excluded. Main owns final architecture.
Reopen query ownership after callback-closure/preflight work grew beyond the
minimal adapter. Decide whether a narrow optional-query rejection at actual
resource replacement boundaries is justified and safe. Read-only review;
not approval for body movement or full sphere/capsule collision correctness.

## Verified evidence versus candidate

| Fact | Classification / artifact |
| --- | --- |
| Original world/physics/solver remain; no body movement exists | [verified: AGENTS.md, IMPLEMENTATION_STATUS.md] |
| Existing public ray is shared; scalar restoration is not a lifecycle transaction | [verified: ROOMSCALE_QUERY_OWNERSHIP_AUDIT.md] |
| Native fresh init/configure/check/consume then deferred cleanup is established | [verified: ROOMSCALE_FRESH_QUERY_GATE.md] |
| Solver workers join10D935 before contactcallbacks10D954 | [verified narrow worker exclusion: same gate; not universal queryownership] |
| Resource callbacks during world/model traversal can pump a global queue | [verified: ROOMSCALE_POSTLOAD_TASK_GATE.md] |
| Five task classes are concrete but reflected/backend closure expands | [verified: same gate; no actual nestedray demonstrated] |
| Ready getter can return1 evenwhenpump selected; refcount/queueempty don'tpinflag | [verified: ROOMSCALE_RESOURCE_ADMISSION_GATE.md] |
| Actual World2911E/modelDA421 replacementdispatch follows receiver+4 bit0 tests | [verified: same gate] |
| Model triangle narrow math adapter is sourceapproved butinactive | [verified: ROOMSCALE_MODEL_QUERY_SOURCE_REVIEW.md] |
| Complete callback coverage, normal failure labels and query purpose binding | [unknown for proposedrejection; reviewmustverify] |

Workspace ss2-vr beneath installedgame. Native Engine/Core/Sam pins in RESEARCH
andinstalled-build.json. Owned static inspection allowed; no edits, delegation,
Windows/game/Wine/XR/network launch. Main concurrently owns docs/integration;
another Astra owns only Sam saw state1-animation reentry. No overlap with its
specific animation/get-idle family. Verify CURRENT-TURN gpt-6-astra/max; aggregate
only, never raw operational telemetry/disassembly exports.

## Main lean: actual optional-query rejection

[unverified candidate] Bind one explicitly mod-owned fresh collision query to
the existing native phase/thread/world/simulation guards. Keep original native
ray init/configure/traversal/result/normalcleanup. At each actual world/model
replacement-dispatch branch, an admitted **mod-owned** query encountering the
branch becomes unavailable and takes an already-existing normal no-result/
failure path. It does NOT invoke that replacement on behalf of this extra
query. Normal stock queries still take the original callback unchanged.

The whole mod result becomes unusable evenifearlier targets producedhits; no
body translation or sampledresult publication follows. Do not report "no wall"
or return a fake resource pointer. Let original world visited-hull teardown and
registered collision/model cleanup run normally. Do not jump past a native
destructor or try rayInit as interrupted-traversal recovery.

This avoids a preflight enumeration of every dynamic resource and closes the
actual conditional pump entry despite flag changes between earlier observation
and laterbranch. It doesnotpause/drain/suppress theglobal loaderqueue or replace
worldgeometry/physics. It postpones onlythisoptionalmodquery; a laterordinary
nativequery/loaderowner canperformnormalreplacement. Exact normal-failure paths
and receiver/dispatch completeness mustbeproven beforeany implementation.

[unknown] Whether aborting at2911E canusenormalworldtraversalreturn without
leakingvisitedhulls; whetherDA421 has a validlocalfailureexit preserving model
cleanup/profileobjects andoldhitstate; whetherotherray/modelpreparationpaths can
invokequeuedcallbacks beforethesebranches. A per-model miss alone isn'ta valid
whole-query success: lexical unavailable status must dominatepublication.

[unknown] Original publicquery reentry fromothertargetcallbackfamilies, model
scratch/workerownership beyondthephysicsdispatch, and native data-stability
contracts ifloader flags change aftertheoriginalbit0test. Do not assume the
proposedboundary closes those unrelated invariants.

## Compare minimal options before changing architecture

1. Actual dispatch-site optional-query rejection (lean): two narrow boundary
   candidates, no replacement/syntheticcollisionworld. Newmod-state is only
   lexicalpurpose/unavailable; originalquery/traversalpolicy remains reusable.
2. Continue full postload/destructor/backend callback closure: originalbehavior
   preserved, but global/reflectedtargetset grows substantially. No nestedray
   actuallydemonstrated; exhaustiveclosure isn't automatically a requiredlayer.
3. Ready/stable preflight: smallerif a genuine native ownerexists, but examined
   getter/refcount/count predicates explicitlyfail toprove requiredinvariant.
4. Existing owned nativequery extent: potentialreuse, but no fullplayer-volume
   finite-move acceptance/settlement isestablished; don'ttreatanoldzeroradius
   queryasbody-sweep proof.
5. Newcontroller/worldtraversal/globalserialization: reject bydefault; unknown
   compatibility doesn'tjustify replacing adjacentnative systems.

## Output / depth budget

≤1600words final. Verifybeforecritique; prioritizeforks anddeep-spec onlytheone
worthproving. GO/CONDITIONALGO/NO-GO foroptionalqueryrejection; exactRVA/ABI/
normalfailurepath/callbackcoverage evidence; necessity/deletion opportunities;
smallest source vertical and portable/compiledchecks; ONE missingproof ifneeded.
No physicsmutation/sourcepatch, numericalTOI/primitiveoverlap/bodygeometry/
networksettlement re-audit. Those remain mandatory latergates, not implied by
query ownership. Challenge whether "rejection" is actually suppressing stock
callbacks or changingnativepolicy ratherthan merely declining theextraquery.
