# Scope image query effects — native boundary

Astra/xhigh read-only native investigation; effective routing verified locally
in aggregate. Conditional GO for a tracker-free ordinary-command query gate.
NO-GO for count irrelevance merely from preserved zero/nonzero visibility.
No edits, delegation, native execution or image transaction approved by this audit.

## Pinned ordinary gun route

Sam2Game4BF40 allocates a14-byte CRenCmd, vtable2A4720, weapon at10, flags8FF00.
Engine CRenCmd155740 appends to the current view's command list. Root Execute
155FF0 dispatches each command at15606E. Weapon command Execute4BDE0 calls weapon
Render virtual1F4 at4BF34/return4BF3A. Execute is void thiscall; weapon Render
takes the established by-value48-byte Matrix34 and pops it. Base render calls
mdlRenderModel4C9E3/4CA48; existing physical sniper deliberately uses this base.

This dedicated route bypasses CObjRenCmd::Execute, so ordinary Scope rendering
is outside object/flare/visibility-box occlusion brackets. Carry the exact entry
return fact in the existing physical invocation. No command registry or new hook
is needed. Existing root/thread/Scope ownership remains required.

## Count-based native consumers

Gfx60B0 Begin is void cdecl(LONG index), allocates OCCLUSION9 at60F4 and issues
BEGIN6105. End6130 issues END6167. GetResult6190 is BOOL cdecl(LONG,LONG&,BOOL),
requests four bytes61DB and copies only on S_OK.

| Consumer | Bracket/results | Effect |
|---|---|---|
| Object culling | Begin155177; End155389 proxy or155452 actual RenderObject1553FC; result154ABA/154ACD | Compare against ren_ctOcclusionPassThreshold2C8EB4, initial8/minimum1 |
| Visibility groups | Begin15FAD5–End15FCB0; result15F514/15F527 | Same count threshold before member addition |
| Flares | Begin158A1C–End158C55; count converted158D35/render158F4F/15954E | Scaled/clamped/divided ratio, not only positivity |

An enclosing count7 with threshold8 changes if an extra cap contributes one
passing sample. Queries count primitive samples, not unique screen locations.
Subset coverage of an already-visible cap does not prove these effects unchanged.
[Microsoft query types](https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dquerytype).

Gfx CreateQuery-slot census has five sites: support probes3FA7/3FF0,
synchronization5ED7/5EF6 and occlusion60F4. Resolved types are EVENT8 and OCCLUSION9;
no timestamp/vertex-statistics/pipeline-timing creation was found in this path.
gfxFinish5E90 prefers EVENT; fallback occlusion BEGIN/END5F08/5F17 has no intervening
draw and polls GetData(null,0,FLUSH) for completion, not count. Extra prior GPU work
can change waiting time, not a count-derived rendering decision at this seam.
[Microsoft lifecycle](https://learn.microsoft.com/en-us/windows/win32/direct3d9/queries).

## Conditional executable gate and limits

Existing owned root/physical Scope admission, exact original weapon-command
entry return Sam4BF3A, and native current-query index Engine2C3F34==-1. Index
starts-1; Begin writes8E795 and End resets8DCF9, the only found relocated writers.
Read at actual candidate admission before a split, not once per frame. Failure
declines optional color/image admission while preserving geometry/native draw.

The scalar is bookkeeping, not an actual GPU getter. Gfx discards Issue HRESULTs;
failed Issue, arbitrary nested brackets and foreign issuers are not covered by an
all-GPU-query-inactive proof. Conditional GO relies on the pinned ordinary command
order and separate transaction containment. No registry, Begin/End hooks, forced
End, polling or result compensation is justified by the established route.
[Issue contract](https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3dquery9-issue).

Current source applies this narrow gate to both pre-draw color observations using
existing weapon/sniper wrappers and invocation. It does not establish uninterrupted
device state through the eventual DIP, approve an image draw, or claim runtime
coverage. Actual ordered-image transaction/source ownership remain unfinished.
Bounded Astra/xhigh SOURCE GO: wrappers preserve their existing native by-value
ABI, caller capture occurs at the actual wrapper, the primitive origin flag uses
existing invocation/suppression lifetime and query read follows currentScopeDraw.
Both color snapshots require this gate; geometry and original forwarding remain
independent. No extra immediate reread is required for the snapshot contract.
Full client/server/host cross-build and all21 portable groups, artifact and
compiled DIP/native-finally checks pass. Source/product fingerprints are recorded
in scope-query-source-checks.json. This is still observation-only SOURCE GO;
no image/failed-Issue recovery or runtime claim. No query dump is committed.

SHA256: Engine da6efc9f72637eb3b6f48eadca2107be89b09c00618b6e72d5d3632938a7d851;
Gfx88749b79be36f0c0dccb623c4f1712685b5af603b451e25ed3c5029bedcdf3ed;
Sam5628b4ed30a966f10c8e8ea46bf0789257a35ea80127bacacebe28b1ce5303df.
