# Native weapon projection reference: current evidence

The V21 neutral capture established independently qualified geometry for actual
repeated ID1 passes. It recovered 36 weapon copies, including nine NoUV56 passes,
with independent model, local palette, view and adjusted projection operands.
Both the existing collapsed reference and uninterrupted double calculation fail
unchanged limits. No reference, tolerance or rendering change is authorized by
that disagreement alone.

Astra traced two exact native material paths against owned binary fingerprints.
The native CPU produces an intermediate adjusted-projection × view matrix, stores
its components as floats, then multiplies that matrix by the selected model and
stores the result as floats. The shader applies the separately uploaded local
palette to the vertex before the resulting model-view-projection matrix. Thus
collapsing model × local palette before view/projection is mathematically equivalent
in real arithmetic but not the native finite-precision operation sequence.

The inspected producers use x87 operations, explicit float stores and differing
addition orders. They do not set their own precision or rounding modes. Native
view-projection and model-view-projection caches can bypass a material's cold
producer. Current model changes invalidate model-dependent caches while retaining
view-projection. Applying projection adjustment again would duplicate an operation:
the captured projection is already adjusted.

A private fixed-instruction interpreter uses independent factors, with captured
constants only as comparison outputs. Under an assumed 24-bit nearest-even mode,
it reproduces 287 of 288 inspected output words across eighteen copies; one differs
by one float ULP. Main reproduced the first diagnostic and independently checked
native hashes, arithmetic-block boundaries, float-store instructions and absence
of precision-setting instructions. Actual precision/rounding and cache-producer
provenance remain unobserved. The other eighteen programs were outside this finite
trace. This is diagnostic evidence, not a production reference GO.

## Required next bounded observation

Observe actual precision/rounding at the native producer, the producer versus cache
hit, independent current operands, and the existing draw identity. A cache hit needs
provenance to the earlier producer; recomputing the current material's cold path
is insufficient. Cached/uploaded output must not become the independent reference.
The two reviewed whole-function observation hooks are now implemented; see
IDLE_EVENT_COLLECTOR.md for the exact source/build and verification limits. No
mid-block patch, forced cache recalculation or floating-state change is adopted.

Once proven, prefer a narrow reference adapter matching the actual operation order,
precision and stores while keeping the local palette separate. Preserve admission,
owner/currentness checks and tolerance. Relevant checks must cover cancellation,
wrong precision/cache provenance, independent operand mutations, both material
layouts and prior V20 regressions. No renderer rewrite or new transport is needed.

## Passive later-material observation

V21's later rejected material declaration contains additional FLOAT2 streams4/5.
The exact eight-row pattern now selects only the existing passive 0/7/8 sampler
from the immutable original failure. Extra inputs remain unknown; production
grammar and replay still reject this declaration. No buffer-copy operation, native
API, owner or success policy changed.

| Astra recommendation | Disposition |
| --- | --- |
| Preserve the native operation sequence and ambient precision as evidence inputs | Adopted; no reference replacement before producer/cache observation |
| Avoid choosing whichever grouping passes | Adopted; both original and independent failures reported |
| Do not double-adjust projection | Adopted; adjusted operand retained |
| Keep later declaration support diagnostic only | Adopted exact passive selector; extra inputs remain unknown |
| Preserve original selection under resampling | Adopted; mutated declarations cannot reselect streams or inherit equality |

Astra/xhigh source GO covers the four passive implementation/test files. Local
routing metadata was independently verified; backend attestation is unavailable.
All four products rebuilt on fingerprint
`a64ddfb9c254c1693d1b5ed8d4472553609cc03a864a994b5cab4908459c4743`, IPC10/wire7.
Debug passed all68 groups; Release passed67 initially and the remaining predicate
check passed with the existing dependency environment. All28 normal/optimized
native/compiled gates and16 passive reader groups in both modes pass. These checks
do not establish native projection-reference agreement, hand alignment, grasp,
first/copy melee release or hardware/multiplayer acceptance. Raw programs, asset
geometry, disassembly and captures remain private.
