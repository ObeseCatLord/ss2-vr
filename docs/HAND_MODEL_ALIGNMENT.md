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
| Archives contain different ZapGun_FP.mdl/Zapgun.bmf versions | Historical archive ambiguity is narrowed by the five exact native-open receipts below. Opens alone do not certify the rendered instance or gripping point |

The resource-open proof below is complete. Next obtain one correlated idle eye sample with physical grip,
original/staged placement, native stretch, model bounds and a positively identified
grip reference. Keep extracted assets and captures private.

If a pivot mismatch is demonstrated, use one measured asset-specific alignment at
the existing native placement boundary. Apply the same transform through muzzle
conversion and observed scope placement, preserving native animation, recoil,
stretch/mirroring and remote child attachment offsets. Do not globally repurpose
physical gripOffset or substitute guessed geometry, new assets, IK or a protocol.
This gate requires no individual weapon-firing test by Codex.

## Actual native resource opens — run213458

The default-off private lab observer extends the existing scene-stream owner;
there is no alternative resource loader. Each original native read-mode open
runs once, then the observer hashes borrowed bytes and restores the stream
position before returning. Exact-name admission, loading/startup thread gates,
per-resource attempt limits and a32MiB file bound apply.

Compiled source fingerprint:
`b8e77f540f92633b65a9df8f00b83f35b834369030acd5bd867e46ecee8b9882`.
The native Jungle scene stream matches10562049 bytes and SHA256
`106f9e287110988720cc717aaaa22190cafcf90fb184fd5a83e9d467d93165df`.
The run observed303 neutral complete native world/UI stereo pairs, captured one
baseline eye pair, and shut down normally with no cleanup errors. No movement or
individual firing probe was selected. Captures and resource bytes remain private.

| Native-open resource | Bytes | SHA256 |
| --- | ---: | --- |
| ZapGunWeapon.ep |1822|44e5ebe3d0def83a6f51f55e6b5a4f3e4c4b2aa04167d06c74a32eb9dd558364|
| ZapGun_FP.mdl |5866|82089ba722ffa682dc7f96ab1395509214a53513ad3550b13b87a3a40069d26c|
| Zapgun.bmf |448875|f50b9999379c66fbdc4c9128ad228a89e7f294b663afa47214e8b75fe6b7163c|
| Zapgun.skl |1961|69068ca9777cda2585e5945a04e71e9ae1057831de5b0339ef10044982fc5746|
| R_Hand.bmf |25008|667e6cb03f08ce3fe3ffd6fe21b8e62de44d5cdb89d8c94cc0233dc34d14ae28|

All five receipts report restored stream position0. These establish successful
native resource-open byte identities; they do not alone associate those resources
with the rendered ID1 model instance, identify an anatomical gripping point, or
certify physical alignment. Astra/xhigh approved the bounded observer source
before this run; actual loaded-byte alignment interpretation remains separate.
The user will perform primary in-VR gameplay testing, including shooting and
vehicles. Further routine firing or vehicle runtime probes are not planned.

## Loaded-byte alignment review disposition

Astra/xhigh rechecked all five matching resource hashes and native boundaries.
Local current-turn model/effort tags were verified; effective backend routing
introspection remains unavailable. The review identifies a finite missing edge,
not a numerical calibration approval.

| Recommendation/evidence | Disposition |
| --- | --- |
| FP model references the native R_Hand mesh, Zapgun skeleton and animation resource | Record the native authored hand path; this is not a new hand mesh or IK system |
| Arm→Palm→Body skeleton chain and hand/gun Body palette references | Positive authored skeleton evidence; Palm is not certified as the controller grasp frame |
| No named Grip attachment; existing Barrel01/PlasmaSmoke children belong to Body | Do not invent a grip socket from the muzzle/effect references |
| Hand mesh bounds do not contain Palm rest origin | Reject blindly inverting the Palm rest pose as a grip correction |
| Native instance scale and hand reflection remain in the render path | Preserve them; no arbitrary scale multiplier |
| Current tracked placement drops native root translation | Investigate the exact dynamic charge residual separately, without treating it as physical grip calibration |
| Shared gripOffset also feeds remote placement | No FP-only correction in the shared network pose producer |
| Necessary further alignment evidence | Exact rendered instance/resource association, winning idle animation/effective stretch, and measured physical grasp frame in the existing aim-orientation/grip-position convention |

A future constant asset-specific correction must update model and muzzle/laser
placement consistently while preserving native animation/recoil/stretch. No
continual inverse animated-bone transform, runtime asset parser or ID13 scope
change is justified by this ID1 evidence. Primary in-VR gameplay testing remains
with the user.
