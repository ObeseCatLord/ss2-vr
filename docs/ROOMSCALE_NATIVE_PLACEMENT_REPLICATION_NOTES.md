# Roomscale placement and replication boundaries under investigation

2026-10-06. Static inspection of the pinned owned Engine/Sam2Game modules only.
No native body reader, placement transaction or network settlement is enabled by
these notes. The implemented query components are still inactive.

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
