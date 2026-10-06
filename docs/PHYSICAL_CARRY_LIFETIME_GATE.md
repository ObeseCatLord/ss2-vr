# Native manual-history lifetime gate

Fresh Astra/max investigation; effective current-turn model and effort verified.
Pinned Sam2Game/Engine/Core hashes match. Static inspection only, no edits or
runtime execution. Result: **NO-GO for native integration**. Main adopts the
post-store candidate and the lifetime objections below.

| Verified native boundary | ABI / consequence |
| --- | --- |
| Operator8E580, store8E75A | thiscall, ESI=this, plainret; original CL loads brain160 at8E750 |
| Post-store8E760 | Before popESI/popEBX/frame teardown; five complete bytes available, adjacent to existing six-byte history seam |
| Default constructorA3760, storeA41FF | EBX=this; zero348; returnthis/plainret |
| AssignmentCC30, storeD465 | EBX=source, EDI=destination; ret4; destination return |
| Copy constructorEA00, storeF29E | ESI=source, EBX=destination; ret4; destination return; this is not assignment |
| Immediate post-write candidates | A4206, D46B, F2A4; identity/metadata targets remain unproved |

Player-specific paths: F8DA0→62190→A3760 default construction,
285C0→281C0→CC30 assignment, 2A610→29FC0→EA00 copy construction.
Base copy routines also serve other puppet classes. The base-constructor vtable
does not identify the eventual player.

## Why existing copied records are insufficient

Authority has18 handle-keyed rows. Generic copy/save replaces the whole row,
so an older save could overwrite a newer native commit. Population excludes
single-player/non-server and unnegotiated samples; full capacity silently drops
a save. Negotiated peer capacity does not bound retained avatar/copy lifetimes.
The existing player deletion hook does not explicitly retire an Authority row.
Main spot-checked these source findings at engine.cpp334/353/1714.

Snapshot tracking/session/renderer resets are not native348 resets. Prepared
records belong to one simulation interval. Neither is the durable owner.

CBaseEntity::mdPostCopy451C0 (thiscall, two-word CMetaPointer, ret8) calls virtual
+104 then Engine58F00/wldAddEntity. The ordinary Core pipeline passes the
destination metapointer, not a demonstrated source/destination pair. Direct
assignment and copy construction need not invoke this callback.

Core hvPointerToHandleC450 is not a pure lookup: its missing-pointer path
allocates/registers a handle. It cannot be called by a scalar instruction helper,
and registration of construction targets at the348 stores is not established.

## Accepted coupling requirement; storage still undecided

Capture original CL&3 in the current operator invocation before projection.
Publish only at8E760, after the native store completes, into prebound stable
metadata. Nested commits advance durable history in actual store order. Native
unwind discards only uncommitted capture; it must not undo an inner commit.
Instruction helpers preserve registers/flags/FP state and perform no resolver,
callback, allocation or exception work. Adjacent linked hooks still need an ABI
check before use.

Prefer a narrow native-history subrecord within the existing Authority registry,
excluded from generic sample copy/save. Store two raw-index bits, explicit
known/unknown status and validated native lifetime identity. Tracking, weapon,
renderer and transport resets preserve it. A missing/unknown record is not zero.
A second registry does not solve copy provenance and would duplicate lifetime
and capacity policy; no current evidence justifies it.

Remaining exact proofs: registration before first projection; durable identity
through constructor/copy/delete/reuse; capacity/admission covering retained lives
and copy targets; source-to-target correspondence for direct and generic copies;
stale-save exclusion; stable metadata at post-store; portable nested/copy/reset/
unwind cases and final linked hook inspection. No candidate hook is enabled.

Later Astra/xhigh evidence adds native348 reset98B41 to the correspondence
requirements. Default constructorA41FF is not the only zero store. Any manual
history subrecord must follow the actual reset, not just allocation identity.
The proposed bits2/3/4 in-byte alternative is rejected because bit2 is native
grenade input; see PHYSICAL_NATIVE_DELIVERY_AND_BITS_GATE.md.
