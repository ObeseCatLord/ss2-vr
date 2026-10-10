# Existing Serious Sam 2 tools — research, 2026-10-09

## Follow-up: editor mod isolation

A separate read-only public-source investigation checked the remaining editor,
SDK and authoring leads against the current project. Croteam's
[July 31, 2015 patch notes](https://store.steampowered.com/news/posts/?appids=204340&enddate=1439488115)
document a "Change active mod" file-menu item and dialog in Serious Editor 2.
The integration owner checked that primary page. This is an additional candidate
for isolating a future authored alignment/control fixture from base content;
its current filesystem behavior and export fidelity have not been exercised.
It is not a native plugin API or proof of mixed desktop/VR network compatibility.

The follow-up found no additional verified SS2 native SDK, plugin headers, symbols
or official scripting contract in its bounded primary-source search. Continue
using the already compared standalone parser for authored relationships and the
existing native observer for the actual loaded instance and evaluated palette.
The smallest editor evaluation is one characterized resource in a private copy,
checking names, transforms, bounds and animation channels against parser results.
No new download, installation, editor launch or runtime test was performed.

The most useful candidate is the bundled Serious Editor 2 with the official
Edit Data released on October 27, 2025. Croteam documents direct editing of
meshes, animations and skeletons. This can reduce authored asset decoding for
weapon alignment and physical vehicle controls. It cannot replace evidence of
which instance, animation/cache, shader and native owner actually render.

The research and read-only installed-file inventory were followed by the
bounded standalone parser evaluation below; Blender/editor integration is untested. The original installation was not modified and no editor/game was
launched. An Astra/xhigh research task supplied candidates; the integration owner
independently checked the primary sources below. Local routing was verified;
independent backend attestation is unavailable.

| Tool | Verified public capability | Practical use here | Remaining check |
| --- | --- | --- | --- |
| Serious Editor 2 and official Edit Data | Croteam's announcement says mesh, animation and skeleton editing is available. | Inspect authored shotgun bones/weights and vehicle Main/Seat/control relationships in an isolated copy. | Exact installed asset coverage and correspondence with pinned game binaries; separate redistribution terms. |
| Andrey/other1 SE2+ Blender importer/exporter | Author documents ASCII AMF/ASF/AAF import/export and binary mesh/skeleton/animation import from resource files, including embedded model data, without Edit Data; requires Blender 2.80 or newer. | Independent private previews and authored skeleton inspection may replace further custom extraction. | Current Blender compatibility, bone-index/name/transform fidelity, license and coordinate conventions. |
| AutoGro2018 | Maintainer describes dependency scanning and packing resources into one GRO archive. | Could simplify future private test-map dependency collection. | SS2 build support, complete dependency coverage and license; no need to replace the existing completed archive inventory. |
| Nokama0 SS2 Mod Menu | Maintainer supplies a Windows native mod, source/technical notes and Unlicense; advertises vanilla/Renovation/InSamnity support. | Already used as an ABI/hook research lead in this port; compare any new findings with owned binaries. | Upstream is single-player-only and uses a conflicting D3D9 proxy; do not co-install or treat it as multiplayer evidence. |
| Croteam public Serious-Engine repository | Repository explicitly targets classic Serious Engine 1.10. | General historical reference only. | Its native entities, networking and headers do not establish SS2/SE2 ABI or behavior. |

Primary sources:

- [Croteam's October 27, 2025 announcement](https://steamcommunity.com/games/204340/announcements/detail/511846967442148994), also available in the [official Steam news feed](https://store.steampowered.com/news/posts/?appids=204340).
- [Importer author's documentation](https://www.serioussite.ru/load/raznoe/serious_engine_2_import_ehksport_dlja_blender/10-1-0-3143).
- [AutoGro2018 maintainer repository](https://github.com/AsdolgTheMaker/AutoGro2018).
- [SS2 Mod Menu maintainer repository](https://github.com/Nokama0/Serious-Sam-2-Mod-Menu).
- [Croteam's classic-engine repository](https://github.com/Croteam-official/Serious-Engine).

## Local findings and adoption order

The protected installation contains SeriousEditor2.exe and mesh, animation,
skeleton, model, material, world and simulation EditKit DLLs. Fourteen installed
GRO inventories were inspected read-only. No standalone path matching the limited
Edit Data/help/SDK search was found; this does not rule out embedded edit data.
Exact private executable identity is recorded in the local handoff.

1. Check the bundled editor/help and resource import/export interfaces against a
   private copy of one already-characterized weapon and hovercraft resource.
   Preserve the original assets and do not upgrade the protected installation.
2. If the editor path is unsuitable, evaluate the Blender importer against known
   mesh counts, channel hashes, bone names/indices, transforms and bounds. Decline
   its output for implementation until those comparisons pass.
3. Prefer established asset tools for authored relationships; keep the existing
   native/API collector for event-time draw, evaluated animation/cache and owner
   relationships. A preview does not prove controller grasp, physical melee
   release, vehicle steering or multiplayer settlement.

The upstream mod menu's single-player support statement applies to that tool;
this port still requires mixed desktop/VR multiplayer with mod users.

A bounded public search found no verified SS2 native C++ SDK/template suitable
for replacing the current adapter. Historical SDK/tutorial package descriptions
are not evidence of current headers or source availability. Console/Macro guides
may help future test-map authoring, but no replacement for the project's verified
startup/save/isolation route was established. No new executable was downloaded,
installed or run, and no game content was added to public source.

## Standalone parser evaluated against characterized resources

The author-linked add-on ZIP was obtained privately and pinned at
f74594d02326cb86671b34967e88418bfd24435195d79e1a89481a5c49d345ed.
Its unchanged stream/meta/type-mapping modules were reviewed and loaded without
executing the Blender registration, import/export UI or mesh conversion. The
comparison process used a bounded CPU/memory budget and exact owned input bytes.
No installation or editor/game launch was needed.

Both previously characterized Auto Shotgun variants matched all five channel
hashes/ranges, both complete buffer hashes/lengths, counts, bounds and palette
names. The embedded Fighter resource matched three LODs/four surfaces, six whole
buffer hashes and Main/Seat parent/local transforms bit-for-bit as binary32.
This supports using the standalone parser for further private authored-resource
inspection. It does not certify Blender's converted scene, arbitrary resource
versions, animation evaluation, runtime loaded-instance association or grasp.
Raw resources, extracted source, comparison scripts and results remain private;
no redistribution license was established for the add-on.

Practical next use: read authored skeleton/mesh/animation relationships with this
parser instead of repeatedly writing a resource-specific decoder. Keep pinned
bytes and independent range/count/hash/reference checks before trusting a new
resource. The native observer is still needed for the actual evaluated Main/Seat
frame: Fighter Loading invalidates a fixed authored Seat-to-Main assumption.

## Further author/official documentation

A bounded follow-up subagent search found useful existing editor interfaces:

| Interface | Primary evidence | Use and limit |
| --- | --- | --- |
| Editor Entity ID and debug-variable menu | [Croteam's 2021 update announcement](https://steamcommunity.com/app/204340/discussions/0/3100138655163313155/) documents visible Entity IDs and the `.` debug menu. | Useful for inspecting an authored entity and available debug controls. No mapping to runtime pointer, network identity or render ownership is established. |
| AWF world export | [Ryason55's own map-port description](https://steamcommunity.com/sharedfiles/filedetails/?id=2993154487) credits SE2 AWF export and other1's Blender tools. | Possible authored world-placement inspection route. The destination is Fusion; its weapon scripts and multiplayer limitations are not SS2 behavior. Export fidelity remains untested here. |
| Native weapon adjustment and left-weapon controls | [Croteam's October 2025 notes](https://store.steampowered.com/news/posts/?appids=204340) name the weapon-adjustment cheat and describe enabling dual wield through the left-weapon toggle when Combo Weapons is active. | Concrete native control names for investigation. No new independent hand-aim, eligibility, placement or replication proof follows from release notes. |

The integration owner checked these primary pages. Historical help/LightWave and
developer-diary mirrors are leads only; their current installed interfaces were
not independently established. No additional current SS2 native SDK, symbol
package or hook framework was verified in the bounded search. This is not a
claim that none exists. No editor installation or launch is needed for the
standalone parser already evaluated.

That parser was additionally used, unchanged, on three characterized Fighter
parameter resources and the Saucer parameter resource. Authored seat names,
attachment names and ride-part names matched the independent decoder normally
and with Python optimization. This expands practical parser use to parameter
relationships. Its serialized SPuppetSeatData object is not a live native
CPuppetSeatData pointer. Exact private identities/results remain local; no
third-party source or game resource was vendored or published.

Next use is authored resource/reference inspection through the existing parser,
then comparison with the current loaded model and occupied native seat. An editor
preview or authored name match cannot supply that live association. Keep native
draw/animation ownership evidence and the current D3D9-to-D3D11 architecture.


The same unchanged standalone parser also matched all 20 Fighter/Saucer child
configuration descriptors, including exact pose/scale words and null configuration
references. This exposed why a normal rendered-seat-child requirement would be
nonfunctional for these assets. The source uses declared flat metadata instead;
no Blender conversion, native child evaluation or physical controls pass is implied.
