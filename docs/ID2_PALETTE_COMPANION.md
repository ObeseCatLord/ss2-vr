# Auto Shotgun draw-local palette diagnostic

This opt-in diagnostic copies the selected native ID2 draw's world matrix,
one to three palette mappings, each referenced global canonical bone matrix,
and the actual D3D9 shader, constants, declaration and stream bindings. It is
separate from count-one affine geometry admission. It does not enable native
multi-palette replay or weapon alignment.

The existing submission owner reserves its ordinal before the first AddRef.
At most ten API payload attempts are retained independently of the geometry
copy counter; failed attempts consume capacity. The native palette stays in
the existing bounded submission row. Existing COM owner slots retain the
pre-sample shader identity through the post-sample and cleanup. No additional
allocator, native cache replacement or protocol is introduced.

The required joined bracket is native copy, API copy, native copy, original
draw, native copy, API copy, native copy. Every copied native bookend itself
requires stable draw/root/configuration/cache identities. Every mapping must
belong to that draw and rendered model, reference a supported canonical bone,
and fit the copied native spans. Raw world/canonical/palette finiteness is
checked without inventing a single affine for a multi-palette draw. Canonical
versus palette disagreement is retained as diagnostic evidence. Repeated
owner/generation/routing guards perform no foreign capability queries. Device
admission queries precede the sampled interval. When legacy geometry was
already certified, the companion serializes its copied program/constants and
retained canonical bindings without releasing or querying them again; the
post-original API snapshot must match, or that geometry observation is retired.

The `Lab idle paletteSummary`, `paletteRow` and `paletteData` schema1 records
join by request, eye, hand, submission index and ordinal. Detailed records
require normal successful original DIP return, matching native/API bookends,
post-Release generation/routing validity and normal complete outer weapon
cleanup. They are emitted after that cleanup from copied values only. A
rejected count-one geometry observation can still supply this diagnostic;
the rejection and geometry limits remain unchanged. Summary records alone
are not qualified copies.

No shader register base, local-index upload association, independent camera
reference, authored grip, rendered visibility or physical release is certified
by this receipt. The next offline consumer must establish those relationships
from the actual program and independent native/reference evidence. Unknown or
fractional shader addressing remains rejected. Do not derive an independent
reference from the GPU upload being checked or relax bounds to obtain a pass.

The existing selected `SS2VR_LAB_IDLE_WEAPON=2` exception is diagnostic only.
Source instrumentation does not authorize fixture grant/equip or game launches.
Current user capture scope, isolation and immutable-fixture rules still apply.
Raw constants, addresses, bytecode, captures and asset identities remain private.

Portable checks cover native bounds, crossed canonical/palette rows, every outer
completion gate, payload capacity, row/ordinal mismatch, missing/interrupted
API copies and Release-time retirement. Source-order checks state their bounded
lexical limits; compiled checks inspect the contained wrappers and eight-vector
scratch cleanup. These do not prove native execution or visual alignment.

## Bounded Astra review and verification

Current-turn local routing was verified as gpt-6-astra/xhigh; the effective
backend was not independently attested. The read-only review returned source
GO after these corrections:

| Finding | Disposition |
| --- | --- |
| Capability callbacks outside the sampled interval | Repeated guards are scalar; capability queries occur before the bracket. |
| Added queries disturb certified count-one geometry | Serialize certified pre-state while retaining original references; post-state mismatch retires geometry. |
| Outer completion precedes final cleanup | Companion emission occurs after normal original/physical-pass/finally completion. |
| Shared reader accidentally widens affine admission | Single-affine policy remains count-one and invertible; copied palette policy is separate. |

All four source-matched products rebuilt with fingerprint
d836a37c958f4e16c6efa8ca3e010adf8c1b3951afbe075c4e064353fbbdf8c1,
IPC10/wire7.81Debug and81assert-enabledRelease groups passed. Fifty existing
normal/-O game/server native/compiled/artifact gates and eight joined-submission
and palette-scratch checks passed against the final objects. No game, headset,
firing, movement, vehicle or network test ran. This is diagnostic source
readiness; actual ID2 association and all-weapon alignment remain open.
