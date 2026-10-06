# Saw recoil schedule — Astra proof and disposition

Fresh gpt-6-astra/xhigh; current routing and all four inspected pins verified.
Static native inspection plus Linux production-helper scratch only. **YES: the
second-pulse release miss is reachable under permitted native scheduling.**
Main rejects the concrete state4/sound1-or2 canonical-stop predicate. This does
not approve a new registry/protocol or exhaust all native metadata possibilities.

Engine Step1B70E0 is voidthiscall. At1B719E..1B71F7 it clamps sim_fStepMin
to[.001,.1] seconds and sim_fStepMax to[min,.1]; pinned defaults are .001/.1,
real-time factor1 and sync rate0. AdvanceCursorTime1B4A90 uses elapsed uptime
for rate0; positive rate uses1/rate. The admitted step is stored atsim+30;
simulation time advances1B72CE..1B7306 before entity manager1B739B. No mandatory
50ms tick excludes the pulse. Sam2.exe15C7→gameproject254D0→EngineStep258D8
is frame-driven; active limiter default10000FPS, inactive20FPS.

Held eligibility uses simNow at4F064 with comparison4F072..7A and held4F090.
Scheduled recoil completion instead calls SetNextThink from4D91B through7700
toEngine58FC0(ret8); scheduling1B7EC0→187C90 uses the state manager's stored
time. That manager updates its time after entity OnSteps1B3C86..99, then runs
timers1B3CA2. Entity dispatch is earlier1B3C51. Uniform20ms steps can therefore
complete recoil sooner than assumed; the proof must account for both deadlines.

| Permitted native time / level | Verified resulting condition |
| --- | --- |
| Prior completed step−5ms; fire at0/high | State8/E8=2; held deadline≈50ms; scheduled completion≈45ms |
|20ms/low | Canonical stop leaves8/E8=3,38=1 |
|40ms/high | Held time gate suppresses query; no start/fire; following timer pass still before45ms |
|60ms/low before original OnStep | State8/E8=3; proposed predicate false, second release bookkeeping missing |
|60ms later | Held-low and eventual scheduled completion do not supply omitted canonical player stop |

Native Time(float) ticks are seconds×2^32:20ms=85,899,344;5ms=21,474,836;
recoil=214,748,368. Thus2×step<recoil−precursor<recoil<3×step, with adequate
rounding margin. This establishes permitted control flow, not measured FPS.

Production manager preparation samples/freezes local input before entity step;
the player's prepared OnStep does not resample. Host input publication is
independent of render-slot availability; producer freshness/sequence/pose checks
impose no50ms dwell and no equality of source and simulation clock intervals.
Single-player needs no ACK. Reviewer reran the actual producer/gate scratch and
confirmed low20/high40/low60 admission. That closes the previous schedule edge.

The minimal existing-state candidate is disproved. Reconsider exact native
metadata or the commit-coupled fallback only with their remaining evidence,
while retaining native combat, manual carry and copy behavior.
