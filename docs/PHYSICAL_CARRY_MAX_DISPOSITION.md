# Astra Max carry architecture disposition

Fresh/effective gpt-6-astra/max, static pinned binary/source review only.
Current physical-primary native architecture remains NO-GO. No runtime executed.

| Recommendation / verified finding | Main disposition |
| --- | --- |
| Gesture high commits combined348; manual0 under positive carry reaches ThrowObject9AC50 even after poll prior rebasing | Adopt as confirmed P0 native counterexample |
| brain160=current,161=blocked,1E8=replication copy; inspected seams cannot distinguish previous manual-high from gesture-only-high | Adopt demonstrated need to retain lost contribution, not blanket claim all native fields exhausted |
| Original commands/history should represent admitted manual contribution; existing logical Snapshot/pose intents continue feeding handheld projection | Preferred incremental candidate; consumer/observer proof precedes source changes |
| Record preprojection raw bits from8E750 at actual native348 store8E75A | Preferred two-bit metadata, coupled to native commit rather than independently sampled history |
| Existing poll gesture-rebasing becomes redundant under manual-only commands | Delete once that candidate is accepted and implemented; current helper is not native history repair |
| Preserve correct manual-only carry continuity | Retain |
| HandleObjectCarrying9B030 directly tests348 at9B207 afteroperatorA6E03 and three virtual calls | Include explicit late-carry consumer/order proof |
| Snapshot/session resets and copied Authority replacements do not match native348 lifetime | Resolve actual entity lifetime, stale copies, nesting and native constructor/copy writes before selecting storage |
| Client replicas are excluded from native-primary ownership while160→1E8 replicates ordinary command state | Trace observer weapon/animation behavior before multiplayer acceptance |
| Action replay/new wire/new history FSM/unused native bits | Reject; preserve original RPC, movement, native combat and upper bits |

Native current/prior sites: operator8E580 plainret, held8E480 ret4;
prior8E5BB=`8A8648030000`, prior-test8E5F5=`849E48030000`,
store8E75A=`888E48030000`. Native controlsEE0F0 ret1C, actionEE260 ret38;
RPC EE441 precedes current storeEE53A. Native copy writesD465/F29E and
constructionA41FF require metadata correspondence. Raw-index bits are not hand
indices or later manualDown samples. Unknown previous contribution is not zero.

Next proof gates: (1) lifetime/actual-commit storage, (2) carry transitions inside
and after immutable native invocations, (3) multiplayer observer consumption.
Portable connected trace must include actual combined commit, gesture-only→carry
manual0 with no throw and manual-high→carry/manual0 preserving release, plus
late9B207, flip/block, nested stores/copy/reset and independently ordered native
action/pose arrivals. Green producer checks alone do not satisfy those gates.

## Main spot-check of the counterexample

A Linux C++20 scratch trace linked the actual `nativePrimaryRead`,
`primaryPreviousCommandValue` and `nativeCommandQuery` helpers. A gesture-only
projection read with kind3 produced committed prior1; narrowing to carry made
the command-cache prior0, but the carry invocation's mask0 retained current0.
Applying the Astra-audited previous/current bit tests yielded:

| Source of prior high | Combined native prior | Cached release | Native release |
| --- | --- | --- | --- |
| Gesture only | 1 | 0 | 1 (false carry release) |
| Manual high | 1 | 1 | 1 (required manual release) |

This is a portable data-flow counterexample, not execution of a native branch
or ThrowObject. It confirms that the two distinct source histories collapse to
the same native prior byte and that the command-cache correction cannot repair
that byte. No native hook or architecture change follows solely from this trace.
