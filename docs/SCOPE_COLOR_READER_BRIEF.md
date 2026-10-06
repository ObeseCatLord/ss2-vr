# Scope color reader — architecture comparison brief

Baseline5c4fe50; actual geometry/UV/VS predicate/rows and disabled opaque image
shader are accepted. Native rendering/query/resource/scene-image integration is
unfinished. Full objective, no teleport, multiplayer and no native runtime
testing remain unchanged. Astra/xhigh reviews; main owns integration.

Verified fact table and native scope ABIs are SCOPE_COLOR_READER_NATIVE_AUDIT.md.
Do not repeat that investigation or infer live compatibility from source filenames.

## Compare minimal adapters before adding hooks

A: three preset/shaExecuteShader/PolyBump scopes with TLS nesting, current override/
config/effective-args association, actual retained eye target and actual PS-bank
association. Benefits: demonstrates ordinary model/material lineage and excludes
traced forced/visualization/modifier paths. Cost: new native seams, transient
ownership/cleanup and override traversal; native args need no image-alpha reconstruction
for the opaque ordered candidate. No second bank/registry/protocol is justified.

B: existing currentScopeRaster plus strict actual retained-eye target/depth/full
appropriate viewport, actual opaque RGB raster state, forced/visualization globals
disabled, actual PS association and a complete narrow returned-PS predicate proving
no kill/depth/MRT. Retain actual VS and inputs/constants for originalcap→RGBcap.
Benefits: observes inputs used by the real draw rather than duplicates material
selection. Open question: can it admit a wrong user-observable purpose despite
exact current physical Scope/root/color ownership? Do not assume B sufficient.

Source-bank handles/module pinning show provenance but may not certify actual
loaded program bytes if source overrides/recompilation occur. Main asks Astra
whether actual PS tokens are necessary in either design, rather than assume the
stock GPU-Programs.asm on disk equals the loaded source. Likewise actual opaque
states/geometry may not exclude every debug/material pass; identify concrete
counterexamples before implementing the larger scope layer.

Both preserve native901/native22/RGB22/native5 order, original cap alpha, native
VS/animation/simulation/caches and current frame/bank lifetime. Neither authorizes
source capture or image draw by itself. Complete resource/state admission precedes
first prefix; uncertain execution/restoration rejects existing pair before publish.

## Review contract

Verify-then-critique and seek deletion. Return <=1100words, minimal conditional
GO/NO-GO, concrete counterexamples requiring A versus B, exact native/file/primary
evidence and smallest source vertical. Separate actual getter algorithm correctness
from unobserved runtime coverage. Do not require unauthorized game testing; actual
runtime getter predicates can implement live admission, but no empirical claims.
No edits/delegation, protocol/renderer rewrite or scene-content/query investigation
in this review. Other Astra owns native firing/physical-melee architecture.

Current read contract is investigation evidence only, not implementation approval.
