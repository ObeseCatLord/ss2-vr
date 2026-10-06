# Native UI integration: Astra amendment and native cleanup boundary

Baseline9a76324. Explicit gpt-6-astra/xhigh reviewer Russell the2nd; main
independently verified effective settings for all five reviewer turns. Reviewer is closed. Read-only architecture/source review;
main owns all edits. No game, host, Wine, XR/server/network process or native
exception was executed. This is preparation for complete UI, not an enabled
GPU/overlay/Ready adapter. IPC7/wire6 and immutable0.2.8 remain unchanged.

## Consolidated amendment critique and disposition

Faithful normalized record: conditional design GO for the minimal native
immediate-draw/once-owner/pair adapter; NO-GO for claiming complete native UI
before these integration conditions are met.

| Astra recommendation | Main disposition |
|---|---|
| Close the output domain | Adopt. Two native draw families remain the duplication reference; fingerprint-bound evidence or scoped rejection observes unsupported desktop-affecting writes. Existing offscreen writes stay native; eventual desktop composition must be admitted. Empty callback completion is not a successful-draw counter. |
| Enclosing owner and exact world admission | Adopt. FEA8A currently selects only marker postlude, not stereo admission. Require exactFEA8A for complete UI under the original brain/overlayflag0/callerEB85D/same resolved player. Preserve existing world-only behavior elsewhere. Missing/nested/changed/partial ownership permanently faults that pending pair. |
| Restore failure is more than pair rejection | Adopt. Attempt every restore independently; target/depth before viewport AND scissor. Unresolved restore failure halts adaptation for that device generation; no claim of desktop recovery without a proved native resynchronization or reset. |
| Singlethread restricted device | Adopt. Record actual successful creation identity/thread, not laterdeviceReady/Reset. Query !PURE/!MULTITHREADED and six clip planes. Native main-thread/owner/resource generation/recursion checks are still required; absence of MULTITHREADED does not detect a client violating its contract. No parallel locking layer. |
| Pre-UI dimming with existing readback | Adopt. Writable pitched rows retain existing RGB lookup quantization, preserve native alpha until final opaque export, unlock before matching UpdateSurface into existing eye RT; skip passthrough. Final RGB readback is after UI and not dimmed again. No new shader/target. Cost/deadline viability unknown. |
| Presented-pair stats suppression | Adopt. Response metadata separate from immutable request; cache only after both uploads/releases; suppress stats only for final surviving complete world pair, including reuse and revalidation after waits. One existing comfort anchor, frozen full-canvas aspect/pose, ABI8 when connected. |
| Delete shader token parser | Adopt. Corrected native assembly route and all six direct position statements close the semantic gate with exactlivebuiltinobject/fingerprint/currentconstant forwarding. Declaration/streamv0 still requires actual admission. No parser retained. |
| Consider smaller local state capture | Adopt. Only raw state actually modified by duplicate draws needs capture/restore; untouched shaders/resources/colors/blends do not justify D3DSBT_ALL. Reject active MRT instead of expanding support. Concrete draw source remains pending. |

## Main verification and compiler-route correction

A bounded Terra investigation initially mislabeled the builtin path as profile-
selected HLSL compilation. Main spotted the compiler branch mismatch and required
correction, then independently checked original branch/assembly thunk. Gfx header
checks branch72BB/72D5/72EF to76FA;76FE invokes6CC0, whose6DAA invokes thunkAFA2
D3DXAssembleShader. D3DXCompileShader7698 belongs only to nonassembly fallthrough.
Engine parser removesCT and may upgrade assembly1.1→2.0; builtin position
m4x4oPos,v0,c1 is retained. Original four-vector constant forwarding is already
proved. Astra independently verified all six builtin position statements and
approved deleting the proposed GetFunction/token-parser machinery.

New production preparation checks both current native handles against one live
builtin pair and both device objects at record0, under bounded readable array
and native-main-thread admission. Future caller must hold successfully queried
VS/PS references through the transaction and prove device/lifetime/declaration.
No native record pointers are cached. The root execution capture now also copies
actual adjusted eye P after155DCB's original depth call returns155DD1, before
marker ortho/root retirement; getter validates actual eye/request/thread/context.
Device creation now records identity/thread and supplies a queried restricted
component gate; it is not a whole frame/resource-lifetime admission.

## Demonstrated unwind incompatibility and minimal boundary

The installed GNU x86 compiler uses DWARF2 exception handling. Native game
functions use Microsoft x86 SEH. Ordinary GNU catch/RAII does not establish
cleanup on native unwinding. Main reopened this implementation boundary.

