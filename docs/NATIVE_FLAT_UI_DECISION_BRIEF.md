# Complete native flat UI: incremental adapter decision

Mostly-worked main proposal, not reviewed/implemented. Solo mod. User requires
complete native overlays and comfortable threshold-following VR UI; native
callback/gameplay behavior must remain original. Actual game/headset/network
testing is explicitly excluded. No new quest model, renderer rewrite or native
callback replay. Baseline3332f60; source/package0.2.8 turret slice is accepted
separately and must not be re-reviewed.

Historical brief retained for review provenance. The consolidated conditional
design decision and main disposition are now NATIVE_FLAT_UI_DESIGN_REVIEW.md.
In particular, main discovered dynamic glyphXYZ and enabled depth testing:
matrix-only projection plus fresh stencil depth is not established as adequate.
Two actual draw families and program objects are proved in
NATIVE_FLAT_UI_CLIP_STATE_AUDIT.md. No source adapter is implied by this brief.

## Verified environment and reusable behavior

| Fact | Evidence |
|---|---|
| Original flat UI has once-only mutations | [verified: native] Brain RenderViewEB7B0..EB87C resolves puppet28 before virtual610 and again before608/retEB85D. Player RenderOverlay105480..105558 invokes native text/HUD/fades; conversation Render176610..176859 changes text/time, ProcRender Render2D10D20..10E80 renders/deletes expired entries, fadeFE830..FEA10 changesTime/COW. See once-owner/alpha audits. |
| Exact world/native owner route | [verified: native] ordinary player RenderView calls virtual600 Render3D returnFEA8A, then original parent/listener/marker/crosshair work. Brain alternate world branch sets overlay flag1 and skips ordinary610. Current local rider/player/request/epoch/native-main-thread guards are reusable; flag/current-player-index alone is not frame proof. |
| Existing stereo transaction/resources | [verified: source] bridge.cpp has native eye color/depth, executed view/fullP from engine.cpp, one Rendering request, both-eye post-native markers, readback and final commit under existing snapshot/MP guards. It currently reads/commits before original flat overlay. Retain those resources/request and extend their actual transaction to the once-only original owner, rather than another queue. |
| Native RGB blend family is wider than normal alpha | [verified: native] Engine8BD00/Gfx54B0 actual modes501..507 include SRCALPHA/INVSRCALPHA, additive and per-channel destination modulation. Default text configB4=501, but current glyph renderer reads configB4 dynamically. A normal-only subset is not full UI equivalence. |
| Built-in shader semantics are available statically | [verified: main native] Engine startup94440..9447D creates six program pairs from embedded source pointers2C40E0/4. Zero cache is not absent source. Position uses c1..c4, texture scaling c5; pixel1 passes color,3/5 multiply texture RGBA by interpolated color. No proprietary source retained. |
| Exact native constants→D3D mapping | [verified: native] UseSimpleShader944C0..94C2F uploads vectors1..5 via2E62FC at94C21; Gfx startup4BED..4BF3 binds6330..638A, forwarding to device slot178 SetVertexShaderConstantF with original start/pointer/count. No register renumbering. |
| Native fade draws are distinguishable | [verified: native] Fadings has two gfuFill sites retFE90A/FEA07. Fill6CFD0 covers original drawport with four equal colors, simple shader1. Original blend is inherited, not replaced by fill. Full-field fades must cover eyes, not only a panel. |
| Existing comfort policy | [verified: source] common/ui.hpp ComfortAnchor starts35deg/stops8/max90deg/s and smooth positional follow. Host generated stats HUD uses its one anchor/layer; native menu/wheels have separate existing anchors. Preserve policy without a second active stats/native-HUD state machine. |
| Formats/limits | [verified: source/API] existing eye images are opaque BGRA/RGBA8 and host UNORM; four-layer minimum includes world/HUD/two wheels. Native grayscale/color modulation cannot generally be encoded as one scalar-alpha quad. Actual native target/state restore and program/draw admission are necessary, not proven by this table. |

ProcRender fingerprint newly audited:
e93a51da739e6a4f0b66457af7de750befbd9514eb8180afe15513ca8d3427f2.
Future ABI use must gate it. Existing five fingerprints remain unchanged.

## Designs compared

A: one original RenderOverlay redirected into a transparent native A8 target,
with separate-alpha coverage, then original desktop composite and existing host
HUD quad/image channel. Smaller initial draw adapter. Demonstrated501 works
mathematically with admitted shader/source semantics; 503/504 emission can be
represented with deliberate premultiplied alpha, but per-channel destination
modulation502/505/506 generally cannot be collapsed into one independent scalar
alpha image. It also requires correlated UI image ABI/host upload/four-layer
admission and full-world fades. Main rejects A as the sole full-equivalence
implementation; a verified subset could be a checkpoint only. Do not silently
reduce the full goal to501 defaults or introduce per-mode UI state machines.

