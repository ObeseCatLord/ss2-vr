# Current completion audit — immersive extension

The original baseline record below is historical. Current scope includes required multiplayer and all immersive follow-ups; teleport and actual runtime testing are excluded. Current IPC6/wire4 products implement local XYZ plus pitch/yaw/roll head/hands, asymmetric native stereo, dual native weapons/wheels, collision lasers, threshold-follow panels, native menu pointing, native multiplayer authority and remote weapon transforms.

The head follow-up adds source for native remote Head/descendant palette adaptation shared by CPU/GPU skinning. It remains explicitly default-disabled because complete native worker/model-lifetime coverage is unproven. Astra source review and follow-up approved default-disabled integration; final builds and artifact checks pass. Native pair admission/commit now has immutable remote samples and local invalidation history, with locks held through Ready. There is no anatomical head/neck IK or head-attached child-model claim.

Nine offline groups pass. Proxy/server and host build, and static verification confirms34 exported hooks, four audited internal entries (one optional), matching x86/x64 IPC6 layouts. No game, host, Wine, OpenXR runtime, headset or network session was launched. Roomscale body collision, physical sniper optics, vehicles, full native overlays and physical melee remain required unfinished work.

## Historical baseline record

### Completion audit — 2026-10-02

Deliverable: a separate source repository and installable **development build**, with OpenXR stereo, tracked independent native weapons and per-hand wheels. Actual game, host, Wine, headset and runtime execution were explicitly excluded by the user. Compilation, static binary inspection and offline checks are the evidence here; none establishes tested playability.

## Requirement disposition

| Requirement | Concrete implementation/evidence | Result |
|---|---|---|
| New folder, Git repo and agent instructions | Repository `ss2-vr`, root AGENTS.md, original MIT source, pinned dependencies and redistribution notices | Delivered |
| Research VR modding and installed game | RESEARCH.md, installed-build.json, MODDING_PLAN.md, native ABI/renderer/anchor records; primary Khronos/Microsoft/Croteam/community sources | Recorded |
| Astra senior review and final plan before execution | REVIEW_BRIEF.md, ASTRA_REVIEW.md, REVIEW_DISPOSITION.md, FINAL_PLAN.md; anchor and transport amendments explicitly reviewed at xhigh | Completed with dispositions |
| OpenXR stereo and HMD tracking | x64 host real instance/session/actions/LOCAL+VIEW/view poses/FOV/D3D11 swapchains; x86 native Render3D preparation/collection/execution/free per eye; exact fingerprint and layout gates | Implemented and built |
| Independent tracked dual wielding | Native per-hand selection/dual mode; handle-specific firing and base/sniper shooting/model hooks; authored offsets retained camera-relative; simulation uses original native inventory/ammo/cadence/damage | Implemented and built |
| Convenient per-hand wheels | Independent grip-held, frozen controller quads with stock labels/ammo/highlight, stick sectors, release-to-select and neutral-to-cancel; inventory validation and pending native changeability | Implemented and built |
| VR movement and usable menus | Existing native input queries, movement/snap heading, jump/use/sprint, physical-release-safe recenter; explicit native mono menu screen and foreground navigation; small health/armor/ammo HUD | Implemented and built |
| Robust frame ownership | ABI3 request epoch, early monotonic atomic invalidation, serialized Ready commit, one pending native transaction, retained cancelled Rendering; immutable complete pairs and original poses/FOV; conditional cache reuse with ownership guards | Offline policy checks and static review passed |
| Separate installable artifact | x86 proxy, x64 host/official loader, license payload, hash manifest, source/docs and collision-refusing installer; no proprietary game assets | Packaged; runtime_verified false |
| Exclude actual testing | No game/host/Wine/runtime/headset launched; installer only synthetic fixtures and read-only installed-game preflight | Observed |

## Verification

Both Windows architectures and the official loader compile/link. The host uses strict warning-as-error compilation. `core_checks`, `frame_checks` and `installer_checks` all pass. These check meaningful portable input/transform/ownership invariants and installer behavior; they do not simulate the native renderer or establish OpenXR-runtime acceptance.

`artifact-verification.json` records x86/x64 PE machines, expected exports and imports, identical ABI3 layouts (83,887,248-byte mapping), all five installed fingerprints and 21 required native hook exports. The root Prepare ABI/caller discriminator and native CRC query-ID behavior were additionally checked against the exact installed binary by the final Terra review. Final source and binary hashes are included in the separate package manifest.

Static integration follows real producer/consumer paths: XR input → native command queries and per-hand fire/selection; XR view request → native eye render transaction → D3D9 owned readback → same-request IPC pair → D3D11 upload/release → OpenXR projection with that pair’s original LOCAL poses/FOV. Native simulation advances only through the original OnStep. No fake poses, placeholder VR callbacks, stereo-expanded mono image or replacement inventory is used.

The final Terra review found a Ready-commit race and stranded post-wait image acquisitions. Both were corrected and confirmed by the focused follow-up in FINAL_FIX_REVIEW.md. That review also found a normal-session-stop ownership gap; the main agent added bounded retirement of all existing swapchain acquisitions before EndSession, with fail-closed teardown on timeout. Resize/destruction attempts wait/release after GPU drain and destroys a still-timed-out unwritten chain without an illegal release. The snapshot lock orders the Ready transition against epoch advancement. Abandoned waited acquisitions release without writes outside IPC ownership, and invalidate cache before changing the latest runtime image. A wait timeout retains ownership for retry, not an illegal release of an unwaited image.

## Practical limits

No measured rendering quality, weapon alignment, comfort, performance, runtime compatibility or playability claim is made. The 150 ms enqueue-age cutoff is provisional and is not a latency guarantee. CPU readback can stall the native game; a one-image runtime may suppress cached projection while its image is reacquired. Invalidation after the final atomic pre-EndFrame check remains a cross-process race.

Single-player stock first-person is the target. Classified full-size 32-bit non-MSAA desktop canvas, dimensions at most 2048, exact installed build and no active MRT are required. Complex effects, HDR/nested command behavior, cinematics, physical scale and Linux/Proton routing require later runtime evidence. Head translation is bounded while preserving IPD; no roomscale body collision or teleport system is implemented. Multiplayer, vehicles, immersive sniper scopes, physical swing melee and full mission/conversation/boss HUD are outside this build.

The installed game’s Bin, settings and saves were not changed. No external publication or field-note PR was made.
