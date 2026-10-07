# Multiplayer server RPC unwind containment

The server receive adapter held the MP metadata SRW lock across native avatar
lookup and used a growing vector while that lock was held. A native MS exception
could bypass the GNU caller cleanup; a direct allocation failure could also
escape the reentered mod callback. This left a real lock-retirement gap even
though the client RPC entries already had explicit native-finally extents.

The reliable and unreliable server entries now enter the same supported native
finally wrapper before decoding or forwarding. Non-mod traffic still reaches
its original receiver exactly once with the original server/RPC arguments and
reliability. A tagged mod decode/adapter failure is not forwarded as native chat.
GNU errors are contained locally; native SEH still propagates unchanged.

Server receive mutation uses the existing explicit peer-lock extent. Its cleanup
releases ownership on normal and abnormal exit and marks an active simulation
interval failed when one exists. This is not a new persistent session-fault flag.
The two possible discarded tokens use a fixed two-element array rather than a
heap owner crossing native calls. Normal transport, credits, native gameplay and
wire6 are unchanged. No new native hook or guessed ABI is introduced.

The compiled multiplayer boundary verifier covers both server entries, the
captured reliability flag, server/RPC pointers, original thiscall forwarding and
stack cleanup, and both peer-lock retirement paths. A private mod-owned COFF
fixture with both unlock calls replaced by NOPs is rejected. A second fixture
that changes reliable reception into unreliable reception is rejected under
Python optimization as well. These are static
boundary checks, not execution of native exceptions, locks or networking.

This bounded fix does not establish arbitrary native getter reentrancy, native
send-object cleanup on foreign exceptions, physical melee observer consumption,
or multiplayer roomscale movement. No game, Wine, Windows executable, headset,
network session or deployment was run.
