# Reproducible private idle candidate index

`tools/build_idle_candidate_index.py` replaces the fixed-filename private exporter
with explicit asset/decoded-metadata inputs and a fresh private index output.
It preserves the asset-keyed dictionary and candidate row fields consumed by
`match_idle_geometry.py` and `replay_idle_geometry.py`. Assets must be inside the
index's parent directory, which remains the replay candidate root. Multiple
assets may occupy subdirectories; generated channel filenames use input ordinals
to distinguish identical basenames.

Example, with private directories chosen by the operator:

```bash
python tools/build_idle_candidate_index.py \
  --asset "$PRIVATE_CANDIDATES/r_hand.bmf" "$PRIVATE_METADATA/r_hand.bmf.decoded.json" \
  --asset "$PRIVATE_CANDIDATES/zapgun.bmf" "$PRIVATE_METADATA/zapgun.bmf.decoded.json" \
  --output "$PRIVATE_CANDIDATES/fresh-candidate-index.json"
```

This is the existing decoded `CRenderMesh` / `GFXHANDLE` domain, not a BMF parser.
Mesh fields 12/13 identify index/vertex buffer reference tables; LOD field 5,
section field 1 and surface fields 6/7/10/11/14 identify the known channels.
Object 0 is a valid decoded identity. References must identify actual typed
buffer objects, and bytes must be integers in 0..255. Channel offsets address
those decoded byte arrays; they are never offsets into the asset file.

The supported surface domain remains 1490 vertices/1332 triangles, matching the
existing collector. Positions FLOAT3, indices UINT16 triangles, weights/local
indices packed four-byte channels and UV0 FLOAT2 retain formats 133/135/128/128/132.
Vertex channels must share the selected decoded vertex buffer. Exact first-local
palette influence remains weights 255,0,0,0 and local indices 0,0,0,0. The historical
`single_body_influence` field preserves that byte predicate; it does not identify
a skeleton bone or an actual runtime palette. File-local palette identifiers
remain distinct from runtime bone IDs.

Typed counts, object/reference/list budgets, buffer slots/ranges, finite positions
and UVs, triangle indices and palette identifiers are validated before export.
Missing/unsupported channels produce explicit unsupported rows. They cannot hide
corrupt supported ranges, nonfinite geometry or malformed palette identifiers.
All channel slices, whole buffers, asset and decoded metadata receive SHA-256
identities. The supplied metadata's association with the asset is explicitly
unverified; a hash is not a decoder round trip or historical-loaded-byte proof.

Inputs/output must remain outside source/Git checkouts. Inputs are read-only;
the output directory must already exist. Existing indexes and channel files are
refused even when bytes agree. New files use exclusive creation and private
permissions; the index is written last. On failure, rollback attempts all still
owned exported paths, preserves changed identities, and reports incomplete
cleanup chained to the original failure. Renamed exports are not reclaimed from
unknown paths. This is bounded file ownership, not a transaction against
arbitrary concurrent directory renames.

All live-association, effective-pose, grasp, decoded-asset association,
historical-loaded-byte, positive-grasp and alignment flags remain false. A generated
candidate is not a rendered-instance association or an accepted grip correction.
No native execution, GPU execution, game launch or runtime probe occurs.

## Offline verification

The 16 synthetic regression groups pass normally and with Python optimization.
They exercise the existing replay channel reader, exact bytes/hashes/ranges,
offsets beyond the decoy asset length, missing channels, mixed missing/corrupt
data, typed references/counts/bytes, nonfinite geometry, malformed triangle
indices, source/output protections, existing/late collisions, failed file
wrapping and rollback with disappeared/replaced exports. No proprietary fixture
is in source. The new check is registered in the core CMake suite.

A read-only, in-memory compatibility comparison with the existing private index
agrees on all 11 candidate rows from the two supplied decoded assets: exact
channel bytes/hashes/ranges and existing row fields agree. Generated filenames
intentionally differ. This comparison wrote no private asset/index/channel file
and establishes no native association or alignment evidence.

The complete 66 core groups pass in Debug and Release. The x86 game/server and
x64 host/official-loader rebuilds pass with matching IPC10/wire7 contracts.
The separate strict-primary-trigger fix is a preceding independent commit;
this exporter does not depend on it. Both commits remain local for coordinated
integration. Original writer collector files and runtime products remain outside
this change.

Astra/xhigh gave scoped source GO after validation-order and rollback findings
were corrected. The reviewer independently checked those fixes in memory;
effective backend model/effort introspection remains unavailable. Source GO and
offline results do not certify any runtime acceptance row.
