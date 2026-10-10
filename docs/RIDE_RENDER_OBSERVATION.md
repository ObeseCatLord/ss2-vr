# Controlled ride and rendered model association

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
interval and lifecycle/rearm handling. ModelWorld times MainCanonical is a bone-frame
basis for verified Main-bone-local geometry; raw authored model vertices may need
native inverse bind. Neither these matrices nor attachment metadata alone admit a
physical grasp. Existing native body/view placement, mode-specific ClientAction and
physics remain reusable. No vehicle/input/runtime test or controls activation occurred.
