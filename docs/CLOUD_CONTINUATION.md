# Cloud continuation — 2026-10-05

The user requested that dot perform development itself in its cloud workspace.
The local task was used only to inspect and transfer files and publish the
reviewed source snapshot. It performed no development or runtime testing.

## Starting checkpoint

Public source commit `02283f5f074c67b9c7512881cea3f9cbb1e62c99` imports the
accepted source. The seven-file physical-melee WIP remains unapplied in
`handoff/unaccepted-melee.patch`. Nothing in this continuation accepts that WIP
or changes the original PC project. The existing source configured, built, and
passed all 23 portable test groups with GCC 14.2 in Debug mode in the cloud.

## Portable verification fix

The core-only CMake configuration now explicitly keeps assertions active in
all build configurations. Previously a Release build failed the scope-optics
source's deliberate `NDEBUG` guard; head and scope-pose checks instead had
unguarded `assert` expressions that a Release build could compile away.
Production game/host flags are unchanged.

An additional harness check verifies assertion expression evaluation and uses
an explicit failure return, not another assertion, to catch a disabled harness.
Compiling it separately with `-DNDEBUG` reproduces the failure. This is a test
harness correction, not a new VR interaction or proof of native correctness.

Verification: all 24 groups pass in both Debug and Release with GCC 14.2 and
CMake 3.31.6. The deliberately disabled-assertion control exits with failure.
The original Release scope-optics target failed before the CMake correction and
passes afterward. MSVC and the Windows mod/host products were not built here.

## Native gate: current findings and remaining evidence

The user supplied the private DLL archive and Sam2.exe. All four fingerprints
match the original research. These files remain outside this source repository;
no Windows executable or DLL was run.

The next bounded proof remains the saw release receipt's allocator-failure path
through conExit. The new read-only tool `tools/verify_saw_exit_boundary.py`
verifies the pinned Core.dll fingerprint and callback-site topology and refuses
unknown builds. It identifies an earlier fatal callback, pre/list/post exit
callbacks, and an eventual MSVCR71.dll `exit` import rather than a direct
ExitProcess boundary. See `docs/SAW_FATAL_EXIT_AUDIT.md` for scope and limits.

The native callback target closure remains unproved. The four supplied modules
do not establish a closed set of loaded plugin/termination callbacks. The user supplied the remaining 41 installed DLLs; their manifest hashes pass
and none directly imports the five shutdown-registration APIs. Dynamic targets
and CRT teardown remain unproved; no code investigation was delegated.
No new receipt hook or physical-melee reconciliation is enabled. The user still
excludes game/headset/Wine/OpenXR execution.

## Inactive native-consumption receipt candidate

Added `src/common/native_consumption_receipt.hpp` and production-helper tests.
This implements only the proposed consumed-level metadata, not native saw logic,
input retention, reconciliation dispatch, or a binding registry. It is not used
by game/host/server code and is not claimed as a completed physical-melee feature.

A receipt is bound to stable record storage, an owner-supplied nonzero unique
lifetime epoch, and the current revision. Every completed observation advances
that revision, even when its logical level is unchanged. Thus a completed nested
observation supersedes an older outer receipt. Replay, cross-record commits and
retired/reused binding receipts are rejected. Initial/replacement history stays
unknown until a proven actual native completion is recorded. Records cannot be
copied or moved into generic sample saves. Revision exhaustion fails closed.

Required caller proofs remain open: stable storage and lifetime epochs, exact
completion boundary, retirement after ambiguous partial native effects, capacity,
copy/replacement ownership, manual carry/command isolation, and observer delivery.
The current copied Authority sample rows must not simply embed this record.

Verification: all 25 portable groups pass in both Debug and Release, including
cross-record rejection, same-address/new-epoch reuse, repeated receipts, nested
completion and retirement. The receipt test passes GCC ASan/UBSan with leak
checking disabled because LeakSanitizer fails under this workspace's tracing;
leak checking is not claimed. The network suite also passes UBSan. No native
hook, Windows product, or in-game behavior is verified by these portable results.

