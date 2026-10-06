# Roomscale placement and replication boundaries under investigation

2026-10-06. Static inspection of the pinned owned Engine/Sam2Game modules only.
These notes include historical inactive-component checkpoints. Current
single-player integration is documented in ROOMSCALE_LOCAL_CONTROLLER.md;
multiplayer body/origin settlement remains unfinished.

## Exact native placement/shape sources

CMechanism::GetRootBody at133B60 checks part count+8 and returns the first part's
body pointer at part+8 through the part array at mechanism+4. This is different
from CMechanism::GetAbsPlacement at130B20: that getter resolves mechanism+38 and
copies its source's pose at+2C. InitializeMechanism at138C80 writes+38 from the
CModelRenderable argument (handle conversion138CFF, store138D05); CreateMeshMechanism
similarly stores its model argument at138DAE. Do not require the mechanism pose
source to equal its root physics body, or mistake the model handle for a body.

CAspect getters expose parent+4, first child+8, next sibling+C, owner entity+48
and absolute QuatVect+2C. CPrimitiveHull's native vtable is Engine209268 and its
16-byte descriptor starts at+78. These are layout facts, not a lifetime proof.
A live player-body reader still needs exact owner/mechanism/root identity,
complete actual collision geometry and an admitted native phase.

## Preserve the original movement-query filters

CPrimitiveHull::CheckMove52030 initializes the native ray at520AB, sets the ray
at520F2 and its maximum distance at52100. It copies the hull's owner entity+48 to
cldSetAvatar, mechanism pointer+70 to cldSetAvatarMechanism, and actual category
IDENT+54 to cldSetRayCategory before cldCheckRay52123. It later clears the avatar
and mechanism exclusions. Those copied native values are the filter reference;
laser-category constants or a guessed physical-collider mask are not substitutes.

Its small-move skip and zero-radius centre ray still do not implement a full
capsule sweep. The new cover/query geometry must supply that missing portion.
Native checked mechanism placement, immediate accepted displacement, world-space
transform rounding and normal fresh-ray cleanup remain to be connected.

## A real native entity-update completion marker exists

Engine CEntity::GetLastUpdateSequence58F60 reads entity+18; SetLastUpdateSequence
124840 writes it. Their native vtable slots are88/8C. CClientInterface::UpdateEntity
F2FB0 reads the incoming message sequence at message+8 and ignores old/equal
updates atF30CA/F30CD. It maps the server target at message+14 through the native
client mapper before deserializing the update.

After native property application, F32B3 calls the entity's PostReceiveUpdate
slot98. Only afterward, F32CA invokes SetLastUpdateSequence with message+8.
Thus a completed actor-update marker is available; merely receiving the mod's
input ACK or reading netGetServerTime is not that marker. The latter reads the
client clock estimate at1ED0 and does not certify one actor's property update.

CNMUpdateEntity::SetData108A10 stores the target handle at+14 and copies the native
update buffer with its length at+18 and owned bytes at+1C. This inspected path
limits the encoded payload to255 bytes. The native serializer and entity mapping
must remain authoritative; no alternate property schema has been introduced.

The remaining replication question is how to order one accepted roomscale
origin adjustment with the corresponding native body update, including loss,
reordering, replacement and local prediction. A mod ACK alone must not rebase
the headset before or after an unrelated native position change. No direct
client body correction, packet queue, serializer change or new wire fields have
been added without closing this association.

## Inactive bounded body reader

`roomscale_body_geometry.hpp` now copies and cross-checks a deliberately narrow
candidate: exactly one hybrid root body and one primitive sphere/capsule child,
with matching entity, mechanism and model identities. It refuses compound,
carried, mounted, rotated-capsule and unknown-class cases. Model placement is
read separately from hull placement; the root body is not confused with the
mechanism's model. Quaternions must already be unit length within a bounded
float tolerance; the reader does not normalize or mutate native data.

Addresses are comparison tokens for an already-owned native extent. The caller
must still establish a no-callback, no-concurrent-mutation phase and supply a
generation-checked resolver and exact copy function. Re-resolving handles is
only an additional consistency check, not a substitute for lifetime ownership.
This header has no game caller and enables no movement. Actual avatar admission,
world-space cover placement and native checked-placement integration remain open.

The new portable fixture tests every read failing independently, identity
replacement, unsupported graph/shape cases, nonfinite dimensions, invalid
rotation, address overflow and pose-change comparison. All41 local Debug and
Release groups pass, as do the fixture's ASan/UBSan and x86/x64 Windows compile-only
checks. These are synthetic-memory checks, not a native body capture.

## Native actor updates and mod ACKs can be selected independently

Pinned CNMUpdateEntity::IsReliable is Engine111F80 (false), while the reliable RPC
used for the mod ACK returns true at1110D0. Native receive102090 tries the next
reliable packet throughED070 only when its reliable sequence matches at1020CC.
If no message was obtained, it scans packets throughED000 at1020F4. That helper
selects a message whose virtual IsReliable slot18 returns false, without checking
the next reliable sequence. Thus submission ordering alone cannot prove that a
mod reliable ACK is observed before the corresponding native actor update.

ShouldDelayMessageEF870 only reaches its sequence-related missing-target check
when the mapped entity does not already exist. It is not a general ordering
barrier for an existing player's body. The native completed-update marker at+18
still identifies actor application, but does not by itself solve the two-arrival
visual-origin transaction. No native queues, reliability flags, entity snapshots
or mod wire fields have been changed. The ordinary native serializer and message
application remain authoritative.

