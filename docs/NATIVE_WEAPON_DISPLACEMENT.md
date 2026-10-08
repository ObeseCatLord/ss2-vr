# Native weapon displacement and muzzle cleanup

This incremental adapter preserves the existing native placement calculations,
tracked transforms, shaders, stretch/reflection, weapon FSM and network transport.
It adds no asset parser, grip/scale guess, animation engine or damage logic. Primary
shooting/vehicle/hardware tests belong to the user. This change has not been run in
game. Source/build verification is separate from native gameplay acceptance.

## Native reference and implemented boundary

Astra/xhigh decoded pinned Sam2Game4BFF0. Native model placement composes
(B·V)·D: V is GetWeaponInViewMatrix4B520, D is the exported base
GetWeaponChargeDisplaceMatrix4B9E0, and B contains the supplied view/native bob.
D returns to4C058, uses identity rotation and native charge/recoil translation.
Its resource accesses can clone/rebind parameters; evaluating it again would
duplicate native work. The adapter therefore captures the existing call only.

The new typed getter forwards once, captures only the exact weapon/4C058 caller
inside an owned original placement invocation, and requires finite translation
with identity3x3. Native-finally restores stack-local observation TLS on normal
return or foreign unwind; duplicate/nested/invalid observed data is rejected by
the tracked adapter. Unmanaged callers retain the original native result/output.
A different virtual getter that never reaches the audited base method retains
the previous zero-residual policy. That fallback does not certify its displacement.

For native relative model rotationR and D translationd, the existing tracked model
uses hand.p + hand.R·(R·d). The flat-reference call captures its own matching
residual and pose in the existing per-hand calibration. Failed/nonfinite/invalid
flat calibration declines tracked placement, preventing a mismatched model/muzzle
reference. Native instance scale/mirroring and ID13 scope geometry remain native.

Native shooting4A806/4A81E obtains in-view/attachment placement without D. The
shared muzzle conversion therefore subtracts D from its cached model root to
obtain the neutral root, bounds the attachment offset, then adds the displacement
outside that bound. Local shots, collision lasers and authoritative placement
use the same conversion. No authored physical grasp-frame calibration follows.

The shared muzzle path also now holds its existing recursion guard through
context acquisition, original getter and retarget callbacks. Native-finally
restores the saved outer depth, revokes the calibration witness on abort and
marks the active input interval failed. Previously an original native exception
could strand depth, while later retarget callbacks ran after it had been cleared.
Nested getters remain native-only, and native exceptions retain native propagation.

## Verification and limits

All x86 client/server and x64 host/official-loader products rebuilt with source
fingerprintff8a2d050e3f81ba18c6f1c4d6df4b7e2955edbf0c903e2206ef8bb71c70d628.
All61 Debug groups pass, including a nonzero residual larger than the attachment
bound and body-motion invariance. Compiled artifacts, matching IPC10/wire6/layouts
and the existing native-finally ABI gate pass. tools/verify_muzzle_unwind.py checks
both native muzzle ret4 entries, charge getter forwarding/output/caller/ret4
provenance, and scalar saved-TLS cleanup in both x86 objects, normally and with-O.
Five modified COFF controls reject a missing original call, one incorrect ret8,
missing placement TLS restore, missing muzzle depth restore and abort calibration
certification. These are compiled/offline checks, not native callback execution.

Astra approved the integrated source after correcting unmanaged forwarding,
flat-reference failure, stale cache admission and rejection/invalidation ordering.
Main rebuilt and passed the required compiled gates for the reviewed engine SHA256
`d6f1af07fc42f152e1965298f70b9b9c4716c8f9b156ad325da6f86b3dd456bb`
and math SHA256
`31054a66321b7b69ad50aa896dd65052f5d73f5bce34454b33c193d38deea57c`.
This closes the bounded source landing, not native runtime/hardware acceptance. Local current-turn Astra/xhigh tags
are verified; backend routing introspection remains unavailable. Actual native
charge/cadence/per-hand firing/lasers and broader lifecycle/MP behavior belong in
the user's comprehensive in-VR test after remaining implementation is ready.

## Final source disposition

| Review recommendation | Disposition |
| --- | --- |
| Capture existing D invocation; do not re-evaluate resource-capable getter | Adopted |
| Native translation-only method and hidden-result/ret4 ABI | Adopted and compiled client/server checked |
| Model and muzzle must account for the same residual exactly once | Adopted, with residual outside attachment bound |
| Unmanaged native result/output must remain unchanged | Fixed; capture invalidation is separate from original result |
| Failed flat reference must not publish unmatched staged model | Fixed; known failure retires matching cache evidence |
| One freshness predicate for cache use and reporting | Fixed; one timestamp and100ms limit; existing fallback on expiry |
| Nonfinite output rejection must invalidate before pose/stage checks | Fixed after managed ownership admission |
| Native unwind must restore saved outer muzzle/placement context | Adopted; compiled scalar cleanup and five negative controls pass |
| Physical grasp/size calibration from Palm alone | Rejected; separate authored/runtime association gate remains |

Matching-cache invalidation is confined to current player/generation/hand/weapon
and changes only its validity flag under the existing snapshot lock. It does not
clear another hand or replacement generation, or modify native consumption history.
