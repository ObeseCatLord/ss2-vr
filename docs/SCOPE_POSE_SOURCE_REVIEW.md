# Astra passive scope observer source review

Explicit/effective gpt-6-astra/xhigh verified; same reviewer, read-only. Final bounded verdict: **GO for this passive observer slice**, with final builds/offline checks owned by main. No blocking correctness, ABI or lifetime issue remains in the reviewed diff. This is not approval of loaded-content admission or completed optics.

Initial review found a mutating thread guard after native-object reads and successful-only publication leaving superseded evidence. Main corrected both. A model replacement follow-up also closes same-weapon/model changes without retaining borrowed pointers.

| Finding | Final verification / disposition |
|---|---|
| Observer called checkNativeThread, which can poison ownership/world eligibility | Corrected. ownsNativeThread loads ownership and evaluates a read-only predicate. Observer, admission and lookup check it before native-object resolution. Unknown, foreign, poisoned and non-main-thread ownership declines observation. |
| Later failed/ambiguous/exceptional draw could expose an earlier pose | Corrected. Clear the identified hand at render-wrapper entry before tracking rejection/admission/native call. Completion replaces or clears the sample; both eye boundaries reset the bank. Lookup rejects physical pair faults. |
| Same weapon can change its FP model | Corrected. Copy native model handle; compare before recording; lookup resolves current weapon/model and checks weapon+24, owner+28 and opposite-hand aliasing. Binding's borrowed instance pointer is callback-local. |
| Missing focused regressions | Added production bank/predicate checks for foreign/unknown/poisoned thread owner, model replacement, pair faults, request mismatch, supersession without completion, unsuccessful completion, independent hands and eye retirement. Existing affine/backlink rejection remains. |
| Observer allocation cost | Nonblocking. Bounded active owned-sniper observations only; does not justify a new persistent cache or broader refactor. |

Admission requires stage4 after successful tracked physical placement and rechecks identity before copying. Selection validates unique named Scope ownership, draw/map backlinks and named Sniper bone. Full affine preserves reflection/stretch/shear. Shared DDE30 calls original native producer, scopes adapter to returnE2E06, preserves nested/non-render behavior and keeps head mutation separately/default-disabled. Observation exceptions never alter world eligibility. Copied samples are pointer-free and contentVerified=false. No public query, render invocation, aperture or palette write was added by this slice.

Main final verification: affected x86 proxy/server incremental rebuild passed; x64 host/official loader previously passed in the same full cross-build. All11 offline groups passed after fixes. Artifact and compiled x86 ABI verification are in scope-pose-artifact-verification.json and scope-pose-compiled-abi.json. IPC6/wire4 and35 exported/5 internal hook gates remain. No game/host/Wine/headset/network execution occurred.
