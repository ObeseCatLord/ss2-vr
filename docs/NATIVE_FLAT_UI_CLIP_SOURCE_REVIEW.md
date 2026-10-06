# Flat native UI clip adapter: Astra decision and bounded source GO

Reviewed baseline780a37d plus NATIVE_FLAT_UI_CLIP_AMENDMENT.md and the changed
geometry/helper files. Explicit gpt-6-astra/xhigh; main independently verified
effective settings for all four reviewer turns. Boyle the2nd was read-only,
reviewed actual source, and is closed. Main owns architecture and writes.

This is a geometry component for the required complete UI adapter. It is not
yet connected to native GPU draws or pair completion and does not implement
complete overlays by itself. No game, host, Wine, XR or network was executed.

## Final architecture critique, normalized record

Astra rejected4mm physical thickness. Main's independent sequential-float32
fixture reported24 sign mismatches at sourceZ offsets at least.001, maximum
error.00261875.10mm still misclassified a source value near+.0001 as-.000841;
inflating thickness merely to pass a fixture was rejected. These are modeled
float arithmetic findings, not observed GPU results.

Astra found a smaller constant-only adapter. Keep the flat panel's eye clip
X/Y/W, but assign synthetic eyeZ=k*sourceZ with positivek. Invertible panel
XYW homography plus this row retains source information for transported six
clip halfspaces, without any physical thickness. SourceZ no longer determines
perspective-depth cancellation. Main adopted k=.5*minimum true panel cornerW;
the entire physical panel must first pass actual eye near/far and positiveW
corner admission. Natural side clipping remains. Source fog/depth-dependent
shading must be absent. Six user planes are a queried capability, not assumed.

Keep original program objects/streams/texture/color/alpha/nativeRGB blend. Delete
scratch stencil/mask and shader replacement from this chosen route. Preserve
immediate draw/once-owner transaction requirements. SyntheticZ requires the
intentional VR legibility policy: added UI draws disable destination depth
tests/writes; original desktop depth remains original. Actual target restoration
must precede viewport **and scissor** restoration. Pure-device/state readability
and device serialization remain explicit gates. Transported geometric scissor
is not a claim of identical desktop pixel-edge rasterization.

## Source findings and disposition

| Astra finding/recommendation | Main disposition |
|---|---|
| Reject physical-volume cancellation | Adopt. Retained the exact original fixture as a C++ regression for the selected helper; no rejected thickness implementation in production. |
| Synthetic outputZ retains flatXYW | Adopt. Production helper composes actual sourceMVP with flat physical-viewport/canvas mapping and transported planes. No replacement shader/resource/queue. |
| Nonfinite check misses destructive narrowing | Adopt. Zero-or-normal finite source/eye coefficient domain; every H/plane/combinedMVP narrowing rejects nonfinite/subnormal/nonzero-double-to-zero output, preserving exact zeros. Result remains local until publication. |
| denorm_min·I loses sourceZ | Adopt. Rejection regression checks unchanged output; additional normal-input computed-subnormal and computed-zero cases cover later narrowing. |
| Existing checks used trivialZ/W | Adopt. Mixed sourceZ=.00025x-.0005y+.5z+.2 andW=.0005x+.00025y+1; classification uses actual rounded original transform, layout/scissor divides by actualW, unequalW edges retain clip parameter. |
| State/capability/owner gates | Adopt for integration. No assumption of six planes, pure-device getters, atomic multi-thread transaction, absent stencil/user planes or actual executed projection. |
| Scope of interpolation checks | Adopt. Edge intersection UV/color interpolation is checked; full rasterized-triangle appearance/equivalence is unverified. |

## Final terminal source verdict

Faithful normalized record: **SOURCE GO for the changed helper and tests. Both
prior findings closed; no blocking findings in the bounded follow-up.** Astra
checked the numeric guard at every narrowing, early source/eye subnormal
rejection, local-until-publication failure semantics, all three underflow
regressions, mixed sourceZ/W rounded reference, perspective viewport oracle,
normalized flat placement and unequalW crossing edges. True eye-depth corner
admission and six transported source halfspaces remain; the header correctly
states absent source fog/depth-dependent shading and D3D Z0..W convention.
No further correction was required before main proceeds to integration.

Reviewer executed an earlier990-witness offline check, then inspected final
changes without additional execution. Main independently ran final **1658
near/far witnesses**, **all14 offline groups** (Python found), and both Windows
x86/x64 compile-only checks with-O2/-Wall/-Wextra/-Werror. Final source/object
hashes and check scope are native-ui-clip-source-checks.json.

The supported coefficient admission closes the demonstrated underflow. It does
not prove sign preservation for arbitrary finite matrices and unbounded vertex
magnitudes. No native/GPU/headset rasterization, stock coverage, concurrency,
appearance or performance was verified. No package/version/IPC/wire change.

## Main spot-check and remaining integration

Main checked inverse/cofactor indexing, positive plane normalization, actual
float combined-MVP checks, independent rounded-source oracle and unchanged
failure output. Also corrected separate native evidence: both vertex/pixel
record device objects are at record0, not vertex4. UseSimpleShader actually
reads adjusted projection2E6438; compiler translation of source to actual
device position semantics remains an admission gate.

Next connect exact native enclosing brain/overlay/fade owner and both immediate
device draw families to this helper; copy actual executed adjusted eyeP before
root retirement, and reuse frozen host comfort geometry. Extend existing pair
through the once-only overlay, retaining pre-UI world dimming/native fade order
and guarded oneReady publication. Readback/cached-pair capability must suppress
the generated stats only for a complete admitted native UI pair. No additional
image channel or comfort scheduler. Full immersive goal remains active.
