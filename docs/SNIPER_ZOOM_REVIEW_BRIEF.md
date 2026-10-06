# Independent native sniper zoom: Astra design brief

## Goal and scope
Solo user's fingerprint-bound native OpenXR mod. Deliver independent hold-to-zoom for each owned sniper, including native multiplayer damage authority, without replacing native zoom timing/progress/events. Actual game/headset/network execution excluded by user. All reviews Astra. This is a design review; no source changes authorized to reviewer.

## Environment facts
| Fact | Evidence/status |
|---|---|
| Workspace | ss2-vr isolated repository; AGENTS.md, FINAL_PLAN.md, NATIVE_OPTICS_AUDIT.md |
| Current source | 2a0a206; IPC6/wire4, pinned x86 Sam2Game/Engine/Core |
| Weapon ownership | engine.cpp nativeHandle: left player804/right800; weapon28 owner handle, B4 type13 |
| Input | host.cpp Actions; click already Sprint both except Vive right Jump; Recenter left click + both wheels |
| Simulation | engine.cpp entityStep freezes server MP before ANY weapon step; update local, authorityStep remote; completeTick after full simulation |
| Native protocol | common/network.hpp OrderedPosePolicy and multiplayer.cpp reliable credit/consumption; no new carrier |
| Lens source | scope_observer.hpp / scope_pose.hpp actual ordinary gun draw sample contentVerified=false; no optics yet |
| Zoom state | Native original callbacks own D4/timestamps/progress/damage. No direct adapter writes to those fields proposed |

## Verified native facts
[verified: pinned owned binary disassembly, NATIVE_OPTICS_AUDIT.md]
- weapon BC is creation-assigned ep_bRightHandWeapon; left0/right1. ParamsBC is unrelated float.
- AltPress171430 calls base48FE0 (sets held2C), toggles Activate171CF0/Deactivate171DF0 based D4. Release49000 only clears2C, leavesD4/damage/FOV.
- ActivateD4=1; BC gates saved owner558->D8, owner558=0 and native simNow E0/E4. Damage native paramB4.
- Deactivate clearsD4; BC gates restore558 and FOV setter1; native damageB0.
- SniperStep172820 calls BaseStep4EE20 at17282B, return172830. Then D4&2C&BC gates native interpolationF4 and F0+(EC-F0)*F4, owner SetFOVv770. BC also gates sound start172979 and stop172A36.
- BaseStep has no directBC reads, but virtual callbacks can read handedness: absence of direct reads does NOT prove safe to override BC during wholeStep.
- OnDelete1719D0 deactivates self via v284(return1719E1), then BC/right deletion can deactivate owner's left sniper via v284 at171A4E(return171A54), then baseOnDelete. PutDown171470 deactivates then native base.
- Stock player attack dispatch does not expose independent two-hand alternative input.
[verified: source] Existing physical weapon rendering retains native model and hands with full tracked poses. Native world P override ignores native owner weapon FOV; per-weapon native F4 can feed future optic camera.
[unknown] Runtime behavior/sound/animation/lifetime untested by user constraint. Neither this brief nor offline checks establish headset equivalence.

## Minimal adapter vs rewrite
Lean minimal: original AltPress/Release + nativeActivate/Deactivate/Step. No separate zoom state/progress/timer/damage machine, no new transport, no fake native handles wire. Ownership records only identify zoom owned by this adapter and guarantee cleanup; native IsZooming remains source of truth. Rejected: new optical zoom animation/damage state; broad BC override; entire renderer rewrite. Existing host actions, snapshot/authority, native reliable ordered snapshots and skin pose observation reusable.