## Reproducible cross-build and ABI checks

The user explicitly removed the outside-review requirement and requested work
continue until the complete handoff feature set is finished. Root owns the work.
AGENTS.md now records that current instruction; no coding/review worker is used.

The cloud now builds the x86 proxy/server and x64 host/OpenXR loader with GNU
MinGW GCC 16.2.0 configured with native TLS, binutils 2.47, and Clang 23.1.2 only
for the existing native-finally C object. Tools were unpacked into the cloud
workspace from official distribution releases; the PC was not modified.
Earlier GCC 12 hit an internal compiler error. GCC 14 compiled the source but
emitted `__emutls_get_address` in the scalar primary predicate, failing its
allocation-free boundary verifier. No game-source workaround or relaxed check
was retained. A new CMake probe now rejects emulated TLS before a game build.
Clang game compilation is also rejected consistently with existing source guards.

The existing camera ABI verifier assumed an older helper signature. The current
wrapper also forwards the native return address separately. Its replacement
checks the complete scalar wrapper sequence: receiver, all twelve camera words,
separate caller provenance, sniper discriminator, call and exact cleanup. Eight
negative/control tests cover wrong offsets, arithmetic, overwrite, extra calls
and discriminator/cleanup changes. The vehicle check now additionally accepts
one precisely checked GNU 16 callee-saved receiver sequence. No native game
behavior was changed for these checker updates. The roomscale verifier now
accepts an explicit --game path for private input files in a cloud workspace.

Linked primary, native-finally, zoom, weapon, world-marker, vehicle-laser and scope
GPU checks pass; the predicate and roomscale compile-only ABI checks pass. Full
artifact verification, packaging, native callback/lifecycle closure and remaining
immersive features are still pending. No native/runtime correctness follows from
these compilation and structural checks.

## Manual-independent gesture preparation

Recovered only the isolated physical-motion helper/tests from the preserved WIP,
not its rejected engine wiring or command-history rebasing. Added a separate
PhysicalGestureInput with owner/weapon/rig/hand and producer/session/action
provenance, fresh quiet arming, replay/age/context rejection and explicit current
sample validation. It consumes const Input and never changes raw triggers or
manual command history. This separation is preparation for the consumed-native
boundary; the helper is still inactive in game code.

All 28 portable groups pass in Debug and Release. Gesture and receipt tests pass
ASan/UBSan with the earlier leak-checking limitation. Receipt, motion and gesture
tests compile for both GNU MinGW x86 and x64. Multiplayer delivery, lifetime
admission and actual native press/release connection remain unfinished. These
results are a foundation checkpoint, not completion of physical melee.

## Canonical target and manual-history separation

The primary-read helper now has an explicitly separate held-only gesture value.
Operator current/press/release/history reads ignore it, and same-pawn invocation
inheritance does not transfer it to another weapon. Existing native callers
leave it empty. A new pure classifier records the exact post-flip canonical
release targets, including coupled dual release and right-only alternative
fire. Only a unique primary target is eligible for its narrow candidate path.
Neither addition connects gesture input to native gameplay.

All 29 portable groups pass Debug and Release. Both x86 DLLs rebuild and the
actual linked primary scalar helper remains call-free. Dispatch/projection
checks pass ASan/UBSan with leak checking disabled for the previously documented
workspace limitation. The other targeted native-finally, zoom, weapon,
world-marker, vehicle, predicate, scope and roomscale ABI checks pass again.

Static default/copy constructor inspection is reproduced by
tools/verify_saw_lifetime_boundary.py and documented in SAW_LIFETIME_ADMISSION.md.
A freshly constructed saw starts released, but copy construction and assignment
retain active native fields. First selection cannot initialize unknown consumed
history to low. Stable native lifetime ownership and release-boundary reentry
remain open. Full artifact verification also awaits the privately supplied
DedicatedServer.exe; the user has been asked for this remaining input.

