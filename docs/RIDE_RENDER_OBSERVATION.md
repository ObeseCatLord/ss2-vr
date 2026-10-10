# Controlled ride and rendered model association

## Completed stereo bank handoff correction

The previous source incorrectly required the active native-draw flag when
publishing copied observations. Stereo ends both eyes, clears that flag, rebuilds
the desktop view and later commits Ready; the exact bank owner remains held.
Thus a successful stereo commit still could not publish its ride observation.
Earlier individual-guard checks missed this normal sequence.

The existing owner gate now has explicit Drawing and Retained read phases.
Seven native sampling/recording consumers keep Drawing by default. Only the
private copied-value publisher requests Retained after a successful stereo commit
or normal mono completion. Both phases retain exact global/TLS/captured bank
tokens, native/thread ownership, readiness, suppression and invalidation checks.
Retained is not a completion receipt and does not authorize native scratch reads.
The publisher still performs its existing native thread-role query; copied
identities/matrices/buffers are never treated as later native pointer leases.

Reset, nested suppression and unwind invalidate ownership. Commit cleanup
invalidates the global token, so a future input handoff must copy values within
the successful commit extent, before cleanup. Keeping the draw flag active through
Ready was rejected because it would alter native frozen-model behavior during
desktop reconstruction. No additional gate/lifetime state was introduced.

Astra/xhigh source GO, current local routing verified/backend unattested. The
portable regression covers draw-end/retained publication and rejected native
reads, invalid/replaced/retired owners, mono and invalid phase values. Four source
mutations cover phase and completion bypasses; all eight actual compiled consumers'
false-gate paths remain checked. These are offline checks, not a live vehicle
stereo handoff. Physical grabbing/steering remains unfinished.

## Current handle transform adapter

The same default-off GPU receipt can now retain the two characterized handle
meshes from actual copied slices. All five complete channel digests must match
one known profile before extracting the fixed ranges. Original seam vertices,
weights/local indices, UVs and triangle winding are retained; no proprietary
coordinates are embedded in source. Fighter uses two25-vertex/30-triangle ranges;
Saucer uses two35-vertex/30-triangle ranges. Owned-resource offline parser checks
pass both profiles. They do not establish live GPU consumption.

During an ordinary stereo root draw, the existing executed root view and adjusted
projection provide an independent camera reference. A value-only accessor
requires current player, eye, tracking/graphics generation, rig, active root and
completed capture; nested views, scope/weapon passes, mono and changed native
camera matrices decline. The copied camera is compared at buffer bookends and
after every COM Release before certification. No raw-cache layout interpretation
or additional projection history is introduced.

Every handle vertex must use the actual mapped Main local slot with rigid
one-hot weights. The bounded existing VS1.1 interpreter uses actual UBYTE4N input
bytes under a separate vehicle policy (up to32 palette entries). Weapon replay
retains its previous three-entry/exact-integer policy. Microsoft documents
round-to-nearest for VS1.1 MOV a0.x; exact halfway values decline because its
[reference](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx9-graphics-reference-asm-vs-registers-address)
does not specify the tie rule.

CPU clip output must agree with P*V*ModelWorld*actualMainPalette at the existing
scope tolerance. An independently inverted P*V also backprojects every result
within1mm of the derived world vertex. Inversion residual and condition limits
reject collapsed or ill-conditioned views; unsupported arithmetic/inputs remain
unknown. This is a conservative CPU admission bound, not GPU precision or
proof that a live shader was observed.

The separate `Lab rideHandleProof` record reports geometryCopied and
positionAgrees only after the existing original/cleanup/bank gates. Schema5
ride-render records remain unchanged; their claim flags remain zero. No grasp
orientation, continuous input-time ownership or physical steering is enabled.
The diagnostic eight/64 budgets remain unsuitable for production continuous
grabbing. Next join copied world geometry to a current native rider/input
interval, preserving joystick fallback, rearm and smooth hand transfer.

| Astra recommendation | Disposition |
| --- | --- |
| Reuse executed root camera rather than infer V/P from raw sampler words | Adopted: value-only active stereo-root accessor; mono remains unsupported. |
| Avoid another projection observer/history | Adopted: existing draw/GPU owner and camera bookends. |
| Separate vehicle input/address policy and prevent collapsed clip agreement | Adopted: weapon policy unchanged; conditioned inverse and1mm backprojection bound. |
| Clear stale mesh payload and strengthen numerical regression coverage | Adopted: charged-attempt reset, noncommuting success and clip-pass/world-fail checks; fresh scoped GO. |

