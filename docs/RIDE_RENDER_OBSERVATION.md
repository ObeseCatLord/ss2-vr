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
