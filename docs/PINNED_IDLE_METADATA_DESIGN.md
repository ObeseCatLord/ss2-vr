# Bounded offline candidate metadata producer design

This separate checkout starts at delivered commit
`0eff50d01340f770ad707edfa3c313c74c9806e3`. The original writer remains at
`0bdd05c`, with changes in native collector rejection diagnostics, their readers/
tests and saved-TLS verification. Read-only inspection found no metadata producer
in those changed files. Delivered patches, bundle and handoff are unchanged.

## Exact contribution and ownership

Produce the minimal decoded `CRenderMesh` / referenced `GFXHANDLE` metadata needed
by the delivered index exporter directly from either of two exact owned asset
fingerprints. Close reproducible offline production of selected serialized
candidate fields; do not implement a general CTSEMETA loader or certify native
loaded-resource, rendered-instance, grip or alignment association.

Disjoint new files only:

- `tools/produce_idle_candidate_metadata.py`
- `tools/idle_candidate_metadata_layouts.json`
- `tests/idle_candidate_metadata_checks.py`
- This design/evidence note, `docs/PINNED_IDLE_METADATA_DESIGN.md`

No delivered file, CMake/source/native file or writer file is modified. Tests run
explicitly normally and with optimized Python, alongside the existing exporter
regressions and core suite. New source-only commits and handoff will be separate.

## Proven format domain

The existing private typed reader and span records supply observed serialized
field boundaries; its object-body encoder round-tripped the supplied assets.
The reader searches framing tags and leaves non-object sections/trailing data
uninterpreted, so it cannot support a general complete-file certificate.

The producer instead requires the full exact asset length and SHA-256 before
using a pinned selected-field layout. Unknown/changed assets fail closed. A
layout contains offsets and expected typed framing, counts and identities,
never geometry bytes or decoded floating-point coordinates.

Validate the pinned OBTY/OBJS table tags/counts, selected object identities/types
and DTTY type declaration identities/names/version/kind/base. Read mesh index/
vertex reference arrays using observed STAR counts and signed four-byte object
references. Read LOD/section/surface array counts using observed STAR framing.
Read surface triangle/vertex counts as signed four-byte INDEX values, element
paths as UBYTE/UBYTE/ULONG and palette identifiers through SSAR framing. Read
referenced buffer arrays through their observed STAR byte counts. All reads
require bounded disjoint spans and exact little-endian re-encoding of the selected
serialized fields. The schema continues to express buffer-relative channel
offsets; only the producer layout contains asset-file offsets.

The delivered exporter validates the resulting geometry/channel domain through
its pure `index_asset` helper before metadata is admitted or written. It remains
unchanged. Generated metadata has the existing `objects` shape and an explicitly
selected-field result scope. A separate precise fact may report selected metadata
was decoded from the verified asset bytes; it must not turn the existing index's
unverified-native/association flags into success or treat an externally supplied
JSON hash as proof. Whole-file decoding/roundtrip and all historical/native/GPU/
physical grasp/alignment flags remain false.

## Planned offline evidence

Synthetic binary fixtures model these already observed tags and typed fields,
without proprietary data. Test unknown/changed fingerprints, truncation, wrong
type/object/reference identity, count/range budgets, duplicated/overlapping spans,
bad descriptors/palette, nonfinite geometry and malformed triangle indices.
Test exact selected-field bytes, deterministic metadata, existing index exporter
and replay-reader interoperability, private output protections and collision
refusal normally and under Python optimization.

Read-only in-memory comparison with existing private decoded candidates must
agree on all selected fields and all eleven currently admitted candidate rows,
hashes/ranges/channel bytes. Do not write the private assets or analysis files.
Any contradictory field semantics, missing typed proof, or need for native
provenance stops this increment at the specific blocker.

## Implemented operation and limits

Run from the source root, using owned input and fresh output outside every Git
checkout. No CLI layout override exists. Only the complete SHA-256/length pins
bundled in `tools/idle_candidate_metadata_layouts.json` are admitted:

```sh
python -B tools/produce_idle_candidate_metadata.py --asset /private/r_hand.bmf --output /private/r_hand.selected.json
python -B tools/produce_idle_candidate_metadata.py --asset /private/r_hand.bmf --verify-metadata /private/r_hand.selected.json
python -B tools/build_idle_candidate_index.py --asset /private/r_hand.bmf /private/r_hand.selected.json --output /private/fresh-index.json
```

The verifier derives metadata again from the exact asset bytes and compares the
canonical JSON including value types, selected field order/count and scoped
result claims. It rejects changed metadata, duplicate keys and nonfinite JSON
constants. Successful verification applies to the supplied selected metadata
only; the unchanged index exporter continues to report its association flags
false. A caller can use producer/verifier success as separate evidence of static
selected-field derivation, never as native/GPU/loaded-resource evidence.

The policy records only type declaration facts, object identities and serialized
field extents. It contains no raw geometry, coordinates, buffer payloads, game
binary or decoded metadata. The layouts were established from the existing
typed object reader and exact span records, checked against both complete asset
pins. The earlier object-body roundtrip leaves 16 and 252076 bytes outside its
object domain respectively; neither reader certifies those trailing sections.
This producer reads selected fields and retains eight UV declarations per
surface, all referenced buffers and every selected surface in serialized order.
It does not interpret omitted fields or give a complete-file roundtrip claim.

## Offline implementation evidence

The 15 synthetic regression groups pass with normal Python and `python -O`.
They exercise unknown/changed fingerprints, type declarations and element graph,
table/object identity, framed reference/nested/palette/byte arrays, span budgets,
containment/order/overlap, malformed geometry, explicit unsupported candidates,
canonical metadata verification, exporter/replay interoperability and private
output collisions/rollback. Output is exclusive mode 0600. Existing outputs,
Git checkout destinations and existing or dangling output symlinks are rejected
before resolution; no caller-owned target is created or removed.

The delivered exporter’s 16 regression groups pass normally and with `python -O`.
A fresh native Release offline core build completes and all 66 existing CTest
groups pass. The new tests run explicitly, keeping this increment disjoint from
the delivered CMake change and the original writer. There is no native product,
protocol or shader change requiring new x86/x64 product packaging.

Read-only in-memory comparison against the earlier private decoded records
agrees on every selected object field and all eleven admitted candidate rows,
ranges, buffer/channel hashes and exported channel bytes. The hand supplies one
candidate; the gun supplies ten candidates plus one retained unsupported surface.
Only the metadata hash changes because the selected output omits unrelated
fields and includes scoped derivation receipts. No asset, private analysis file,
original working tree, runtime, display or gameplay state was changed.

Full native loaded-resource association, geometry-stage-3 collection, physical
grasp/controller correspondence and grip alignment remain outside this change.
The latest writer’s startup/collector work remains independent.

## Scoped source review

The independent reviewer returned SOURCE GO with no remaining findings. It
verified 204 layout/type/span comparisons against the recorded private evidence,
all selected fields/candidate bytes, ten pure synthetic groups normally and with
`-O`, and the bounded policy/evidence scope. A P2 dangling-output-symlink finding
was fixed by checking the raw path before resolution; the preservation regression
passes and a mutation restoring the old order fails as expected. The reviewer
did not independently run file-writing tests or builds; the implementation owner
ran the complete offline validation listed above. Astra/xhigh was requested, but
the tool did not expose effective current-turn model/effort for attestation.

The final read-only ownership check found the original writer clean at
`f4e7a10` (idle rejection/TLS cleanup checkpoint), with no overlap in these four
new files. The new increment depends on the separately delivered exporter commit
`0eff50d`; do not integrate it without that dependency.
