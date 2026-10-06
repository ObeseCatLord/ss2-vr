# Opaque scope RGB pass — senior review brief

Historical proposal below: Astra rejected the late overlay for depth ownership/
suffix ordering. Final disposition is SCOPE_OPAQUE_IMAGE_REVIEW.md; the narrower
ordered candidate remains conditional and has no native image consumer.

Baseline6f75812/8c95b6a: actual cap, UV declaration/slice, actual VS1.1 UV predicate,
actual finite c8/c9 and derived image coordinates under existing observation
lifetime. All20 portable groups/build/static ABI checks pass; no native runtime.
Teleport excluded; multiplayer required. Main owns all writes; Astra/xhigh reviews
and investigates. Physical-melee investigation is disjoint.

Verified native alpha differences are in SCOPE_NATIVE_ALPHA_AUDIT.md. The earlier
experimental main consumes COLOR0 alpha while PP2 consumes TEXCOORD2.w with
different precision. This demonstrates an incompatibility, not a reason to
replace adjacent native VS, animation or state machines.

Compare previous pre-admitted901/22/5 split to Astra's smaller opaque adapter:
retain the original928-triangle draw once, then one cap-only22-triangle RGB draw
with unchanged native VS/streams/indices/viewport/depth. Actual alpha-test/blend/
stencil/fog must be OFF, depth test/write originallyON, ordinary PS family0/1,
and material/root/source-image identity admitted. Mask output alpha writes;
depth-EQUAL, no depth writes. Native target alpha remains untouched. No alpha
reconstruction, split-prefix rollback or native cache mutation.

Unknown/failed getters/source admission decline before additional draw. Any
uncertain draw/state restore rejects the existing whole pair, with idempotent
TLS/native-finally cleanup. No second original full draw, new cache, protocol,
generation, bank or rollback engine. Actual device state must restore exactly.

Load-bearing unknowns: root COLOR ownership; selected material/config association;
source cut/world completeness and color transfer; original PS no kill/depth output;
MSAA/depth equality and inherited clipping; same frozen source/final animation.
An opaque alpha path narrows the original admission burden without resolving
those unknowns. No live adapter or source capture exists yet.

Prepared source: scope_image.hlsl shares RGB lookup in scopeImageColor. main is
explicitly PP COLOR0-only/unadmitted; opaque has TEXCOORD3 and s0 only, no native
color/diffuse alpha input, alpha0 for a future RGB-only write mask. Neither entry
is linked/enabled. compile_scope_shader.py adds main|opaque entries and separately
audits generated interfaces. Linux vkd3d compile/disassembly passes: main151slots,
opaque146slots, each4temps; retained conservative PS2-extended cap contract.
Actual state binding, native GPU precision and imagery remain unverified.

Review contract: verify-then-critique minimal adapter versus split, challenge
MSAA/occlusion/earlyZ/kill/depth/root-transfer assumptions and seek deletion.
Give architecture GO/NO-GO and bounded source GO/NO-GO for the two preparation
files. <=1000words, exact native/file evidence and primary API sources. No edits,
delegation or runtime. Missing evidence must stay explicit, not become production
defaults. Smallest vertical proof remains one sniper, ordinary offhand and two
native eyes with an actually admitted scene resource; no full equivalence claim.
