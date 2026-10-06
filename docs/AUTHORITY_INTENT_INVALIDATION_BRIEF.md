# Astra authority copied-intent cancellation source review

One narrow source correction in engine.cpp authorityStep/authorityAfterStep/invalidWeapon: replace four affected-hand fireMask clears with the existing Astra-approved network::invalidateWeaponIntents(value.sample.pose,mask), already used on native peer frozen/pending/active records. No new policy/API/state.

Reason: the native Authority value copies the frozen pose before invalidation of the peer records; clearing only fireMask leaves historical pulse/zoom fields in this local copied sample. Future zoom admission must not inherit them after replacement/equip/deletion. The same existing helper clears affected fire/pulse/desiredzoom/pulsezoom and preserves raw trigger/release evidence plus the opposite hand. All current producer zoom values remain zero; no zoom/input enablement here.

Read-only exact four call sites and shared helper, <=500words GO/NO-GO. Existing network helper regressions cover partial hand cancellation and physical evidence preservation; affected x86 proxy/server builds passed. No runtime/agents/edits. This review is separate from native zoom neutral admission design.
