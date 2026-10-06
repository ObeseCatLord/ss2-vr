# Physical primary — Astra design disposition

Explicit fresh reviewer `gpt-6-astra`/`xhigh`; effective settings verified as
aggregate only. Base4baa4e8. Read-only source review; no runtime execution.
Bounded game-side architecture GO; initial helper wiring NO-GO pending fixes.

| Finding | Disposition |
| --- | --- |
| Inactive hysteresis is not fresh quiet; inferred neutral could defeat ACK rearming | Adopt: explicit fresh quiet witness, consumed by existing gate/neutral sampler |
| Backward sequence/time must not establish a baseline | Adopt: preserve watermark and reject replay; corrected while review ran |
| Grip-to-aim position fallback manufactures motion | Adopt: gesture requires actual finite valid grip plus aim; other weaponTracking users unchanged |
| Native calls may change identity after fire calculation | Adopt: one sample, late checks only revoke and establish a later-input boundary; no second motion evaluation |
| Broad tracking generation couples snap-turn and other-hand equip | Adopt: explicit per-hand identity and raw action provenance; existing broader admission remains separate |
| Same derived sample must feed packet/commands/ACK/native join | Adopt: logical fields in existing snapshots/control cache; raw XR/menu/render fields remain original |

No new transport, physics, combat, history, ACK scheduler or trigger-arming FSM.
Normal primary edges do not advance source generation. Portable tests must cover
quiet→new ACK→inactive hysteresis, grip loss/recovery, independent manual/gesture
sources and retained high→low; helper tests alone do not prove native wiring.
Sampling cadence and player comfort remain unverified tuning, not physical XR
velocity. Roomscale/camera/scopes remain separate active requirements.
