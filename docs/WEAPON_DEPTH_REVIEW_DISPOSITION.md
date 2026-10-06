# Weapon depth review disposition

Fresh reviewer selected and independently verified as gpt-6-astra/xhigh. Read-only Astra review of baseline24d3c2a plus exact owned binaries; no runtime execution. The earlier resumed reviewer became Sol/high and declined to review; its output is not a design verdict. Main spotchecked the decisive Engine155CA6/155DB0/155DCB instruction chain.

| Astra finding/recommendation | Main disposition |
|---|---|
| Prepared root projection differs from executed projection after native clip-distance update155CA6 | Adopted. Capture installed raw P/view/depth at root initial ActivateExecution depth-return155DD1, inside existing scoped root Execute. Keep Prepare for provenance. |
| Keep original Matrix34 input for authored gun placement; replace inverse result4C915 instead | Adopted. Preserve original native GetWeaponAbsPlacement input, model/hand animation, stretch and damage. |
| GfxD3D native P callback69A0 handles raw/adjusted/backend state | Adopted. No adjusted-P substitution, device-only viewport override or replacement shader math. |
| Broad renderingWeapon context leaks into unowned nested render | Adopted. Every wrapper creates its own bounded context/suppression barrier; restore on unwind. |
| Frozen hand placement must share projection/view/depth admission | Adopted. Exact native placement caller, immutable request tracking and finite successful retargeting; failed placement returns0 to prevent model draw and faults pair. Final current freshness remains at commit. |
| Native placement failure skips P/depth restoration | Adopted. Save entry raw P/depth; observe native restoration, use native callbacks/cache invalidation for missing restore, suspend substitutions during cleanup. |
| Pair fault must survive endEye and desktop restoration | Adopted. Reset only at beginStereo, fold into existing Ready eligibility. No IPC/protocol change. |
| Pin GfxD3D and retain headless exclusion/actual callback identity | Adopted. Two graphics hooks only; native modules remain architecture. |
| Cache equality cannot prove actual GPU restoration | Retained limitation. Do not claim runtime/GPU/wall-occlusion verification from offline checks. |

Astra conditional design GO requires these changes and a source follow-up. Native execution evidence: Prepare156370 copies view+14/P+44/depth+90/+94; ActivateExecution155A50 optionally updates Z clip at155CA6, sets view155CFF, installs raw P155DB0 and depth155DCB; dispatch follows15606E. Gun4C740..4CA9C retains original by-value camera, uses inverse hidden result at4C915, raw P4C8E9, depth4C9A6 and native restore4CA93. Native depth56A0 returns57AF and ignores device HRESULTs. This artifact is a normalized record of the review, not a claim of complete optics.

## Source review closure

Astra source review found stage0 fault→native graphics fallback→placement rejection could bypass cleanup. Adopted explicit graphics-setup responsibility separate from successful stages, including callback-mismatch setup, and a fail-before-setup regression plus untouched-return converse. Tightened ABI proof to require exact destination stores/no intervening arithmetic. Bounded Astra follow-up gives GO for both fixes. Main final cross-build, all10offlinegroups,35exportedhooks/5internalentries, IPC6 equality and strengthened compiled ABI checks pass. No runtime was executed; native GPU restoration/wall appearance and full optics remain unverified.
