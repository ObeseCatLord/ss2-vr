# Native hand/weapon alignment gate

Current tracked placement is implemented, but physical gripping alignment is not
accepted. A gun near a controller pose is insufficient proof of a correctly sized
and gripped native model. No new numerical offset or scale is installed here.

Astra/xhigh inspected the owned native binary and current source, then the owner
checked the load-bearing placement boundary. Current-turn local tags confirm the
requested model/effort; backend routing introspection remains unavailable.

| Finding | Disposition |
| --- | --- |
| weaponAbs keeps camera-relative native rotation and places the model root directly at physical hand position | Verified source. A model-root-to-grip equivalence is an assumption requiring positive authored/runtime evidence |
| Native model placement is4BFF0, rendered through4C9B6;4A6E0 is shooting placement | Corrected native reference. Do not use the muzzle getter as a model-pivot certificate |
| GetWeaponInViewMatrix4B520 uses authored single/dual desktop position/angles and handedness | Retain as native layout evidence, not a measured physical grip transform |
| Native model placement also incorporates GetWeaponChargeDisplaceMatrix4B9E0 | Preserve any verified dynamic residual when adding measured grip alignment; rebuilding only rigid root position may discard translation |
| Native model-instance stretch/mirroring remains in rendering4C9FB/4CA31/4CA48/4CA67 | Rebuilding a rigid placement matrix alone does not prove loss of intrinsic model-instance scale |
| Actual193332 aim and grip positions agree | Excludes an aim-versus-grip position mismatch for that sample only; it does not locate the mesh gripping point |
| Archives contain different ZapGun_FP.mdl/Zapgun.bmf versions | Loaded resource identity is unresolved. Mount listings and a scene hash do not certify the winning weapon resource |

Next finite proof: hash the successfully resolved model, rendering parameters,
skeleton and mesh streams in the existing private loading extent, preserving
native stream position. Obtain one correlated idle eye sample with physical grip,
original/staged placement, native stretch, model bounds and a positively identified
grip reference. Keep extracted assets and captures private.

If a pivot mismatch is demonstrated, use one measured asset-specific alignment at
the existing native placement boundary. Apply the same transform through muzzle
conversion and observed scope placement, preserving native animation, recoil,
stretch/mirroring and remote child attachment offsets. Do not globally repurpose
physical gripOffset or substitute guessed geometry, new assets, IK or a protocol.
This gate requires no individual weapon-firing test by Codex.
