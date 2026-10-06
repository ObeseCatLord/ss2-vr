# Native sphere-query driver (inactive)

The driver resolves the existing Engine ray/filter functions and applies the
complete prepared body cover one sphere at a time. It preserves native world
traversal, hull filtering and ordinary cleanup. Every sphere must complete with
no hit and no adapter/owner/resource failure; a partial successful cover cannot
approve movement. The component is now called by the development-gated single-player controller;
the default remains disabled. See ROOMSCALE_LOCAL_CONTROLLER.md.

Each sphere follows init, six-float ray, maximum parameter, zero minimum,
positive radius, both actual source-hull categories, avatar/mechanism exclusion,
fluids disabled, native CheckRay, copied hit result, and ordinary init cleanup.
The mathematical scope requires whole-path-clear certificates. It is nested in
the checked-placement resource scope and the isolated FP math frame. It refuses
missing or redirected triangle/primitive hooks; resource scope entry similarly
checks the installed targets of all 26 gates, rather than treating queued hooks
as enabled. Normal cleanup never clears a failed scope's status.

The caller must supply a current-owner predicate that validates the exact
simulation phase, world/body lifetime, worker and query scratch exclusion and
the known cleanup-list shape. That predicate is rechecked before and after init,
after native traversal and before success. Native unwinding remains native:
there is no attempted visited-array repair or cleanup call from an abnormal
unwind. The existing query observer's quarantine and final owner invalidation
must be connected by the controller.

## New owner/cleanup evidence

The pinned game caller invokes CSimulation::Step at Sam2Game258D8 and resumes at
258DE before the subsequent send-enabled network step. Engine's full simulation
step calls physics at1B7406; its suffix only clears simulation+4C and returns.
The physics dispatcher waits for each worker before clearing pool+10. These are
candidate phase constraints, not a global lock or a proof about arbitrary
callbacks/third-party code.

rayInit's known collision and model cleanup callbacks (28B50 and D8880) are
scalar-only leaves. Core CListNode::Remove F270 only unlinks the next/previous
pointers and zeros its own links. A portable copied-list validator recognizes
only those two object links, expected vtables/callback targets, consistent
bidirectional links, the native sentinel and genuinely detached unused nodes.
It rejects unknown/cyclic/shared lists. Readability and a copied list do not
confer ownership, and no code restores or manually unlinks native list nodes.

The owned native import inventory also contains InSamnity2Game's simulation
caller; that different module is not admitted by the supported Sam2Game binding.
No game module was executed.

## Verification

All47 portable groups pass Debug/Release; cleanup-list checks pass ASan/UBSan
with workspace leak checking disabled. Both x86 products build. The compiled
sphere-driver verifier checks all 13 native pointer calls in order, actual
six-float ray capture, sphere radius, maximum/minimum, category and owner filter
arguments, cdecl cleanup and repeated owner/resource checks. Normal/-O checks
pass; replacing the radius argument with its adjacent coordinate is rejected.
The native owner/cleanup verifier, resource-gate verifier, triangle/primitive
ABI checks, native-finally checks and artifact/export/IPC checks pass.

The single-player controller now supplies bounded phase/scratch admission,
checked-setter connection and origin settlement. Default enablement and
authoritative multiplayer pairing remain open.
No Windows, Wine, game, headset, OpenXR or network session was run.
