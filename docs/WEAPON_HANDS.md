# Native weapons and visible hands

Static read-only Terra review of the exact installed build; no game/headset execution.

Each controller retargets a complete native first-person weapon model instance through GetWeaponAbsPlacement. Native Render and animations remain intact. There is no separate hand/bone/arm hook in the current development build. Native local injection resolves right/left handles at player +0x800/+0x804 and calls each weapon Render (vtable+0x1f0); base Render RVA0x4c740 calls GetWeaponAbsPlacement then Engine mdlRenderModel RVA0xe2b10. Sniper normal rendering delegates to base when its zoom flag is zero.

Installed native model-resource references establish that several first-person assets include hand or forearm components:

| Weapon model | Components referenced in native resource |
|---|---|
| Colt_FP.mdl | Colt.bmf, R_Hand.bmf |
| Uzi_FP.mdl | Uzi.bmf, R_Hand.bmf; Arm/Hand names |
| ZapGun_FP.mdl | Zapgun.bmf, R_Hand.bmf |
| Klodovik_FP.mdl | Klodovik.bmf, R_Arm.bmf; LowerArm name |
| SeriousBomb_FP.mdl | SeriousBomb.bmf, Hands.bmf; Arm_Left/Arm_Right names |

Those bundled components follow the same weapon-instance transform. Exact skinning and detachable-child/bone topology are not established by resource strings alone, and other weapons may have hands inside the primary mesh. This does not implement independently articulated VR fingers or full-body arm IK. No proprietary meshes are included in the mod package.

The smallest current improvement is per-weapon grip calibration while preserving the native asset/animation path. Separating authored arms requires verified attachment/bone/mesh APIs and identifiers rather than assumed topology. Generic attachment placement exports exist (mdlGetAttachmentAbsolutePlacement RVA0xd8fa0 and its bone counterpart0xd8d60), but a usable stable grip identifier is unverified.

## XR sniper visibility follow-up

Native sniper Render0x171F20 checks zoom flag+0xD4: when zero it delegates to ordinary base weapon rendering; when nonzero it replaces the gun with an orthographic screen overlay. The current source uses the same base weapon/model path for an owned sniper inside XR eyes, preserving its visible tracked gun/native hand geometry. The final desktop render retains the native sniper/zoom path. This avoids a flat scope pass hiding the gun or overwriting headset projection. Immersive magnified optics remain unfinished;0.2.1 contains this follow-up, which the0.2.0 checkpoint predates.
