# Stock sniper alignment: reference and passive association

The fixed virtual handle reference for stock ID13 is established from the inspected
archive geometry. Its live placement correction is not enabled yet. The missing
edge is association with an actual rendered ID13 instance; numeric configuration
identity or an archive filename alone cannot certify that edge.

## Authored reference

Both retained stock sniper mesh variants yield the same virtual point in native
pre-stretch model coordinates: `(0,0.003122344828,0.006718217565)`. A position-welded
pistol-grip component has16triangles/12vertices. The means of its six shoulder-rim
and six heel vertices define the handle axis. Its perpendicular midpoint section
is a closed12-segment ring; the planar area centroid is the declared reference.
Independent owner triangulation reproduces the centroid and area for both variants
with maximum coordinate difference1.13e-17, and rechecks selected mesh vertices.
The geometry and decoded assets remain private.

The declared authored `Idle` reference uses zero translation/identity quaternion
at frame20. The inspected single `Sniper` bone has identity bind/inverse bind;
both gun surfaces are fully weighted to it. The reference mesh-to-model transform
is therefore identity. Do not invert subsequent animated Local each frame.
The selected FP assemblies contain the gun and muzzle/effect children, with no
native hand mesh. This virtual gun-handle convention does not certify physical
skin contact, an animated live pose or another weapon's reference.

## Native route and incremental design

Unzoomed native sniper rendering delegates to the base weapon render, retaining
the same placement factors, placement return before selector-driven reflection,
model handle and signed stretch floor. The existing tracked wrapper already
retains a native FP draw while zoomed; association collection admits only the
unzoomed state. No new renderer, IK, resource loader, inventory operation or pose
protocol is needed.

After actual association, reuse the existing position correction `-R(S*c)`,
borrow-phase and fresh model/configuration/stretch checks, correction before the
muzzle bound, and native charge exactly once. Carry native ID/reference identity
in the consumed binding. ID1 remains unchanged. The observed optical frame derives
from ModelWorld times Palette; correcting the root should propagate to the scope
source. A second scope offset would double-apply it.

At unit stretch in the declared reference, the authored muzzle is1.634908386native
units from the handle point, beyond the existing0.5muzzle-offset bound. Neither
this reference nor the unchanged bound proves rendered-muzzle coincidence. Actual
stretch/placement and native aim need their own evidence; do not silently widen
that policy or claim this limit is accepted by the user.

## Separately labelled passive collector

The existing private collector now has exact environment selectors
`SS2VR_LAB_IDLE_WEAPON=1` for ID1 and `=13` for ID13. Missing, malformed, padded,
multi-ID or unsupported selectors disable it. The selected original ordinary
invocation must be in native idle state1; ID13 additionally must be unzoomed at
admission and subsequent draw borrows. It never gives/equips/switches weapons,
starts a scene, presses controls or activates an alignment correction.

The same bounded animation-query, palette, program, buffer, ordinal draw and
post-original recheck paths apply. Each new schema4 draw header carries explicit
`nativeId`; the offline assessor retains that label and rejects unsupported IDs.
Historical records without the field retain the previously fixed ID1 meaning.
No proprietary bytes or decoded meshes enter public source.

Required association: the actual unzoomed draw's weapon/model/owner/hand/selector,
instance/configuration/file/resource identities and effective stretch, its own
Sniper palette/root and copied gun positions/indices/weights, matching either
retained mesh variant and rechecked after native calls. No passing alternative
from another draw or stale resource is admissible. The existing neutral Zap lab
cannot supply that ID13 observation; no automated equip is added or run. A future
independently user-originated draw is needed before enabling this correction.
This is source collection support, not a completed sniper alignment or a prepared
user launch instruction.

## Review disposition

| Astra recommendation | Disposition |
| --- | --- |
| Fixed handle-section reference, declared authored idle | Adopted and independently reproduced for both variants. |
| Keep native animation/stretch/charge and existing adapter | Adopted design; live correction deferred. |
| Associate actual ID13 model/draw before activation | Adopted; separately labelled passive selection added. |
| Correct scope root once, no second optics offset | Adopted design; no extra offset installed. |
| No authored hand / existing muzzle reach limit | Retained as explicit limits, not user acceptance. |
| New IK/loader/renderer/protocol | Rejected; existing boundaries suffice. |

Astra explicitxhigh reviewed the selected assets/native/source; local turn tags
were verified, independent backend attestation unavailable. Ten object-payload
roundtrips succeeded; this is not a whole-file decoder or runtime-precedence
certificate. Source checks and actual collection remain separate.

## Concrete remaining collector operation

The passive selector is implemented, but the complete gun-content association
path is not runnable yet: existing copies/replay/candidate bounds are1490vertices
and1332triangles, while the inspected sniper Black surface has2899/2904vertices
and2245triangles. It is rejected by the unchanged bounds. A smaller scope surface
cannot stand in for the handle-bearing gun surface. No user action closes that
source limit.

