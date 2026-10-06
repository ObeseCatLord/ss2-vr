# Native vehicle laser design review and disposition

Reviewer: explicitly selected and independently verified `gpt-6-astra/xhigh`,
baseline ec3deac. Read-only review; matching installed Sam2Game/Engine/Core
fingerprints independently checked. No runtime, delegation or edits. This is
a conditional design endorsement, not source acceptance.

## Final critique (faithful bounded transcription)

Conditionally endorse A; reject B as written.

1. Highest priority is distinguishing the restored child-traversal flag from
   destructive shared model scratch. Engine2C7F9C initializes to1 and controls
   child traversal atE1F3D/E1F7C/E1FE0/E201F. D8FA0 saves/sets/restores it; it
   does not establish idle scratch. E28B0→E1640/E1720 appends shared records;
   DAD90 clears counts and evaluated pointer2EAB68. An outstanding model
   evaluation would be corrupted by this getter. Existing exact21AE69/no-eye/
   simulation-thread seam is credible: handheld sampling reaches the same
   getter through4A81E, and native LOS finishes placement before21AD70. Require
   native main-thread ownership and quiescent actual record/evaluation state
   before either evaluation. An idle-state check is not a cross-thread lock;
   unhooked worker concurrency remains unknown, not a reason for a rewrite.
2. A preserves the original origin transform. Turret table2B39E8 has
   AC=8EFF0,550=8EE70,5A4=155CE0; shooter table2B2398 calls owner AC/purpose1
   at160F95. Engine15C290 composes low-level D8FA0 attachment with renderable
   placement2C. That matrix is model-relative, not already world placement.
   Sam8F213..8F45E then orthonormalizes/converts it. B omits demonstrated
   behavior. Keep native purpose1 reconstruction and explicit existence:
   15C290 substitutes identity on missing attachment, which can look finite.
3. Narrow direction claims. Native155CE0 supports the turret aiming beam.
   Executor160FCB obtains5A8 exact target, subtracts origin and later clamps
   at1617CE/161D31. Exact-target90260 can return a foe position; its fallback
   uses5A4 and purpose1. Thus reported native direction does not prove final
   projectile direction/scatter/all shooter clamps. Do not invoke fire for
   sampling; correct the broad claim.
4. Native COW is compatible, stale borrowing is not. Active8EE8E..8EEFC may
   replace47C, then returns selected blast field4. Idle8EF05..8EF22 uses native
   Barrel01 conversion without COW. No custom idle selector is justified.
   Revalidate rider/ride/seat/model/instance/selected attachment and relevant
   resource/config continuity around callbacks; reject/reacquire changed COW
   views. Gate audited native implementations, not every ride type. Valid
   action IDs alone do not prove unchecked process/blast lookup success.
5. Carry provenance through existing publication and two-eye freezing. Branch
   mounted before handgun requirements. Recheck source/model/rider after
   collision and at freeze. Include direction in cache identity or omit
   vehicle reuse. Reject zero/nonfinite direction/rotation; preserve native
   origin; exclude actual vehicle mechanism; guarantee native ray cleanup.
   Compile-check hidden selector/direction ret4, origin ret8 and cdecl
   attachment three-stack-argument/int-return callers. Regressions must cover
   COW/reference changes, missing attachment, callback invalidation, direction
   changes and identical two-eye freezing. Query opportunity/moving freshness/
   appearance/performance remain runtime-unverified.

## Main spot-check and final decision

Main independently inspected exact DAD90..DADD8: eight model/mesh/draw/bone/
mapping/palette counts plus2EAB40 clear, and2EAB68 clears. E1640 appends the
model/bone sentinel. Main verified GetModelInstance15B220..15B223 is the
read-only ECX+5C pointer return and15C290's12-byte cdecl attachment call,
missing identity fallback and subsequent native placement composition. These
facts support A and invalidate the original cursor/world-matrix wording.

| Recommendation | Disposition |
|---|---|
| Preserve purpose1 native world reconstruction | Adopt A; no replacement matrix/world policy |
| Quiescent scratch/main-thread admission | Adopt actual nine zero counts/evaluated-null checks before/after getters, exact existing query seam |
| Correct traversal-flag and projectile-clamp claims | Adopt; source is a native reported aiming beam, final native scattering remains native |
| Resource/config/selection/current model lifetime | Adopt existing sample provenance and callback/freeze checks; COW rejects the changed sample and next query reacquires |
| Audited implementation/lookup presence | Adapt to exact turret table2B39E8 initially; bounded current pointer-array presence validation before original selector |
| No vehicle cache reuse/new system | Adopt; retain existing sample bank/query/native ray/stereo draw; no IPC/wire changes |
| Native ABI and behavioral checks | Adopt offline source/caller checks; no runtime claim |

Implementation is approximately220 changed native lines including fresh-request
and publication checks, exceeding the original100–160 estimate because review
requires real scratch and unchecked-lookup admission. It adds no state machine,
queue, model manager or gameplay policy. Source review must explicitly challenge
this added complexity and seek simplification. Shared readableMemory is moved
unchanged from remote_render into native_memory.hpp for reuse, not reimplemented.

