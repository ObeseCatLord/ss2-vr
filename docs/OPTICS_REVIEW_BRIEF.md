# Astra optic architecture brief — 2026-10-03

Solo mod, preserve native renderer/gameplay. Decide the smallest adapter for a physical magnified sniper optic, with independent hands and multiplayer. Do not approve guessed ABI or call the current stock-model preservation a scope. Actual game/headset testing is explicitly excluded; offline/build/static evidence is authorized. Teleport is excluded.

## Environment and evidence

| Fact | Status / check |
|---|---|
| Native game is PE32; OpenXR host is PE64; native D3D9 renders actual eyes and CPU readbacks feed D3D11 projection | [verified: src/game/bridge.cpp stereo, src/game/engine.cpp camera/project, src/host/host.cpp] |
| Existing IPC6/wire4 ownership and native reliable multiplayer authority must remain | [verified: common/protocol.hpp, game/multiplayer.cpp; do not redesign] |
| Native zoom is per-sniper activation/deactivation, timed interpolation, original owner bookkeeping, shooter damage switch | [verified: NATIVE_OPTICS_AUDIT, installed Sam2Game.dll RVAs171CF0,171DF0,172820,160340, shooter vtable2B2398] |
| Gun/arms render follows actual tracked grip; XR suppresses flat sniper zoom overlay | [verified: engine.cpp weaponAbs/sniperRender] |
| Scope cap candidate has one circular boundary and disk coverage; pinned mesh parser round-trips slices | [verified: tools/audit_sniper_geometry.py; run with ../Patch_02_068.gro] |
| Geometric rear cap is the authored optical lens | [unknown: material/UV and model placement not closed] |
| Native zoom input/event dispatch can be requested per hand on server through existing controls | [unknown: alt fire already means independent-hand firing in VR, not freely available] |
| One native Render3D invocation builds/executes/frees a collection; root Prepare isolates stereo query IDs | [verified: bridge.cpp stereo, engine.cpp viewPrepare/viewExecute] |
| Existing RT0/depth0 can be reused for extra capture BEFORE world eyes | [verified: allocation and sequential ownership in bridge; new capture still unimplemented] |
| Final-world lens can reuse root Execute injection phase used by lasers | [partly verified: current laser draw phase; textured draw/depth/postprocess suitability unverified] |
| Host quads can provide true world occlusion | [unverified; current host lacks world depth submission; reject foreground lens quad as default] |

Environment: repo is ss2-vr under installed game directory; owned modules in ../Bin; owned mesh in ../Patch_02_068.gro. Capstone/pefile via PYTHONPATH=deps/python. Static helper /tmp/ss2-vr-equivalence/inspect_native.py; no proprietary dumps in repo. Pinned deps restored to deps/vendor; old build caches need explicit paths. Remote head investigation is independent, owned by Atlas Patch; don't redo it.

## Mostly-worked incremental design

Keep native zoom state/timing/damage and native shooter. Trace the original zoom authority/event route before selecting a controller gesture. Derive aperture placement from the current native animated FP model and a proven stock optic profile; reject ambiguous/changed resources rather than applying an arbitrary controller offset. Use original full HMD FOV for surrounding world.

For active optic(s), run native Render3D additional capture(s) BEFORE normal eye rendering; temporarily use RT/depth0 and distinct scoped camera/FOV/root ID, retain actual eyeIndex array bounds, hide owned local gun and aiming lasers inside that capture. Copy the result to a small D3D9 render-target texture. Then render both normal native eyes, injecting a circular textured aperture at the physical lens through the root Execute phase with world depth testing. Freeze eligibility, profile, transform, zoom progress and texture transaction for the pair. On texture/native capture failure, hide that optic without invalidating a successful world pair. Use eye-relief/axis guards; preserve physical head/hand poses, don't magnify the HMD. Final native desktop Render3D restores engine caches as today.

Direct D3D calls, if needed only for this aperture, capture state at injection entry and restore it immediately with no intervening native cache mutation; an old pre-world state block would be wrong. Keep render target/depth and cache restoration explicit. Scope-only capture IDs must preserve native nested/shadow IDs. A native textured helper is preferred if a usable ABI is available.

No new renderer, material mutation on shared resources, generic pose manager, standalone scope-image IPC, foreground host layer or alternate damage protocol. Two native optics can be independent if native zoom/owner policy is demonstrated; do not assume player-global FOV interpolation safely represents both. Per-weapon EC/F0/F4 is the promising native zoom source, but its exact use as optic FOV still needs derivation.

## Open decisions and current lean

1. Native depth-tested aperture versus host quad: lean native injection; foreground overlay fails physical occlusion and grows layer/IPC management. Challenge root Execute depth/postprocess suitability and smallest proof.
2. Lens profile: lean narrow native mesh query once per current resource, checked against the geometric cap and authored material; avoid hundreds of per-frame GPU locks. A measured constant alone without resource/animation identity is insufficient. Parser is audit-only. Identify the smallest remaining proof, not a generic asset framework.
3. Zoom authority/gesture: native activation/deactivation mandatory because damage changes. Current lean is original native player/event dispatch plus a per-hand request, only after exact route is established. Automatic eye proximity is a possible gesture, not permission to mutate shooter state on render thread. Don't duplicate native toggle/owner restoration.
4. Extra capture placement/ownership: lean GPU native capture before both eyes, no IPC extension. Compare against after-world capture/reupload; reject unnecessary RT/state/protocol scaffolding. Determine pair coherence and all critical lifetime guards before implementation.

## Review contract

Read-only review; no production edits, game, host, Wine or runtime launches. Verify the cheap load-bearing claims from source/owned static binaries and script before critique. All reviews are Astra, no delegated reviews. Output <=1400 words: prioritized findings with evidence; adopted-design recommendation; explicit implementation gates; deepest spec for whichever decision you rank highest; honest go/no-go on source implementation now. Review parser correctness too. Don't revisit head fade, full transport, locomotion or remote animation. Missing evidence must be reported, not replaced with a heuristic. Return the review as final message; main owns disposition and integration.
