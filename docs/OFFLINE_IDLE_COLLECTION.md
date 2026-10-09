# Private offline idle collection

`tools/collect_idle_evidence.py` combines the existing idle assessor, candidate
matcher and compiled position evaluator. It launches only the bounded offline
arithmetic evaluator; it never launches SS2, sends input, queries native game
objects or changes settings. All input and output evidence stays outside source.

Supply an existing private native log, private candidate index, expected compiled
source fingerprint, selected native weapon ID (`1` or `13`), and the SHA256 of the
built evaluator. Choose a fresh output directory beneath the private root:

```sh
python3 tools/collect_idle_evidence.py \
  --log "$PRIVATE_LOG" --candidates "$PRIVATE_INDEX" \
  --private-root "$PRIVATE_ROOT" --output "$FRESH_OUTPUT" \
  --expected-source "$SOURCE_FINGERPRINT" --native-id 13 \
  --evaluator build-core/idle_position_replay \
  --evaluator-sha256 "$EVALUATOR_SHA256"
```

This is an offline postprocessor, not an instruction to run an unprepared native
fixture. The sniper requires an actual unzoomed ID13 draw; an initial-Zap log or
synthetic arithmetic cannot supply it. Private cheat/save fixture preparation is
separately user-authorized and still requires a verified native route and sealed
isolated launch configuration.

The collector checks bounded stable input snapshots, log source and selector,
and evaluator fingerprint. It executes a private copy of the exact checked
evaluator bytes, preventing later replacement of the original path from changing
the executed arithmetic. Existing consumers check referenced asset/channel
lengths and hashes. Only the collector's temporary arithmetic files are removed.
Existing outputs are refused. A post-admission failure preserves inputs, receipt
and `failure.json` for diagnosis.

Outputs include `input-receipt.json`, input copies, `idle-evidence.json`,
`position-replay.json` and `summary.json`. The summary lists only completed draws
with a unique consumed-channel match, qualified reference and agreeing position
replay. Historical retained copies remain diagnostic in the replay report and
are excluded from that list. Pose-only logs may produce a successful report
with zero qualified draws.

Even a qualified draw does not automatically accept alignment or grasp. Confirm
the handle-bearing surface, actual assembly, reference identity and integration
with model placement, muzzle and scope. A smaller lens surface cannot certify the
gun handle. GPU execution, physical contact and gameplay acceptance remain
separate. `alignment_accepted` and `positive_grasp_verified` stay false.

Run `tests/idle_collection_wrapper_checks.py` normally and with `python3 -O`.
The checks cover selector/source/fingerprint failures, private path confinement,
output collisions, retained failure evidence, pose-only non-acceptance, summary
qualification/retained exclusion and exact evaluator snapshot handoff to replay.
Existing matcher/replay tests cover draw arithmetic
and channel association.

## Review disposition

| Astra finding | Disposition |
| --- | --- |
| A source ancestor accepted as private root could copy private evidence into source | Fixed by requiring disjoint roots; the regression requires that exact rejection. |
| Snapshot handoff test does not prove executable execution | Corrected the coverage wording; actual arithmetic remains the existing consumer's responsibility. |
| Wrapper qualification and retained-only exclusion lacked assertions | Added positive, ambiguous, unqualified, disagreeing and retained-only controls. |
| Native selector must match sealed configuration and environment | Added one shared exact typed1/13 selector with default1 and disabled/tamper rejection. |

Astra/xhigh final scoped source GO; all three local review-turn tags verified,
independent backend attestation unavailable. Wrapper9 and runtime evidence23
groups pass normally/optimized. The existing immutable neutral ID1 log also
postprocesses to10completed/22rejected observations and40qualified draw records,
with grasp/alignment false. This reuses historical captured evidence and does
not constitute a new native run or sniper association.
