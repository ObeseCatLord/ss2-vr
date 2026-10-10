# Observer gesture receipt context

The retained physical-melee pulse helper now binds a consumption receipt to the
exact context captured by `sample`. A callback from an earlier hand intent epoch
cannot consume or validate a replacement pulse whose source sequence, gesture
generation and local receive millisecond happen to match.

## Reproduced source defect

Before this change, `Receipt` contained the helper address, hand, gesture
generation, gesture sequence and receive time. `receive` resets the retained slot
when the pose context changes. That reset permits a replacement pulse carrying
the same physical observation under a newer hand intent epoch. Millisecond receive
time is not a unique lifetime identity.

The regression receives a valid two-hand pulse, captures both receipts, then
receives another valid pulse with a newer overall pose sequence and one changed
hand intent epoch at the same local time. Both gesture sequences and generations
remain equal. The unmodified helper reports both old `current` and old `finish`
as true. With this change both return false, the replacement consumes once, and
the opposite hand retains its own receipt.

This is a reproduced portable production-helper failure. The actual native
callback/relay interleaving has not been observed in gameplay.

## Repair and consumer boundary

One context key contains the existing comparison's client/server nonces, tracking
generation, hand intent epoch, native weapon ID and gesture generation. `receive`
and `sample` retain the same admission policy. `sample` copies that key into its
receipt; `finish` and `current` check it against the retained pulse. The former
duplicate receipt generation field is removed.

`finish` still requires a pending pulse. `current` intentionally remains valid
after successful completion for an already admitted immutable native interval.
Cancellation, expiry, duplicate suppression and capacity discard keep their
existing behavior. Manual trigger/history, native damage/release, protocol bytes
and independent-hand dispatch are unchanged.

The existing `finishObserverGesture` native wrapper validates avatar incarnation
and recipient nonces, then delegates receipt matching to this helper. It does
not itself compare the captured hand intent epoch or tracking generation.
`observerGesturesCurrent` has additional presentation/context checks; those do
not replace the completion check. No native wrapper or callback was changed.

## Review and verification limits

An explicitly requested Astra/xhigh read-only review returned bounded source GO
for the helper and tests. The exposed runtime metadata did not report effective
model/effort settings; requested settings and the reviewer's self-report do not
verify them. Independent backend attestation is also unavailable. The user
explicitly approved this disclosed review mode for isolated integration. That
exception permits using this bounded source review; technical checks and the
formal review of the final integrated change still remain required.

| Review recommendation | Disposition |
| --- | --- |
| Bind receipts to the original context | Adopted using the existing six-field comparison |
| Avoid duplicated generation storage | Adopted; receipt context carries generation |
| Preserve post-completion immutable intervals | Adopted; `current` does not require pending |
| Test both hands and global tracking replacement | Adopted; nonce replacement controls added too |

The existing registered `network_checks` group contains the new cases; no CMake
registration is needed. Baseline and fixed focused predicate receipts distinguish
the regression from the existing cancellation-only test. Six source mutations
remove either receipt check or one of the newly exercised context components;
the regression rejects all six.

All 82 Debug and 82 assertion-enabled Release groups pass. The network suite
passes ASan/UBSan with leak checking disabled. All four Windows products rebuild
with matching IPC10/wire7 source contracts. Native-finally, both x86 products'
melee unwind and multiplayer dispatch checks, and artifact/installed ABI checks
pass normally and with Python optimization (12 reports). Exact private logs,
product identities and compiled-check receipts accompany the worker handoff.

These are source, portable and compile-time results. No game, headset, shooting,
vehicle or network session was run. Native first-use/copy/release/contact,
dual-wield feel and multiplayer gameplay acceptance remain user-operated gates.
