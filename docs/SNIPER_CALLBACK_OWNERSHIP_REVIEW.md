# Astra callback ownership review

Reviewer: effective gpt-6-astra/xhigh, independently verified during the read-only interval. Main spot-checked the cited source deletion ordering and owned native Stop/source-deletion bodies. Source-only review; no game, host, headset or network execution.

Conditional GO for a small callback-owned zoom adapter; enablement remains NO-GO pending native cleanup lifetime/thread evidence. Cleanup authorization must survive input expiry, equipment replacement and snapshot invalidation, but an ownership token never keeps a weapon alive. Invoke no deferred cleanup on stored raw pointers; use only the actual native callback borrowed lifetime.

Source deletion wrappers invalidate before original native deletion. Native sniper deletion calls base deletion before deleting its zoom sound source. Native OnStep can skip Stop and still clear FC, so FC=0 cannot prove sound cleanup. Revoke activation immediately but retain cleanup-only identity through the admitted actual Stop tail or source deletion.

Mutable claims must live outside copied/replaced Authority and Snapshot records. Native setWeapon can reenter deletion between copy and save; a lock around each copy does not prevent stale replacement. Reuse existing live-player/hand checks, adding only bounded adapter claims keyed by weapon handle, owner/incarnation, hand and non-reused serial. Capacity exhaustion rejects new claims rather than evicting live/retiring ones. Read native IsZooming; duplicate no native timing, damage or desired-input machine.

Use lexical operation-specific context frames. Legitimate Step->Fire nesting needs exact token revalidation after base execution; post-base held reconciliation cannot create or resurrect ownership. Reserve claims before native activation and do not recreate claims retired by reentrancy. Adopt no already-zooming unmanaged weapon. Hold no metadata lock across native calls. Preserve native callbacks, arguments, returns, scheduler and damage.

Right-delete cross-call suppression requires the exact demonstrated call origin, corresponding right-deletion context, surviving owned/admitted desired left and no owner teardown. Mark owner teardown before invalidation. Toggle cleanup belongs immediately before the actual mutation, not the exported entry's possible no-toggle branch. PutDown/Delete should retain their existing native deactivation rather than add a competing call.

| Recommendation | Disposition |
|---|---|
| Cleanup independent of activation input | Adopted for future adapter; native callback lifetime still required. |
| FC=0 as cleanup proof | Rejected; require actual native Stop/source deletion completion. |
| Claims inside copied Authority | Rejected; bounded ownership metadata only. |
| Deferred pointer cleanup/new scheduler | Rejected. |
| General left-managed cross-call suppression | Rejected; exact origin/owner-teardown admission only. |

Unresolved native gap: inspected bodies do not prove all managed PutDown/Delete/toggle/sound cleanup callbacks run on main or establish cross-thread object-lifetime synchronization. Main-thread activation does not supply that proof. No native zoom adapter is implemented by this review.
