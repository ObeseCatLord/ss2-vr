# Live scope UV — Astra review disposition

Astra explicitly selected and locally verified gpt-6-astra/xhigh. The design
review gave conditional GO for the fifth copied slice and reconstruction math,
then bounded SOURCE GO for the frozen implementation. Both effective review
turns were locally verified Astra/xhigh. Main owns integration; only Linux
offline work runs. No blocking source defect was found.

| Recommendation | Disposition |
|---|---|
| Validate narrowed float reconstruction, not only double fit/coefficient bounds | Adopted: independent local-fit and final float error budgets of1e-5; native float UV varying is checked against centered direct-affine coordinates. Finite offset4096 counterexample rejects. |
| Bound both native transform and raw covariance conditioning | Adopted: determinant versus Frobenius norm squared / covariance trace squared, plus finite bounded coefficients. Rank-deficient and poorly conditioned maps reject. |
| UV unconditional, weights independently optional | Adopted: stream roles0/5/3 precede optional6 in both binding snapshots; exact FLOAT2/TEXCOORD3, identity/offset/stride/frequency and declaration required. |
| Preserve position-owned VB and existing Lock ledger | Adopted: UV uses the same sequential copySlice(false) owner; no extra uncertain reference or cleanup/recovery policy. All locks/releases precede hashing. |
| Mandatory UV acceptable but optional UV does not inherently require an FSM | Adopted: corrected brief. Broader diagnostic coverage is not a demonstrated requirement; unsupported UV retires the existing invocation only, ordinary draw still forwards. |
| Remove fixture-driven hasUv/optional parsing | Adopted: all five slice sizes required; raw UV uses exactly the position's remapped source index; valid covers the complete payload. |
| No derived mapping cache/generation/bank | Adopted: coefficients remain caller-derived; existing bank and rejection/reset ownership retained. |
| Require actual native program/purpose/c8/c9 before using image coordinates | Adopted: raw-input provenance only. No default identity masquerades as live shader observation; image shader remains disabled. |

Portable checks cover required UV metadata and conflicting declarations, omitted
or UNUSED stream6 with valid UV, fifth sequential acquisition failure/unwind in the portable ledger,
retirement before/after valid observations with no revival, malformed and
non-affine maps, large world translation, reflection/shear and perspective
triangle interiors. Fingerprint-bound owned-mesh verification traces every
remapped UV/position pair and all22 actual triangles, using four unequal-W
interior samples each. No owned asset is retained in the repository.

These checks do not invoke Windows COM calls or native exceptions. The lock-ledger
checks exercise offline policy; actual foreign-call cleanup remains source/static
ABI evidence. Neither copied UV nor accepted reconstruction proves current
native COLOR purpose, source-image transfer or ballistic reticle alignment.
contentVerified stays false. Native image enablement remains out of scope for
this bounded review; the full immersive goal remains active.

Source acceptance retains three nonblocking limits: vertex float evaluation and
sampled triangle interiors do not guarantee every pixel/native GPU precision;
ledger tests do not execute COM-reference transfer or foreign Unlock cleanup;
and identity UV defaults are explicit mathematical fixtures, never evidence of
successful live constant capture. Future image admission must test accepted
transforms near the numerical rejection boundary and capture actual current
program/purpose/c8/c9 coherently.

Final main verification passed: x86 proxy/server, x64 host/official loader and
all19 portable groups; five-slice owned checker (30,320 copied bytes), artifact/
IPC layout, actual DIP forwarding ABI and native-finally compiled shape. Hook
counts remain47 exported/11 internal, IPC8/wire6. No runtime was executed. Exact
source/product evidence: scope-live-uv-source-checks.json. Installed files and
the immutable0.2.11 archive remain unchanged.
