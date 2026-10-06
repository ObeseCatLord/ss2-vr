# Optics and native muzzle review disposition — 2026-10-03

Astra reviewed the verified design brief, owned native code, geometry audit and implementation. Effective gpt-6-astra/xhigh was independently verified for this reviewer. Final source verdict: GO for the muzzle correction and pinned offline geometry audit; integrated optics remains gated. Main ran builds/offline checks separately. No runtime or installed-game mutation occurred.

| Finding / recommendation | Main disposition |
|---|---|
| Zoomed native sniper placement is view origin, not the gun attachment | Adopted. Shared shooting dispatch selects the original base attachment getter once for an accepted VR context. Native zoom/damage/timing remain unchanged. |
| Getter selection and old authority/local retargeting had unequal eligibility | Corrected. Removed the weaker authorityShot path and separate selector. One transient validated context drives getter and retargeting. Rejected/nested calls remain native, unretargeted, with calibrated=false. Actual server shots keep authority precedence; explicit local laser snapshots keep their presentation input. |
| Frozen authority must not be replaced by a weaker/newer sample afterward | Adopted. Preserve accepted avatar/incarnation/sequence/hand/sample; recheck lifecycle and current handles. Native placement references still evaluate at the muzzle boundary. |
| Lifetime guard must precede weapon/player pointer reads | Corrected. Resolve native weapon and owner handles before reading post-getter ownership or equipped-hand fields. Local epoch/freshness are rechecked before the anchor. |
| Existing transform checks do not exercise dispatch/rejection | Adopted. Added production-helper checks for local/authority eligibility, hand/incarnation/alias/age/ownership rejection, exactly one getter, nested/native fallback and reference parity. These are offline helper coverage, not runtime bullet/laser equivalence. |
| Cap topology needed more than Euler/radial checks | Adopted. Pinned audit verifies connected boundary, reversed interior edges, uniform nondegenerate face winding, circularity/coverage, buffer framing/bounds/finiteness and byte-exact slice round-trips. |
| Native capture before both world eyes avoids unnecessary IPC/layers | Adopted as the next design. Reuse an existing native eye target, retain optic D3D9 textures, freeze the pair and give scope roots distinct IDs without changing nested/shadow behavior. No foreground host optic quad or alternate renderer. |
| Geometric cap alone is not authored lens placement | Gate retained. Prove current loaded resource, actual animated affine transform, lens/axis/reticle semantics and angular mapping. |
| Native weapon depth range0–0.1 can reject a world-depth aperture; using weapon depth alone does not prove wall occlusion | Gate retained. Compare a narrow owned-sniper projection/depth adapter with an earlier insertion point and prove exact instance cap treatment. Do not apply arbitrary depth bias or shared-material mutation. |
| A completed capture reinserted before renFinishRender may receive fullscreen processing twice | Gate retained. Match capture/insertion color stages before the optic renderer is implemented. |
| Stock AltFire cannot provide per-hand dual zoom | Gate retained. Trace an authoritative per-hand native transaction preserving alternative-held state, +BC conditions, interpolation, damage, release/put-down/deletion and shared third-person restoration. |
| Ordinary optic copy failure differs from device loss/failed restoration | Adopted. Future allocation/copy failure may hide an optic; failed device/state restoration must invalidate the world pair. Preserve final desktop native cache restoration. |

Astra's final pass found no remaining blocker in the reviewed muzzle/helper/parser diff. All eight offline groups pass; x86 proxy/server and x64 host/official loader build. Static verification retains34 exported hooks, three internal entries and matching IPC6 layouts. The existing0.2.2 archive is unchanged; packaging of this follow-up is recorded separately.

Physical optics, binocular comfort, wall occlusion and performance remain runtime-unverified. These gates are technical evidence requirements, not a request for user permission. Full immersive equivalence remains active and unfinished.
