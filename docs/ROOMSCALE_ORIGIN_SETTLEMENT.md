# Translation-only tracking-origin settlement (inactive)

`settleTranslatedAnchor` computes the stage-space origin correction for an
actual accepted native anchor translation. The before/after anchors, head pose,
old origin and turn are copied inputs; it neither reads native objects nor
publishes a snapshot. Requested displacement is not evidence of accepted
movement. An invalid result after a native commit does not make that commit
retryable.

For unchanged anchor orientation, the accepted world displacement is transformed
back through the anchor and turn, then into the origin's stage axes. The origin
orientation stays unchanged. The helper checks the resulting head position
against the pre-move world position within the caller's explicit error budget.
Nonfinite values, changed anchor rotation, excess accepted distance, unsupported
arithmetic mode and clamped old/new physical head offsets are refused. It does
not silently absorb a native rotation or erase excess physical displacement as
a recenter.

48 portable groups pass Debug/Release. The production helper passes ASan/UBSan
with workspace leak checking disabled and compiles for Windows x86/x64 without
execution. Tests cover 3,000 varied origin/anchor/turn configurations, both eyes
and hands, orientation preservation, rejected clamp/rotation/nonfinite/excess
movement, and 10,000 accepted translations around a closed physical path.
These are numerical tests, not live controller/transport ownership evidence.

## Required controller publication

The upcoming controller must make native body movement and origin publication
one owned main-thread transition. No stale captured rig may be used during or
after that transition. It must not clear manual command history or resample
input, and it must distinguish a clean refusal before the first write from an
uncertain native mutation. Native unwind cleanup may only invalidate metadata;
it must not invoke gameplay getters, undo native movement, or retry a possibly
consumed displacement.

A successful settlement must update the current origin without overwriting
unrelated snapshot changes. An uncertain commit or failed post-read must retire
that movement admission and hide invalid presentation until a supported recovery
boundary. Continuous body translation must not be implemented as a fresh
recenter/intent reset on every step. Existing frame and weapon lifetime rules
remain in force. Multiplayer body/origin pairing needs its own authoritative
sequence and cannot be inferred from reliable ACK order.

This helper is not connected to engine.cpp. No body controller has been enabled,
and no game, Wine, OpenXR, headset or multiplayer process was run.
