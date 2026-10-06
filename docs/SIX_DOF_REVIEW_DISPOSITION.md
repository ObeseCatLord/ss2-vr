# Six-DOF rig review disposition — 2026-10-03

Reviewer: Astra, effective `gpt-6-astra`/`xhigh` independently verified from this exact reviewer's settings. Main drafted the source-verified brief, Astra read the real producer/consumer/relay paths, and main spot-checked retained tap composition, pulse clearing and relay construction. No raw operational telemetry or proprietary disassembly is included. The user's latest instruction requires Astra for subsequent reviews.

The previous0.75m spherical head clamp truncated a1.1m physical crouch and reduced simultaneous horizontal lean. Separately bounding controller positions from the original calibration point also changed head-to-gun offsets at the boundary. The smallest fix preserves existing native rendering/weapon/network behavior: separate horizontal and vertical head bounds, share one head correction across the rig, and bound extraordinary hand reach relative to the physical head.

| Finding / decision | Main disposition |
|---|---|
| Common rig correction preserves eye separation, normal hand offsets and complete pose rotations | Adopted. Production eye/head cameras, model/muzzle transforms and multiplayer producer use the same helpers. Head horizontal limit0.75m, vertical[-1.8,+1.0]m, physical hand reach1.3m. |
| A retained tap preserves its original grip while the packet carries a newer head; current-head reach validation can reject the whole outgoing snapshot | Verified/adopted. Active grips are validated by distance to the permitted head volume, within1.3m plus0.2mm rounding slack. No new historical witness, retransmission or transport state. |
| A pulse-only validation exception breaks subsequent relay after the server clears pulseMask | Verified/adopted. The same active-grip envelope applies before and after consumption, including Relay encoding. This deliberately validates a historically producible position rather than simultaneous reach from the current packet head. |
| A broad3.25m absolute radius alone is too permissive | Adopted. Keep that finite outer bound, plus the tighter distance-to-head-volume check. Inactive hands retain a current-head-relative bound. |
| Producer calls a finite default/stale grip canonical even when tracking is invalid | Adopted. Invalid hands keep the neutral head-position fallback; only valid hands receive tracked conversion. Existing validity/fire/neutral gates remain. |
| Tests bypass the decisive tap/head-motion/codec/relay composition | Adopted. A scenario uses production PendingIntents, codec, OrderedPosePolicy and ConsumptionCredits: independent boundary taps from different head positions, newer head/release, encoded consumption, cleared-pulse relay, exact ACK and next neutral update. Added envelope-exterior/corner/nonfinite/Wire3 rejection and combined-rotation/asymmetric-eye/turn/recenter boundary checks. |
| Multiplayer validation semantics changed | Adopted. Wire4 requires matching server/client products; IPC ABI6 and native chat/RPC transport remain unchanged. Old packages are immutable checkpoints. |
| Need a movement rewrite or another protocol | Rejected. This correction changes pose mapping and validation only. Native roomscale movement/result attribution remains separate unfinished work. |

Seven portable check groups pass after the fixes, and x86 proxy/server plus x64 OpenXR host/loader cross-compile. Full native static artifact verification and packaging are recorded separately. These results do not establish physical scale, headset comfort, actual collision, multiplayer playability or full immersive equivalence. No runtime was launched.

Astra performed a second source verification after the fixes and reported no remaining blocking findings within this scope. It independently inspected the production regression composition and boundary coverage; main performed the actual build/offline executions.