`tools/verify_roomscale_replication_boundary.py` fingerprints the owned Engine,
checks exported reliability functions and virtual slot, receive-selection sites,
and PostReceiveUpdate-before-SetLastUpdateSequence ordering. It passes normally
and under Python optimization. This evidence rules out an unsafe ordering
assumption; it does not implement roomscale replication or test a network session.

The pinned layout verifier now also checks the real player binding sequence:
Sam2Game83CF1 stores the mechanism handle,83CF9 resolves the player's model at+120
for native CreateMechanism83D10, and83D32/83D3B obtains and stores the root-body
handle separately. It verifies the exported hybrid/primitive tables, graph/pose
getters, mechanism part-array root lookup, model pose source, and the original
movement-query owner/mechanism/category loads. Run
`tools/verify_roomscale_body_layout.py --game <private-game-root>`; normal and
optimized Python runs pass. This confirms binary layout roles, not the actual
loaded player model's shape or safe live body ownership.

## Owned player assets: complete native shape capture

The privately transferred player model directly references Player.mch. The
fingerprinted mechanism asset establishes three relevant hybrid-root profiles:
Default has one capsule (template width1,height2.2, centreY1.1); Crouch has one
capsule (width1,height1.5, centreY0.75); Swimming has two capsules, including one
rotated90 degrees around X. Each of these profiles has zero child mechanism
parts. These are serialized template measurements, not hardcoded live dimensions.
`verify_player_collision_asset.py` checks the exact asset, name/type framing,
profile/body references and complete shape/child arrays; it passes normally and
with Python optimization. No proprietary asset bytes are redistributed.

The inactive body reader now captures the complete leaf-hull chain, bounded to
these two-hull configurations. It retains each hull's own pose, shape and query
category; rejects cycles, excess hulls, descendants and identity mismatches;
and compares both shapes during revalidation. Every read failure is tested for
both one- and two-hull cases. Rotated-hull capture is only metadata: the current
upright cover cannot be used on that rotated hull. A native query must cover all
captured shapes under their real transforms or reject the entire movement.
General rotated-cover integration, native query ownership and body settlement
remain open. No body movement or new hooks are enabled.

Verification for this capture change: all41 local Debug/Release groups pass,
as do reader ASan/UBSan and x86/x64 Windows compile-only fixtures. These checks
do not replace native gameplay or headset validation.

## Packet identity and error-completion follow-up

The single-player controller is now connected behind the default-off Roomscale
setting; see ROOMSCALE_LOCAL_CONTROLLER.md. Earlier inactive-reader/query status
above records the investigation sequence, not the current single-player wiring.
Multiplayer roomscale remains excluded.

Further pinned inspection closes two important negative assumptions:

- CNMUpdateEntity serializes a one-byte property-payload length and at most255
  bytes. Pack108600 writes the target and length before copying those bytes;
  Unpack108820 reads the same framing. GetMaxSize1085F0 returns payload+6.
  Appending arbitrary mod data to this property buffer is not a free extension.
- The incoming actor sequence is inherited from its containing native packet:
  packet unpackED388..ED391 copies packet+0C/+10 to message+8/+0C. SetData does
  not itself assign the eventual received sequence. Any sideband pairing must
  identify the actual submitted packet and exact actor payload, not merely the
  time when the native property serializer ran.
- LastUpdateSequence is not a sufficient successful-property-application receipt.
  UpdateEntity's pinned MSVC C++ exception descriptor has a CException handler
  atF3274 for its protected copy interval. Its normal continuation isF3296,
  which rejoins before PostReceiveUpdateF32B3 and the sequence writeF32CA.
  Thus the sequence can advance after a reported copy exception. Native update
  completion must be checked with its actual outcome and owned body state;
  neither a mod ACK nor the sequence field alone certifies a paired correction.

The expanded replication-boundary verifier checks the framing, packet-derived
sequence and actual EH descriptor/type/continuation. No new packet kind,
serializer, queue, client body mutation or origin correction is enabled here.

## Existing client RPC lifetime correction

During integration work, the existing native client RPC adapter had two uncovered
unwind extents: its TLS context was restored only on normal return, and a peer
metadata lock surrounded native mapper/getter calls without native-finally
retirement. A native exception crossing either extent could leave a dangling
stack context or a permanently held lock. Direct GNU decoding/allocation errors
also needed containment inside the reentered mod callback.

Both reliable and unreliable client dispatch now use the existing native-finally
boundary to restore their prior TLS context. The reentered ExecuteRPC adapter
contains its own GNU errors, and its peer metadata work uses the established
withPeerLock helper. Aborted paths mark native input unhealthy; they do not send
packets, retry gameplay or fabricate ACKs. Normal native mapping and unknown-RPC
forwarding remain unchanged. This fixes an existing integration boundary; it
is not multiplayer roomscale implementation or native exception runtime proof.

Verification: all51 existing portable groups pass in Debug and Release, both
x86 products build, and native-finally/artifact checks pass. The new compiled
client-dispatch ABI check passes for game/server objects and with Python
optimization. A modified-object fixture removing one actual TLS-restoration
store is rejected. The expanded native replication check also passes normally
and with Python optimization. These checks do not execute native SEH or a
network session.