Review routing was verified as current local Astra/xhigh on every review turn;
independent backend attestation is unavailable. Reviews approve this source
boundary only, without live shader, alignment or vehicle input acceptance.

## Current schema5: copied GPU inputs and program

The same default-off collector now optionally copies GPU input evidence for the
two characterized Fighter/Saucer surfaces. An explicit Ride buffer policy reuses
the existing structural range checks, binding snapshots, shader-copy function,
scratch arrays, lock ledger and crypto owner. It does not use a weapon ID or the
ID2 submission owner. Only shared index scratch grows to2806 triangles; existing
weapon admission limits and scope behavior remain unchanged.

Collection requires initial bound inputs/program and a fresh Main bookend, five
bounded copies with completed unlocks, hashing of those owned copies, and second
inputs/Main agreement before the original call. Third inputs are sampled after
original success; the final native Main copy follows those getters. Existing
release-tail/reentry/owner/routing/generation checks and normal native-finally
completion gate an immutable optional receipt into the same render bank.
Unsupported layouts or ordinary copy failure leave independently valid Main/Seat
and draw metadata available. Uncertain unlock retains the existing fatal behavior.

Attempts, including failures and unpublished frames, are charged before GPU work:
eight per eye/bank and64 per process, with publication exhaustion also checked.
No additional reservation lifecycle or collector is introduced. Scope/idle/
submission owners exclude use of their binding slots. Raw ride float constants
bypass the scope's UV-specific c8/c9 finite-value admission; shader consumption
is not inferred from a declaration match.

Schema5 retains the schema4 inventory and adds gpuCopied to each draw. A successful
optional copy adds one mainDrawGpu record containing actual stream bindings,
descriptors, object comparison keys, declaration/program/float-constant counts
and five slice digests. Ordered mainDrawGpuWords families are program, constants,
then packed declaration; offsets are contiguous and chunks have at most64 raw
words. Limits are4096 program words,256 float-constant rows and65 declaration
elements. Schema1–4 compatibility remains. The assessor rejects malformed,
crossed, missing, duplicate or oversized inventories and reports copy presence
separately; all authority, program, grasp and steering verification stays false.

The digests describe five separately sampled owned byte ranges. Matching input
objects/constants and Main bookends do not prove whole-draw content immutability,
complete shader state or actual position-program consumption. Exact expected
content matching and admitted position semantics are still required to derive
world-space grip geometry. Neither the collector nor reader activates controls.
The bounded GPU payload is under a conservative5MiB encoded budget; the assessor CLI
limits its complete supplied text to5MiB, so oversized combined logs must be
extracted into an appropriately bounded private evidence input.

Astra/xhigh design ADAPT, frozen native source GO, consumer GO and final bounded
checker-fix GO; effective local routing was verified for each review turn, backend
unattested. All four products rebuilt at
fingerprint dfce2d83925c815089118a80cfe5f7d746ba1aa031e5a9457fb9d31cce623fd4,
IPC10/wire7.82Debug/82assert-enabledRelease groups and28 relevant normal/-O
compiled/artifact reports pass. Initial full CTest ran while reader work was
active and failed two stale source-mutation selectors; corrected checks pass.
16 reader cases and31 idle-reader/source cases pass normal/-O, including maximum
chunks and both checker-bypass regressions. Finite lexical source-order/mutation
checks are not general CFG/lifetime proof.
No runtime, vehicle/input probe, installation or staging occurred.

| Astra design recommendation | Disposition |
| --- | --- |
| Put the final Main sample after post-original getters | Adopted; no redundant full Main query inside finishRideGpu. |
| Bound failed and unpublished collection work | Adopted; existing-bank counters charge eight per eye and64 per process. |
| Keep ride policy independent of native weapon IDs | Adopted; shared structural mechanics with unchanged weapon limits. |
| Do not impose scope UV constants on raw ride observation | Adopted; float-constant bits remain raw. |
| Reuse existing lock/reference/crypto cleanup | Adopted; no second resource owner. |
| Hashes do not certify an immutable draw epoch or shader semantics | Adopted; evidence remains explicitly limited. |
| Marker presence alone accepted skipped charging and an OR comparison | Fixed; exact direct/conjunctive expressions and two reproduced negative controls. |

## Previous schema4: actual Main draw mapping

