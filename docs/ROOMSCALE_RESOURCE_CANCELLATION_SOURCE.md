# Optional native query cancellation: source boundary

2026-10-06, root-owned implementation; static owned-binary inspection and
compile-only checks. No hook is enabled by engine attach, no query is initiated
and no body movement is implemented by this component.

## Why this boundary

The native query uses global traversal/model scratch. Resource replacement can
pump a global task queue. A ready getter is not a lock, and returning a fabricated
replacement pointer or suppressing the global queue would alter native policy.
The bounded alternative is to decline this extra, explicitly owned query while
preserving the normal native cleanup suffixes. Existing stock queries outside
the lexical scope keep the original instructions and callbacks.

The earlier six-site model-preparer inventory was incomplete. Its call at E1CC4
enters mdlGetSurfaceMaterial (E6910), which can replace either the inherited or
direct shader resource at E693C/E695B. These two hidden dispatches now have
explicit gates, and cancellation returns to an immediate E1CC9 propagation gate
before its result is consumed. The getter's normal zero-argument cdecl return
preserves the caller's pending argument; the preparer bailout retires it with
the rest of the outgoing scratch stack.

## Implemented control-flow adapter

Ten actual resource flag branches cover world, outer model configuration, six
model-preparer resources and the two material branches. Five post-call gates
propagate failure through the material return, both recursive preparer returns,
outer preparation and model query tail. The adapter owns only lexical failure
state, never a queue, resource pointer registry, collision world or solver.

Each resource gate is before the original TEST/JZ pair. In a scoped query it
reads the same byte flag once: clear selects the original clear-branch target;
set marks the whole query unavailable and takes a checked cancellation suffix.
There is no readiness-preflight/retest gap. Outside the scope its trampoline
executes the original pair, leaving the original replacement CALL in place.
Cleared-branch continuations overwrite flags before any conditional consumer,
or return through an ordinary flags-volatile cdecl boundary.

- E1720 restores ESP to its saved-register extent (EBP−58), then uses E18A1's
  native minus-one epilogue. Both recursive return gates prevent ignored child
  results from continuing the preparation. No C++ object destructor is skipped
  in this native frame; this is a normal control-flow cancellation, not SEH.
- E2472 retires five pending arguments and enters E263E, retaining native
  profiler/SEH cleanup while skipping DBC90 on a partial model.
- DA4E1 retires three arguments, sets the normal false return in ESI, invokes
  the native scalar DAD90 scratch reset and enters DA4F9's mode/SEH restoration.
  DDE30 and DA170 never consume the cancelled model.
- DA419 cancellation takes DA476 before query-specific mode changes. World
  cancellation takes29160, including normal visited-hull/container cleanup.
- Every assembler gate saves/restores all general registers, flags and FX state;
  its aligned C++ decision callback is a call-free native-TLS leaf. It neither
  allocates nor calls native code. Trampolines/abort destinations are installed
  transactionally by the existing caller-owned hook registrar. A relocated
  absolute operand is checked against the actual Engine load address.

The fifteen entry prefixes and normal suffixes are pinned and checked before
queueing. The full Engine fingerprint remains the caller's responsibility. All
queued gates must be successfully enabled before a scope is allowed; rollback
and quiescent removal remain the registrar's responsibility.

## Evidence and remaining integration

verify_roomscale_resource_abi.py checks the owned Engine fingerprint, all ten
TEST/JZ branches, all five propagation prefixes, native cleanup contracts,
callback-free scratch reset and the emitted GNU x86 gates/leaf/TLS/stack shapes.
Portable tests exercise the same sticky-failure decision helper. They do not
execute native instructions or emulate the game. The component is built into
both x86 products but is not queued by engine attach.

This closes a bounded source/control-flow slice only. Complete native descendant
coverage, query-purpose provenance, worker exclusion and orderly query cleanup
must be established by the eventual owner. Allocation failures still propagate;
DAD90 is not rollback of a failed native allocation. Native TOI conservatism,
primitive initial contact, body geometry/coverage, checked placement and
multiplayer settlement remain independent gates. Earlier hits never authorize
movement after the lexical unavailable flag is set.

