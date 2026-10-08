# Native dual-wield exploratory gameplay probe

Full native dual wield remains required, including multiplayer. Native eligibility,
projectiles, ammo, charge, reload and command history are retained. Two displayed
weapons and source tests do not establish independent firing or complete compatibility.

## Corrected starting fixture

Live Jungle runs190009 and190429 publish weapon IDs[1,1], with ammo[-1,-1]. The
weapon ID comes directly from native weapon+B4. Astra/xhigh verified ID1 maps to
ZapGunWeapon.ep, while ID12 is Colt. The earlier pictured-gold-Colt assumption was
incorrect. The Colt-only fixture was never run to a firing acceptance result.

| Native evidence / recommendation | Disposition |
| --- | --- |
| samGetWeaponParamsPath217060 branch21707D maps1 to ZapGunWeapon.ep; constructor175010 writes1 and vtable2D2CA0 | Require actual two ID1 weapons; never infer class from pictured appearance. |
| Ammo mapper2172D0 gives1 the index−1; F59F0 returns−1 and F5BB0 accepts without decrement, while inventory ownership remains required | Unchanged ammo is expected. Finite ammo acceptance requires another supported fixture. |
| Generic FB090 dual toggle retains carry/mount, virtual veto, excluded ID11 and state1/7 checks; Zap veto returns0 | Preserve native eligibility. Live combo/dual and distinct identities/buttons remain required. |
| FB040 dual state plus FB590 combo/hand/flip mapping determines independent buttons | Record actual compatible topology; successful firing is a separate receipt. No forced combinations. |
| Zap vtable244 uses baseDoTheFiring4CDC0; OnFire1756C0 obtains placement and reaches player projectile route8AA20→9DE30 | Observe original normal returns; projectiles/impacts/damage need separate actual image review. |
| Zap vtable204 uses baseOnFireReleased48FF0; native charging release can invoke4CDC0 | Evaluate a complete press–release cycle. Do not require a charged gun to fire during the high phase. |
| Native OnChargingEnd493B0 can fire automatically at full charge | Retained-hand phases preserve ordinary held input; neither timing nor exact shot rate is replaced. |
| Zap also has a separate reload counter+D0 despite infinite inventory ammo | Preserve native reload. Constructor defaults are not verified loaded parameters or runtime cadence. |
| Native handle lookup does not pin callback receivers | Copy incoming scalars before originals; no receiver read after return. Addresses are identity hints, not lifetime/ABA proof. |

Astra explicitly used xhigh, with local current-turn model/effort tags verified;
effective backend introspection was unavailable. Static investigation made no
source edits or runtime/input changes.

## Implemented bounded observation

SS2VR_LAB_TRACE=1 installs the base release observer; production remains unchanged.
It forwards the original void method once with native-finally propagation. Cleanup
stores scalar flags only; file logging occurs after return. No new native getter,
receiver post-read, physical-melee consumption or manual160/348 history write is added.
Fire/release receipts copy owner/weapon/hand/state plus input tick, triggers and
logical action generations. Topology records existing native getter results.

Config native_dual_probe=zap-initial-inventory selects ordinary simulated input:
neutral → right → neutral → left → neutral → both → right retained → neutral →
both → left retained → final neutral. Loaded timing is unknown; durations are
bounded exploratory observation windows, not native cooldown policy.

Each phase requires fresh alive gameplay, both native ID1 weapons and tracked
primary actions, closed wheels, frozen tracking/action identity and normal native
foreground/exclusive input. Initial controller convergence is allowed; every later
contradiction and input regression is rejected. High phases require original
compatible topology. Falling edges close cycles only when the same native pair has
successful normal fire and release receipts; a release receipt must observe that
hand neutral. Native release-generated shots are allowed. Neutral phases require
advancing UI and a measured quiet interval after release, with stable fire counters.
This is bounded cessation evidence, not a future-lifetime guarantee.

Active/retained/final-neutral native eye images are tied to confirmed input,
expected controls and frozen session/reference/tracking/action identity, followed
by readiness recheck. Trace budgets are65536 topology and256 fire/release events;
explicit exhaustion invalidates evidence. Cleanup sends ordinary neutral again;
failure invalidates completion. Cleanup itself can legitimately discharge a charged
Zap gun. No native history or gameplay policy is changed to avoid that effect.

## Review and acceptance limits

Astra's earlier source NO-GO caught logging during native unwind, compatibility
mislabeling, wrong local generation, silent trace exhaustion, contradictory sample
filtering and insufficient phase-image/neutral binding. These findings were adopted.
Complete compiled cleanup symbols, including abnormal branches, contain no calls
or external tail jumps in the reviewed client/server products. Fresh source changes
require full rebuild, exact package/helper/harness hashes and refreshed checks.

The corrected generic observation slice received conditional source GO. Astra then
rejected the unchanged Colt probe and approved a native-ID1 adaptation conditionally
on charge/release cycle semantics. Final adapted source review gives conditional GO after correcting a pre-rise
attribution error: each cycle starts with the first confirmed high sample,
including that sample and excluding previous neutral convergence packets. Actual
completion timestamps anchor quiet UI observations. Sixteen negative controls pass.
Current compiled source88717c8e229682cd15f3f814df612221dbff5cf6b402556690b98791c32eded3
has all61 Debug groups and compiled artifact/layout/native boundary checks, with
complete cleanup symbols checked again. Fresh matching package/helper/harness
pins remain required before execution. No firing,
impact, aim, reload, lifecycle, hardware or multiplayer compatibility pass exists yet.
The complete later matrix covers supported combinations, equip/unequip, finite ammo,
scopes, death/respawn, multiplayer and both required platform/device targets.
