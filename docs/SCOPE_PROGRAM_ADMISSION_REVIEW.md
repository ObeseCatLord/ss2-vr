# Scope UV program admission — Astra disposition

Astra/xhigh design GO and bounded SOURCE GO. Effective profiles were checked
locally in aggregate; no telemetry was exported. Main owns integration. No game,
Windows code, native COM, XR runtime or network session executed.

The actual-DIP reader now owns the actual vertex shader, bounded GetFunction
copy and finite c8/c9 rows in its existing TLS owner. A complete conservative
VS1.1 decoder establishes only the UV interface: unique TEXCOORD3/v3 declaration,
one unmodified DP4 each to oT3.x/y from v3/c8-c9, and no other UV write or c8-c9
definition. Unknown instruction encodings decline. COMMENT and DEF payloads
are opaque, relative constants have no extra VS1.1 address word, and the final
END must consume the entire bounded stream.

Both binding snapshots retain canonical shader identity and exact row bytes.
Rows copy to separate owned storage before snapshot release. GetFunction finishes
before native buffer locks; all references release before five-slice hashing.
Publication follows successful original DIP and fresh raster identity/affine
validation. Derived coordinates use the existing per-eye bank and clear with
geometry retirement. No extra hook, registry, cache, generation or IPC field.

| Astra finding | Main disposition |
|---|---|
| Complete token boundaries and exact UV writes are sufficient for UV-only proof | Adopted; explicitly not whole-program/material/image validation |
| Actual successful getters are required because native setters ignore HRESULT | Adopted in both binding snapshots |
| Preserve existing lock uncertainty containment and native forwarding | Adopted without new cleanup policy |
| Default-F matrix and scalar raster masks lacked direct examples | Added positive examples and oT3 rejection counterexamples; checks pass |
| HLSL fixture rejection label overclaimed independently proved cause | Renamed compiled_hlsl_fixture_rejected; exact direct-fixture disassembly remains independently checked |
| Repeated-map conflict and both production retirement branches lack independently executed tests | Source reviewed; portable bank tests do not claim native-path execution |
| Native GetFunction families/material/skin/alpha/source image remain unobserved or unadmitted | No image enablement; contentVerified remains false |

Reproducible read-only assembler evidence: relative owned module
`../Bin/d3dx9_25.dll`, SHA256
`4c54df27ce84d21b2924e64ff79b13e7876ce85d8e0c9c1d0abd8da73888187a`.
D3DXAssembleShader entry RVA EB374; VS1.1 internal mode F41C5; declaration
emission F1D13; mode0 omits instruction lengths F1CCA–F1CEA. Static encoding
evidence and mod-owned Linux fixtures support the predicate, not observed native
compatibility or hardware precision.

Primary contracts: [VS declarations](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dcl-usage-input-register---vs),
[relative addressing](https://learn.microsoft.com/en-us/windows-hardware/drivers/display/shader-relative-addressing),
[GetFunction](https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3dvertexshader9-getfunction),
[instruction tokens](https://learn.microsoft.com/en-us/windows-hardware/drivers/display/instruction-token),
[COMMENT tokens](https://learn.microsoft.com/en-us/windows-hardware/drivers/display/comment-token),
[DP4](https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dp4---vs),
[SDK register/opcode definitions](https://raw.githubusercontent.com/microsoft/win32metadata/main/generation/WinSDK/RecompiledIdlHeaders/shared/d3d9types.h).

Full x86/x64 build, 20 portable groups, mod-owned token fixtures, owned cap slices,
artifact/IPC layout and compiled DIP/native-finally checks pass. Source/product
fingerprints are in scope-program-source-checks.json. Magnified source capture,
material/COLOR-alpha admission and cap image substitution remain required.
