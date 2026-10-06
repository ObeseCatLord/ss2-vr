# Live cap UV input and mapping extension

Baseline a6f78e6: actual native DIP observer with four managed source copies,
hashes and current full affine; derived optical frame in the existing observation.
No native image is enabled. Extend this reader with the actual bound stock UV
slice and the inverse map needed by the reviewed image shader. This is a narrow
adapter extension, not a new graphics acquisition/lifecycle manager.

Verified owned UV0 descriptor84/buffer0/offset181824, stride8,884 coordinates,
name3 and SHA256199e21cfca88d708d605db3ee54f35bba76191bf698c725baf4a39034480b2fd.
tools/audit_sniper_geometry.py checks complete stock mesh first, then actual
cap UV planar correspondence. Native installed Poly Bump emits UV3 via
dp4(v3,c8/c9) and COLOR0 alpha after fade/translucent fog. That source evidence
does not establish every current native VS/material/purpose.

Minimal design: keep the original DIP/sequential READONLY lock ledger and TLS
owner. Capture bound stream3's exact FLOAT2/TEXCOORD3 declaration, zero element
offset, same canonical VB as positions, stream offset181824/stride8/frequency1.
Add one7072-byte copy and fifth hash after all unlocks/releases. No extra native
hook, COM-resource cache, borrowed pointer or writer synchronization policy.
Original draw still forwards once; successful draw plus existing fresh raster
identity precedes observation publication.

The live supported observation now requires UV admission together with geometry,
as one complete current-draw payload. Missing/unsupported
UV retires the existing diagnostic geometry for this invocation and forwards the
ordinary native draw. This only narrows a diagnostic observation; original game
rendering/gameplay remain unchanged. Optional UV could preserve broader diagnostic coverage without a new state
machine, by replacing/clearing each current payload. That broader coverage is
not a demonstrated requirement here. Mandatory UV keeps the existing consistency
rule and complete payload without a separate presence bit. The parser requires
all five exact sizes; portable fixtures supply synthetic UV.

Actual cap-local UVs are copied in the same remapped vertex order as cap positions
and indices. Existing bank/reset/rejection owns them. ContentVerified stays
false: this is raw bound input provenance, not a guarantee about post-VS UV,
live material, skeleton, color purpose or valid source image.

For shader coefficients, fit the two-dimensional affine raw UV -> local cap
position using centered double-precision covariance, reject singular/non-affine
or nonfinite data, and measure A*(p-center) in the rigid optic basis to avoid
large-world subtraction. Compose the inverse of the actual native c8/c9 UV
matrix (FLOAT2 input expands z0/w1) only after future native shader/purpose
admission. Coefficients supplied to the HLSL are finite bounded UV -> optic-local
XYZ. Both covariance and native transforms require bounded conditioning; local
fit and narrowed float reconstruction have independent 1e-5 error budgets. Render image identity remains under the existing frame owner.

Verification: production metadata/range helper counterexamples for wrong stream,
identity/type/stride/frequency/UV range; same-index extraction; malformed/singular
UV transformations; reflection/shear/translation invariance; raster interpolation
against actual cap mapping and explicit c8/c9 transforms; actual owned five-slice
checker, cross-build and existing compiled ABI checks. Only Linux offline code
executes. No Windows/game/Wine/XR/server/network session.

Architecture/source review must challenge unnecessary state and any weakening
of prior native lock containment. No broad transport/gameplay/UI review. Main
owns all edits. A separate Terra read-only worker investigates the universal
first-gun command/model color boundary, not this UV region and not reviews.
