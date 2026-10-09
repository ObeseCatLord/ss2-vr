# Desktop and VR participants with the mod installed

Mixed desktop/VR multiplayer is required. Every participant and the server use
matching mod products/wire versions; a desktop participant does not need an HMD.
Native desktop rendering, mouse/keyboard gameplay and inventory remain native.
User-operated multiplayer acceptance is still pending.

## Transport capability and gameplay ownership

A mod Hello or a fresh neutral heartbeat establishes transport capability, not
XR gameplay ownership. Previously nonce-only ownership could reserve neutral
native primary bits and admit headset-free players to VR equipment policy.
The existing ordered peer now records one additional ownership fact only after
successful validation/admission of a pose with a valid tracked head.

Known-avatar primary reservation, authoritative freeze/use and server presentation
all use that fact with a bound nonce. Malformed, replayed, foreign-capability,
rate-limited and occupied-slot packets cannot establish it. Established XR
ownership survives tracking loss; its unavailable actions remain managed-neutral
until existing release/rearm rules permit them. Exact same-brain/avatar capability
renewal preserves ownership while clearing frozen/transport state; native avatar
replacement and disconnect clear it.

Desktop neutral packets still freeze and retire their exact ACK credit. Relay
recipient admission uses transport freshness and native binding, so a desktop
client can receive a VR participant's pose without claiming XR gameplay itself.
No wire layout, native ClientAction, fire state machine or projectile mechanics
have been replaced.

Portable checks use the production ordered admission and ownership predicates,
credit retirement, native primary projection, renewal/replacement and existing
relay codec. All desktop low-byte primary/history combinations preserve an upper
sentinel; established XR loss suppresses only the two native fire bits. Native
Peer renewal, RPC delivery, authority returns and wrapper gates are source-reviewed,
not executed by these tests. Astra/xhigh approved the bounded patch; local routing
tags verified, effective backend unattested.

## Remaining acceptance and presentation work

The existing mono remote-weapon path remains connected. The mono remote-head
source now freezes the same presentation bank before one outer native world
draw, using existing stock resource/lifecycle/thread and palette ownership rules.
Bounded source and finite verifier review received Astra/xhigh GO. An exact saturating32-bit token
owns the bank, with lock-free CAS retirement and constant-initialized TLS.
Stereo stores its token in the existing UI frame and retires it on normal,
failed-publication and reset cleanup. Mono retires only the token it acquired.
Foreign or obsolete cleanup cannot clear another owner's bank.

Nested world renders are routed to the original once before stereo admission;
they invalidate the inherited bank and suppress both adapters before binding
reads. A nested abort faults its parent frame without tearing it down. Reset of
a suspended draw suppresses both adapters until the outer finally restores the
prior scope only at the outermost exit; nested returns cannot undo a reset's
suppression. Reset outside an active draw does not latch that suppression.
The stereo desktop rebuild remains native-headed within this bounded slice.
Explicit `RemoteHeadTracking=0` preserves the option's disabled behavior.

Portable owner tests cover abandon/re-admit, foreign/stale cleanup, exhaustion,
nested abort/continuation and mono reset/continuation through production helper
decisions. The compiled checks inspect scalar callback-free32-bit CAS helpers,
saved TLS retirement, actual thiscall ECX capture/original forwarding and exact
finally callbacks. They are finite checks, not a native getter-lifetime or
appearance proof. IPC-unavailable transport maintenance is not proven by this
ownership fix. Runtime network delivery and actual native appearance remain
separate user checks. All products rebuilt with fingerprint
63eebe666f9deb0c2c2dbe07111af634c4ad25e897f91c4da37acf9ab251db41,
IPC10/wire7.81Debug/81Release groups pass. Actual native unwind and network
appearance remain unverified; no gameplay was launched for these changes.

| Astra recommendation | Disposition |
| --- | --- |
| Separate transport capability from XR control | Adopted in the existing ordered peer |
| Keep neutral consumption and desktop relay admission | Adopted; native transport unchanged |
| Reuse the frozen bank before mono production | Adopted; no post-producer head-anchor capture |
| Retire abandoned stereo banks by exact owner | Adopted in the existing frame release/reset |
| Use a bank token rather than an XR request for mono | Adapted to saturating32-bit lock-free ownership |
| Reject nesting before stereo admission | Adopted; original draw remains once-only |
| Preserve parent owner on nested abort | Adopted; only outer cleanup tears down UI ownership |
| Suppress suspended draws after reset | Adopted with restoration in the outer finally |
| Bind compiled callbacks and reject closure corruption | Adopted in finite callback/receiver checks |

Non-scoped native shooting dispatch and nearest-hit selection are established
for the pinned routes in [laser coverage](LASER_NATIVE_COVERAGE.md); visual
calibration and grasp alignment remain separate user acceptance.

The [user procedure](USER_VR_TEST.md) includes desktop listen host + VR client,
VR listen host + desktop client, both clients on a matching dedicated server,
native desktop controls, coherent VR head/hands/weapons and native desktop
presentation, reconnect, death/respawn and level changes. Non-scoped laser aiming
is required independently of scope zoom and has separate native-coverage checks.
