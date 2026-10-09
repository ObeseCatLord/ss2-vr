# Auto Shotgun neutral collection boundary

This extends the existing private idle collector to native weapon ID2. It does
not enable an Auto Shotgun alignment correction or certify physical hand contact.
Production alignment remains limited to the already admitted ID1/ID13 routes.

## Native preparation

`prepare_idle_fixture: true` requires an explicit neutral `idle_native_id` of
2 or13. The existing `prepare_sniper_fixture` option remains an ID13-only alias;
simultaneous opt-ins reject. The launcher clears both inherited preparation
variables before setting the selected option. Native configuration independently
checks the selector, private online isolation and owned executable/current directory.

The existing single preparation state machine, simulation/current-owner checks,
sole-local-player roster, neutral controls, deadline, failure latch and native
unwind cleanup remain shared. Native GiveAll is followed by inventory admission,
CanChangeWeapon(right), SetNewWeapon(selected,1,0,0), and native save. Refused
selection or missing inventory cannot force a field or retry the native action.
Bindings are reacquired on later ticks and after save.

ID2 readiness requires the exact AutoShotgun class, matching native ID, owned
nonaliased right-hand binding and common base idle state1. Sniper zoom fields and
callbacks are used only by ID13; ID2's field at+d4 is not treated as sniper zoom.
The fixed ID2 output is `Temp/SS2VR/autosg-id2.sav`, with a separate `Lab autosg`
receipt prefix and `idle_fixture_preparation` manifest record. The save/preload
must be absent before launch; returned files, ordered stages, owner and process
incarnation are verified. Legacy sniper filenames, log prefix and schema1 receipt
remain unchanged. These fixtures are cheated, private and reload-unverified.

## Content evidence versus pose evidence

Diagnostic storage supports3017vertices/2673triangles. Native admission retains
separate exact limits: ID1 1490/1332, ID13 2904/2245, ID2 3017/2673. Existing draw,
program, constants and total evaluator input budgets remain unchanged.

Two exact stock AutoShotgun mesh fingerprints have selected declaration/span
policies. Proprietary buffer payloads stay private. An unused serialized null
buffer reference preserves its table index; a channel selecting that slot rejects.
Explicit ID2 extraction admits rigid255/0/0/0 weights and first palette indices
0/1/2 with zero unused components, bounded by the actual file-local palette.
It does not label these candidates as single-body influence.

A match requires explicit candidate ID2, the rigid-palette classification, exact
counts, buffer lengths, channel ranges and all five channel hashes. ID2 replay
then returns `autosg-palette-arithmetic-unsupported` **before** executing the
single-body evaluator or either transform-reference fallback. Content collection
can identify the actual mesh variant; it cannot supply a qualified rendered pose,
world geometry, handle alignment or native release evidence.

The next implementation step is to use an authorized neutral native collection
for the loaded resource/render association and actual consumed-index/palette
arithmetic. Do not activate the authored handle candidate before that evidence.
Current ID13 runtime preparation authority is not expanded by this source change.

## Review disposition and verification

| Astra recommendation | Disposition |
|---|---|
| Reuse preparation state/owners and native actions | Adopted; one state machine |
| Separate ID2 readiness from sniper zoom | Adopted; exact class/base-state branch |
| Keep legacy ID13 receipts and distinct ID2 saves | Adopted |
| Enlarge storage while retaining per-ID admission | Adopted |
| Separate multi-index content matching from pose replay | Adopted; unsupported replay explicitly rejects |
| Require actual compiled ABI/dispatch checks | Added bounded production-object gate and in-memory instruction mutations |

Astra/xhigh scoped source reviews accepted native preparation and offline consumers.
Local model/effort tags were verified; independent backend identity is unattested.
All79Debug/79Release groups pass. All four products rebuild with matching IPC10/
wire7 contracts. Twenty compiled gates pass normally/under Python optimization,
including selection argument/receiver staging, four-argument callee-pop repair,
cdecl save argument and target-specific zoom dispatch. The compiled gate covers
those finite emitted boundaries; it does not replace native lifetime source review.
No ID2 runtime, firing, vehicle, network or physical-headset test is claimed.
