# Public query lifecycle and native worker ownership

Astra/xhigh bounded read-only audit of pinned Engine/Sam2Game/Core; no native
execution or source edits. Keep automatic body-query activation gated. The
NativeSolver join is demonstrated, but a complete exclusive, fresh public-query
extent is not yet established. Missing evidence does not justify replacement.

Engine dispatch10EE90 starts workers at10EEB8, runs index0 on the caller at
10EEC7–10EEC8, waits for all workers10EED8–10EEE6, and clears active pool+10
before returning10EEF6. Core WaitOnThread62AA0 waits on worker completion;
628EA invokes OnExecute and628F1–628FC marks inactive/signals completion.
The narrow worker-joined slot isEngine10D935, after dispatch10D930 and before
contact callbacks10D954. Physics is called from simulation1B7406. Native
thrGetMainThreadID63B50 and thrIsThisMainThread63BA0 can verify main-thread
identity; neither establishes ray ownership. Placement mutex2ECEC8 is not
acquired by public ray operations. Do not add global engine serialization.

The public ray ABI copies six floats without normalizing at1B34B0. GetRay1B34F0
uses explicit hidden output/EAX,24bytes. Max setters/getters1B3530/1B3550,
min1B3580/1B3590, radius1B3560/1B3570 are cdecl; scalar results useST0.
SetMax also overwrites hit distance. Hit distance/type are1B35C0/1B35D0.
CheckRay29200/Continue29290 share unprotected global state. Public filter
setters provide no corresponding getters for avatar/mechanism/categories.

Scalar push/pop is insufficient. CheckRay links cleanup object2D9750 through
rayAddCleanup1B3490. rayInit1B35E0 invokes registered virtual cleanup+4 at
1B3601 and unlinks nodes before resetting. Collision cleanup28B50 resets
filters/hit metadata. Traversal sets hull+6C at2F9CA and records visited hulls
in2D9708; normal return29170–2917C clears flags/releases the container.
Collision cleanup does not perform that traversal cleanup. Do not copy cleanup
list pointers or claim TLS restoration repairs interrupted native traversal.

The path includes world smart-object dispatch2911E, hull CheckRay2F9D1 and model
smart-object preparation callbacks. No nested public query was demonstrated in
the inspected model traversal, but complete callback closure is not established.
Model traversal CBB05→CB180, recursively entering CAE80/CAAD0 and Core triangle
atCB08D, is a narrower mathematical candidate; profiling remains. The current
triangle ABI cannot distinguish an unrelated call inside its scoped extent.

Hit aspect28C10/material28C60 are borrowed raw pointers; normal28C20 copies
12bytes through hidden output/EAX, triangle index28C50 returns signed integer.
Copy required values while native extent/lifetime remains valid; no retention
across another query or world replacement is supplied by these getters.

Primitive CheckRay525B0 calls Core19A40 at5278E with four cdecl slots:
explicit Box1 output, Ray reference, borrowed PrimitiveDesc reference, radius;
EAX returns output pointer. Engine527C4–527EC checks only interval entry against
minimum minus radius/current hit. Target sphere radius2/query radius1 gives
[-3,+3] at an inside start; min0 rejects entry below−1. The model adapter does
not repair primitive overlap. This remains a separate narrow correctness gate.

Next: establish a fresh-query lifecycle at the joined main-thread boundary,
with native normal cleanup and actual callback ownership. Preserve native
physics/world traversal. Native float TOI accuracy, body coverage, checked
placement and authoritative multiplayer settlement remain unproved.

## Root continuation: native query extent observation

The source now observes same-thread extents of rayInit, cldCheckRay,
cldContinueRay and mdlModelCheckRay, using stack-owned frames above the existing
native-finally boundary. Native calls/arguments/results remain intact. Normal
return and native unwind restore the parent mod frame and laser-query flag;
unwind does not attempt to repair native traversal or cleanup lists.

This also fixes the existing extra-ray adapter: it no longer starts optional
laser/head probes inside another observed native query, and its querying flag
cannot remain stuck after a crossing unwind. A process-lifetime fault latch
quarantines further optional probes and their presentation after a query abort.
Stock ray initialization/checks still execute. This is observation, not a global
lock, worker exclusion, a private copy of native ray state or enabled roomscale.

The compiled observer ABI verifier checks four cdecl wrappers, the actual
three-argument model forwarding/result, retained native caller address and
native-finally/TLS boundaries. Generic native-finally binary checks remain in
use. Windows exception execution and complete body-query ownership are untested
and unfinished respectively.
