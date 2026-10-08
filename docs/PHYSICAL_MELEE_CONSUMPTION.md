# Native circular-saw consumption adapter

The source now connects the existing local gesture sampler to native saw input.
It is a default-off, single-player development slice, not full melee completion.
Multiplayer gesture/observer delivery and automatic first/copy admission remain
open. Individual firing/contact tests belong to the user. No such probe ran for
this change.

## Existing owners and native behavior

The existing local snapshot owner contains two stationary, noncopyable hand
records. Copied pose snapshots never copy receipt storage. Each binding has a
monotonic epoch and receipt revision. Metadata uses the existing snapshot lock;
native getters and callbacks run unlocked. There is no new entity registry,
combat timer, damage system or manual-input history mutation.

Only an owned, handheld, noncarrying, uniquely mapped native circular saw is
admitted. Native raw button, flip, selected handles, combo/dual setting, class,
weapon ID, hand and rider are revalidated. Coupled/multitarget/alternative routes
remain native. Each admitted desired level is manual OR separately eligible
physical gesture. Native160/348 remain manual-only, and gesture is scoped to the
exact weapon's original held query, preserving native blocking161.

Native cooldown, ammunition, cadence, damage, ray selection, sound and weapon
state machine stay authoritative. The existing tracked muzzle supplies native
saw hit queries; this is not a swept-blade collision replacement.

## Completion and cancellation

Unknown initial, replaced or copied state is never inferred low. A real normal
native release must complete before subsequent fresh quiet motion can arm a
swing. There is no synthetic initializing release. For this development slice,
use the trigger normally through a press/release cycle after selecting the saw;
then hold still before physical motion. Tracking recovery itself never rearms.

| Native boundary | Adapter observation |
| --- | --- |
| Canonical primary press7FFC0 | High only after original normal return and current binding validation |
| Saw release through base48FF0, incoming return165498 | Low immediately after original normal return, before subsequent sound processing |
| Original base weapon step4EE20 | An unchanged, already known level advances receipt revision only on normal return |
| Foreign/native unwind | Restore mod TLS and conditionally retire the entered receipt; no native callback/rollback |

The operator callback and pre-base-step reconciliation share the same stationary
record. A skipped held query cannot erase a completed high's release obligation.
A known high remains cancellation-only recognition after snapshot initialization
is lost; it grants neither new gesture high nor a new binding. Exact native
owner/world/weapon/rider checks still precede release.

Opaque native callbacks classify their actual primary targets independently of
VR topology eligibility. Only affected weapon records retire; an unrelated gun
cannot discard the other hand's saw release obligation. Alternative callbacks do
not retire primary accounting. If target capture fails, gesture permission is
withdrawn while the pending release remains for a later safe boundary.

Copy/assignment, derived deletion and successful put-down retire matching
metadata before native callbacks. Refused put-down preserves consumption and
uses lexical admission blocking. Nested same-level completion supersedes an
older outer receipt; stale aborts cannot retire newer completed observations or
reused storage.

## Review disposition and evidence

| Review finding | Disposition |
| --- | --- |
| Player-wide retirement on unrelated callbacks | Replaced with native target-specific classification and exact weapon identity |
| Tracking loss blocks release of gesture-only high | Cancellation-only stored-owner recognition; no new source admission |
| Nested normal same-level step leaves older receipt current | Added normal-only known-level observation, without release/quiet witness |
| Manual-history gesture projection | Rejected; native160/348 stay manual-only |
| First/copy state guessed from idle/constructor fields | Rejected; initial level stays unknown pending real release |

The x86 compiled checks cover10 linked callbacks,11 native cleanup extents and
transitive scalar cleanup helpers. Private compiled-byte negative controls reject
floating point, indirect/outgoing branches, unapproved calls and removed TLS
restoration. Native entry windows were independently checked with the existing
MinHook decoder and Capstone against pinned owned binaries. These checks do not
execute native exceptions, install trampolines or prove contact/MP equivalence.

Source review and exact final artifact identities are recorded in
IMPLEMENTATION_STATUS.md. Hardware feel, native dual melee where actually
eligible, contact/miss behavior, interruptions and multiplayer remain separate
acceptance rows; disabled/incomplete rows must not be marked passed.
