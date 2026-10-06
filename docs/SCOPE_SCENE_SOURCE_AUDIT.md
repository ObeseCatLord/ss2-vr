# Scope scene-source boundary — Astra/xhigh static audit

General first-gun scene admission is **NO-GO**. Read-only owned-binary audit;
no game, Windows, XR or network execution. This records a concrete content
counterexample rather than treating an unknown renderer as grounds to replace it.

Engine CViewRenCmd::Execute155FF0 sorts at156033 via155940 using command+8's
low24 bits, then float+C for ties. Execute dispatch is15606E. Verified primary
keys include models30000/40000, fading models/effects/trails80000, ordinary
physical weapons8FF00, flares90000, visibility testsA0000, bloomB0000,
configurable post-render callbackC0000 and 2D textD0000. This is not an exhaustive
custom-renderable/particle census. Equal gun keys do not establish hand order.

Engine flare159DF0–159E0C promotes its command to the outermost root. A visible
source-camera flare therefore follows the first gun and is absent from an image
captured there. Later world-eye flares do not supply equivalent magnified source
content. BloomRenderEffect in ProcRender36D0 includes color copying; capture
after bloom and insertion before world-eye bloom would apply processing again.
The C0000 command calls a supplied callback at14AE1B; its sort key does not
identify a fixed supported effect.

The common ordinary weapon command is Sam vtable2A4720, Execute4BDE0, weapon
pointer+10, thiscall/plain return. It dispatches weapon virtual1F4 at4BF34,
return4BF3A. A source capture would require a first-attempt latch at this actual
common boundary, before validation, rather than assume Base/Sniper Render
wrappers observe the first offhand command. Original command executes once.
The enclosing native puppet Render3D94380 owns prepare9476C, finish9477C,
cleanup94784; retain native auxiliary collection, animation and cleanup.

Actual target identity remains necessary. HDR begins atEngine14BFDC and may
select formatE5 via8CAF5, actual GfxD3D table110EC maps to113 (A16B16G16R16F).
Intermediate dimensions may change at8CA7D/8CA99. After root execution14BFFA,
HDR disable/copy14C06E reaches copy8CC4F/StretchRect1373; no depth copy is
established. An arbitrary retained current RT proves lifetime, not typed root
COLOR ownership. Current strict direct-eye target matching can decline HDR.

For a restricted direct-target transfer, source/capture/destination must have
matching intended dimensions and actual A8R8G8B8/X8R8G8B8 format, no MRT/MSAA,
and a successful distinct owned default-pool RT-texture StretchRect with equal
rectangles/D3DTEXF_NONE and required capability. Stored RGB is pre-suffix; do not
claim linear color from an instantaneous state getter. The smallest subset
requires cap sRGB write and sampler sRGB decode disabled. Actual retained target
identities are compared with getters; endpoint observations do not prove
uninterrupted device state.

A bounded typed suffix examination admitting only supported ordinary gun
commands could justify a restricted first-gun slice. It would reject flares,
bloom, callback, text, nested and unknown types and still require actual list
stability/late-insertion proof. Its practical coverage is unobserved. Full scope
content needs a native gun-free source composition boundary preserving suffix
processing, not a later copy alone, command replay or a replacement renderer.
Current `contentVerified=false` remains appropriate.

Pinned binaries: Engine da6efc9f72637eb3b6f48eadca2107be89b09c00618b6e72d5d3632938a7d851;
Sam2Game 5628b4ed30a966f10c8e8ea46bf0789257a35ea80127bacacebe28b1ce5303df;
GfxD3D 88749b79be36f0c0dccb623c4f1712685b5af603b451e25ed3c5029bedcdf3ed;
ProcRender e93a51da739e6a4f0b66457af7de750befbd9514eb8180afe15513ca8d3427f2.

## Native gun-free collection follow-up

