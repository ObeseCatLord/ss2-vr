# Physical vehicle grip integration — next source operation

This is an Astra/xhigh-reviewed design disposition, not an implemented grip
feature. The completed-bank phase fix and handle geometry/program proof are
implemented separately. The full requirement retains one-/two-hand driver wheel
grabbing where actual controls are established, wider vehicle coverage and
multiplayer. The first positive geometry is Fighter/Saucer hovercraft handles;
it does not establish a literal wheel or an articulated control pivot.

## Reference and ownership

Use the last successfully submitted native world pair's actual handle geometry
as a sampled interaction surface. Native Ready is insufficient: the host can
discard or expire a pair before submitting its projection layer. The existing
host successful-frame branch already knows the submitted world request. It must
return a small versioned receipt through the existing IPC, without transferring
geometry or adding another transport. Receipt evidence is compositor submission,
not physical headset scanout or runtime alignment acceptance.

Copy the geometry candidate within commitPair's success extent before its finally
invalidates bank ownership. Keep bounded local pending/active value copies matched
to producer, session, reference, tracking generation and request sequence. Missing
matches decline. A durable world-continuity epoch must preserve omitted-world
interruptions through latest-value coalescing. Cached resubmission must not renew
the geometry's original age.

Store verified world vertices relative to the captured native rig anchor. Compare
current grips using the existing bodyHandTracking mapping in that same space.
Applying the current anchor to both operands gives equivalent world contact and
accounts for vehicle world motion without assuming a fixed authored Seat/Main
offset. Require coherent rig keys, justified paired-eye agreement, unambiguous
topology and a100ms maximum source age. Newly submitted animated geometry replaces
the surface while a held controller baseline persists.

These are sampled visual-reference semantics. No exact current-input native
Main/Seat query is established or required for this contract. GPU precision,
perceptual alignment and the practical age budget remain runtime acceptance.

## Input and continuity

At the original borrowed ride look-clamp callback, re-establish current
operator-brain reciprocity, renderable handle/type/owner/instance and configuration/
file/resource/stretch, with complete bookends including mode/abilities/parameter.
Reuse established reads while keeping render and input entry predicates distinct.
Historical numeric keys authorize comparisons only.

Add producer and rig revision to the existing ControlSample. Copy geometry/control
values and release locks before native getters. Commit handoff performs value
work only. Keep original native clamp, physics and ClientAction transport.

Grip-pose loss can disappear between game reads: current squeeze admission checks
aim pose but not grip pose, and latest input overwrites old samples. Add per-hand
grip-pose continuity using the existing ActionStream pattern, preserving source
availability loss through coalescing. Keep shared squeeze admission; do not
duplicate its policy. Changed pose epoch or geometry/mount/rig interruption cancels
grasp and requires a causally newer eligible Low then Down. Cached neutral cannot
rearm; a remaining valid hand may continue after rebasing.

Acquire against actual handle triangle surfaces. One-hand raw-grip yaw and
two-hand horizontal-span yaw are candidate gestures, with singular projections
rejected and hand/handle transitions rebased to the previous target quaternion.
Convert the target with the existing quaternionEuler function; do not guess native
angle units. For the positively verified mode2 route, replace only heading before
the original native clamp. Preserve pitch/bank handling, movement, fire and
joystick fallback. Native heading is coupled to native look, so this does not
promise independent yaw aiming or mode3 wheel semantics.

Production collection needs separate enablement and one-attempt-per-eye accounting
in the existing bank/GPU owner. Preserve diagnostic lifetime caps. Continue
candidate counting after selection so later ambiguity is detectable.

## Astra disposition and verification

| Recommendation | Main disposition |
| --- | --- |
| Ready is not successful world submission | Adopted: receipt at the existing successful world-layer submission branch. |
| Anchor-relative submitted geometry | Adopted: actual copied mesh and current rig mapping, no authored offset/native scratch query. |
| Borrowed callback identity and ControlSample rig/producer | Adopted: fresh reciprocal input owner, value-copy locks and complete bookends. |
| Durable grip-pose and omitted-world interruptions | Adopted: existing action-stream/IPC patterns; no parallel squeeze policy. |
| Native mode-specific actuation and bounded production collection | Adopted for verified hover handles; wider controls/literal wheels remain required. |

The existing Shared record can carry new feedback/provenance fields, allowing
Input/Request payload shapes to remain unchanged if that layout is chosen. This
is a proposed minimal placement, not an implemented ABI. Bump IPC ABI and rebuild
all four products for any layout change; do not reuse reserved bytes or alter wire
transport without evidence. Geometry stays local; no texture-sharing rewrite.

The first vertical checks must reject Ready-but-never-submitted data, preserve
original age on cached submission, handle animated surface replacement and
vehicle motion, retain skipped grip-loss interruption and preserve smooth
one↔two-hand transfer through exactly one native clamp. Offline fixtures alone
cannot prove actual headset/vehicle/multiplayer behavior. The user primarily
performs those gameplay tests.

Current local Astra/xhigh routing was verified for this review; independent
backend attestation is unavailable. The verdict was ADAPT, not runtime GO.