## Private input and final-artifact checks unblocked

The user authorized a transfer-only local task for DedicatedServer.exe and the
stock Sniper.bmf/Sniper.skl assets, then attached its private archive. All three
hashes match the pinned inputs. Root materialized them only in private analysis
storage. No proprietary files entered this repository or public GitHub.

tools/verify_artifacts.py now passes against the four actual linked products:
x86 game/server, x64 host/loader, 48 exported-hook declarations, 16 internal
boundaries, dedicated-server identity and equal IPC8 layouts on both Windows
architectures. The mesh/material/UV/skin audit also passes with the supplied
skeleton, including the single Sniper root and identity inverse bind. These
static results do not establish live content admission or rendered optics.

Additional pinned stock inspection covers all 18 weapon command/render tables;
see SCOPE_STOCK_COMMAND_COVERAGE.md. Roomscale preparation inspection found
another replacement boundary and explicit cancellation-propagation obligations;
see ROOMSCALE_PREP_CANCELLATION_FOLLOWUP.md. Neither investigation enables its
unfinished feature. The full immersive goal remains active and incomplete.

## Ordered scope cap execution preparation

Added executeScopeCapDraw for the previously selected native901/native22/RGB22/
native5 ordering. It preserves every original triangle once and the final five
triangles' precedence, uses index-unit offsets, and stops immediately on any
failed draw, failed restoration or invalidated owner check. There is no full-draw
fallback or replay after partial execution. Callbacks must be nonthrowing; the
future native owner still needs NativeFinally for foreign unwinding and COM/state
cleanup. The helper provides execution order only, not image/resource admission.

All 30 portable groups pass Debug and Release. The new helper tests pass
ASan/UBSan (leak checking disabled as above) and compile for both Windows
architectures. Tests exercise every draw-failure and ownership-invalidation
boundary and exact per-triangle coverage. No native caller uses the helper yet;
source capture, color transfer and the complete live GPU transaction remain open.

## Scope shader constants and native Linux compilation

Built vkd3d1.17 from its official WineHQ source release in the cloud workspace,
using privately unpacked official Debian build dependencies. No system packages,
Wine, game or Windows executables were run. Source archive SHA256 is
bc61cb9e84d5045cbcaffbdd707940d399d8bf62874663dfe5809a0bfb87e9b6.
The native ELF compiler reproduces the mod-owned HLSL's RGB-only146-slot and
experimental alpha151-slot programs, both four temporaries and only color0
output. The existing independent UV/color token-fixture checks also pass.
These outputs remain build artifacts, not installed shaders.

Added the exact six-row constant builder for scope_image.hlsl. It copies the
admitted UV-to-optic map, transforms eye position into the proper optical basis
using centered double arithmetic, and constructs the existing angular lookup.
Visibility and reticle dimensions remain explicit inputs. Invalid/nonfinite,
behind-plane and out-of-range inputs clear output and decline. The builder
does not sample native zoom, choose visibility policy or authorize an image.

All31 portable groups pass Debug/Release. Constant tests compare the shader
interface numerically to the optical ray mapping for independent eye offsets,
UV positions and a rotated/translated optic; sanitizer and both Windows
compile-only checks pass. Native image binding/source capture remain unfinished.

## Connected per-weapon native zoom observation

The existing admitted scope-pose observer now copies this exact sniper's native
active/alternative-held state and F0/EC/F4 interpolation values. Sam2Game's
1728B3..1728D9 path uses F0+(EC-F0)*F4; activation171D03 writes activeD4=1.
The reader requires the exact pinned sniper vtable after existing live
owner/weapon/model checks. It calls no gameplay function, writes no native
field and does not read the player's shared FOV multiplier. Inactive, nonfinite
or out-of-range observations decline independently of geometric evidence.