The default-off observer now connects an occupied ride's existing Main/Seat
frame to an actual native indexed draw through the existing render bank and GPU
probe owner. Historical schemas1–3 below remain readable. This is passive source
implementation; it does not enable vehicle grabbing or prove live GPU geometry.

The new internal RideMainMapping policy admits an exact root instance, model,
configuration/resource, selected LOD and global Main bone/definition. It validates
every palette-map member's draw and model ownership and requires exactly one Main
mapping. The actual local Main slot is copied; an authored palette ordinal or bone
name is not substituted. The diagnostic budget is32 mappings per draw, not a
native engine limit. SingleAffine and the ID2 three-entry weapon policy are unchanged.

This distinction matters: the inspected Fighter LOD0 Default surface has one
palette entry and2806 triangles; the Saucer LOD0 Saucer surface has15 entries and
2626 triangles. The positive Saucer handles belong to that larger surface, so a
whole-draw SingleAffine requirement would reject it. Authored one-hot weights and
local-index bytes are candidates until actual bound bytes and position-program
consumption agree. The current2673-triangle idle storage also cannot contain the
entire Fighter surface. Neither incompatibility warrants borrowing a weapon ID
or replacing the existing probe/cleanup architecture.

Before and after the original successful indexed call, complete native bookends
must agree on the frame and copied draw. Raw actual Main palette and model world
matrices are retained independently of canonical-source equality. Sticky reentry
denial starts before any public draw guard and remains through all COM Releases.
Cleanup queries the current owner after every Release, then samples fresh reentry,
reset generation, forwarding and routing/suppression conditions. Only normal
native-finally return can snapshot an immutable receipt, clear its readiness and
attempt the existing bank's final owner-gated record. No released device is
dereferenced, geometry is replayed or native input is changed.

Schema4 adds mainDrawCount and mainDrawOverflow to each identity. Up to eight
copies per eye have contiguous ordinals and exactly one mainDraw, one
mainDrawLayout and two mainDrawMatrix records (modelWorld and actualPalette).
Each record joins row/bank/eye/ordinal and carries native draw/surface/instance,
resource/LOD/Main identity, API arguments, channel descriptors and raw matrix words.
All buffer, program, grasp and steering claims remain0. Overflow is explicit;
eye visibility and counts can differ, including zero draws without loss of the
core Main/Seat frame. Rejected or unsupported draws are not inventoried.

The assessor's declared_draw_inventory_complete means that the declared bounded
log inventory is complete without overflow. It does not certify complete native
draw coverage, GPU semantics, authenticated source, usable grasp or input-time
freshness. draw_mapping_copies_present separately reports whether any copies exist.
Raw unsupported numerical values remain diagnostic; no transform is synthesized.

Astra/xhigh design ADAPT and final source GO; current local routing verified,
backend unattested. The finite native verifier pins both complete Core thread-query
bodies (13 instructions), export, Kernel32 import and executable Core dependency.
The proof assumes unmodified pinned code/imports and normal loader lifetime; the
uninitialized sentinel does not prove initialized main-thread identity. Seven
actual compiled owner-gate consumers reject a false gate before ordinary bank
reads; mutation controls exercise target, test register, polarity and entry bypass.
Those checks are not general CFG, callback-lifetime or native runtime proof.

All four products rebuilt at source fingerprint
fe0b725923e0e08543d6ccf134ba7bdb54a86e1bf58a83dfdbc83b76fc9d92ae,
IPC10/wire7.82Debug and82assert-enabledRelease groups pass, as do relevant compiled
forwarding/unwind/dispatch, reader and native mutation checks normal/-O. No game,
editor, vehicle, headset, firing, input or network test, installation or staging ran.

| Astra recommendation | Disposition |
| --- | --- |
| Observe actual Main mapping on a multi-bone surface | Adopted; no fabricated whole-draw affine or authored slot assumption. |
| Keep missing draw evidence independent of Main/Seat | Adopted; zero draws and asymmetric eyes retain core evidence. |
| Query owner before the final fresh cleanup flags | Fixed; certification follows all Releases and the query. |
| Do not publish a mutable TLS receipt across admission | Fixed; immutable value snapshot, readiness cleared before record. |
| Inventory completeness must not imply draw coverage | Fixed; declared inventory and copy presence are separate. |
| Prove the native query's narrow callback/lifetime premise | Adapted to pinned query/import/dependency checks, with explicit assumptions. |

Next: reuse the existing bounded buffer/range and program-copy plumbing with an
explicit vehicle capacity policy, bind actual indexed Main-weighted grip vertices
to this same draw, then integrate freshness/rearm/hand transfer at the original
mode-appropriate ClientAction boundary. No new collector service is required.

