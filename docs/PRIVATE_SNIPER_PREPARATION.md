# Private sniper fixture preparation

The user authorized verified cheats or save editing to create a sniper fixture.
This adapter uses the stock cheat, inventory, selection and save implementation
inside a fresh isolated single-player lab. It does not enable cheats for normal
gameplay or online sessions. Alignment is still inactive pending native draw
association. No preparation run has yet been accepted.

The verified data argument is `+sam_iEnableCheats 1`; the stock argument dispatcher
assigns the registered enum. Native host update propagates it into game-info
cheating state. That state is sticky: restoring the setting does not certify a
non-cheated save. Keep this fixture private and separate from normal profiles.

The native menu's Give All action reaches game-info `OnGiveAllCheat` (Sam2Game
RVA `0xd5b70`), which retains native local-player targeting, weapon-mask and ammo
rules. After confirming ID13 entered inventory, the adapter calls the same
`SetNewWeapon(13,1,0,0)` route used by native selection (RVA `0x102340`). Selection
retains native eligibility and transitions; a void return is not equip success.
No manual inventory bits, transition flags, zoom deactivation or release priming
are written.

After observing the current owned right-hand sniper in state1, unzoomed and
nonaliased, the adapter invokes the normal save entry (RVA `0x22f00`). The new fixed
destination is `Temp/SS2VR/sniper-id13.sav` beneath the exact private game's root.
Both the save and possible `.sav.preload` destination must be absent. Native save
may temporarily switch worlds; no borrowed weapon observation survives it.
Only a later completed simulation interval may confirm restored ownership,
idle sniper state and actual output.

The adapter is absent from normal behavior unless exact private opt-ins select
`SS2VR_LAB_PREPARE_SNIPER=1` and idle ID13, with installed online isolation. It runs
after original simulation work on the main thread, outside native presentation.
Its roster guard pins game-info's brain/puppet getter targets and requires exactly
one populated native slot, cross-linked to the same owned local pawn and brain.
It holds no snapshot or multiplayer lock across native callbacks.

Each native action is issued once, with its persistent stage advanced before the
call and a busy guard established before callback-capable validation. Progress
requires a later outer simulation tick and fresh neutral eligibility. Exceptions,
identity loss or the bounded preparation deadline permanently stop the adapter;
it does not retry grant, selection or save. Native firing/movement input remains
zero. This preparation does not prove physical first-use melee release.

The launcher requires `prepare_sniper_fixture: true`, exact neutral ID13 selection,
a sealed isolated lab and the verified cheat argument. Before capturing baseline
eyes it requires one ordered grant/select/save/complete receipt with a single
owner, the exact private destination, nonempty bounded save bytes and hashes, and
observer confirmation of right-hand ID13. The optional preload companion is also
hashed. Records explicitly label the fixture cheated, reload unverified and
alignment unaccepted. Opened-world identity remains the verified archived scene;
do not label that scene receipt as a save reload.

Native `.sav` reload through `+level` is statically verified, but the current
archive-scene harness does not admit loose saves. Preparation can collect ID13 in
the existing scene without reload. A later save-loading extension must separately
certify loaded save bytes and world identity.

Owned module fingerprints match the installed-build gate. Astra/xhigh verified
native metadata/dispatch and the main independently checked grant/save callsites,
getter exports, selection vtable target and enum13. Source review and actual
preparation acceptance are separate; no new runtime result is claimed here.

## Source review disposition

| Astra finding | Disposition |
| --- | --- |
| A contained callback failure can return normally and permit a later stage | Adopted: actual shared progress cleanup latches Failed on abort or unhealthy interval. Regression covers a fresh later interval and nested admission. |
| Nested simulation can invalidate an otherwise plausible owner check | Adopted: pin exact native caller, current interval/world/manager, cleared in-step flag and unchanged revision; recheck presentation idle before every action. |
| Selection can legitimately invalidate old weapon snapshots | Reacquire current bindings on a later tick; do not carry tracking generation. |
| Postprocessing could consume a different candidate index than the preparation seal | Private wrapper compares consumed receipt hash before its readiness summary. |
| Save return alone or an idle image establishes completion | Rejected: later restored owner/sniper and actual file hashes required; reload/grasp/alignment remain unverified. |

Astra/xhigh scoped native source GO. All five current local native-review turn
tags were verified; independent backend attestation unavailable. All four products
rebuilt on `d458bd6474c1b12cd89996c2b3fa66b30a2c70859a87164a880a209ffb4a4168`,
IPC10/wire7.77Debug/77Release checks and36normal/optimized native/compiled gates
pass. Runtime evidence tests26groups pass normal/-O. The private collection path
has no remaining reviewed source blocker after its consumed-index seal check;
actual grant/equip/save and ID13 draw association remain the next native probe.