The copied values travel with the existing request/input/owner/weapon/model/hand
observation and clear on its existing invalidation paths. This records current
native fields, not proof that interpolation completed in the current interval.
Tests cover different simultaneous hand values, model replacement, pair faults,
failed observation and eye retirement. Debug/Release31 groups, rebuilt x86 DLLs,
linked native ABI checks and full artifact verification pass after this change.
Scene-source capture, base-angle interpretation and image drawing remain open.

The source-cut ordering helper now rejects unknown command classifications,
late scene work, duplicate guns and second-gun retries. All32 groups pass in
Debug/Release; the helper also passes sanitizer and Windows compile-only checks.
Native command-list layout/sort/execution evidence and the remaining live
classification/ownership limits are in SCOPE_SCENE_CUT_CONTRACT.md. It is not
yet connected to scene capture and provides no framebuffer-content guarantee.

## Compiled GPU source-copy adapter, not yet invoked

Added captureScopeSource to both x86 products. Given a caller-owned admitted
source and distinct single-level destination texture, it checks actual current
RT identity, surface/device identities, exact matching dimensions/UNORM format,
non-MSAA/default-pool/render-target descriptions, complete viewport and absence
of additional MRTs. The supplied bridge copy must be original/unscaled/unfiltered.
It rechecks RT/viewport and the caller's frame/resource guard after copying and
after releasing its temporary references. Failed or partial copies stay unusable.
Every partial COM output is explicitly owned above NativeFinally and released;
the adapter itself performs no state or binding changes.

This is compiled GPU code but has no connected native source-view caller. The
caller still must prove the scene cut/color representation, retain resources,
and supply a sticky interference guard; final pointer equality alone is not
an uninterrupted-copy proof. No GPU/COM behavior or fault injection ran offline.

All33 portable groups pass Debug/Release. The actual source-layout/viewport
predicates are covered by malformed, format mismatch, MSAA, pool/usage, size and
depth-range tests, sanitizer and both Windows compile-only checks. The x86 DLLs,
native-finally verifier and full artifact verification pass after integration
of this source file. There is still no magnified image in the game.

The copy adapter additionally requires D3D_OK from TestCooperativeLevel before
inspection/copy and after the copy. Microsoft's D3D9 lost-device contract allows
S_OK with discarded work; successful getters/copy alone are not pixel validity.
Later loss/reset remains the frame owner's rejection obligation. Reference:
https://learn.microsoft.com/en-us/windows/win32/direct3d9/lost-devices

The later native scene-source audit already supplies the selected gun-free,
pre-bloom callback-command route. Removed the newly added narrow first-gun
ordering helper/tests before connection rather than retain a parallel path
that excludes later flares. Its historical test result is not source-content
acceptance. SCOPE_SCENE_CUT_CONTRACT.md records this correction; the next native
source adapter follows SCOPE_SCENE_SOURCE_AUDIT.md's rankAFFFF command.

## Native capture-command adapter preparation

Implemented the selected native command builder, without configuring or invoking
it from a source view yet. It checks the pinned exported allocator/constructor
addresses and callback vtable, requires the actual FDA0B injection return and
the caller's current source owner, then uses the native24-byte allocation and
native constructor to append. It never edits an executing command array.

After a normal constructor return it immediately completes a harmless native
callback command at rankAFFFF. Only confirmed current-root/parent/array/count/
last-entry ownership arms the requested callback/context. Rejected linked
commands remain harmless and native-owned; the caller must not retry or remove
them. Unlinked native pile allocations and abnormal native construction are not
freed or synthetically recovered by the adapter. Source context lifetime through
native execution and root/pile cleanup remains a caller requirement.

The native callback deleting destructor ultimately reaches a no-op delete at
EngineEE960; normal finish calls Core CPileAllocator::FreeAll at14C057. The
compiled production bodies pass an exact24-byte stack allocator argument and
the returned storage in ECX to the constructor; the harmless callback has plain
cdecl return. tools/verify_scope_capture_abi.py reproduces those checks for both
products. Portable membership checks cover replacement, missing identity/count,
duplicate append and bounds. This does not prove abnormal native allocator
recovery, execute a callback or enable a magnified image.


