# Opaque scope image pass — Astra disposition

Astra/xhigh verified-then-critiqued the native alpha evidence and narrow candidate.
SOURCE GO for the two disabled preparation files; late full928→RGB22 overlay
NO-GO. Main adopts the architecture correction. No native runtime or state binding.

| Finding | Main disposition |
|---|---|
| Depth equality does not identify the cap's winning samples | Reject late overlay; no bias/epsilon workaround |
| Native LESS cap may fail against equal depth while later EQUAL overlay paints | Original LESSEQUAL required for the narrower ordered candidate |
| Final five triangles can win equal stored depth and then be overwritten by late cap | Preserve suffix precedence with native901→native22→RGB22→native5 candidate |
| RGB-only image substitution alone loses original cap alpha | Retain native22 first; additional pass masks alpha/no depth writes |
| Whole VS fingerprint unnecessary when actual native VS/inputs/constants stay stable | Keep current UV predicate and actual retained VS; no lighting/skin replacement |
| Native ordinary family0/1 source has no kill/depth/MRT | Require actual retained PS plus proved current bank-record/source association; cached handle insufficient |
| MSAA coverage/depth is per sample | Require unchanged samples/mask, raster/clip/scissor/viewport/bias, inputs/constants; no equality ownership assumption |
| Extra pass changes occlusion sample counts | Native query absence/irrelevance remains a gate; no new query manager |
| Reentrant draw forwarding is not exclusion | Existing busy flag alone insufficient; no intervening draw/clear/target/resolve/transfer |
| Capture actual state/resource admission before first mutation | Existing TLS/finally owner, attempted-write bookkeeping, restore touched actual state, reject whole pair before publication on uncertainty |
| Root material/source alias and color transfer remain unknown | No image enablement/default assumption |
| Tool output scan allowed hypothetical oDepth/oC1+ | Added explicit only-oC0 predicate; both entries compile/audit pass |
| Opening alpha comment incorrectly limited COLOR0 computation to PP | Corrected: both VS families produce it; PP2 consumes TEXCOORD2.w instead |

The ordered candidate changes the earlier requirement that original full928
complete before any image pass. Main accepts reopening that requirement: native
prefix/cap/RGB/suffix preserves order and alpha but must admit the complete
transaction before prefix, never replay full-DIP after partial work, and reject
uncertain execution/restoration before publication. It remains conditional;
no native rendering adapter or source capture has been implemented.

Static native no-kill/depth/MRT evidence: GPU-Programs.asm1091–1101,1116–1139,
1158–1171,1189–1219. Shaders7F44–7F52 chooses PS handle35EB8; Gfx6FCA–6FE6
resolves the COM object, with unchecked cache update7068. Actual PS/source-bank
association, current material/config lineage and root COLOR identity are still
needed. Prior native fingerprints are in RESEARCH.md/SCOPE_NATIVE_ALPHA_AUDIT.md.

Primary contracts: [depth comparisons](https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dcmpfunc),
[per-sample depth](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx9-graphics-reference-asm-ps-registers-output-depth),
[sample/raster render states](https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3drenderstatetype),
[occlusion queries](https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dquerytype).

The shared RGB helper preserves image lookup; opaque needs s0/t3 and no color or
diffuse-alpha input. PP-only main needs s0,s3/t3/v0. Both compile with native
Linux vkd3d to PS2-extended,146/151slots respectively,4temps, only oC0 output.
Neither is linked or enabled; contentVerified stays false. Exact preparation
fingerprints and offline compiler evidence belong in scope-opaque-source-checks.json.
