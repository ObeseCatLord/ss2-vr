# Saw level-stop gate

Fresh Astra/xhigh, effective current turn and three owned module pins verified.
Static native inspection only. **NO-GO** for the concrete predicate
`!L && (B0==4 || E8 in {1,2})` followed by canonical player StopAttack.
This is not an impossibility proof for all existing native fields.

| Native path | Verified consequence |
| --- | --- |
| Saw constructor164680/16471D | E8=0; saw flags60=2 |
| State1/7 held4EF85→165460→base4CC40/FA550 | Native start attempt can select4 |
| State4 ready4EF3F→49100 | Fire before later held check |
| Successful49100→4CDC0→1654B0/4CB70 | Sound request2 at1654CC then native cutting callback |
| Recoil49314→4D870/4D87D | State8; default0.05 seconds; held4F090 only after time gate |
| Release165490→48FF0 | First-shot38 becomes1;4CAA0 changes4→1; state8 remains8; sound3 at16549C |
| Recoil completion4D930 | Reload/usability/idle path plus owner3C4; no sawE8 write |

E8 is the last recorded cutting-sound request, not current playback or accepted
firing. The setter164F80 writesE8 at1650AC or1650FB after native sound/stop
work. Its direct callers are start16547C(request1), successful fire1654CC(2),
and release16549C(3). Start's request occurs even after rejected base admission,
unlessE8 already1/2. Bring-up/put-down164DF0 usesE4, notE8. No automatic sound
completion reset to0 was found. Invalid/absent parameter rejection before write
exists; ordinary stock reachability of that failure was not established.

The predicate handles low after initial start, rejected start or active recoil.
It must also suppress tracked-handheld manual release while logical input remains
high; native carry keeps its original manual dispatch. However, after release
in state8/E8=3, another high/low pulse before a start/fire callback leavesE8=3.
The predicate then misses that release. Calling StopAttack on every low duplicates
bookkeeping instead: FA770 first calls7F450, conditionally clears player1C8 when
C0==6, then MarkNonIdle7F0F0 updates148/14C before choosing a weapon.

Native MoveWeaponParts4A440/4A4A7 can change54/58 from held input, so all native
memory is not claimed identical across that pulse. Those fields were not proved
release witnesses; canonical release does not reset them.

Canonical targeting uses semantic post-flip buttons, not hand numbers. Valid
combo+9BC routes0→right atFA7D3 and1→left atFA859. Coupled fallback0 releases
both right/left(FA7D3/FA827). With no left, button1 invokes right alternative
releaseFA886. StopAttack ABI is voidthiscall(player,int32), ret4. No synthesized
call is approved without exact native routing/admission.

## Production input spot-check and remaining schedule edge

A Linux C++20 scratch trace used actual GameplayPrimaryProducer/TriggerGate,
manual0, valid LOCAL poses and20ms input/sampling increments. It admitted:
initial high, continued high, low20ms after the designated recoil start,
new high40ms, new low60ms. This is a production-helper data-flow result, not
native execution or proof that the game schedules20ms actor steps.

The end of the second pulse need not precede0.05s recoil eligibility: if its
high was suppressed by the native time gate and low arrives at the next step,
the pre-OnStep predicate still sees state8/E8=3. Native completion/held-low does
not supply the missing canonical player bookkeeping. This narrows the schedule
question beyond requiring the whole pulse to fit within recoil.

Remaining proof: establish or exclude this sequence under native actor timing
and actual production sampling order, especially an admitted20ms step. No added
history/carry-selector architecture is justified solely by this partial trace.
