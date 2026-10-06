# Scope cap stage and owned geometry — Astra review

Explicit/effective `gpt-6-astra` / `xhigh` was verified for the design, ownership
follow-up and source acceptance. Read-only review; no native runtime execution.
The native per-hand zoom controls in0.2.11 are the existing behavioral reference
and were not reimplemented.

Astra rejected replacing lens pixels after a finished eye image: depth cannot
recover transparent foreground contributions, and completed scope color may
receive scene effects again when inserted earlier. Capturing before HDR resolve
alone does not establish matching color stages. The next image proof is the
actual cap's appropriate native material/color pass and the relevant root-command
suffix, preserving original depth/stencil, native passes and later effects.
There is no accepted magnified-image renderer yet.

The implemented slice uses one selection result for the actual model/mesh/draw/
palette association. Native indices remain callback-local. The existing observer
copies the selected surface's counts and four channel format/ordinal/offsets
into its pointer-free pose sample. No buffer pointer, graphics callback, shader
change, lock, extra native hook or resource registry was added. Descriptor
observation remains diagnostic: `contentVerified=false`.

The common owned-byte cap parser copies the exact22 triangles/24 vertices and
preserves their actual winding under full affine transforms, including reflection
and in-place transformation. The private asset verifier checks the owned mesh,
skeleton and all four slice hashes before passing23,248 bytes to the portable
Linux checker. Its temporary file is outside the repository and removed after
verification. No game asset is compiled into the mod or committed as a fixture.

| Astra recommendation | Main disposition |
|---|---|
| Reject late finished-color lens replacement | Adopted. No late GPU lens was implemented. |
| Classify actual cap pass and root effects before scope captures | Adopted. Native preset/dispatcher investigation is the next bounded image proof. |
| Reuse exact selection instead of another association registry | Adopted in `selectScopeDraw`; the native observer copies only selected-surface value descriptors. |
| CPU type/lock/range checks do not pin buffer storage | Adopted. The candidate unlinked native reader was removed; no new live buffer read is enabled. |
| Narrow lifetime proof to existing selected resource retention and native read/mutation boundary | Adopted. Inspect CVertexBuffer creation/update/clear and E12B0→DC510/E1200→DC6D0 ownership; no new lifecycle manager. |
| Original four sequential read locks may reuse proven native ownership | Conditional only. Access93 and matching cleanup are reference behavior; locks alone are not lifetime proof. GPU backend null still increments native count and must not be treated as no acquisition. |
| Keep full content admission distinct from layout/archive checks | Adopted. Offline asset success and copied descriptors do not change `contentVerified`. |
| In-place transform clears its input | Fixed. Copy precedes output reset; focused XYZ regression passes. |
| Release checker can silently skip assertions | Fixed. `NDEBUG` compilation is explicitly refused; compiler rejection checked. |

Final bounded verdict: **SOURCE GO for descriptor observation and owned geometry
extraction/transform**, with both source qualifications closed. Native buffer
integration and image enablement remain conditional. Main owns cross-build,
portable/owned-asset verification and integration; Astra inspected source only.

Final checks: x86 proxy/server and x64 host/official loader cross-build PASS;
all16 portable groups PASS; actual owned cap/reflective affine check PASS;
artifact/IPC and compiled native-finally checks PASS. No Windows program, native
callback, game, XR runtime or multiplayer session was executed. Existing0.2.11
archive is unchanged; this source follow-up is not a new optics package.
