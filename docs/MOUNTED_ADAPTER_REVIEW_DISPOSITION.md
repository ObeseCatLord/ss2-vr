# Mounted adapter implementation disposition

Main spot-checked Astra's `gpt-6-astra/xhigh` source/design findings in MOUNTED_ADAPTER_DESIGN_REVIEW.md, including remote anchor reconstruction, cached input, authoritative equip/dual toggles and the non-eye camera branch. Effective settings were independently verified. Final architecture is conditional GO, requiring the implementation below and a separate source review. No vehicle feature is completed by this record.

| Recommendation | Main disposition |
|---|---|
| One mount/input transaction across command and clamp use | Adopted. Extend existing Snapshot/control provenance with live rider/seat identity and captured tracking/origin/turn; no new pose bank. Reset existing rig/gates/calibration/generations on transitions. Preserve compatible captured input rather than relabelling it with newer poses. |
| Separate mounted native fire from handheld epochs | Adopted. Reuse TriggerGate/raw action provenance for native vehicle control commands; on-foot Pending/ACK checks remain. Zero player-weapon network/equip intent and enforce the live native mode again on server and at use. Actual vehicle weapons stay native. |
| Server/listen/dedicated authority must suppress handheld mutation | Adopted. Gate equip/dual changes, fire, calibration and muzzle adaptation by the freshly resolved rider mode, including after native callbacks and freeze. Existing both-hand invalidation clears copied intent/selection on mode changes while tracking replication continues. |
| Share mounted anchor with remote presentation | Adopted; narrow scope extension to remote_render.cpp. One borrowed native anchor helper is used by local/authority/remote consumers. Existing binding/frozen-pair guards include rider identity and suppress handheld retargeting while mounted. No wire field or transport change. |
| Reuse native body pose; avoid attachment queries/caches | Adopted. Preserve native eye position/view height; use native rider body orientation when seated. Reject unset/degenerate/changed native identities before/after getters. |
| Root-only first-person avatar collection | Adopted. Override only native943B6-origin query in the admitted mounted local eye, leavingFDA0B gun injection and non-eye cameras native. No558 write or mesh rewrite. |
| Remove class maps and bit4 admission | Adopted. Native seated identity/owned brain and original clamp/dispatch suffice; native runtime selects physics/control route. Neither known-class aliases nor bit4 initialization becomes mod policy. |

Two caller-scoped exported hooks remain the intended view/control addition. Host, bridge, IPC7/wire6, native rendering/animation/physics/inventory/RPC and optional remote-head policy remain reusable. Vehicle-mounted aiming lasers are a separate use of the existing collision-query phase; they are required follow-up and not implied by this adapter. No runtime testing is authorized.