Next source work: adapt fixed copy capacity, per-selected-weapon range admission
and the corresponding assessor/replay/candidate byte budgets to the inspected
ID13 domain, with overflow/truncation controls, while retaining ID1's original
bounds and production scope admission. Only then prepare and review a concrete
collection path. Do not ask the user to run the incomplete collector or claim
asset association, alignment activation or rendered-muzzle coincidence.

The new original-End source gate checks its complete shared-selector guard owns
the block immediately; a stray semicolon is rejected. Negative controls retain
exact env/selector and native entry invariants. Mixed native-ID logs are rejected;
historical missing-ID entries may coexist only with explicit ID1.

The passive selector and corrected verifier/consumer received Astra/xhigh SOURCE
GO. Local current-turn tags verified; independent backend unattested. All4products
built on fingerprint `a0a70ce20f36ca996aa480d42cb334ed6e7f59a7c7d116e6207a54ee5722698d`,
IPC10/wire7.75Debug/75Release checks pass;32unchanged native/compiled gates plus
4adapted final idle gates pass normally/optimized. Selector tests retain assertions
in Release. Subsequent consumer24groups and verifier7groups pass normal/-O.
This is the selector landing, not approval or verification of forthcoming capacity
changes or an actual ID13 capture.

## Capacity implementation superseding the preceding blocker

The capacity blocker above is now closed in source. ID1 admission stays1490/1332;
ID13 alone admits2904/2245, unknown IDs reject. Fixed idle storage covers the larger
domain without changing production scope caps. The selected ID is copied before
AddRef and passed through both input reads and the intervening range check.
Completed and retained assessor limits are per-record; match/replay/missing rows
retain the ID. Generic offline candidate storage and evaluator input are bounded
at2904/2245 and128KiB, preserving formats1/2/3 and numeric rejection checks.

Both inspected Black candidate exports succeed. Private normalized metadata
removes only verified unused trailing-1table slots; present channels still reference
slot0, original asset/buffer bytes and complete lengths/hashes are preserved.
Normalization and five channel exports were independently reviewed. This one-off
private derivation uses assertions and is not a reusable optimized-mode validator.
Live-association, grasp and alignment flags remain false. Synthetic identity
arithmetic checks all2899/2904vertices but does not establish native shader/world
or GPU execution.

The legacy replay KeyError was reproduced and fixed by defaulting only missing
nativeId to1. Regressions cover completed/retained legacy and explicit13 labels,
missing required binding fields, per-ID bounds, overflow/truncation, maximum
vertex arithmetic and file overflow. Astra SOURCE GO; all9local turn tags confirm
xhigh/Astra, independent backend unattested. All4products rebuilt on fingerprint
`ec5541eabcf9365d802fc4b2ea8314015b2f0274286fc9f7570f9d663646753e`, IPC10/wire7.
75Debug/75Release and36normal/optimized native/compiled gates pass; subsequent
consumer25/replay10/candidate16groups pass normal/-O. No new native runtime.

The source path can accommodate Black. It is not an already prepared user launch
procedure: an actual unzoomed ID13 draw with unique native-content association and
qualified replay remains necessary. The correction is still inactive. No new
loader, IK, inventory/equip operation or alignment policy was installed.

| Capacity review recommendation | Disposition |
| --- | --- |
| Separate per-ID admission from storage | Adopted; ID1 unchanged, ID13 bounded, unknown reject. |
| Snapshot selected ID before COM and pass it through all range sites | Adopted. |
| Preserve native forwarding/lifetime/program/scope fences | Adopted; unchanged gates and compiled checks pass. |
| Preserve legacy IDs and reject mixed logs | Fixed and tested, including retained replay. |
| Normalize only unreferenced metadata tail slots | Adopted privately; raw assets/buffers unchanged. |
| Add another evaluator protocol/native-ID policy | Rejected; existing assessed-record policy and bounded arithmetic suffice. |
| Claim activation from static/synthetic arithmetic | Rejected; actual draw association remains open. |

## Actual native gun association and independent eye observation

Native single-player preparation created an isolated sniper save and preload.
Seven immutable first-eye/right-hand Black draws uniquely match the patch
variant's complete consumed channels and buffer ranges. Their own Poly Bump
source2 pairs/API rows independently corroborate all copied positions under the
unchanged tolerance; this is conditional uploaded-transform evidence, not GPU
precision, world provenance or earlier cache-producer continuity.

The other eye renders qualified native draws but can bypass original animation
query evaluation through a native cache hit. The collector rejects missing event
evidence. Source review did not establish a safe transferable cache incarnation.
The smallest next lab experiment alternates the temporal first eye by request
sequence, keeping every camera/target/depth/projection/readback slot indexed by
its actual physical eye. Both passes and original bodies remain intact. Scope
preview's eye0 convention remains unchanged. No cache invalidation, borrowed
event, forced evaluation or animation-name relaxation is introduced.

The option requires a sealed neutral ID13 configuration and native private-display
ID13 environment plus installed lab isolation. Default production order stays
left then right. Real own-event records for both eyes are still required; cache
hits or exhausted32attempts may produce an inconclusive capture. It does not
certify cached-eye continuity within a single request. Correction remains inactive.