Separate Astra/xhigh read-only investigation gives GO for a narrow collection
adapter, not image enablement. CPuppet Render3D94380 has no flags parameter;
CViewRenCmd::Prepare156370's fourth argument is a history identifier, not a
weapon mask. Player InjectRenderingCommandsFDA00 calls RendersIn3rdPerson at
FDA05, returningFDA0B; nonzero takes its original early-return branchFDA7C.

The complete injector performs only third-person/show-weapon/eligibility checks,
resolves two weapon handles and calls collection virtual1F0. All18 audited
exported weapon vtables select4BF40 there, including Sniper. That collector only
allocates/constructs/links a native0x14-byte command, stores the weapon pointer
and8FF00 key. It advances no animation or gameplay state. Later virtual1F4
weapon rendering is separate and is not authorized for whole-method skipping.

Prefer the already-installed contextual renderThirdPerson adapter: return
nonzero only atFDA0B for an explicitly admitted local source-purpose invocation
under the verified frozen frame/render owner. Preserve root943B6's current
policy. No fake eye, global third-person state, player558 write, additional
injection hook or shared hud_bShowWeapon write is needed. The latter is a real
32-bit BOOL at3F9E0C (metadata40F564/40F56C/40F574), but is a broader shared-state
alternative and is not selected.

Original prepare9476C, injector94776, finish9477C and avatar cleanup94784 stay
native. Finish executes root14BFF7, destroys it14C050, frees storage14C057,
cleans lighting and performs HDR/fade. Later flares/bloom/callbacks therefore
remain in the completed gun-free view. This solves gun exclusion, not the
color-stage mismatch: a completed source has postprocessing that could be
applied again when inserted into a pre-postprocessing world cap. Source camera,
purpose/history admission, actual RT/depth/format, transfer and callback effects
still need an explicit reviewed transaction. No source-image pass is enabled.

## Pre-postprocessing capture decision

Astra/xhigh gives conditional GO for a bounded native capture-command vertical,
not image enablement. Late redraw remains NO-GO. A bloom hook plus root-return
fallback is inconsistent: late textD0000 (ProcRender11400→1153D/1155B, borderC46E)
can write pixels after the bloom cut; the no-bloom fallback would include them.
Shared pre/post callback setters assign10000/C0000 and do not expose this cut.

Reuse one native callback command in the existing source-only collection. Native
prepare14C558–14C59A/14C5B1–14C5F0 demonstrate0x18-byte allocation via exported
CRenCmd::operator new1556F0 and constructor155740; the latter links the current
collection2EF17C. Native callback vtable218F50 has deleting destructor14BF60 and
Execute14ADE0; callback/context are+10/+14, cdecl callback(void*). Execute retains
its native fog handling. Root destruction156151/finish14C057 owns command/pile
cleanup. No replacement callback or executing-child-array mutation is selected.

Queue at admitted source injectionFDA0B after nativeprepare reactivates root
collection14C543. Choose adapter rank0xAFFFF (not a native enum), finite tie0,
strictly after visibilityA0000 and before bloomB0000. Capture once there with
bloom present or absent, then let the full native suffix/cleanup run discarded.
Flares90000 and ordinary transparent content80000 precede this cut. Native
CEffectRenCmd::Execute153B40 dispatches renderable+44 at153B61; actual bloom
RenderEffectProcRender36D0/ret4 calls efxBloom1C10, with first copy1FCB and later
highpass/blur/add passes. None are replaced by this adapter.

Capture must keep the actual pre-bloom COLOR representation. HDR may be float113
and scaled; an eight-bit conversion is not equivalent. Native HDR disable is a
surface transfer, not an established shader tone-map stage. Current strict
direct-target identity remains the first subset; unknown intermediate ownership
is declined without changing global settings. Exact root membership, constructor
failure containment, callback lifetime through cleanup, source camera/FOV/history,
target/transfer, and original opaque cap insertion still need source proof.
The selected rank is not a census of arbitrary custom effects. No command or
image is implemented by this read-only finding.
