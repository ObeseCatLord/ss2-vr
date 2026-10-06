# Roomscale model-query source acceptance

Independent Astra/xhigh read-only review of the six bounded adapter/fixture files.
Initial SOURCE NO-GO found one winding defect; the main-agent correction received
bounded SOURCE GO. No native execution, hook activation or movement occurred.

The adapter preserves native traversal and TOI, using one explicit lexical
max-parameter/depth-budget/failure scope. Initial-contact support bounds can
ignore a triangle only within the stated extra-depth budget; deepening blocks at
zero and uncertainty fails the whole query. Outside a scope the original six-slot
cdecl/x87 kernel forwards unchanged. Positive facing swaps B/C and negates the
normal; no extra world, collider, body controller or full TOI kernel is added.

| Review finding | Final disposition |
| --- | --- |
| Adapter dot order differs from native `(Y+Z)+X`; a tiny positive X survives only native order and takes early miss | Fixed: exact binary32 products, native-order addition interval certificate, uncertain sign fails |
| Exact Y/Z cancellation can retain tiny positive/negative X | Explicit exact-product cancellation path; both signs/forwarding regressed |
| Merely changing parentheses does not certify x87 rounding | Bound both admitted53/64-bit precision additions; exact products need at most48 significand bits and exponents fit binary64 |
| Initial-contact support bound is budgeted, not exact nondeepening | Documentation states distance(t) ≥ initialDistance−budget and contact uncertainty band |
| Pure math does not establish query/body ownership | No installation/activation; public-query lifecycle, primitives and body/MP remain gated |

The first blocker used triangle(0,0,0),(4,−4,0),(4,0,−4), normal components
0.577350269f, origin(1,−2,1), direction(2^-80,1,−1), radius1. Native Y/Z-first
cancellation leaves positive X while the old adapter's X/Y-first order lost it.
Initial edge distance sqrt1.5 exceeds1; distance at t.5 is below1. The fixed
certificate either establishes safe orientation or fails explicitly. Opposite
products cancel exactly at both supported precisions; reversing all products
preserves sign symmetry under admitted round-to-nearest arithmetic.

Verification:446 portable checks in optimized and UBSan main-agent builds;
Astra additionally ran x87 `-mfpmath=387 -frounding-math` with precision settings
0x200/0x300. The pinned Core export/fingerprint, six cdecl slots, x87 return,
compiled original trampoline calls, B/C swap, normal negation, aligned entry and
native-finally linkage pass the compile-only verifier. See
roomscale-model-source-checks.json. The native kernel itself was not executed.

Remaining limits: native TOI numerical conservatism is unproved. The callback
extent must contain only the admitted query; shared scratch/cleanup ownership,
worker exclusion, minDistance0, actual positive radius and compatible units are
caller obligations. Primitive initial contact is not repaired. Actual body
geometry/coverage, checked placement and authoritative multiplayer settlement
are not implemented by this vertical. Source acceptance is not full immersive
equivalence or a certified movement fraction.
