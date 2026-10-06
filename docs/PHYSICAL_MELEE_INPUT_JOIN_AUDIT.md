# Physical melee — native input join follow-up

Astra/xhigh read-only investigation; effective routing locally verified. No
edits, delegation or native execution. Main adopts NO-GO for an operator-only
brain-bit lease and the confirmed ordinary-trigger release path below. This
narrows the next architecture review; no gesture or brain mutation is added.

## Native operator and hand mapping

Sam2Game ExecuteOperatorFiring8E580,void thiscall(puppet),ret8E765, compares brain
byte160/current with puppet348/prior for raw bits0–3. Brain161 blocks all callbacks
for the bit; a blocked release clears blocking but does not invoke release.
Slots520/524/528 dispatch down/press/release. Primary press timestamps378/37C and
the entire native current byte commit at8E75A must remain native-owned.

Player hand enum0=left/1=right is established by FB520/104310 and weapon+BC;
handles player804/800. The enum is distinct from raw fire indices:

| Native mode | Right primary bit | Left primary bit |
|---|---|---|
| Single weapon present | 0 | 0 |
| Combo and player9BC nonzero, no flip | 0 | 1 |
| Same with flip | 1 | 0 |
| Fallback identical weapon IDs | 0 | 0 |
| Fallback distinct valid IDs | 1 | 0 |

Held mapping FB590; fallback-distinct reachability unproved. IsFlippingFireButtons
1022B0 requires combo/both weapons/9BC and uses9F0 or its inverse for gamepad.
After operator flipping, semantic release0 releases right and also left unless
combo+9BC; release1 releases left, or right alternative fire if left absent.
Fallback identical weapons therefore remain natively coupled. Current independent
held-query override alone does not prove independent native release in every mode.

Down callback is a no-op. Press7FFC0→DoAttack101F00 does not directly start a
primary0 weapon; held processing starts it. Release7FFE0→slot40C StopAttackFA770
dispatches weapon204 primary release or208 alternative release. Button callbacks
are thiscall(one32-bit argument),ret4. Exclude carrying callbacks.

## Existing commands already support ordinary release

engine.cpp command producer2771–2868 maps plcmdFire to right and plcmdAltFire to
left. Compatible high→low samples retain history; inactive controls allow native
release. Native command reads F34F2/F3530 produce bits01/02, then ProcessControls
EE0F0→ClientActionEE260→RPC EE441→accepted byte store EE53A. This is a supported
static standard-trigger release chain, not a confirmed general missing hook.

IsWeaponFiringPressedF6F00 maps FB590 and blocking-aware IsFireButtonPressed8E480.
Current fireQuery override bypasses those rules. OrderedPosePolicy retained taps
are frozen independently of native brain commands, so native release/blocking
and retained-pulse equivalence remain unproved. Do not add gesture fireMask alone.

## Lifetime and candidate lease limitations

Native OnStepA69F0 verifies DoYouOperateMeEDB00, processes controls, reacquires
brain and vehicle permission, then slot52C atA6E03/returnA6E09. Operator repeatedly
resolves brain handle puppet38C, including after callbacks; it retains no pin.
Core hvHandleToPointerB370 validates generation under a short CSyncLock that ends
before returning the pointer. hvPointerToHandleC450 can register a handle and
cannot serve as a pure cleanup identity lookup. No durable brain lifetime proved.

Existing multiplayer brain/avatar binding155–181 and simulation preparation921–965
plus freezeInput703–730 are reusable ownership/intent boundaries. An eventual
candidate must share one frozen intent with both operator and held consumers,
preserve native mapping/blocking/unrelated bits and history, guard exact native
A6E09/current simulation identities and reentrancy, and prove callback-safe
restoration. Native-finally supplies unwind cleanup, not pointer lifetime or
exclusion of intervening native writes. An operator-only lease restores too early
for later held queries and does not meet these requirements.

Main seeks a reviewed executable producer/join design preserving original commands,
weapon FSM, cadence/damage and RPC before adding any gesture. Smallest proof is
one right-hand CircularSaw neutral→held→neutral through command→brain→operator→
native release, alongside held mapping/blocking, then retained pulse and callback
lifetime. Full independent dual mode remains required beyond this first slice.

Verified SHA256: Sam2Game.dll
5628b4ed30a966f10c8e8ea46bf0789257a35ea80127bacacebe28b1ce5303df;
operator8E580,length1E6,
5e3f54ccf7a9fc240d575b41b9747b4bb6a8dd2069201a5a2f811511f77bca9f;
Core.dll7a1bd56b9bfa3edfbb23f4d3c96490e40a7c0b031b85e797e1af91b2ba3cf207.