Capture-command checkpoint validation: all 33 portable test groups pass in both
Debug and Release; x86 game/server compile and link. The command predicate test
passes ASan/UBSan with leak detection disabled and both Windows architectures
compile-only. Production capture ABI, native-finally ABI and the full artifact
verifier pass. These are offline checks, not native command execution.

The user now authorizes ongoing fetching of needed SS2 inputs and regular
source pushes to GitHub. After implementation, research VR mods, VR
implementations and SS2, then perform a thorough root-owned whole-project
review and fix issues. This does not authorize game/headset execution or
publication of proprietary game files.

## Native base-angle observation connected

The existing projection/frustum hooks now copy the actual unzoomed horizontal
angle for the current native eye before XR projection replacement. Exact root
return94398 scopes the observation; exact frustum return9436F samples its radian
argument and the pinned Player getter's scalar858. The live virtual slot5F8
must still select F8050, whose complete body only loads that scalar and returns.
No native call occurs from the getter through the frustum invocation. Recovering
base = actualArgument / scalar undoes shared zoom only for base-angle evidence;
each hand still derives magnification from its own sniper interpolation.

The root's native aspect adjustment and 45..135-degree clamp remain native.
Nested projections reject the outer observation; duplicate/mismatched frustum
calls, unexpected vtables, malformed inputs and failed native completion leave
zero/unavailable. The native-finally owner restores observation TLS on unwind.
Per-eye observation retirement also clears this base angle. No FOV/zoom field is
written, no new hook installed, and native projection still executes once.

tools/verify_scope_native_fov.py checks
the pinned DLL hash, root/getter/frustum chain, virtual slot, radians constant
and clamp immediates. Scalar tests cover distinct shared multipliers recovering
the same base and independent native interpolation-based magnification. All33
Debug/Release groups, x86 DLL builds, weapon/native-finally/capture ABI and full
artifact checks pass; optical math passes ASan/UBSan with leak checking disabled.
This closes the angle-source question, not source-view or cap-image integration.

The user-supplied shader archive was privately materialized and verified against
54bbcce7347aa721754c9b6b92aa025f64f85837614c9745418d8f9dc65e49a1.
It contains the expected PP2 families; no proprietary shader bytes are committed.

## Connected magnified-scope source and ordered cap path

Native scope image rendering is now connected in source. One admitted ordinary
preview provides copied optical poses; explicit per-hand gun-free source views
queue the native pre-bloom capture callback, then the final real eyes consume
matching source images through the actual Scope cap. Native prefix/cap/suffix
remain ordered; no full draw is replayed after a partial split. Root-owned
transaction/lifetime checks and limits are detailed in SCOPE_SOURCE_INTEGRATION.md.
The original native zoom state, timing, gameplay and surrounding XR FOV remain
unchanged. No existing archive or installed game file was replaced.

The mod-owned opaque PS2a bytecode is reproducibly emitted from HLSL into a source
header. CMake rejects a stale source stamp; the embedding checker verifies both
hashes and byte-identical local compiler output. This is shader build evidence,
not observed native shader coverage or headset verification.

Final connected-scope checkpoint validation: all35 portable groups pass in both
Debug and Release with assertions enabled. All four Windows products rebuild;
full artifact verification, primary/zoom/predicate/weapon/world-marker/vehicle/
native-finally/capture-command/capture-callback/DIP ABI checks pass. The original
DIP wrapper argument/caller proof remains unchanged. Native FOV and roomscale
read-only checks also pass. Position/UV/layout/capture/reticle correspondence
checks run under ASan/UBSan with unsupported leak checking disabled, including
2000 bounded malformed shader-token mutations. Position checks compile for both
Windows architectures. Mod-owned skinning/SUB tokens were independently decoded
by Linux vkd3d; opaque image shader is158 slots/four temporaries and2548 bytes.
No Windows executable, native COM call, shader draw or game/headset session ran.

