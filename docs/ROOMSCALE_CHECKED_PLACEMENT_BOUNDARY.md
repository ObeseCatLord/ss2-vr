# Checked placement: pre-write decision boundary

This component is compiled but not queued, armed or called by engine attach.
Roomscale body movement remains unfinished. It provides a bounded native commit
boundary for the eventual geometry/ownership/origin-settlement owner, rather
than treating CMechanism::SetAbsPlacement's result as a swept-volume certificate.

## Native sequence and cancellation

The pinned CMechanism setter134010 takes a pose reference and flags, ret8. The
component submits flag1 and requires exactly one current part, its captured
root pointer and an immutable copy of the requested model pose. The original
native preflight runs. Before the first part setter call134D7B, a full
register/flags/FP gate rejects a failed request or unexpected part.

CMechanismPart::SetAbsPlacement131870 completes its native pose composition
before calling CAspect::SetAbsPlacement575D0 at131AB9. The proposed root pose
there is the actual native result, not a guessed copy of the model target.
The part setter has no non-stack writes and no other calls before this root
setter; its other branch targets the model fallback, which is not admitted.
The typed aspect wrapper calls the owner's collision check before the first
root write. The callback must copy any borrowed pose it retains and must have
separately established the complete native-query extent and body geometry.

If declined, the wrapper does not call the root setter. A second gate at134D80
then enters the native failure suffix135C62 before joint/model callbacks. The
native frame has364h bytes of locals and three saved registers. The cancellation
stub restores ESP to EBP-370h; this also retires the still-pending pose argument
at the pre-call gate. The native suffix pops registers, returns0 and ret8.
No pose write, fake success, partial-part rollback or repeated unchecked move
is used to represent rejection.

For approval, the component ends its resource filtering immediately before the
first root write. All original placement and subsequent native callbacks then
retain native policy. A native write being entered is recorded independently
of successful return. The outcome exposes `mayHaveMoved`; a fault or unexpected
outer failure after this point cannot be retried as an unconsumed request.
The caller still must validate actual pose readback and atomically settle the
VR origin, presentation and multiplayer state.

## Invocation and trampoline provenance

All request/receiver frames live above their native-finally calls. Exact
mechanism invocation, part receiver, root receiver and native return provenance
separate this extra request from ordinary or nested native setters. Outside the
owned pre-write boundary, setters retain their original calls and arguments.
No persistent entity registry or mirrored native physics state is introduced.

The pre-part CALL is relocated into a MinHook trampoline. The part wrapper's
return address is therefore trampoline+5, not Engine134D80. Queueing checks that
the relocated first instruction is the expected CALL to131870 and binds that
actual return address. The root-aspect call remains at native131AB9/return131ABE.
Both normal and cancellation paths preserve general registers, flags and FX
state. The gate decision is a call-free native-TLS leaf.

Queueing is not activation. Explicit arming checks all five installed JMP
bindings. Each request checks them again before entering a native setter;
removed/replaced guards cause refusal. The owner must fingerprint the module,
enable all resource/math/placement components transactionally, and quiesce and
remove every partial registration before resetting their pointers.

## Verification and unresolved work

The production progress helper has portable normal/rejection/duplicate/missing-
root/post-commit/interruption checks. All43 Debug and Release groups pass; its
ASan/UBSan fixture passes. Both x86 products build. The new ABI tool verifies
native exports, exact pre/post call sites, part write/call inventory, native
failure stack/ret8, compiled setters' stack cleanup, two full FX gate paths and
the call-free TLS decision. Normal and Python -O checks agree. A corrupted
cancellation stack offset is rejected. Resource-gate, native-finally and full
artifact checks also pass.

This does not supply the swept geometry for the actual proposed root pose,
worker/query ownership, local origin publication or multiplayer settlement.
These remain required before activation. No native executable, game, headset,
Wine or multiplayer session was run.

## Further native precheck evidence

CMechanismPart::CheckMove1315D0 normalizes the candidate quaternion before calling
root virtual+20. The admitted primitive-hull implementation52030 uses the
primitive's Core GetInnerRadius scaled by0.9 as a movement threshold. Smaller
translations return without issuing a ray. For larger translations it resets
ray state, uses a normalized centre ray with maximum length plus that radius
allowance, and copies the hit fraction/aspect. It never sets a positive ray
radius after rayInit resets it to zero. Thus this native mechanism precheck is
not a complete capsule sweep and cannot replace the additional whole-body cover.
The extra queries do not weaken or replace this native precheck.
