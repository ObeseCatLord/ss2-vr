# Reviewed parallel source integration

The original integration owner read the full handoff and verified the source-only
bundle SHA-256 `45f82f5642b5ecdc8ee0f74f12eb348e85e2693bcaf8a817b4b163b07037ff2d`.
Its prerequisite was already present. The four commits were cherry-picked in order
onto `bb95eb45439abc8af3c8aebbea06b9bf467e0785`, preserving all intervening collector
changes and the original installation/checkout/WIP. No conflicts occurred.

| Delivered commit | Integrated commit | Change |
| --- | --- | --- |
| `4818ee12dac2b91c1513edb4fedc1bbffe6fa9ee` | `bc9f958f1caa84b7b117d2353587797ac5c9fd54` | Reject malformed trigger samples as action loss |
| `0eff50d01340f770ad707edfa3c313c74c9806e3` | `27bc9085b73d6a98105bb8025b95cc528b975a15` | Bounded private candidate index exporter |
| `a25061141b8e3bf779b0883a75962e6e27af94ef` | `695ddb5d368e0b0bc890dec1b762292ce28f2396` | Pinned selected-field metadata producer |
| `aa17a2c2f7f268e52bd0443a119c0d0ce993ffc2` | `f1bb7f5e5eac7221801f195f4b5726dc4b0b6dcd` | Register producer regressions |

## Combined verification

All x86 game/server and x64 host/official-loader products rebuilt. All67Debug and
67Release CTest groups pass. Both new suites pass with Python optimization
(16exporter +15producer groups). All28normal/optimized native/compiled checks pass,
including both x86 products' idle, melee, muzzle, roomscale and MP boundaries.
Existing dependency environment was reused. No runtime or original-file deployment
occurred. No workflows were added; Actions remains disabled. Full ancestry gate
passes with62reachable commits and empty author/committer emails at the integrated
tip. Native association and hardware acceptance are not inferred from these checks.

Build input fingerprint: `162790f5b7f37e6887ff76d44a5f76319af8cdd0fee960c6043cd95d68f8ad85`. IPC10/wire7;
Input272, Request440, Ui352, Slot33554928, Shared83887840 agree across architectures.

| Product | SHA-256 |
| --- | --- |
| `d3d9.dll` | `ed3c66855969c9986831a1dd213be38306291bfb516f519d9f7f6bd1e4910493` |
| `ss2vr_host.exe` | `d43b2d9a0c5beda4f18c070578c8c25406e4b33cb6013a8a0fe3dab3499ff3e7` |
| `libopenxr_loader.dll` | `bb011caa82528c541a73967ce6408f82198ff4fd0358b38b54884719d863bd1d` |
| `SS2VRServer.dll` | `552942e9ae32074d53c20fd10f39ab88bb61c6b274bbb3c2873e4b35da1bc84a` |

## Review and evidence scope

The delivered reviews gave scoped source GO, with requested Astra/xhigh routing
unattested. The integrated metadata contributions received a fresh scoped source GO from
Astra; local current-turn metadata reports gpt-6-astra/xhigh, independent backend
attestation remains unavailable. Main verified
trigger producer/generation compatibility, narrow host changes, test registration,
private-output protections and unchanged evidence flags. The exporter consumes
selected metadata; the producer derives selected fields only from exact pins.
Neither proves complete-file decoding, native/GPU association, winning cache
ownership, physical grasp or alignment. Private assets, payloads, observations
and transfer handoffs remain local.

The latest neutral capture predates integration and remains immutable. It exposed
native declared7/8 streams versus collector5/6 selection; its rejection is a
concrete next adapter question, not fixed by these metadata tools. No acceptance
gate was removed, no old eye event was reused and no release was primed.

| Integration recommendation | Disposition |
| --- | --- |
| Preserve selected-field evidence limits and false native association flags | Adopted; unchanged producer/exporter/reader claims checked |
| Preserve existing matcher/replay schema and native collector changes | Adopted; contribution files match delivered source, adjacent tools unchanged |
| Rebuild final integrated products and use fresh seals before any capture | Adopted; all products rebuilt, no old product import or attempted-fixture reuse |

The reviewer independently compared fixed commits and prior hashes, inspected
layout policy and checked the scoped diff. Main executed the aggregate builds,
normal/optimized suites and compiled/native gates. Review covers the integrated
metadata slice; it does not give full-project or runtime acceptance.