## Historical core observation

The optional `SS2VR_LAB_RIDE_CONTROL=1` diagnostic now joins the current local
operated ride to its model during the existing world-render borrow. It uses the
existing frozen presentation bank, not the earlier getter receipt or a new
cross-frame registry. It is disabled by default and on the headless server.
This is source implementation of a historical observation. No vehicle runtime,
installed-resource geometry, seat/grasp reference or physical steering pass is
claimed.

## Native ownership

Admission requires the original outer Render3D extent, native main thread and
current brain/player render owner. The two pinned hover classes are Sam2Game
vtable RVAs `0x2a8558` and `0x2b8420`. Every sample resolves current handles anew:

| Relationship | Required evidence |
| --- | --- |
| Brain → local living player | Existing UI owner; brain `+0x28` resolves to player |
| Player → occupied ride/seat | Native rider identity, seated state and valid seat |
| Ride → controlling brain | Ride `+0x38c` equals that brain's handle |
| Ride → model renderable | Ride `+0x120` resolves to standard game table `0x2a4648` |
| Renderable → ride | Renderable `+0x48` points back to this ride |
| Renderable → instance | Original Engine getter `0x15b220`, matching field `+0x5c` |

The constructor writes the owning entity link at Sam2Game `0x83a20`.
SpreadOperatorOwnership publishes the brain handle at `0xedc98–0xedca3`;
ProcessOperatorInput consumes it at `0x8d952` and reads brain `+0x28` at `0x8d962`.
The direct instance getter is scalar. No replacement-capable GetPuppetModel,
recursive DoYouOperateMe or resource-loading virtual query is introduced.
Whole-module pins remain enforced. Main independently checked the load-bearing
owner/operator/getter and palette-index instructions against the owned modules.

## Same-invocation render and animation evidence

Freeze captures only copied identity/configuration tokens before production.
At the existing Engine `0xe2e06` palette continuation, the observer requires one
matching model record, reciprocal evaluated cache ownership and bounded arrays.
Model first/count and selected skeleton LOD must agree with configuration,
skeleton definition membership and the evaluated cache's LOD row.

The evaluated cache header at `+0x2c` has capacity, row pointer at `+0x30` and
count at `+0x34`. Count must equal model-record count. Selected row stride is
eight, with LOD at `+4`. One Main definition must belong to the selected model's
validated bone range and skeleton LOD. Its matrix uses the invocation-wide
NativeBone index: native `0xdde8e–0xddea5` indexes canonical matrices with that
global index. Subtracting the model's first bone would select the wrong matrix.

Two complete fresh reads compare identity, configuration, skeleton/LOD/cache
bookends and raw model-world/Main matrix words. Missing or ambiguous Main,
selected invalid data, repeated captures, lifecycle invalidation, nesting,
reset, wrong thread or abnormal completion decline the observation. An unrelated
model's evaluated cache is ordinary visibility absence.

Only normal original mono completion or successful existing stereo publication
exports provisional copies. Exact owner retirement remains unconditional. Stereo
requires both eye captures. Observation with remote-head tracking disabled is
allowed, while head writes still respect that setting. Rendering applies no
steering, grasp, simulation or physics changes.

## Private log schema and limits

At most32 successful observations produce `Lab ride render schema=1` identity
rows, matching `Lab ride render binding` skeleton/LOD/cache rows, and raw
`Lab ride render matrix` rows for `modelWorld` and `MainCanonical`. Their join is
`row,bank,eye`; mono eye is `-1`, stereo eyes are `0/1`. Numeric addresses are
event-time tokens only and must never be dereferenced from a log or later cache.
All `resourceClaim`, `seatClaim`, `graspClaim` and `steeringClaim` fields are0.
Keep logs and geometry private. Empty logs do not establish a vehicle pass.

The source review is Astra/xhigh, with current local routing tags verified and
backend unattested. Compiled checks establish original mono forwarding,
normal completion/owner arguments and scalar cleanup. Three actual consumer
paths reject a false owner-gate result without reading ordinary bank storage;
changed gate targets, return-register tests and rejection branches fail the
byte controls. Native thread-query semantics and lifetime remain source/native
evidence, not a general compiled-path proof or a runtime pass.

