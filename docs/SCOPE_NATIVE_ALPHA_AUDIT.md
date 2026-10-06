# Native scope alpha — Astra investigation

Astra/xhigh read-only static investigation, effective routing verified locally.
No native execution, edits or delegation. Modules are the owned pinned build in
RESEARCH.md. This evidence narrows the image plan; it does not enable imagery.

GPU-Programs.asm988–1016 computes spherical-harmonic normal lighting from c12–20,
multiplies RGB by c5.rgb, and sets alpha to c5.w times the squared saturated
clipZ*c11.y+c11.w. It writes the same vector to COLOR0 and TEXCOORD2. Fog output
is clipW*c11.x+c11.z. Shaders8148–81DC converts effective packed color to c5 by
1/255;8CA1–8CD8 obtains fog factors and uploads c1–20. Actual GPU values remain
unobserved; unchecked uploads cannot certify them.

Effective args+48 passes through helper6E10 at7D11–7D23. For ordinary blend500,
alpha test disabled and partial fade,6EDF–6F07 changes blend to501 and quantizes
255*fade into color alpha. Serialized material alpha alone is insufficient.

| Native family | Final alpha input |
|---|---|
| PP bumpOnly/bumpSpec | sampled diffuse alpha * COLOR0.a |
| PP2 bumpOnly/bumpSpec | sampled diffuse alpha * TEXCOORD2.w |

Evidence: GPU-Programs.asm1100/1170 and1137/1217. Input COLOR precision differs
from texture-coordinate precision under the [D3D9 register contract](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx9-graphics-reference-asm-ps-registers-input-color).
The earlier experimental image shader always consumed COLOR0 and therefore
cannot claim an exact replacement for PP2. It remains disabled and is now
explicitly labeled PP-only. Retain native VS1.1 and PS2-compatible replacement;
[SM3 pairing rules](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/shader-model-3)
exclude simply moving the replacement to ps_3_0.

## Material and actual binding

Ordinary preset dispatch Engine78147/return7814C, then effective post-modifier
dispatch77FE5/return77FE8→Shaders7CA0. The final target alone does not identify
ordinary rendering; selected Scope preset/config association and root COLOR
ownership remain required. Reject overrides/modifiers/fallback/unmatched nesting.

Effective argument fields: blend18,depth comparison20,alpha test28,double-sided38,
constant color48,diffuse texture58,diffuse UV identifier60,height textureD0.
Shaders7FB3–7FBA binds diffuse stage3; missing texture takes fallback. Initially
exclude fallback, double-sided multipass and offset/parallax variants.

Native PS handles occupy Shaders35EB8+4*family; initialization9090–90ED associates
families0/1 with bumpOnly/bumpSpec. Gfx6FCA–6FE6 resolves through Engine2E6580
(base+4,count+8,12-byte records,shader pointer+0); setter is unchecked. Successful
actual GetPixelShader/canonical identity matched to the bounded associated
record, or a proved narrow returned PS predicate, is required. Successful actual
GetTexture3 must be a2D texture with unchanged stage3 sampling. No shader registry
is necessary. Retaining native VS plus the original PS's exact alpha interpolant
can avoid whole-VS lighting/skinning identity proof for a later alpha path.

## Proposed smaller first image slice

Astra recommends one additional cap-only RGB pass after successful original full
draw, rather than replacing the full DIP or reconstructing fade alpha. Initially
require alpha testing, blending, stencil and fog disabled, valid depth test/write,
known ordinary family0/1, owned root COLOR target and independently admitted scene
image. Keep native VS/geometry/streams/indices, mask alpha writes, use depth-equal
without depth writes, restore touched actual state. Original target alpha stays
intact. Main seeks senior review of this narrower architecture before enabling.

Live root COLOR association, material admission and scene image ownership remain
unknown; geometry/ordinary shader/color-enabled target cannot substitute for them.
contentVerified stays false. Runtime precision and visual correctness unverified.
