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

The existing mono remote-weapon path remains connected. Remote head palettes
currently require the admitted frozen stereo extent; mono remote-head coverage
is under review. IPC-unavailable transport maintenance is not proven by this
ownership fix. Runtime network delivery and actual native appearance must be
checked separately from portable packets and counters.

The [user procedure](USER_VR_TEST.md) includes desktop listen host + VR client,
VR listen host + desktop client, both clients on a matching dedicated server,
native desktop controls, coherent VR head/hands/weapons and native desktop
presentation, reconnect, death/respawn and level changes. Non-scoped laser aiming
is required independently of scope zoom and has separate native-coverage checks.
