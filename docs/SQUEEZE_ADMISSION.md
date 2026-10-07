# Squeeze cancellation for weapon wheels — 2026-10-07

An unavailable squeeze action previously became low input. If a highlighted
weapon wheel was open, that could commit selection as though the user released
the grip. Producer-only transient blocking cannot repair this: simulation can
skip every interrupted sample and see only the recovered low. Reusing the comfort
block would also disable otherwise healthy primary/zoom actions.

The existing adapter now carries a durable per-hand squeeze cancellation epoch
and separate wheel-admission mask. The host queries both boolean/analog states,
requires valid values for every active source, and arms only on an available
release. Source replacement, invalid values, tracking/views/focus loss, profile,
session/reference changes and recenter retire admission. A held recovery cannot
arm. The epoch persists even when publication fails or intervening input is
overwritten. Exhaustion is terminal.

The existing weapon wheel clears its old selection and hold latch on a changed
epoch or missing producer admission. It adds no second release gate. This permits
a valid new press even if simulation misses the producer-observed release.
Ordinary comfort cancellation retains its prior hold behavior. Native inventory,
pending selection, fire/zoom eligibility and manual160/348 are unchanged.

## Review disposition and evidence

| Astra/xhigh finding | Main disposition |
|---|---|
| Transient producer block can be overwritten before simulation | Adopted durable epoch through existing input path. |
| Existing comfort block also gates fire/zoom | Adopted distinct wheel admission; original fire/zoom predicates retained. |
| Preserved old hold latch swallows recovered press | Fixed latch retirement; tested skipped recovery low and initial held/skipped release. |
| New test target preceded assertion options | Moved below options and added compile-time NDEBUG guard. Initial Release success did not cover its assertions. |
| Artifact verifier reported hardcoded old ABI | Both artifact and head reports now derive labels from validated ABI declarations/layout. |

Final read-only source follow-up is GO for this bounded slice, with effective
Astra/xhigh verified. Main verified all60 groups in Debug and Release after the
assertion-order fix, UBSan, and a deliberately NDEBUG-disabled compile that fails
at the new guard. Production producer/consumer tests cover missed interruption,
lost publication, action loss alone, missed recovery release/new press, duplicate
polling, source replacement, independent hands, malformed inputs/thresholds,
comfort cancellation, immutable frame identity and exhausted epochs. Native
HMD/controller interaction is not verified by those tests.

IPC is now **10**, multiplayer wire remains **6**. Both Windows architectures
agree: Input272, Request440, Slot33554928, Shared83887840; current products were
rebuilt together and their build contracts match. Older IPC9 archives stay
immutable and cannot be mixed with these products.

This is connected weapon-wheel input repair. **Physical vehicle grab-and-turn
steering remains unimplemented**: content geometry/owner/frame proof and its
native controls/runtime acceptance are separate. No melee adapter is activated.
