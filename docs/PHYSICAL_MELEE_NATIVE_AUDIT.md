# Physical melee — native CircularSaw audit

Astra/xhigh read-only investigation, effective settings verified locally. No
edits, delegation, builds or native/Windows/game/XR/network execution. Main owns
implementation. This establishes one native family, not completed swing combat.

## Native weapon and authority

Sam2Game CircularSaw constructor164680 installs vtable2CDD10, weapon ID0 at+B4,
flags2 at+60. CallFireStart165460 invokes base4CC40; native CanFireFromWeaponNow
validates and flags2 select state4. Held predicate F6F00 is integer thiscall with
one4-byte weapon handle,ret4; base OnStep calls player slot730 at4EF85/4F090.

Base DoFiringsFromLastFireTime49100 calls slot244 DoTheFiring(float);4CDC0 checks
owner alive/usability and invokes slot20C OnFire. Saw1654B0, integer thiscall(float),
ret4, calls base/native ammunition before slot280 CutWithSaw165570. Native recoil
duration164100 returns[this+D8] in x87; constructor default0.05. Keep native cadence;
do not introduce damage timers or promise one damage event per gesture.

CutWithSaw calls shooting placement slot1D0 at165629 with hidden output buffer.
The saw resolves to base4A6E0, already hooked by engine.cpp GetShootingPlacement.
It constructs three stock query endpoints, invokes helper21B0D0 at165837 and loops
through165BC6. The helper uses native ray init/distance/radius/category and
cldCheckRay21AEFA. Existing tracked muzzle placement therefore reaches hit queries.
Native hit selection remains authoritative; swept blade collision is unproved.

Hit damage uses native D4*D8*owner multiplier and target slotF8 at165A33. Puppet
OnReceiveDamage9E290 rejects nonhost9E29B–9E2A3; ReceiveHealth7F5C0 rejects remote
mutation. Do not bypass these native authority/collision/damage paths.

## Edge and held input must agree

Saw OnFireReleased165490 calls base48FF0, native first-fire state and
FireReleasedInternal4CAA0; state4 returns to1, saw cutting state becomes3.
Native ExecuteOperatorFiring8E580,void thiscall(puppet),ret8E765, compares brain
current fire bits160 to puppet prior348, honors brain blocking161, dispatches
down/press/release slots520/524/528 and commits native history. Release proceeds
through7FFE0→player StopAttackFA770→weapon slot204. Native ProcessPlayerControls
EE0F0/ClientActionEE260 populates bits and retains VM RPC EE441.

Current VR fireQuery overrides the weapon held predicate. A retained network tap
can freeze one interval in OrderedPosePolicy; that alone does not prove native
edge/release coherence, especially state4 firing before later held checks. This
is an inference requiring investigation, not a confirmed standard-trigger bug.
The existing native command producer may already supply some matching edges.

Changing only Snapshot.fire or fireMask for gestures also contradicts raw-trigger
neutral provenance in engine.cpp update. A physical-swing primary producer must
feed one coherent logical stream through existing gates/generations/local commands,
neutral witnesses, network intent/freeze/ACK and held/edge consumers.

## Main disposition and bounded follow-up

Conditional GO for a source vertical proof, NO-GO for swing-to-fireMask alone.
Preserve native weapon FSM, cadence, ammo, hit query, damage and RPC. No gesture
implementation until native held/release integration is proved. A candidate is
an exact ExecuteOperatorFiring adapter using existing frozen intent, temporary
brain input borrowing and native committed history; safe lifetime/hand mapping/
reentrancy are unknown. Prefer the existing native command producer if it already
provides the join, rather than add a parallel input policy.

First proof: one right-hand saw/single-hand mode, fresh neutral→swing→held motion→
neutral, through local and delayed retained authority to native start/cadence/hit
placement/release. Include pulse expiry, equip/tracking loss and cooldown rejection.
Speed estimation must precede locomotion/recenter transforms. Threshold/sampling
adequacy and physical-contact/runtime/network timing remain unverified.

SHA256 evidence: Sam2Game.dll
5628b4ed30a966f10c8e8ea46bf0789257a35ea80127bacacebe28b1ce5303df;
CutWithSaw RVA165570,length687,
90890fe45e9e6923010cba62944de91948efd1796b20ebc2f693b3eb83a70a5a;
ExecuteOperatorFiring RVA8E580,length1E6,
5e3f54ccf7a9fc240d575b41b9747b4bb6a8dd2069201a5a2f811511f77bca9f.
