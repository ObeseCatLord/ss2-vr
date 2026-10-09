# Passive native sniper attachment observation

The private sniper fixture can now copy the attachment selected by the EXISTING
native base shooting-placement call. This is diagnostic source work for the
remaining ID13 origin/alignment decision. It does not fire a weapon, add a native
lookup, change the 0.5 m retarget bound or replace the native selected origin.

The stock authored `Barrel01` and visual `Muzzle` children are different points.
The attachment IDENT is an opaque runtime token, not a file ordinal or guessed
hash. The native selection evidence is pinned to the owned build recorded in
`installed-build.json`; the observed call returns at Sam2Game RVA 0x4A824.
`mdlGetAttachmentAbsolutePlacement` takes three cdecl arguments: model instance,
by-value IDENT, and a 48-byte output matrix. Failed native returns do not supply
an output copy. No proprietary code/disassembly is included here.

## Existing-call extent

Only the isolated non-headless sniper lab registers the passive attachment hook.
Every hook invocation forwards exactly once and preserves native result/output.
The receipt is armed only during the selected existing original call in an
accepted, neutral, unzoomed right-ID13 local snapshot. Stack storage lives above
its native-finally boundary. Normal completion restores outer TLS immediately;
cleanup restores it on unwind. Nested shooting poisons the receipt. Duplicates,
failed returns, aborts, nonfinite matrices and current model mismatch reject it.
Publication follows the ordinary final owner/weapon/configuration/resource checks.

Calibration adds copied private serial/request/input/eye and the flat native model
matrix used to establish that calibration. The serial is published under the
existing snapshot lock. It is not an animation fence or production admission
policy. The `calibrationNativeModel` matrix is that reference, not an assertion
about an upload's evaluated model world matrix. A visual comparison must still
use the producing draw's own captured factors and demonstrate actual transform
agreement. Matching identities, time or serial alone cannot do that.

## Receipt and consumer

The one-shot `Lab sniper attachment schema=1` row contains current input,
generation, owner, weapon, hand, native ID, model/instance, opaque IDENT, call
count/result, calibration serial and producing request/input/eye, cache age,
configuration/file/resource, and the copied original shooting pose. Two matrix
rows carry the same serial, named `nativeAttachment` and
`calibrationNativeModel`, each with twelve exact float32 words.

`tools/collect_attachment_observation.py` requires exactly one complete receipt,
checks schema, finite values and associations, and writes only beneath the
specified private root using exclusive output creation. It retains these limits:

- Same-original attachment-call provenance is observed.
- Draw animation equivalence and visual muzzle alignment are unverified.
- Native firing and physical-device acceptance are unverified.
- The existing retarget bound remains unchanged.

No raw native receipt or game capture belongs in public source. Use a fresh
sealed private lab through the reviewed private-display launcher, never an
attempted fixture or the original installation. Current authority permits neutral
captures; this document grants no additional input or gameplay testing scope.

## Review and checks

Astra/xhigh source review accepted forwarding, receipt lifetime, publication and
concurrency within the existing native containment contract. Local routing was
verified; independent backend attestation was unavailable. The portable admission
and strict parser checks supplement compiled x86 forwarding/output-copy and
muzzle cleanup gates. Both compiled gates are layout-specific checks of the actual
current objects, supported by manual inspection, not general dataflow proofs.
Negative altered-object controls reject missing forwarding, failed/duplicate
admission, incomplete copies and removed normal/abort depth/TLS restoration.
These checks do not prove actual native unwinding, concurrent execution or visual
alignment at runtime.
