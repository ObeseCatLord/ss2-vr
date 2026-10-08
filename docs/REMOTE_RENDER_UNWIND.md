# Remote renderer native-unwind retirement — 2026-10-07

Current follow-up: UI-fault retirement uses a constant-initialized TLS pointer to
the already admitted frame, avoiding lazy frame construction during native unwind.
The compiled gate inspects both actual helper variants and requires all branches
to stay inside their inspected bodies. Both x86 products pass; eight modified
compiled-object controls for calls, FP and outgoing/indirect tail jumps reject.
Remote heads now require a frozen VR stereo pair and are enabled for that bounded
extent. See REMOTE_HEAD_RESOURCE_OWNERSHIP.md. Historical findings follow.

Root-owned source correction; no game, Windows/Wine, host, OpenXR, headset or
network session executed. This does not close native worker/model lifetime or
enable the default-off RemoteHeadTracking option.

## Concrete failure and bounded correction

The model producer set a thread-local reentry flag before original DBC90 and
cleared it only after normal return. The palette producer's portable helper used
a GNU destructor for restoration around original DDE30. An MSVC native unwind
can skip those GNU frames. Binding/MP guards and invocation-owned vectors also
relied on destructors across native getters. A skipped lock release can prevent
later presentation; a skipped reentry restore can suppress later adaptation.

Both native producer entries now retain the previous TLS state above the existing
native-finally boundary and restore it explicitly on normal or abnormal exit.
Native originals still run once, and DDE30 adaptation still requires return
E2E06. Nested producer work invalidates outer adaptation; model selection checks
that invalidation after native callbacks and before writes. No palette-pointer
swap, alternative skeleton or native cleanup/rollback is introduced.

A shared presentation extent preserves binding-before-MP lock order. It releases
MP then binding ownership explicitly, including on native unwind. The role query
still occurs before acquiring either lock. An already active adapter extent
refuses nested adapter reads before reacquiring nonrecursive locks. Freeze and
Ready commit use the same extent; locks remain held through guarded publication.
A final active/invalidation recheck follows native binding validation.

Model, head and scope scratch vectors live in outer optional storage, above the
native-finally frame. Cleanup resets that storage once and releases its ordinary
C++ allocations. Cleanup performs no allocations, FP arithmetic, native game
callback, model write or speculative restoration. It only frees owned scratch,
retires scalar state and releases acquired locks. Normal portable helper errors
remain contained in the mod callback; native SEH is not caught or converted.

An interrupted producer retires the whole current pair, even when no remote
players were admitted. This differs from ordinary unsupported-remote fallback,
which can still publish a native-only pair without dereferencing remote objects.
The next normal freeze creates a new bank. Cleanup does not reset Pose structures
with floating-point instructions during foreign unwind.

## Verification and limits

All55 portable groups pass Debug and Release. The head helper's explicit cleanup
cases cover normal/abnormal retirement, nested prior-active state and idempotence;
its ASan/UBSan run passes. Both x86 game/server products and x64 host compile.
The compiled verifier checks seven native cleanup boundaries, saved TLS restore
on both paths, explicit lock/scratch retirement, pair revocation and absence of
FP or indirect native cleanup callbacks. It passes normally and with Python
optimization for both objects. A modified COFF object with both palette TLS
restores replaced by NOPs is rejected. Existing native-finally, head integration
and artifact gates pass.

Portable tests and compiled inspection do not execute actual MSVC unwind, prove
all concurrent native mutation paths, validate visual normals/extra passes, or
establish Windows/Proton runtime behavior. Those limits remain explicit. This
checkpoint is a cleanup correction, not completion of multiplayer roomscale,
physical melee or the entire project review.
