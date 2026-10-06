# Uncoupled identical dual weapons — native capability

Astra/xhigh read-only owned-resource/native investigation; effective routing
verified locally in aggregate. No edits, delegation or runtime. GO for stock
menu plus original native dual route; NO-GO for assuming loaded capability from
archive filenames. Main retains native inventory/weapon transitions/RPC.

## Typed owned resource evidence

Member: Content/SeriousSam2/Databases/GlobalGameParameters.rsc. Offsets are within
the decompressed resource. Only three matching root archive members were found;
no matching loose stock resource was found.

| Archive | Combo field offset | Four serialized bytes | Value |
|---|---|---|---|
| All_PC_01.gro |2082|00000000|CWU_NEVER0|
| Patch_02_090_593613.gro |26DB|02000000|CWU_ALLOW_CUSTOM2|
| Patch_02_100_800000.gro |26DB|02000000|CWU_ALLOW_CUSTOM2|

Latest patch metadata: CGlobalGameParams type6B5, member nameC40 referencing
enum type31, ComboWeaponsUsing metadata195E/four-byte storage. Object1 payload
258F–294B; complete stream validates through EDOB2A9F/METAEND2AA7. Native enum
Sam2Game4C50B8 values0never/1item-controlled/2allow-custom; property descriptor
4C1A00 maps ggp_cwuComboWeaponsUsing to native object18C. No extracted resource
or disassembly is committed.

Core4C267–4C298 assigns increasing archive priorities; comparator4CC30 sorts
duplicates by descending priority and4E520/4E640–4E6AB retains the first. Later
enumerated archives win. Enumeration uses _wfindfirst/_wfindnext65AFE/65BEC;
static evidence does not prove mounted enumeration order, mod or overrides.
Loaded effective value remains unknown until the actual native getter admits it.

## Stock configuration and native getter

Stock CMSGameOptions, CMSSinglePlayerSettings and CMSCooperativeSettings resources
all expose USE COMBO WEAPONS with YES/NO options. GameOptions handler1DC3D0 calls
SyncOptionsFromGadgets1DC0A0; selected index0 becomes true1DC344–1DC34E, preference
60 at1DC351 and actual GameInfo144 at1DC35B. SP1EFE80 stores preference; coop1D7960
updates preference/session settings. Calling handlers without real menu/gadget
context is not an established setter. Set YES through the existing UI.

Export ?IsComboWeaponsEnabled@CGameInfoEntity@SeriousEngine@@QAEHXZ, RVA D6640,
is int thiscall(GameInfo*), no explicit arguments. It requires loaded global18C==2
atD66BC, GameInfo144!=0 atD66C8, modeE0==1or2 atD66D2–D66E0. The getter can
perform native copy-on-write: revalidate context/handles after native calls.
samGetGameInfoEntity20A720 returns through a caller-owned four-byte buffer;
the existing multiplayer.cpp GameGet/resolve route supplies the reference.

Require original IsDualWieldingFB040 true: both handles resolve and native9BC
is nonzero. Existing guarded engine preparation already calls original
ToggleDualWieldingFB090(player,0),void thiscall(int),ret4. Native code owns toggle
writeFB16D and carry/vehicle/weapon transitions; RPC-capable FB300 also remains.
Do not add another toggle loop or force9BC. Missing hands can make IsDual false;
toggle is not a setter.

Actual combo capability plus valid dual state permits native held mappingFB590,
flip1022B0 and releaseFA770 to uncouple identical weapons. Fallback identical IDs
share bit0. The five-site primary adapter must observe this capability and native
mapping rather than manufacture independent semantics. Source getter integration
and actual gameplay remain pending; no runtime outcome is claimed.

SHA256: Sam2Game5628b4ed30a966f10c8e8ea46bf0789257a35ea80127bacacebe28b1ce5303df;
Core7a1bd56b9bfa3edfbb23f4d3c96490e40a7c0b031b85e797e1af91b2ba3cf207;
latest patch resource495e830654eac1871d34ef019a289a5723d37c8e2bdcfeb43a6a54117dc2b4f8.