GitHub was connected successfully in the parent cloud conversation. Direct
repository publishing is now available; the earlier manual bundle-download
wait is superseded. Public checkpoints contain source only. Private game inputs
and all generated/native build products remain outside the publication tree.

## 2026-10-06 — Windows/Proton startup hardening

Both platforms remain required, with runtime execution still excluded. Replaced
unchecked fixed-size module/system path buffers with bounded UTF-16 retrieval;
invalid/truncated paths fail closed. Host startup now specifies its executable
explicitly and inherits the game's environment and prefix. The D3D9 proxy uses
an absolute system path and restricted dependency search, rejecting self-aliases
and missing mandatory exports. Host diagnostics identify the Wine/DXVK OpenXR
boundary without changing runtime registration or logging sensitive paths.
See PLATFORM_SUPPORT.md for the architecture, upstream evidence and setup limits.

All36 portable groups pass Debug and Release; x86 game/server and x64 host/loader
builds pass. Artifact, primary/zoom/predicate/native-finally/weapon/world-marker/
vehicle/scope-capture/scope-view/scope-GPU ABI checks pass. Path checks also pass
ASan/UBSan (leak checking disabled) and compile for both Windows architectures.
No Windows, Wine, graphics, game, headset or network runtime was executed.

Publication now has an explicit read-only privacy gate for every outgoing ref.
It walks raw commit parents with replacement objects disabled, requires anonymous
project identities with empty email fields, rejects possible embedded addresses,
shallow histories, annotated tags and embedded signatures/tags. Its temporary
repository regression covers hidden dirty ancestors, replacement/graft tricks,
invalid ranges, private names, message addresses and clean duplicate refs.
All37 Debug/Release groups pass after integrating that regression. This gate
prevents publishing the private development ancestry; source trees are transferred
into fresh anonymous publication history instead. It does not claim deletion of
GitHub caches or third-party copies.

## Optional roomscale query cancellation component

Added a compiled, inactive native resource-cancellation adapter. Inspection found
two material-getter replacement callbacks beyond the earlier six model-preparer
sites; both and their caller are covered. Ten resource branches and five native
post-call propagation/cleanup gates preserve native state outside an explicit
mod-owned scope. This is not activated body movement. Current design, assembly
contracts, evidence and remaining owner/geometry/settlement gates are recorded
in ROOMSCALE_RESOURCE_CANCELLATION_SOURCE.md.

## Primitive initial-contact component

Implemented the inactive primitive counterpart to the triangle query adapter.
It uses bounded support-plane checks for the exact admitted sphere/box/capsule/
cylinder descriptors, repairs initial-contact admission without inventing hit
normals, and preserves native positive-entry execution. Type4 remains unadmitted.
The hidden-output/native axis contract and compiled entry are verified; all39
local groups pass both configurations, plus sanitizer and Windows compile-only
checks. See ROOMSCALE_PRIMITIVE_CONTACT_SOURCE.md for evidence and remaining
query-owner/geometry/placement/multiplayer gates. Roomscale is not yet connected.

Physical CModelHull collision uses a separate CB3B0 path, reached through CBF90,
not only the earlier rendered-model preparation route. Its eight resource
branches now have cancellation gates with native profiler/SEH/ret8 cleanup;
none cancels while its optional vertex buffer is locked. The adapter now covers
18 resource branches plus five propagation points. Both x86 products and all40
local groups still pass; normal provider/collider closure and the actual query
owner remain pending. See the resource-cancellation source record.

## Optional immersive swimming — 2026-10-06

