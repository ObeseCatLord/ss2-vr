# Stock ID1 virtual handle alignment

The tracked native ID1 assembly now places its handle centre at the calibrated
controller position in a declared reference idle pose. This position-only change
uses the existing placement/muzzle adapter. Native animation, recoil, scale,
reflection and weapon mechanics remain native; no asset parser, IK, renderer or
pose protocol is added.

## Reference and native sequence

Owned Zapgun.bmf has SHA256
`f50b9999379c66fbdc4c9128ad228a89e7f294b663afa47214e8b75fe6b7163c`.
Matched gun and hand channels share Model/Local factors. The authored hand holds
the rear inclined handle. Its22-face lateral shell has a closed16-segment section
perpendicular to the heel-to-shoulder axis at its midpoint. The section's area
centroid defines a virtual anchor without palm estimation or whole-mesh bounds.
Extracted geometry, plots and traces remain private.

V27 request31/eye0/right supplies the declared copied Local reference, an observed
single-contributor idle pose. The fixed pre-stretch model point is
`(-.00960000045598,-.00474976426398,-.536467618906)` in native model units.
Independent owner reconstruction reproduced the shell, centroid and transformed
point. Nine captured poses retain their native idle residual, at most.000792065
units. This is not a rest/bind frame or physical-contact certificate. Native
hand/handle interpenetration is preserved, not repaired.

For native relative rotation R, effective stretch S and reference c, root
correction is `-R(S*c)`, applied through the existing hand transform alongside
native charge. Local animation continues; no animated bone is inverted each frame.

Placement returns at Sam4C9BB. The upcoming branch reads weapon+BC. Nonzero draws
with existing stretch. Zero negates X, calls SetStretch, draws, then restores.
SetStretch floors every component magnitude to exactly1e-5 with sign preserved,
including signed zero. The adapter predicts that operation from finite current
base stretch; it does not call the setter or assume mirrored unit scale.

## Binding and muzzle

The copied binding includes owner/weapon/hand, selector, resolved model,
configuration/file/resource identity and bit-exact base stretch. Placement copies
it in the exact pre-reflection extent and rechecks after the flat getter and
remaining native calls, before model/calibration publication. Cached addresses
are identities only; consumption resolves/copies current values anew.

The existing render wrapper saves/restores activity depth through native-finally,
including nonphysical calls. Independent corrected muzzle reads require no active
weapon-render extent. Local and authority paths recheck binding after final actor
validation. Changed or ambiguous evidence declines corrected certification; no
guessed zero alignment/unit stretch is substituted. Native forwarding and exception
propagation remain intact.

Headless authority uses the existing simulation-thread identity/main-thread
predicate. Simulation entry records that identity before original work. The
configuration type is pinned unconditionally from Engine; reads require no server
renderer initialization. Actual headless FP-model availability during authority
reads remains a runtime acceptance row.

Let m be the existing neutral root-to-attachment offset, d native charge, and
a=R(S*c). Converted muzzle position is
`hand.p + hand.rotation*(bound(m-a,.5)+d)`. Correction precedes the grip reach bound;
bounding at the old distant model origin then shifting can put the laser behind
the handle. Charge remains outside the bound exactly once. Saturated corrected
reach is bounded, not certified coincident with the rendered attachment.

Shared gripOffset, native remote child placement, Scope13 and other weapon defaults
remain unchanged. ID1 evidence does not establish their grip references.

## Review disposition and verification

| Astra recommendation | Disposition |
| --- | --- |
| Explicit handle convention instead of Palm/bounds or surface-marker inference | Adopted; reproducible matched-assembly section. |
| Freeze model-space reference; retain animation | Adopted; no animated inverse. |
| Actual native selector/floor/sign behavior | Adopted; no setter invocation. |
| Binding rechecks across getters/publication/consumption | Adopted for model, local cache and authority. |
| Correction before reach bound; charge outside | Adopted; long-root model/muzzle regression. |
| Remove headless renderer prerequisite | Fixed; existing simulation owner and pinned configuration type. |
| Preserve remote children/controller calibration/unrelated optics | Adopted; IPC/wire unchanged. |

Astra/xhigh bounded source GO; all7local turn tags verified, independent backend
attestation unavailable. All four products rebuilt on fingerprint
`424b6917ea95b893f0ed3287ee7f72dd2c36d007ccd83abc17c331d886d22bac`,
IPC10/wire7.73Debug/73Release and36normal/optimized native/compiled gates pass.
Cases cover reflection/nonuniform scale, signed-zero/floor/nonfinite values,
stale bindings, render phase, reference anchoring, retained idle/charge/body motion,
model/muzzle agreement within the grip bound and saturated reach. Fresh x86
objects restore saved render depth before callbacks on normal/abort paths;
four separate in-memory instruction controls per object reject losing saved
value/store on either path. This is a bounded compiler check, not a general
lifetime proof or executed exception test.

No runtime, firing, movement, device, vehicle or network probe ran for this landing.
The user remains primary gameplay/hardware tester. Remaining weapon references,
physical driver controls and comprehensive acceptance are open.

## Subsequent neutral observation and laser review

Source3ada6e8 reached the pinned Jungle scene with127native stereo pairs and clean
owned-process/display shutdown; protected files remained unchanged. Fixed handle
reference error was at most8.99416e-6native units in10complete first-eye copies.
This is copied-factor arithmetic, not GPU precision, contact or second-eye pose
certification. The whole collector remained unqualified; no firing/movement test.

Before/after eye images show a laser regression: both aim lines were visible before
alignment and neither afterward, with lasers enabled. Astra/xhigh completed a
bounded source/native review; local turn tags verified, backend unattested. The
active rejection or visibility cause is unknown. No correction change is approved
by that evidence.

| Review recommendation | Disposition |
| --- | --- |
| Do not assume shooting poisons absolute-placement calibration | Adopted; verified shooting uses in-view matrix/attachment getters, not a proven absolute-placement call. |
| Preserve phase, age, binding, reach and ownership fences | Adopted; no speculative relaxation. |
| Distinguish cache/muzzle rejection from successful but hidden line dispatch | Adopted; next source work is capped passive records at existing attempts, with request/input/hand identities. |
| Preserve getter order/count and short-circuit behavior | Adopted; diagnostics must copy existing values, never add native queries. |

Remaining weapons and physical driver controls stay open. This runtime observation
supersedes the preceding source-landing-only runtime statement.
