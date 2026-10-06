**[Verified] The proposed separation is sound, but a bare base-method call also removes native view-height interpolation. I recommend a small adaptation.**

The installed `Sam2Game.dll` matches the recorded fingerprint (`5628b4ed…5303df`). All instructions in the four supplied listings match the installed binary. I independently confirmed the stated CPlayer vtable entries.

The base `CPuppet::GetViewOrigin(0)` path calls native relative-view placement, absolute placement, and look direction. Its shooting-attachment branch is entered only for purpose **1** (`0x8f1c2–0x8f1cb`), so that branch does not undermine the proposed purpose-0 accessor. `GetLookDirEul@CPlayer` simply copies the native Euler fields (`0x80550`); this establishes a **pre-animation native view**, not a proven pure body/yaw transform.

`AnimateViewOrigin` performs real quaternion modifications and adds two world-Y offsets (`0xfd1c8–0xfd1db`). Further static tracing distinguishes them:

- `+0x8bc` is driven by exported `UpdateSwimWobbling`.
- `+0x8c0` is maintained by `UpdateViewOriginInterpolator` (`0xf7d80`). It compensates changes in native relative-view height over 0.1 seconds.
- `+0x8dc` participates in `UpdateDampingValues`; its complete experiential meaning remains unverified.

Thus, treating everything removed by the base call as disposable camera shake would be incorrect.

| Priority | Decision | Disposition |
|---|---|---|
| 1 | Separate world calibration from authored camera references | **Adopt.** Use a pre-animation native anchor for tracked head/hands; retain original animated camera placement for native shot/model-relative calculations. |
| 2 | Use the base accessor unchanged | **Adapt.** Preserve the verified native height-interpolation correction without introducing another interpolator. |
| 3 | Gate solely on `!RendersIn3rdPerson()` | **Reject as insufficient.** Require native `IsAlive()` too, alongside existing ownership/tracking checks. Dead players can return false from `RendersIn3rdPerson`. |

The minimal concrete seam is one accessor, resolved through the exported base method under the existing fingerprint gate. Its verified ABI is hidden-return `Pose* (__thiscall*)(void*, Pose*, int32_t)`, with `ret 8`.

For an eligible local CPlayer:

1. Obtain **B** by directly calling base `CPuppet::GetViewOrigin(purpose0)`, bypassing CPlayer’s override.
2. Add the **verified** native `+0x8c0` height correction to `B.p.y`. Read completed native state; the updater temporarily writes sentinel `0x7f61b1e6`, which is finite and therefore requires explicit rejection. Do not call its updater from rendering.
3. Obtain **C** through the original animated `GetCameraPlacement`.
4. Use **B** for the existing shared tracking transform. Use **C** exclusively for native-authored references.

This retains native placement, stance height, and height interpolation without replacing their policies. Excluding the remaining animation/damping from the tracking anchor is a deliberate presentation choice; static evidence does not establish its comfort.

**Do not mechanically replace every existing `anchor` with B.** In [engine.cpp](../src/game/engine.cpp#L366), shooting needs a hand world pose derived from **B**, but `retargetShot` must still receive **C**. Likewise, the calibration calculator must use `matrix(C)` and cache `inverse(C) * nativeModel`. Keep actual model rotation relative to its caller-supplied camera. Otherwise, camera animation leaks back into weapon offsets or gets subtracted in the wrong frame.

Capture B/C coherently before VR overrides; keep the render anchor fixed across both eyes. Apply the same eligibility decision to head, hand, model, and shooting adaptation. Preserve native callbacks and weapon state machines; no global animation suppression or entity-placement writes are needed.

**[Uncertainty]** This establishes separation from the inspected CPlayer animation layer. It does not prove every writer of native look fields is animation-free, cover special camera modes, or demonstrate physical alignment/playability. Offline checks should establish authored-reference invariance under changes to B and preservation of native height interpolation.

No edits, additional agents, or game/headset/Wine execution occurred. Effective `gpt-6-astra`/`xhigh` startup-header verification remains with the parent.


Parent provenance: effective CLI settings verified; ephemeral read-only review. Line references describe the pre-fix snapshot. No runtime execution.

## Disposition

Adopted with the recommended adaptation: a resolved base `GetViewOrigin(purpose0)` accessor plus the verified native `+0x8c0` height correction supplies the shared world anchor. The finite unset sentinel is explicitly rejected. The original animated camera remains the authored shot/model reference, and both anchors are frozen for stereo. Native IsAlive and RendersIn3rdPerson gate adaptation along with local ownership/tracking. Native callbacks and height-interpolator updates remain intact.
