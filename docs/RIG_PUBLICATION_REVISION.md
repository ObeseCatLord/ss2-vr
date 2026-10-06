# Internal body/origin publication revision

The native snapshot now carries an internal rig revision, separate from tracking
and network intent generations. An odd revision represents an incomplete
body/origin transition; only the current even revision is usable. Old snapshots
remain stale after a completed transition, and a stale completion ticket cannot
finish a newer transition. Counter exhaustion refuses another transition.

VR-session admission, frozen stereo commit, in-pair weapon presentation, laser
sample admission, weapon calibration and world-marker admission check the rig
revision. Snapshot publication refuses a stale revision. Existing snapshot resets
preserve the current revision. A genuinely new tracking origin or explicit
recenter can retire an incomplete revision while retaining the existing ordinary
epoch-reset rules. The controller must hold the existing snapshot lock when
publishing its origin and completing the revision.

This is presentation metadata, not an engine lock. It does not replace native
worker/lifetime ownership or make concurrent native body writes safe. The
existing stereo commit's native calls are now counted inside native-finally
cleanup, so the future body owner can reject reentry while that snapshot read
lease is held. A read-only bridge admission checks the entire pending stereo/UI
extent, including gaps between eyes, scope previews and deferred retirement;
checking only the current eye index would be insufficient.

The development-gated single-player controller now owns transition begin/finish.
The default remains disabled; ordinary operation retains a stable revision. See
ROOMSCALE_LOCAL_CONTROLLER.md for the connected path and remaining limits. This
is not a finished or runtime-verified roomscale release.

At the initial metadata checkpoint,49 portable groups passed Debug/Release, the production revision helper passes
ASan/UBSan with workspace leak checking disabled, and both x86 products build.
The query observer and swimming ABI checks still pass after the private snapshot
layout change. No IPC/wire layout or manual input-history generation changed.
No native Windows, Wine, game, headset or network session was executed.
