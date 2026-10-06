# Astra laser review disposition

Reviewer: gpt-6-astra, explicitly selected xhigh and independently confirmed. Three read-only static passes; no game/host/headset execution. Main spot-checked the load-bearing native call and lookup facts, corrected the comparison enum from the installed backend table, and applied the dispositions.

| Finding | Disposition |
|---|---|
| Arbitrary ray insertion can destroy a live native query | Adopted: run only at the audited native CLOS rayInit return site (Sam2Game RVA0x21ae69), on the recorded simulation thread and outside eye rendering. Original initialization retires previous native cleanup; each mod query uses the same lifecycle; final original initialization leaves the caller its clean baseline. |
| Native collision setters do not themselves register cleanup | Adopted: restrict the insertion to the verified caller, whose setters follow initialization. Do not snapshot unknown native global hit/cleanup state. |
| Root Execute does not restore root matrices; line shader consumes current model | Adopted: require native current canvas, use gfuOrtho to establish identity model, then explicitly activate the exact root view. Draw through native gfuDrawLine3f. |
| Depth/write alone leave blend/alpha/comparison inherited | Adopted: native depth enabled, write disabled, blending and alpha test disabled, comparison set to installed native enum42; restore prior native values through wrappers. GfxD3D RVA0x5640 indexes RVA0x11274; index42 contains D3DCMP_LESSEQUAL=4 at RVA0x1131c. |
| Cache read/expiry separately per eye can show only one beam | Adopted: freeze both samples and eligibility once in beginStereo. Both eyes consume that immutable pair. Frame invalidation rejects the whole world pair. |
| Simulation tracking sequence can differ from render request | Adopted: service a real pending Requested slot's immutable input at the audited query phase; preserve session/reference/epoch/owner validation. Exact request-input sequence required for laser submission. |
| Fresh native model lookup can mutate asset references and change actual shooting semantics | Adapted by deleting the extra model lookup from shooting. Existing actual-shot calibration path remains; laser sampling requires native render-populated, handle-matched calibration no older than100ms. First-render grip fallback is not shown as a muzzle beam. |
| Early reuse bypassed equip and muzzle-calibration changes | Adopted: evaluate per-hand eligibility and derive muzzle before reuse; compare exact muzzle/body/owner/weapon/epoch/input identity and80ms age. Pending equip is copied in the native snapshot and checked again at stereo freeze. |
| Need replacement physics/renderer | Rejected: retain the original collision query, draw helpers and full native per-eye collection. |

Final scoped review found no remaining load-bearing issue after these corrections. Source and both PE products compile; portable checks cover shared age-boundary freezing, ownership/weapon replacement, equip suppression and body drift. Native query timing, visual beam/hit-marker appearance, availability during motion and performance remain runtime-unverified.

The implementation draws each hand's beam along the shared native retargeted muzzle aim, stops at native bullet-category geometry, and draws a depth-tested hit cross only for a reported hit. It suppresses invalid/stale/equipping/wheel hands. config/SS2VR.ini controls enablement and maximum length. These are source-level implemented behaviors, not a tested headset claim.
