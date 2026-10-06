# Native flat UI: once-only owner follow-up

Main fingerprint-bound static inspection, no runtime or installed changes.
Investigation only; not a reviewed/implemented UI adapter. Complements
NATIVE_FLAT_OVERLAY_AUDIT.md while the independent turret source review runs.

## Additional demonstrated behavior

Conversation HUD Render176610..176859 is also mutating. It calls native
CTextQueue SetText at1767C1, original simulation-time/Time functions, clears
texts at176835 and finally Render2D at17683E. Replaying only individual HUD
Render callbacks is therefore not an established safe shortcut around the
previously demonstrated parent/fade mutation. Keep original overlay callbacks
once; a capture must borrow their actual output.

Original RenderOverlayFadingsFE830..FEA10 has exactly two demonstrated original
gfuFill call sites: returnFE90A (time-derived black/alpha packed into high byte)
and returnFEA07 (original native gradient GetColor result). It does not emit a
four-corner gradient at that boundary. Engine gfuFill6CFD0..6D033 creates a box
covering the current drawport and passes the same color to all four gradient
corners at6D028. The native gradient drawing body6CB40..6CBF3 installs simple
shader/vertex and color channels and issues the original draw. The bounded body
does not install a new blend type. Current blend/shader/viewport/color-space
ownership still needs admission before any world-wide fade reproduction;
capturing only a color is not sufficient proof. The original function owns
its native Time/COW updates and must not be replayed.

HUD leaf examples use the existing native graphics family: SimpleHud
RenderBarPart183B00..183BEC calls gfuPutRotatedTexture at183BC3; RenderIcon
183C10..183DB9 calls gfuPutGradientTexturePart3f at183DA7. These are concrete
draw-family leads, not proof that every native HUD/text/score/Netricsa element
has one universally capturable alpha/blend mode. All current asset and native
draw policy remain reusable; no new quest/message model is justified.

## Actual bridge constraint

bridge.cpp stereo finishes both eyes, reads their opaque pixels, restores/
rebuilds the original desktop Render3D and calls commitStereo/Ready **before**
the original RenderView parent reaches native RenderOverlay. Present currently
captures a MenuFrame only when menus/non-gameplay are visible. MenuFrame has
no world-request/session/reference/epoch identity and its pixels are explicit
mono menu/loading content; host excludes it from projection presentation.

Thus simply attaching an uncorrelated native HUD image or modifying an already
Ready eye after overlay would break the existing immutable pair contract.
An incremental candidate extends the existing native Rendering→Ready
transaction through its original once-only overlay owner, captures native UI
output and full-field fade separately, then commits them together. Reuse the
existing image channel and comfort anchor rather than another queue/quest
state machine. The same original owner/frame/epoch must survive every native
callback; failure must retire the existing transaction rather than revive an
older HUD. This is a candidate, not an approved architecture.

Main further inspected the actual Brain RenderViewEB7B0..EB87C. Native
brain+28 is resolved separately before player virtual610 atEB833 and again
before virtual608 atEB857. The overlay argument is a local flag initialized
to0 atEB7E5; an alternate world-view branch sets it to1 atEB800 and skips the
ordinary player610 call. It is not a returned RenderView value. Thus a future
once-owner transaction must positively associate the original brain invocation
and exact same resolved player through both callbacks; current brain index
global3F9E48 alone is not lifetime/frame proof. Flag1/no admitted stereo must
not create a gameplay HUD image based on a previous request.

Actual player RenderViewFEA20 calls player virtual600 Render3D atFEA84
(returnFEA8A), then continues original parent/listener/marker/crosshair work.
The existing exact Render3D caller gate is reusable, but that parent is not a
substitute for the later original flat overlay owner. The native parent
postlude remains once-only; drawing or capturing a UI image does not authorize
replaying its simulation/listener calls.

Before Astra design review, establish supported actual native blend/shader/
alpha families, transparent coverage and desktop/cache restoration, precise
once-owner identity/lifetime and pair/image correlation. Compare capture plus
native desktop composite with immediate draw duplication; reject any route
that replays native simulation/element callbacks. Existing35-degree comfort
threshold and physical sizing remain the presentation reference. No complete
native UI feature follows from this investigation.
