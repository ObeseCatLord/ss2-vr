# Existing Serious Sam 2 tools — research, 2026-10-09

The most useful candidate is the bundled Serious Editor 2 with the official
Edit Data released on October 27, 2025. Croteam documents direct editing of
meshes, animations and skeletons. This can reduce authored asset decoding for
weapon alignment and physical vehicle controls. It cannot replace evidence of
which instance, animation/cache, shader and native owner actually render.

This is research and a read-only installed-file inventory, not a tested tool
integration. The original installation was not modified and no editor/game was
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
