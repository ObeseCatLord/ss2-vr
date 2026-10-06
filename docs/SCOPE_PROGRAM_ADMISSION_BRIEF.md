# Actual scope UV program admission — implementation brief

Solo-user port, full immersive goal active. Teleport and native runtime testing
excluded; multiplayer required. Astra/xhigh owns review/investigation, main owns
all edits/integration. Main extends the existing live-DIP reader; another Astra
investigation owns disjoint physical collision APIs.

| Fact | Evidence/status |
|---|---|
| Baseline | [verified]9a2e364, clean worktree; code8029af5 accepted by Astra/all19 offline groups |
| Native UV stream | [verified]existing scopeBufferRanges and reader admit stream3 FLOAT2/TEXCOORD3, five actual copied/hash-checked slices |
| Native UV outputs | [verified source/native path]SCOPE_NATIVE_COLOR_ADMISSION.md; dcl_texcoord3 v3 and dp4 oT3.x/y v3 c8/c9, unchanged across25 source/weight families |
| Binary token contract | [verified static]Astra checked VS1.1 DCL/operand encoding against installed assembler and primary SDK references; actual native GetFunction bytes remain unobserved |
| Native cached handles/constants | [verified]native setters ignore HRESULT; actual successful getters are necessary |
| Observation ownership | [verified]scope_gpu.cpp TLS owner above NativeFinally; currentScopeRaster/fresh original-draw success; ScopePoseObservation existing per-eye bank |
| Current image consumer | [verified]experimental HLSL is neither linked nor enabled; contentVerifiedfalse |

Minimal extension: retain actual GetVertexShader reference and canonical identity
in both existing binding snapshots; actual finite GetVertexShaderConstantF(8,2).
Before locking any native buffer, use bounded GetFunction (size query then exact
owned copy) and a complete conservative UV-interface recognizer. Same canonical
shader identity after copying certifies the same immutable object; rows must also
match. All references release before existing content hashing. UV slice Lock
still uses position-owned VB0 and unchanged uncertain-reference cleanup.

Derive image coordinates from current copied cap + exact admitted c8/c9 and freshly
revalidated affine in recordScopeGeometry after successful original DIP. Append
one derived imageCoordinates to existing ScopePoseObservation; clear it with
existing rejection/retirement/reset. Require consistency on repeated admitted
cap observations. No borrowed object/token pointer survives owner, no second
bank/cache/generation/recovery/UV-presence flag or hook. Unknown programs/Getters
or invalid mapping decline observation and forward ordinary original draw once.
Fatal native Lock/Unlock containment remains unchanged.

Alternative of trusting native cached bank handles is rejected by demonstrated
unchecked setters. A new shader registry/cache is unnecessary; actual immutable
COM shader + current device values meet this input boundary. Broad renderer/VS
replacement is unnecessary and would lose native skin/alpha. Decoder should
reject unknown/control-flow/malformed tokens, mismatched declarations, modified
or extra oT3 writes, c8/c9 definitions/input writes. Astra verified the supported
token contract against primary headers/docs and the installed assembler;
source-family enumeration is not exact bytecode identity or observed native
GetFunction compatibility.

This establishes only raw input / native UV output relation and derived math.
It does not certify native position/skin/alpha, selected material, root COLOR,
source image or hardware arithmetic. All remain gates before image enablement.
The experimental shader stays unconnected and contentVerified stays false.
No new IPC/wire/gameplay state or installed files changes.

Smallest proof: production decoder malformed/semantic counterexamples; actual
owned Linux-assembled mod-owned UV fixture where possible (not proprietary
shader fixtures); actual getter/identity/row lifetime static source/compiled ABI;
existing owned cap map + common bank no-leak/reset checks; full x86/x64 build/
portable/owned/artifact/native-finally checks. No native COM/GPU/game execution.

Review contract: verify-then-critique within this extension; challenge whether
DCL source survives GetFunction, whether the binary predicate is proved, and any
unnecessary state. No broader transport/physics/UI or image-enable review.