B (main lean): duplicate only immediate **device GPU draw operations** during
the one original native overlay invocation into retained eye targets, using
original shaders, buffers/streams/texture/vertex/alpha/RGB blend state. For
admitted built-in programs, modify only position constants c1..c4 to place the
native screen output on a comfort-following plane per eye; UV/color/fog/programs
remain original. Native element/time/COW/listener callbacks run once. Two exact
fade sites use unchanged full-eye screen coverage, after panel content, retaining
actual blend. All other native desktop draws remain original once.

Reuse original brain/render-overlay ownership and existing Rendering request;
defer readback/Ready until both original world and once-only overlay finish.
Missing/changed owner/epoch/program/target/draw admission rejects the whole pair.
No queue, copied model/quest policy, native program rewrite or per-mode texture
bank. World markers/root retirement stay unchanged. Retained raw GPU resources
are existing mod-owned COM targets, not borrowed native model collections.

Capture actual physical device state at the immediate original draw boundary;
perform extra device operations through scoped original API entries and restore
constants/streams/shaders/viewport/scissor/depth/targets before native execution
continues. Native engine cached values remain unchanged through the extra draws.
A saved D3D state block here is potentially valid because no native setter runs
between capture/restore; confirm its actual coverage and separately restore
target/depth. This does not authorize an old state block across originalRender3D.

Native screenshot clipping requires explicit preservation: transformed source
viewport/scissor bounds become projected polygons, not an eye-space rectangular
scissor. Main proposes one reusable mod-owned stencil depth target only for the
UI duplicate draw phase, drawing a mask for the projected intersection of the
source viewport/scissor and retaining original source near/far admission. Exact
source Z/clip semantics and any original enabled stencil must be proved; bounding
box scissor approximation or flattening otherwise-clipped geometry is rejected.
The proposed stencil target isolates this demonstrated incompatibility without
changing existing world depth/native scene collections.

Comfort ownership: reuse ComfortAnchor geometry/config for the native HUD and
retire the generated host stats-HUD anchor/layer when an admitted world pair
contains complete native UI; menu/wheels remain host-owned. Prefer existing pair
capability/version admission over a duplicated HUD scheduler. Shared tracking
rig correction must apply consistently without applying head reach bounds to
the panel offset. Actual physical size/aspect should preserve readable native
layout; reserve larger full-native panel dimensions than the current narrow
stats strip. Technical sizing is main's decision, no new user confirmation.

C: replay native overlay per eye or rebuild native messages/HUD. Reject because
mutation is demonstrated and adjacent working native policy is reusable.
Recording/replaying arbitrary future draw buffers is rejected because their
native transient lifetime is not established; duplicate immediately instead.

## Open decisions and bounded source gates

Main asks Astra to choose the smallest adequate full-goal route, challenge B's
necessity/complexity, and identify deletions. Is a complete admitted native
capture route possible without losing proven RGB semantics? Does the proposed
one-pass GPU duplication/stencil introduce avoidable policy/lifetime state?
Merge owner, pair and HUD capability questions where they are one transaction.

Before source: prove actual admitted built-in program/device identity and
immediate draw families; exact source viewport/scissor/Z clipping and native
target/device restore; parent-to-overlay pair lifetime and full-field fade
classification. These are [unknown/unverified], not evidence for a rewrite.
Native multiworker/device concurrency and actual appearance/performance remain
[unknown/unverified]. Do not infer game/headset execution permission.

Expected implementation estimate:350–600 draw/geometry/lifecycle lines plus
production-helper math/ownership regressions and compiled native caller checks.
If it grows into a renderer/state manager, pause and reopen; preserving native
operations is the behavioral reference. Smallest eventual vertical proof:
same original owner/request world pair → once-only native text draw → actual
original GPU commands projected/clipped in both eyes → exact restore → both
readbacks/one Ready, with original time/expiry once and fullscreen fades once.
Offline fixtures must exercise production transforms/clipping/commit failures;
no counters/mocks are native appearance proof.

## Astra review contract

Explicit gpt-6-astra/xhigh, effective verified; read-only design review.
Read AGENTS.md and senior-review skill. Exact scope: this choice, native evidence
in NATIVE_FLAT_UI_* audits and existing bridge/host comfort/native owner seams.
No edits/delegation/runtime, no turret/transport/mounted/head/optic/roomscale
re-review. Main owns architecture/integration and all writes. Return terminal
<=1300words prioritized evidence-backed critique, selected minimal route,
conditional GO/NO-GO and narrow next proof; genuine human taste/budget questions
only if unavoidable. Missing evidence should remain bounded, not a new subsystem
or invented permission gate. This is not source GO or full goal completion.