| Astra finding | Disposition |
| --- | --- |
| Reuse the existing render bank | Adopted; no third unassociated collector |
| Require operator and renderable owner reciprocity | Adopted, with fresh dependent resolution |
| Bind Main to selected LOD/cache and global index | Adopted with two complete bookends |
| Delay export until normal owning completion | Adopted; exact retirement remains unconditional |
| Foreign callbacks can race early bank reads | Fixed: thread/token gate precedes every ordinary bank read |
| Separate source observation from physical controls | Adopted; all implementation claims remain explicit |

Next implementation needs actual installed resource/seat/control geometry joined
to this event, including Fighter Loading, and simulation-time freshness for the
existing rider/rig/input owner. Then one-/two-hand grabs and smooth transfer must
join native mode-appropriate input. Mode2 lateral X and mode3 sign-based steering
must remain distinct; no synthetic wheel/pivot or guessed offsets are admitted.

## Paired evaluated Main and Seat

The optional observer now requires one distinct Main and Seat definition in the
same admitted model's selected skeleton LOD. It copies both canonical matrices
using their global native bone indices and includes Seat identity/raw words in
the existing two-bookend comparison. Missing or ambiguous Seat declines this
observation. No extra sampling-time native query, evaluation, allocation, owner
or cross-frame registry was added. Both native IDENTs are initialized and checked
during the existing configuration path.

Schema2 adds the actual source fingerprint, seat bone/definition and SeatCanonical
matrix. Every identity, binding and matrix row uses the same schema. The existing
assess_ride_control.py now has --kind render; it requires a complete mono or stereo
observation, exact schema-specific inventories and selected model/LOD/bone bounds.
Stereo eyes share frozen rider/ride/seat/configuration identities; each eye's
transient cache, indices and raw matrices are validated independently. Raw matrix
words are retained, including unsupported numerical values, without normalization.

Historical schema1 remains readable but reports source as unavailable and Seat
as absent. The expected source supplied by a caller does not authenticate those
old rows. Schema2 compares emitted source with the expected fingerprint while
still denying independent source authentication and runtime acceptance.

Named Seat membership is not a certificate of the operated-seat attachment or a
usable grasp transform. Resource, seat, grasp and steering claims remain zero.
A complete rendered frame is not simulation-time freshness. Joystick/native aim,
ClientAction and physics behavior remain unchanged. The next feature integration
still needs current resource/operator-seat correspondence and a safely current
input-time frame for the actual handgrip geometry; Fighter Loading prohibits a
fixed authored Main/Seat replacement.

| Astra design finding | Disposition |
| --- | --- |
| Historical source is unavailable | Adopted; schema1 source/Seat remain unknown. |
| Complete schema-specific record grammar | Adopted; exact owner/binding/matrix inventories and finite32-observation budget. |
| Separate stereo ownership from cache values | Adopted; shared identity/config with independent per-eye native bounds. |
| Reuse owned cache/global indexing and raw bookends | Adopted; no new sampling callback or owner. |
| Seat name does not prove operated-seat attachment | Adopted; all feature claims remain zero. |

The source-order checker states its finite lexical scope; existing compiled
scalar-copy and declined-entry checks do not prove all Seat-selection semantics
or actual native execution. Private scripts/captures/assets remain private.

Final source review caught accepted lookalike log prefixes. Main reproduced the
identity/binding/matrix counterexamples, added exact reserved-prefix/delimiter
validation and independent negative cases, and obtained corrected-prefix source
GO.9normal/-O reader cases pass, now registered with CTest.82Debug and82assert-
enabledRelease groups and54normal/-O current compiled/artifact/mutation reports
pass. All four products match source fingerprint
1a453f2210cbefa6199c866aaac2f14317fbb106bc2cfc6d79e978fb0b17aec2,
IPC10/wire7. Local reviewer routing was Astra/xhigh; backend unattested.
No vehicle/runtime acceptance follows from this passive source change.


## Optional flat occupied-seat mapping

Schema3 adds optional declared attachment metadata to the existing frame copy.
The sampler first re-establishes the same native render owner and reciprocal ride
identity. It reads the ride's managed parameter reference and declines bit0 pending
resources before inspecting their seat array. Native getters would replace those
resources; agreement between old raw copies alone would not establish the native
selection. The selected native seat datum must have the pinned class and a unique
name matching the current rider's seat; its attachment IDENT is copied separately.

The current model instance owns the child-descriptor array. Every admitted member
must have null embedded configuration and child-state pointers, making this actual
attachment tree flat. The selected descriptor must uniquely match the native
attachment IDENT, and its parent-bone IDENT must equal the selected Seat definition's
name. Numeric global bone indices are used only for an existing record's parent
binding. Both complete raw memberships and selected pose/scale words are compared;
these optional reads sit between the original core frame bookends.

