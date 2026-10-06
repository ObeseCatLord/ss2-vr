# Native collision query family — bounded Astra evidence

Astra/xhigh read-only investigation, main verified effective routing locally.
Initial self-verification uncertainty caused no native analysis; main confirmed
the selected profile and the same agent continued. No edits, delegation or native
execution. NO-GO for treating the inspected public API as an existing full-player
translation sweep. Full roomscale/multiplayer remain required.

| Native API | Module/RVA and ABI | Established meaning |
|---|---|---|
| cldCheckRay/cldContinueRay | Engine29200/29290; int cdecl() | Shared ray/filter hit/traversal, not body acceptance |
| cldInvokeCollider | Engine27980; void cdecl(CHull*,CHull*) | Registered hull-pair collision without displacement/time/fraction |
| cldGetHullsInRange | Engine29E50; void cdecl(Vec3 const&,float,CDynamicContainer<CHull>&) | Stationary spherical overlap enumeration |
| cldGetHullsInBox | Engine2FD00; void cdecl(container&,Box3 const&) | Collision-grid candidate enumeration |
| mthIntersectThickRayPrimitive | Core19A40; Box1 cdecl(Ray3 const&,PrimitiveDesc const&,float) with hidden output/EAX | Ray-radius versus target primitive; no moving avatar descriptor |

Complete Engine cld export family contained no demonstrated shape/hull/capsule
sweep. PrimitiveHull CheckRay525B0 transforms shared ray into target frame and
calls Core19A40 at5278E using its descriptor78. Target branches sphere19A5C,
box19AED and capsule19AB8 expand dimensions by scalar radius; the capsule routine
19710 concerns the target. Native avatar28BC0/avatarMechanism28BD0 filter identities;
traversal2F902 skips avatar hull, not imports its shape. Ray1B34B0 is origin plus
direction, radius1B3560 and hit-distance1B35C0 (x87 float) remain shared queries.
Hull/normal/material results28C10/28C20/28C60 are not retained owner references.

Sam puppet SetCurrentMechanism83B90 gets root Engine133B60 at83D32, stores handle118;
mechanism handle114. Part CheckMove1315D0 initializes fraction1 and dispatches
body/fallback aspect slot20 at13185E. Body uses Aspect57F70, transforms child old/new
poses and dispatches5834E. PrimitiveHull52030 then skips positional length below
0.9*InnerRadius52091→52180; larger moves use zero-radius centre ray toL+r and reduce
fraction=max(0,hitDistance-r)/L. No arbitrary-small full-volume/orientation proof.

Primitive descriptor is16 bytes at hull78; template CreateHull52B70 copies from24
and applies stretch52BDA–52C0B. Actual player dimensions/shape must be read, not
guessed. Mechanism deletion99400 clears/destroys body tree; part131F80 destroys
aspect children. Resolved body/hull pointers are borrowed.

Full-volume overlap exists: registration30960 selects primitive/primitive301F0
and primitive/model307F0. Sphere pair4E4D0 reads stored hull poses/dimensions and
compares current separation;4E5F8 sets cld_bHullsIntersect2D97AC. Exported
cld_bDoNotGenerateContacts2D97A8 suppresses contact generation. CollisionDomain
GetHullsInRange29CA0 builds a temporary stationary sphere, invokes candidate
colliders and destroys it. It supplies no travel/time-of-impact result. Scratch
and a collider reentry guard are shared.

Endpoint overlap/bisection cannot prove absence of an intervening thin wall.
Unchecked placement cannot repair that query gap. The subsequent conservative
moving-sphere cover proposal is a separately reopened design in
ROOMSCALE_QUERY_ADAPTER_BRIEF.md; this audit does not validate its kernel/contact
semantics, settlement or multiplayer. Native solver/body remain unchanged.
Module fingerprints match RESEARCH.md; no extracted asset or dump is committed.
