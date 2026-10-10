# Physical vehicle grip integration — next source operation

The sampled submitted-geometry adapter is implemented for positively identified
Fighter/Saucer hover handles. Native mode2 heading remains the control seam;
original clamp, throttle, aim/fire, physics and ClientAction transport remain.
The full requirement retains literal driver wheel grabbing where actual controls
are established, wider vehicle coverage and multiplayer acceptance. These hover
handles do not establish a literal wheel or an articulated control pivot.

Default Vehicles.ImmersiveGrips=1 enables the narrowly admitted source path.
Release squeeze before grabbing. A new available Low then Down acquires within
8cm of an actual submitted handle triangle. One-hand grip yaw or two-hand span
yaw provides relative heading; joining/leaving a hand rebases to returned native
look heading. Unavailable/invalid head, grip or reach, rig/model/brain changes,
entry rejection or world interruption cancel admission. Cached neutral cannot
rearm. Joystick fallback and native actions remain available.

Astra source reviews returned ADAPT; all identified source/gate must-fixes were
closed. The final bounded caller-wiring/relocation review returned GO for those
fixes. Main accepts this source slice with the documented evidence limits. Local
Astra/xhigh tags were verified per review; backend attestation unavailable.

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

IPC ABI is now11, wire7 unchanged. Shared appends two grip-pose continuity
fields and a40-byte world-submission receipt: total83887888 bytes. Input272
and Request440 payloads are unchanged. Geometry stays in bounded local value
copies; no new transport or texture-sharing service.

The host publishes receipt only after successful xrEndFrame with the actual
world projection in final layers. Original source age is retained on cached
resubmission. Geometry handoff remains inside successful commit, before owner
retirement; native samplers keep Drawing and copied publication uses Retained.

Reviewed fixes and dispositions:

| Finding | Adopted correction |
| --- | --- |
| Callback reentry could bypass an earlier admission test | Exact borrow/observation latches after validation and conversion; heading restored only on rejected live normal return. |
| Invalid head/reach samples could disappear through IPC coalescing | Shared producer/consumer pose eligibility and durable per-hand generation. |
| Local entry rejection or brain replacement could inherit grasp | Compact interruption and submitted/current brain equality. |
| Enlarged publisher allocated before owner gate | No-inline copied-value helper after retained admission. |
| Cleanup verifier did not prove caller wiring | Exact saved TLS/capture/context/run/finish construction, argument and relocation checks. |
| Unknown/additional relocations evaded verification | Generic recognition, unsupported-kind rejection and exact inventories including empty lists. |

Native heading units/sign/angular wrap are established statically; see
NATIVE_HOVER_HEADING.md. Native wake/change bookkeeping is not fully periodic.
The adapter retains that native behavior. Source tests cover seam crossings,
multiple turns and transfers, with no claim of native execution.

All four products match source fingerprint
46e1a609659bc0c5002ce9cbe2ff14dfa83335f55192670f2ef4352e7c254520,
IPC11/wire7.82Debug/82assert-enabledRelease groups pass. Normal/-O compiled
checks cover both control wrappers, actual byte/COFF corruptions, legacy model
joins, remote unwind/false-owner consumers and artifact layout/contracts.
No runtime, installation or staging occurred for this source checkpoint.

The implemented offline vertical checks reject Ready-but-never-submitted data, preserve
original age on cached submission, handle animated surface replacement and
vehicle motion, retain skipped grip-loss interruption and preserve smooth
one↔two-hand transfer through exactly one native clamp. Offline fixtures alone
cannot prove actual headset/vehicle/multiplayer behavior. The user primarily
performs those gameplay tests.

Actual contact precision, draw/material availability,100ms usability, native wake
effects, comfort, broader controls and vehicle/multiplayer runtime acceptance
remain open. Offline fixtures alone do not establish those results.
