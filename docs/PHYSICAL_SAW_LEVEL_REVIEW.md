# Physical saw native level integration — Astra Max disposition

Fresh gpt-6-astra/max; effective current turn and owned pins verified. Static
review only. **No fork is approved for native integration.** Main adopts the
bounded saw stop-equivalence proof before adding durable manual history.

| Verified finding / recommendation | Main disposition |
| --- | --- |
| Primary0 DoAttack101F00 calls MarkNonIdle7F0F0 and returns without weapon start | Adopt: combined press history may not be necessary for the exact saw's start; other native lanes still require proof |
| Saw vtable2CDD10 slot4C uses base OnStep4EE20; states1/7 held at4EF85 call saw CallFireStart165460 | Adopt native level-start path |
| Base start4CC40 chooses state4 for saw flags at4CCF4..4CD01 | Preserve native admission; no synthetic attack loop |
| State4 can fire at4EF3F before checking held; state8 later reads held4F090 | Held-low alone cannot safely stop a short gesture |
| Saw release165490→48FF0 resets first-shot38;4CAA0/4CB52..61 changes4→1;164F80 requests sound3 | Canonical release effects must run before firing and during recoil; do not substitute state writes |
| Manual1/gesture1→manual0/gesture1 supplies a premature manual release | Native release needs logical-level isolation for exact tracked saw, while carry retains manual behavior |
| B0==4 predicate misses recoil/state8 release effects | Reject state4-only stop guard |
| E8 stores native sound state;165460 runs sound logic even if base start rejects | E8 is not an accepted-firing latch; a rigorously established predicate may still reuse it |
| StopAttackFA770 first calls7F450 bookkeeping/non-idle | Direct weapon release does not establish full stock equivalence |
| Existing observers need admitted logical-primary delivery at native replication boundary | Candidate boundary; side effects/binding/order remain proof gates |
| Commit-coupled manual-history candidate still solves a real information loss | Fallback only; has not earned all storage/carry/observer layers |
| No unused native field/bit proved | Reject guessed metadata/native protocol changes |

Concrete held-only counterexample: manual0/gesture-high starts native state4;
gesture-low with manual still0 has no native manual release; state4 readiness
can fire before its held-low query. Keeping projected current but manual prior
also repeatedly supplies presses during sustained gesture and lacks its release.

The next decision-separating proposition is whether exact native saw state plus
current admitted logical level can preserve canonical stop/bookkeeping, cancel
state4 before firing and suppress premature manual release under logical-high,
without additional durable input history or repeated synthetic attacks.
Required states: short gesture before first firing, continuation, release during
recoil, rejected start, and manual release under continued gesture. Carry must
retain original manual throwing. Native family:4EE20,4CC40,48FF0,4CAA0,
165460/165490,164F80 and player StopAttack/bookkeeping.

If proved, the minimal candidate becomes manual commands/manual348, existing
logical held reads, a canonical native stop adapter and bound native observer
projection. That could delete durable manual history and all four carry
selectors. This is a proof target, not architecture GO or a completion claim.