A GNU-target Clang SEH probe failed and is not evidence. The chosen single
native_finally.c uses Clang22.1.8 targeting i686-pc-windows-msvc, without a Windows
SDK or C++ runtime change. It emits the original Windows finally mechanism:
FS:[0] registration, legacy_except_handler3,12-byte scope table and unwind
funclet. Normalcleanup receives0 before unregister/return; unwindcleanup receives1
from the saved callback/context. No filter consumes/converts the native event.
Existing C++ and all adjacent engine systems remain on MinGW.
The cleanup/search relationship and process-termination limit follow Microsoft's
[try-finally contract](https://learn.microsoft.com/en-us/cpp/cpp/try-finally-statement?view=msvc-170).
The legacy image admission is separate from Microsoft's
[SafeSEH handler table](https://learn.microsoft.com/en-us/cpp/build/reference/safeseh-image-has-safe-exception-handlers?view=msvc-170);
this bridge does not manufacture a SafeSEH table for the combined image.


Astra gave conditional design GO for this boundary. Main adopted the following
corrections before bounded source review:

| Finding | Main disposition |
|---|---|
| Deferred GNU rethrow could cross native frames | Delete rethrow/exception_ptr. run contains direct GNU errors to a bool; cleanup sees aborted; withNativeFinally returnsfalse/noexcept. Reentered mod callbacks still must contain GNU failures locally before intervening native frames. |
| MSVC callback stack may be only4-byte aligned | Local force_align_arg_pointer on run ANDfinish; compile-only witnesses force16-byte work storage in both and inspect emitted alignment. No global flag change. |
| Above-fence caller destructors may also be skipped | Only trivially destructible closure types admitted; context contains non-owning references/bool. Document explicit persistent resource/lock/TLS ownership, no pointers into inner unwound frames, publication before nativeentry, idempotent bounded allocation-free abort. The restriction does not prove future body-local resource coverage. |
| Only unwind crossing fence invokesfinally | Document first-chance/continued events/process termination limits; inner resource-owning reentry needs its own boundary or a proved exclusion. No inference about adjacent gun/engine adapters. |
| GNU ld does not process SafeSEH symbol-index metadata | Strip ONLY.sxdata in the narrow object step. Retain actual.text/.xdata scope/handler/funclet relocations. Verify both final legacy images have no.sxdata, NO_SEH or SafeSEH table. Reopen if SafeSEH introduced. |
| Import library does not prove runtimeprovider availability | Verify exact installed ProtonHotfix32-bit API-set mapping/export below; document conditional availability, not universalWindows/Wine or msvcrt-only support. |
| Cleanup noexcept is not fault recovery | Keep aborted cleanup constrained: fault/clearTLS/deferredidentity/haltuntilreset and explicit raw-device restore only. No native callbacks/resynchronizers during unwind. Actual resource ownership and recovery remain future adapter gates. |

The installed ProtonHotfix i386 apisetschema.dll has namespaceV6 and default
api-ms-win-crt-private-l1-1-0→ucrtbase.dll mapping. Its schema SHA256 is
2c60fb0f224970a62cdd96d47dcdf30cfdc338035d6330c05542e85ff242e549.
The same installed i386 ucrtbase.dll SHA256 is
a7618cf891f9d2eabba705b34e6c2a84aad9aa6b311d198b1af96885d9e8c7c3;
_except_handler3 is a nonforwarded exportRVA24E10. Its original body begins a
real exception-handler implementation and tests unwind flags6. This proves
local schema/export availability, not actual prefix routing, runtime unwind or
universal support. No runtime file copied/distributed or modified.

## Offline evidence and current limits

Full x86 proxy/server plus x64 host/official-loader cross-build and all14 offline
groups pass. Actual compiled boundary verifier checks scope/handler relocation,
FS install/remove, saved callback/context, normal0/unwind1 arguments, cdecl,
legacy final images/import and GNU catch/local-alignment fixture. The fixture
is never linked as an executable or run; symbol checks alone are not full unwind
proof. Main manually inspected actual emitted GNU callback capture/catch/return
and C finally frame/funclet, plus final-image linking. Native exceptions, GPU
rasterization, ownership callbacks, appearance and performance were not tested.

The new boundary and native gates are not connected to new UI callbacks or GPU
draws. Complete UI still requires closed output domain, persistent ownership,
inner callback containment, restore-failure policy, pre-UI alpha-correct dimming,
comfort request geometry and final presented-pair metadata. Roomscale collision,
scopes/native zoom, physical swing melee and complete remote-head enablement
remain active requirements; teleport remains excluded. No new package/IPC/wire.


## Final bounded source verdict

Faithful normalized terminal verdict: **SOURCE GO affirmed for the native-finally
boundary and capture/builtin/device preparation; no blocking findings.** Astra
checked actual source and strengthened linked-code/scope/funclet/IAT identity in
both products. It independently found all four required HIGHLOW base relocations
in both final DLLs, then recommended adding those to automation. Main adopted
checks for scope pointer, handler pointer, scopefunclet pointer and thunkIAT
pointer; the updated verifier passes without changing production source.

This source GO is deliberately bounded. Future UI owner/draw integration remains
NO-GO until the output-domain, ownership, restore and presented-pair conditions
above close. Main spot-checked context lifetime, direct GNU containment/no
rethrow, local callback alignment, static program records and actual root adjusted
projection timing. native-ui-boundary-source-checks.json records current hashes
and exact evidence scope. No enabled native UI or runtime verification is implied.


## Next closed-output proof

A main-owned private candidate scan finds the already-proved immediate calls
9BB7/9B82/A00B and candidate Clear602A/687E/68B8 plus StretchRect1373/15A7/8670.
Bounded receiver inspection confirms the latter clear/copy candidates use the
original Gfxdevice18F0. Absence of UP/patch candidates in that scan is **not**
full coverage proof: arbitrary-start/indirect-register and other module paths
are not closed. Use narrow rejection-only observation where proof is missing;
duplicate only the two admitted draw families. This evidence does not enable
complete-UI metadata, omit unsupported output or justify another renderer.


## Later declaration correction

Connected integration rechecked the parser prefix gate: optional1.1→2.0 upgrade
is specifically ps.1. (pixel), not a VS1.1 upgrade. All six builtin vertex headers
remainVS1.1; queried POSITION0 maps to legacy register0. Earlier version wording
above was incorrect. The original m4x4/c1..4/assembler proof is unchanged.
See NATIVE_FLAT_UI_CONNECTED_BRIEF.md for exact verified branch/import facts.
