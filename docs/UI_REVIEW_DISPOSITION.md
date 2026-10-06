# Astra UI comfort review disposition

Reviewer: gpt-6-astra/xhigh, explicitly selected and verified by the reviewer. Read-only review; no runtime/build execution. Main verified the load-bearing source facts and integrates the changes. Review brief remains in private scratch; summarized normalized evidence is below.

| Verified finding / recommendation | Disposition |
|---|---|
| Existing textures are up to1024, minimum512; runtime layer capacity4 is already checked | Corrected the brief. Preserve both limits. |
| Dynamic wheel list has at most16 usable slots; native names already abbreviated | Preserve inventory-dependent ordering and existing labels; center exposes full selected label/ammo and ring boxes are bounded/ellipsized. |
| VIEW HUD/menu move with every head rotation | LOCAL ComfortAnchor with independent per-panel state; level yaw, enter35deg and stop8deg residual; max90deg/s. Position and facing share one transform. |
| Anchor lifecycle/reset and vertical-gaze edge cases | Reset on session/reference/tracking/visibility/long-gap/recenter edge; preserve heading during vertical gaze; never reset merely for native weapon-selection epoch. |
| Translation hysteresis does not enforce viewing distance | Explicit panel-plane distance guard; hide/reseed near or behind panels. Frozen wheels reserve a shared layout; unsafe proximity cancels selection through an explicit host presentation-block mask, not synthetic physical button releases. |
| Held Use/stick can act when focus/tracking recovers | Added PressGate/MenuNavigation valid-release/neutral guards and deterministic one-action priority (Back, Confirm, direction). |
| Pointer native polling/cursor and interactive menu classification remain unknown | Keep original native keyboard menu route; no guessed mouse callback or additional IPC pointer path. |
| Shrinking native menu from53deg to42deg reduces text legibility | Keep original53deg default until native glyph metrics are available. |
| Dual-wheel0.85m/.48m arrangement spans~72deg | Shared binocular-FOV-aware reservation, max28deg diameter at1.45m, symmetric positions, both opening times use same layout; projected-corner offline checks. |
| Text/image geometry must match | HUD uses a rectangular4:1 texture and4:1 quad, separatehealth/left/right rows; keep GDI ownership instead of a replacement UI framework. |
| Comfort defaults are not runtime proof | Defaults and textangular geometry are recorded; actual headset comfort/readability remain unverified. |

Primary design references: [Microsoft comfort](https://learn.microsoft.com/en-us/windows/mixed-reality/design/comfort), [typography](https://learn.microsoft.com/en-us/windows/mixed-reality/design/typography), [Meta spatial UI sizing](https://developers.meta.com/vr/design/hands-ui-best-practices/). No clinical/device-specific comfort guarantee is inferred from these guidelines.

The same pass separates physical controller grip position from runtime aim orientation for gun attachment. Original FP meshes/animations remain native. This requires pointer-free ABI4; both architectures must emit identical layout bytes.