## Physical model-hull collision route follow-up

The public hull dispatcher at2F9D1 calls vtable+40. CModelHull's51430 entry
forwards at51568 to mdlModelCollisionCheckRay CBF90, whose stack-owned84-byte
query object enters CB3B0. This is a distinct route from the rendered-model
DA280/E1720 preparation path; the earlier fifteen gates were not complete
coverage of public model-hull collision.

Added eight exact resource flag gates at CB407, CB438, CB46C, CB49B, CB4C8,
CB4F4, CBA64 and CBEF3. The scoped clear path uses each original clear branch;
a pending resource cancels the whole optional query. The normal failure suffix
CBF5F restores its outer profiler/SEH state and returns false with ret8. The
cancellation entry restores ESP to EBP−C0, the native saved-register extent.
Six gates precede collision-data preparation, one precedes the optional vertex
buffer lock, and the last is after the matching CBB29 unlock and inner profiler
cleanup. No cancellation entry jumps out of a held vertex-buffer lock.

There are now18 resource branches and five propagation gates. The verifier checks
all23 bindings, the additional native frame/ret8 cleanup and all emitted entry
save/restore paths. Both x86 products and all40 local Debug/Release groups pass.
The gates remain inactive. Normal graphics buffer-provider behavior, allocation
failure propagation, other hull families, actual owner/worker exclusion and
query setup/settlement still require the enclosing transaction; this expansion
must not be read as a completed roomscale implementation.

## Primitive-hull material replacement

Continued inspection found another actual callback after the primitive math:
CPrimitiveHull::CheckRay525B0 reads its material smart pointer at+88, tests
resource+4 bit0 at528E4 and calls virtual0C at528EE. Mathematical-kernel coverage
does not exclude this resource replacement. The 24th gate uses saved ESI, skips
an already-clear branch to52905, and cancels to the original false-return suffix
52918. Its ordinary saved EDI/ESI/EBX/EBP cleanup and plain RET remain native.
The hit distance/normal may already have changed before this gate; the shared
sticky failure invalidates the entire optional query, including earlier hits.
It does not roll back scalar state or claim an interrupted traversal is repaired.
Normal traversal return and subsequent native fresh initialization are still
required of the eventual caller. Total:19 resource branches,5 propagation sites.

Other concrete hull inspection found CDummyHull's4EBC0 calls the thin primitive
intersection regardless of query radius. This is a further shape-admission
boundary for an optional thick query, not evidence the dummy collider can be
silently ignored. CFluidHull4FA20 begins with the native fluids-enabled getter
28C00 and returns false when disabled. CForceFieldHull's111F80 is an immediate
false return. The abstract base slot targets CRT purecall. These facts do not
prove the complete dynamic hull/provider set or authorize activating the query.

The pinned native-prefix/cleanup and compiled gate verifier passes normally and
with Python optimization enabled; both x86 game/server products cross-build.
No native runtime was executed and the gates remain inactive.

## Fail-closed hull dispatch admission

An additional inactive gate now checks the actual native class table at the
world traversal's virtual CheckRay call2F9D1. Only the exact pinned primitive,
model, fluid and force-field tables and their unchanged CheckRay slots are
admitted. A dummy, unknown, derived or changed table cancels the entire optional
query before calling that target. Unscoped stock traversal remains unchanged.
An unknown table is never dereferenced by the admission helper.

The native traversal has already registered the visited hull and set its flag
at2F9C1/2F9CA. Cancellation continues at2F9DE, the native no-hit continuation,
so ordinary traversal still retires the visited set; it does not publish this
hull as nearest. The sticky failure makes any previously changed hit fields
unusable. This is rejection of an unsupported optional query, not permission
to pass through an unsupported collider. The eventual movement owner must
refuse to move on failure and perform the normal native ray cleanup.

There are now25 inactive gates:19 resource branches,5 propagation points and
one hull-dispatch admission. Both x86 products and all41 local Debug/Release
groups pass; pinned native and emitted-register/FP/TLS gate checks pass normally
and with Python optimization. This bounds the virtual hull target set, but does
not close all downstream callbacks, native query ownership or placement.
