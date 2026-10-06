# Frozen-frame scope integration

This is the root-owned implementation plan after the native pre-bloom and base
FOV findings. It supersedes the first-gun-cut proposal, not the evidence gates.
No game or headset execution is authorized.

## One existing frame owner

Keep the bridge's current Rendering slot, retained device/resources, generation,
request and engine frozen snapshot. For a current zoomed sniper, perform one
ordinary first-eye preview, copy its admitted per-hand optical observations
before endEye, then render at most two explicit scope-source views. Finally
render the real eye pair and rebuild desktop once. No new input, simulation,
calibration, native animation evaluator, query manager or pose bank is needed.

A preview uses the real first-eye transform but a distinct native history salt.
A source uses an explicit purpose and hand; eyeIndex remains -1. Its camera/FOV
come from the copied optical geometry and that weapon's zoom. History salts must
be distinct for preview, both sources and both final eyes at the existing exact
root-Prepare caller. Nested native view identifiers remain untouched.

Reuse retained eye0 color/depth as disposable preview/source scratch, clearing
before each invocation. Explicit bridge source routing maps original desktop
aliases to scratch without pretending it is a final eye. Copy pre-bloom pixels
to separate single-level RT textures; suffix/cleanup run normally and scratch is
later overwritten by final eyes. Never retain native HDR/intermediate surfaces.

## Source lifetime

Queue the native callback once at source-only FDA0B, then suppress only that
native handheld collector with its existing early-return policy. Root camera,
projection and source ownership must agree before arming. Native rankAFFFF
preserves flares and visibility work and precedes bloom/text with or without
bloom. Capture only exact direct scratch RT, same-format A8/X8, non-MSAA, full
viewport and no extra MRT. Copy must use original unfiltered StretchRect.

The callback's storage lives above the complete native invocation and cleanup.
Capture success alone does not publish a source: require callback once, normal
native finish, unchanged frame/resource ownership and no interference/fault.
An abnormal source invocation permanently disables further scope-source use;
never recycle a context which an incompletely retired native command could
still reference. No native allocation recovery or command-list deletion.

## Final image transaction

Final per-hand draw must correspond to the preview's owner/model/geometry,
affine, native zoom and image-coordinate evidence under the same request.
Missing or changed observations decline the optional image, preserving native
full draw. A source texture alone is never enough.

Before splitting, retain current native bindings and prove actual pinned Scope
geometry/UV, native opaque color/query conditions, unchanged VS inputs/constants,
expected eye RT/depth and image-source ownership. The existing root invocation
remains the authority. Preserve native VS and all its constants. Use mod-owned
PS2a RGB-only shader and actual aperture indices, source sampler0 with sRGB decode
disabled, depth EQUAL/no writes, RGB-only color writes, no alpha/blend/stencil/fog.
No gamma conversion: accept only original sRGB-write-disabled stored-RGB subset.

Execute native901, native22, image22, native5. Restore every state changed by the
image pass before suffix. Any failure after splitting invalidates the pair; no
full-draw fallback, retry or late cap overlay. Resource loss, reentrant output or
unproven restoration invalidates the existing pair. No unknown managed-buffer
lock recovery is added.

Each stage must be built and checked before connection. Pure tests and native
static evidence remain separate from GPU execution and playability.

## Connected source checkpoint (2026-10-05)

The bridge now implements that sequence inside its existing frame owner. It
allocates at most two private single-level source textures and the mod-owned
opaque shader, reuses retained eye0 depth/color only as disposable scratch, and
keeps source callbacks outside the final-eye index namespace. Native collection
is skipped only at FDA0B. The preview/final/source salts are 40000000,
80000000/C0000000 and A0000000/E0000000; original desktop uses no added salt.

The GPU adapter retains its first native input/color references through the
ordered transaction, snapshots all supported VS float constants before/after
buffer copies and again after hash/source preparation, and never modifies VS
state. Before splitting it also revalidates the exact live pose, native query
boundary, source identity, color and geometry/UV correspondence. It saves PS,
texture0, c0..c6, ten changed sampler settings and three changed raster states.
The original cap writes native depth/alpha; the image writes RGB only, then
restores and reads back all changed state before the original suffix.

All thirteen hooked draw/clear/transfer/RT/depth entry families participate in
a sticky output-interference guard. The indexed draw notice lives in the cdecl
scope adapter so the narrow WINAPI argument/caller wrapper remains unchanged.
This is ordinary pinned-device interception, not a guarantee against foreign
unhooked issuers or arbitrary concurrent COM mutation. Native query Issue
failure coverage retains the explicit limits in SCOPE_QUERY_NATIVE_AUDIT.md.

Root self-check found and fixed two lifecycle details before this checkpoint:
reentrant duplicate capture must remain rejected even if an outer copy later
returns success, and invocation device/source/destination references must
survive an early enclosing-frame retirement. The production duplicate latch
has a regression test. Early source-owner retirement permanently disables source
reuse; it cannot relabel stale native callback storage as another invocation.

Optical eye relief is a bounded presentation setting, default0.1m, separate from
native zoom timing. Reticle half-width/length are fixed image-space presentation
values; its center uses the same frozen native muzzle/collision point as laser
aiming, projected into each optic's actual source camera. Missing/behind/outside
native aim suppresses the cross instead of inventing an optical-axis zero.
Zoomed scopes may use the existing admitted query opportunity even when visual
lasers are disabled; no render-time query or extra simulation phase is added.
This is the existing native aim guide, not prediction of later projectile impact. Actual visual coverage,
performance, GPU/COM failure injection and native callback execution remain
unverified because no game/Windows/headset execution was performed. Unknown
geometry/PS profiles, changed preview correspondence, HDR/intermediate targets,
MSAA and unsupported device/filter capabilities retain the native full draw.
This first implemented subset is not universal shader/mod compatibility.


## Actual cap-position proof

Root self-review also found that retaining VS state alone did not establish
correspondence between the observed optical affine and actual raster positions.
The existing UV-only decoder deliberately made no such promise. Image admission
now additionally checks the exact captured program/constants on all24 admitted
cap vertices against the executed adjusted eye projection, captured world view
and observed model/palette affine. It propagates unknown inputs/arithmetic
instead of manufacturing missing normals, tangents or addressing. Exact hashed
zero local indices and unit first-bone weights supply the sole skin-input subset.
Relevant vertex-declaration semantic collisions now decline as well.

This read-only position admission does not replace native skinning or upload a
computed pose. It follows the actual instruction order with float operations;
unknown/fractional relative addresses and missing constants reject. Shader-local
DEF values override API uploads globally, including definitions appearing later
in the token stream, per [Microsoft's DEF contract](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/def---vs).
SUB arity was added to the complete VS1.1 decoder; the independent Linux vkd3d
disassembler validates a mod-owned skinned-position/SUB fixture. The checker
compares every clip component within1e-5 absolute plus1e-5 relative tolerance.
This is bounded CPU admission, not a claim of GPU floating-point equivalence.

Fixtures cover direct and relative-palette transforms, truncated programs,
missing/nonfinite constants, changed palette translation, undefined outputs,
UV-to-position substitution, global late DEF override and ignored color-only
arithmetic. The former UV-only acceptance never suffices for an image pass.