The parameter and child arrays are bounded to 32 members. This covers the inspected
nine Fighter and eleven Saucer descriptors, without claiming wider resource support.
The optional mapping may decline; the existing Main/Seat observation still publishes.
Normal rendering can omit null-model attachments, while the native attachment query
includes them. Record presence is reported from the admitted produced record array;
no evaluated child world is synthesized, and childWorldAvailable remains0.

Schema3 identity adds attachmentMapped. Successful mappings add exactly one attachment
record and one attachmentPose record (seven raw pose and three raw scale words).
Stereo requires shared mapping identity/pose/scale agreement; disagreement clears
both optional mappings. Per-eye record presence/index may differ. The assessor retains
schemas1/2, rejects incomplete or crossed schema3 records, and keeps all authority,
resource, grasp, steering, input-freshness and source-authentication claims false.

Astra/xhigh design and source GO, current local tags verified and backend unattested.
The actual fourth compiled guard consumer uses JNE to admission and a false fallthrough
to scalar return. The verifier walks that false path; polarity/target/test-register
mutations remain rejected. Both raw-copy helpers are checked for absence of FP/SIMD
instructions. Finite lexical checks are explicitly not general CFG/lifetime proofs.

| Review recommendation | Disposition |
| --- | --- |
| Reject pending parameter resources before reading seats | Adopted. |
| Resolve native lookup ambiguity, including possible descendants | Adapted: prove every current member is a flat leaf and select a unique name. No shadow traversal. |
| Require a normal rendered child world | Removed: it would suppress these null-model seats. |
| Add simulation-query TLS or another attachment query | Removed; the existing managed references/render borrow suffice for metadata. |
| Keep metadata failure independent of Main-based grips | Adopted; optional mapping never becomes a new physical-control gate. |
| Copy parent-bone IDENT separately from canonical index | Adopted. |
| Extend scalar compiled checks to the new helper | Adopted. |

The physical-control critical path is actual grip geometry associated with the
current resource, mesh/palette and applicable LOD, followed by a valid input-consumption
interval and lifecycle/rearm handling. The copied canonical matrices are sources
for the native draw palette, not ordinary bone-local placements. The pinned DDE30
copies the selected global bone's twelve canonical words directly into its draw
palette. For a nonnull definition, native DB140 instead forms a bone placement by
multiplying canonical P by the native rigid inverse of the definition's stored
inverse bind at +48hex. These frames differ when that stored matrix is nonidentity;
the null-definition branch copies P unchanged. The rigid inversion import is not
evidence of arbitrary affine inversion support.

For an independently admitted one-hot Main-weighted vertex in the actual draw,
the reusable rendering transform is ModelWorld times its selected draw palette.
Multiplying the stored inverse bind into that palette again is not justified.
A bone-local authored point requires its own bone-placement derivation; do not
silently use it as an asset-space vertex. The render log does not establish the
actual draw-map or loaded grip content, so the assessor explicitly labels the
canonical role while keeping bone-placement-copy and draw-mapping claims false.
The existing postPalette adapter can write the palette before observation;
producer-copy identity therefore does not certify final-draw equality. Use the
actual admitted draw palette, or prove equality within that same borrow.
This corrects the earlier bone-frame description above. The existing native
steering verifier checks the pinned Engine copy/inversion seams in addition to
the Sam2Game control route; finite static checks do not certify a loaded draw,
shader, geometry or grasp. Neither these matrices nor attachment metadata alone
admit a physical grasp. Existing native body/view placement, mode-specific
ClientAction and physics remain reusable. No vehicle/input/runtime test or
controls activation occurred.

| Matrix review recommendation | Disposition |
| --- | --- |
| Distinguish canonical draw source from bone placement | Adopted in the existing reader and current documentation. |
| Avoid an additional bind transform for admitted authored vertices | Adapted: require the actual same-borrow draw palette and proven vertex/weight inputs. |
| Do not infer final palette equality from DDE30 alone | Adopted; postPalette can modify the palette and equality remains unverified. |
| Reuse existing draw and bounded buffer ownership | Adapted by schema4: Saucer requires Main mapping in a multi-bone draw rather than whole-draw SingleAffine. |
| Add a new native query, collector or inactive grip helper | Rejected for this slice; existing draw/copy boundaries remain reusable. |