The requested choice is now connected in source, with default head-directed
joystick water input and optional bounded forward arm pulls. Nonzero joystick
input retains priority in either mode. It uses native ProcessPlayerControls and
ClientAction rather than a movement rewrite or direct pose writes. See
[SWIMMING_CONTROLS.md](SWIMMING_CONTROLS.md) for admission, native ABI evidence,
configuration and explicit runtime limitations. Other unfinished roomscale,
melee and multiplayer settlement work remains open.

## Root target geometry continuation — 2026-10-06

The checked-placement component now has bounded geometry preparation for its
actual native root target. Captures retain separate root, model, absolute hull
and relative hull placements; active root/hull propagation is required. Native
child recomposition is enclosed across caller precision/rounding choices, then
combined with the captured hull in the existing affine capsule cover. Ray
endpoint and world-grid error consume the explicit radius allowance. This is
still inactive: native query ownership, checked placement integration and
local/authoritative origin settlement are not complete.

46 portable groups pass Debug/Release, three changed geometry suites pass
ASan/UBSan and compile for Windows x86/x64, and the pinned body/propagation layout
check passes normally and with Python optimization. See
ROOMSCALE_BODY_SWEEP_GEOMETRY.md. The separately added FP math frame preserves
native caller state and is likewise inactive; see ROOMSCALE_MATH_FRAME.md. No
runtime verification is claimed, and this does not complete the full mod.

## Local controller integration — 2026-10-06

The source now connects a development-gated single-player roomscale controller
at the checked post-simulation boundary. It supplies camera/brain/actor, native
scratch, presentation and reentry checks; complete body sweeps; the original
checked setter; actual-anchor origin correction; and rig-revision publication.
The fixed readability cache is bounded to that admitted read-only extent and
retired before native commit callbacks. The packaged Roomscale setting remains
0, and multiplayer movement remains excluded until body/origin pairing is done.

51 portable groups pass Debug/Release. Game x86 and host x64 cross-builds pass;
controller, query/swimming, placement/resource/math, native-finally and artifact
checks pass. Metadata/readability helpers pass sanitizers and Windows compile
checks. A corrupted FP cleanup is rejected. No game or headset process was run.
See ROOMSCALE_LOCAL_CONTROLLER.md for the exact connected scope and limits.

## Handheld head-volume and frame clearance — 2026-10-06

Current source adds the default-off post-simulation handheld head-volume path
and response-owned IPC9 clearance receipts. The host restricts cached clear
images to the checked head/eye space and query lifetime; rejected/unknown native
attempts produce opaque world output. Requested frames are not consumed before
a matching native query opportunity. Mounted head protection remains excluded.

All54 portable groups pass Debug/Release, both game DLLs and the host build, and
changed helper sanitizers plus Windows compile-only checks pass. Compiled/native
query/owner, strict-contact, cleanup, swimming, multiplayer RPC and fixed-layout
checks pass. Native gameplay and Windows/Proton headset behavior remain untested.
See HEAD_VOLUME_INTEGRATION.md. This does not finish the multiplayer body/origin
correction problem, physical melee, remaining vehicle/head work or final review.

## Seated world head-query binding — 2026-10-07

Head queries now bind the actual player, mechanism and pose source independently
of foot-body collision geometry. Positively owned seated riders use the same
strict world-volume query and IPC9 receipt. Native player-category and self/parent
mechanism filters are preserved; the ridden vehicle can be excluded, so its own
interior is not claimed protected. Missing native view resources reject the
anchor instead of using the getter's global fallback. Body movement retains its
previous hull categories and single-player gate.

All55 Debug/Release groups, x86 game/server and x64 host builds pass. The binding
fixture passes ASan/UBSan and x86/x64 compile-only checks. Pinned binary and
compiled query checks pass normally and with Python optimization. No runtime or
deployment occurred. HeadComfort remains default-off; multiplayer roomscale,
physical melee, remaining head/vehicle work and final review remain unfinished.
See HEAD_VOLUME_INTEGRATION.md for the exact scope and limits.