## Proposed end-to-end design (unimplemented)
1. Add logical Zoom button64 and OpenXR action bound each thumbstick/trackpad click. Contextual input routing consumes same-hand Sprint/Jump only when owned sniper, retains other hand. Existing recenter+wheel suppression wins. ABI7 despite unchanged struct layout, to avoid mixed host interpretation. No claim TFE uses identical binding.
2. Add desired zoom mask to existing PosePacket; wire5, validate subset valid head/hands, blocked mask, nativeWeaponId13. Include mask in reliable edge detection. It is held intent, not short-fire tap; do not create duplicate retention scheduler. A click wholly between simulation samples has no held zoom interval. Off transitions reach next granted reliable snapshot; stale input forces native cleanup locally/server.
3. Per-owned-hand managed token (owner native identity/incarnation+weapon handle/hand) tracks adapter ownership, not native zoom state. Native IsZooming determines transition: on request, originalAltPress only if off; exit originalAltPress if on then originalAltRelease. Always held while desired; wheel/equip/loss/menus/thirdperson/recenter/death/capability cleanup delivers off for adapter-owned weapon. Native-owned preexisting zoom requires explicit normalizing native off before adopting; never use uninitialized left timing.
4. Scoped BC1 for managed left only inside originalActivate/Deactivate. For Step wrap originalSniperStep with transient context; detour originalBaseStep calls original once with REAL handedness, and only exactcaller172830 enables BC1 after it returns until outerStep restores. Gate thread, exact owner/weapon/hand. No writes to D4/E0/E4/F4/D8/owner558/FOV. Other native callbacks unaffected outside these call scopes.
5. Owner558 conflict: eligible VR requires firstperson, initially0. Each managed activation native saves0 and restores0. Native shared FOV remains stock last writer, ignored for XR world. If unmanaged preexisting zoom saved1/thirdperson transition complicates this, normalize/deactivate before adoption or decline instead of inventing shared owner policy. Need reviewer select robust minimal rule.
6. Right OnDelete cross-deactivation: candidate suppress ONLY exact caller171A54 for still-live managed left of same deleting right owner. Own deactivation and ordinary native deletion unchanged. Alternative preserve native coupling and reenter left next interval (causes reset/flicker, seems worse). Exact cleanup/handle invalidation must be sequenced with current sniperDeleted->invalidWeapon.
7. Server authority applies desired mask before weapons step; local client predicted native state allowed only if original callbacks are local visual/gameplay objects and server remains damage authority (to verify dispatch path). Death, disconnect, stale/binding loss cleanup on simulation thread.

## Independent dispatcher follow-up
[verified: complete bounded native branches] DoAttack101F00 button1 invokes rightAltPress at101F85(return101F8B) only if left handle resolves null. StopAttackFA770 button1 uses leftPrimaryRelease if left exists; otherwise rightAltRelease. Existing VR plcmdAltFire uses left trigger; no-left transitions can therefore toggle stock right zoom accidentally.
Candidate simplification: original Activate/Deactivate matching IsZooming + native baseAltPress48FE0 (only held2C=1) /Release49000 (only2C=0) instead of toggle-to-target SniperAltPress. Managed native step may need to reassert held through base method after native player releases it. Prevent scoped stock alternative toggles for managedVR snipers. Main supplied this to reviewer; not implemented.

## Open decisions / reviewer task
Verify claims against source/native audit before critique. Rank the design risks; challenge whether scoped handedness adapter and cross-delete suppression are necessary, and seek deletion/simplification. Select a concrete minimal approach for left progress + owner cleanup, or give a specific next evidence gate. Main lean above, rejected alternatives explicitly stated. Suspected overlap: desired mask and lifecycle ownership are one input adapter, not separate protocol/service. Do not impose another scheduler or body/optics rewrite.

Depth budget: bounded architecture critique <=1800 words. Output prioritized blockers, chosen approach, required invariants/verification, GO/conditionalGO/NO-GO. Human taste choices can be resolved with this user's established autonomous scope; no permission required. Do not re-review stereo, lasers, remote head lifetime, locomotion or package. No source edits, runtime launches, proprietary extracts, raw telemetry or nested agents.

## Host action reference
[verified: Khronos release-1.1.53 input chapter] Equal highest-priority action sets process colliding bindings; this port uses one active action set. Multiple logical actions may therefore share click bindings, with application-side same-hand contextual consumption. Reference: https://github.com/KhronosGroup/OpenXR-Docs/blob/release-1.1.53/specification/sources/chapters/input.adoc . This does not justify sharing a physical button without suppression rules.

## Integration invariants to preserve
- No native object reads or writes on OpenXR host/render worker for zoom; simulation thread only. Headless server must install the same native gameplay adapter without requiring graphics symbols.
- Managed ownership must outlive snapshot invalidation through own native deactivation/PutDown/OnDelete, then retire before memory reuse. Unknown/stale owner must not be dereferenced merely to restore a temporary handedness value.
- An actual native equipment change clears incoming intent for affected hands before native weapon step. Server matches received nativeWeaponId13 to current actual type and rejects requested equip/alias/untracked hands. Native handles remain local.
- Native-owned preexisting right zoom may have saved thirdperson1. Deactivating it can restore1; recheck firstperson eligibility after normalization before adopting. Native-owned left zoom may lack initialized timing; never silently treat it as managed.
- Existing snapshot invalidation (renderer loss, recenter, player delete) may clear current state before callbacks; cleanup must use native lifetime-bound managed identity, not revived input. Desired state still lives in existing snapshot/ordered sample, not an additional scheduler.
- Current reliable coalescing replaces unsent held state; an unsampled press-release has no native held interval. Existing short primary-fire retention stays independent; zoom must not survive as a historical tap or be inherited by a replacement gun.
