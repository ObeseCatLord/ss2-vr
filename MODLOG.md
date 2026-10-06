# Mod journal

## 2026-10-05 — Cloud resumption and optimized portable checks

User resumed development in dot's cloud workspace after a public source-only
transfer. Original PC source and unaccepted melee WIP remain untouched. Fixed
core CMake assertion configuration: optimized portable checks now retain their
assertions instead of failing explicit NDEBUG guards or silently dropping
unguarded checks. Added an assertion-harness regression. Debug and Release each
pass all24 groups; a deliberately NDEBUG-built control fails as expected.
No Windows product, game, host, Wine or headset executed. Native saw receipt
investigation awaits access to privately transferred owned DLLs. See
docs/CLOUD_CONTINUATION.md; no new melee/native integration is accepted here.

2026-10-01: created isolated Git repository. Read-only inspection of installed binaries and archive names. No game/config/save edits, no launches. User authorized installing tools and requested OpenXR. pefile and capstone available; MinGW x86/x64 and CMake available. No shared field notes found. Existing Unlicense SS2ModMenu found as useful engine ABI research. Astra review requested before implementation.

Static investigation improved stereo boundary: CPuppetEntity::Render3D at Sam2Game RVA 0x94380 calls GetProjectionMatrix (hidden pointer, ret 4), camera placement virtual +0x5f4, renPrepareRender, InjectRenderingCommands virtual +0x5fc, renFinishRender. CPlayerPuppet vtable +0x5fc resolves to exported injection function 0xfda00. This is narrower than RenderView and recomputes visibility rather than replaying already culled commands. pefile default export cap 8192 truncates this game; use max_symbol_exports=65536.

Original Astra tool reviewer could not introspect effective settings and returned no review. Rerun with Codex CLI explicit gpt-6-astra and model_reasoning_effort=xhigh; effective startup header verified both. CLI ephemeral, read-only; raw operational output remains outside repo. Root filesystem has no free space; repository storage has ~148 GiB free. No cleanup performed. OpenXR SDK release-1.1.53 cross-compiled x64 loader successfully (toolchain setup, no runtime execution).

Astra review completed (gpt-6-astra/xhigh effective header verified). Rejected command Execute replay due collection-time culling/LOD and shared mutable graph. Adopted native Render3D transactions. CBaseWeapon AddRenderingCommand RVA 0x4bf40 creates a native command carrying weapon pointer; native Render3D local injection includes weapon rendering. Found identical-weapon input coupling in GetWeaponFiringButton; bypass per-weapon fire query only. Left enum 0/right 1 now verified by named LeftAmmo/RightAmmo HUD accessor bodies, correcting original ABI uncertainty. Final plan and disposition saved.

Capstone moved from newly installed user package to ignored deps/python to avoid system disk use; root reserved free blocks still make ordinary writes fail. Symlink restoration could not run (sudo needs password); analysis uses explicit PYTHONPATH. No existing user files cleaned. Closing completed tool agents failed because thread-store disk writes fail; they are completed, not running. CLI ephemeral review unaffected.

## Final integration, 2026-10-02

Implemented the x86 native adapter/proxy and x64 OpenXR host, native per-hand selection/fire/shooting/model transforms, separate wheels, movement/snap turn/recenter, headset stats and explicit mono menu presentation. Pinned dependencies and complete license notices are included. The repo preserves native engine behavior instead of introducing a second renderer/inventory/weapon simulation.

Terra's native review validated the declared x86 boundaries and found alias, stale command, deferred-slot and lifetime issues; all four have concrete fixes in NATIVE_REVIEW.md. The narrow canvas audit identified the real GfxD3D target binding. Final-target/depth/MRT/format guards were added; complex command-family coverage remains unverified. Native forward command semantics were verified and corrected. Both eyes now share one game snapshot/anchor and each request's exact predicted hand data. Shader/texture state remains native to avoid an old state block desynchronizing engine caches.

Both Windows products and the official loader build. Host compilation treats warnings as errors; native adapter compilation reports GCC's non-class-thiscall attribute warnings. Installed disassembly plus Terra ABI review support the declared calling conventions; no runtime ABI proof is claimed. No extra MinGW runtime DLLs are imported. Final verification checks all 20 hook exports, required other symbols, exact fingerprints and identical x86/x64 IPC layout bytes. Offline controls/math/color and synthetic installer integrity/collision tests pass. Installed-game installer preflight is read-only and passes; no deployment, game launch, runtime/headset test or publication occurred.

The package contains mod-owned binaries, source, plans, notices and collision-refusing install/removal tooling. IMPLEMENTATION_STATUS.md describes implemented behavior and remaining limits. Native postprocess/HDR/shadow/reflection coverage, physical weapon alignment/scale, performance and Proton runtime routing remain unverified. This is an untested development build, not a verified playable release.

Delegated workers/reviewers have finished. CLI workers were ephemeral. Tool-agent close attempts encountered the already-full root filesystem's thread-store write failure; those completed agents were not reused. No user data, Codex histories or approval databases were cleaned/modified to work around it. Local field notes were written; no knowledge-base PR was published.

Final offline check: CTest passed both core_checks and installer_checks (2/2). The artifact report matches the final proxy, host and loader hashes, with 20 native hook symbols and matching IPC layouts. Final changes require tracked (not merely estimated-valid) head/hand poses and pin the proxy DLL for detour lifetime. The initial staging artifacts are preserved under an -initial name; the final package is regenerated from current binaries and includes the full original source/documentation.

## Goal completion audit refinements

The prior turn made concrete source/package/commit progress; this continuation inspected authoritative source and found additional correctness failures. Terra verified absolute muzzle-cache contamination and stale post-toggle ownership; camera-local calibration and post-native-step refresh fix them. Astra xhigh reviewed the body-camera seam and caught native height interpolation that a naive base accessor would discard. The implementation preserves that verified +0x8c0 correction and rejects its finite unset marker. Physical controller values remain intact through recenter, with controls suppressed until chord release. Head-center clamping now preserves IPD.

The renderer audit found native surface copies bypassing target binding and avatar-keyed occlusion history shared by the eyes. Original-surface copy/read/fill routing, exact native color format and a caller-scoped root-view identifier tag address those boundaries. Native renderer/query/simulation policy remains in use.

Two explicit Astra xhigh transport passes replaced strict 35ms starvation with polling through existing slots, early monotonic native-epoch invalidation (ABI3), one outstanding including cancelled Rendering and Ready retries, and eligible reuse of the last fully released stereo pair. Both images must be writable before either is replaced; acquired cached indices cannot be presented. The 150ms enqueue-age cutoff is provisional, not a measured latency guarantee. Final epoch checks narrow but cannot eliminate changes after the final cross-process check. No additional process/thread/queue/GPU transport was introduced.

Three offline check groups pass after these changes. No game, host, Wine, OpenXR runtime or headset execution occurred. The earlier development package is superseded by the regenerated final artifact.

2026-10-02 completion verification: final integration Terra review verified the root Prepare x86 ret16 ABI/caller and all epoch/pair guards, then found two concrete issues. Ready now commits under the native snapshot lock; post-wait abandonment releases existing waited images without upload and invalidates cache. No-write abandonment also retries only existing timed-out acquisitions. Both Windows products/official loader compile; all three offline groups pass. Final PE/static verification records 21 hook exports and equal ABI3 layouts. Final package regenerated from current source/products; manifest and archive hashes checked and installed-game read-only preflight used. No game, host, Wine, runtime or headset launch, deployment, settings mutation or publication.

2026-10-02 final follow-up: Terra confirmed completion serialization and post-wait retirement fixes, then identified STOPPING ownership retention. Main added bounded all-chain acquisition retirement before EndSession and best-effort no-write retirement before drained swapchain destruction. The pinned destruction contract does not require an illegal release of an unwaited image. Strict host compilation passed; no runtime was executed.

## Immersive extension, 2026-10-02

User requested feature equivalence, final immersive features, an explanation of gun/hand rendering, and a comfortable UI size/follow-threshold pass. Clarification explicitly requires multiplayer VR replication and excludes teleport. Existing no-game/headset-testing constraint remains. Native gun/hand topology and multiplayer action/message seams are independently investigated read-only by Terra; main handles UI/native locomotion/plan. Native engine/protocol architecture must be retained; no duplicate UDP server or invented physics/packet ABI. Astra UI review brief prepared from verified current head-locked panels and text geometry.

### Immersive extension: UI, grip and native feedback

- Teleport explicitly excluded; multiplayer remains required and unfinished.
- Applied Astra UI review: LOCAL threshold-follow panels, shared frozen wheel reservation, distance guards and blocked-selection mask, rectangular HUD, recovery-safe native keyboard navigation.
- Added OpenXR grip actions, native model/muzzle shared alignment, optional native-shot/health-drop vibration and user comfort/alignment configuration. IPC ABI4 requires matching game/host products.
- Both PE products cross-compiled; four offline groups passed. No game, host, Wine or headset runtime executed. Existing0.1.0 package retained; extension source not yet packaged.
- Network audit recovered native receive-slot/avatar mapping and stale binding on disconnect; capability and native message ownership still under investigation.

### Immersive extension: laser aiming and review corrections

- Added the requested per-hand world-space lasers and native bullet-collision hit markers. User requirement remains multiplayer and no teleport.
- Astra verified the native ray cleanup/caller order and caught inherited model/view state, asymmetric stereo expiry, render-request tracking mismatch and premature calibration reuse. Applied explicit native draw context/raster restoration, caller restriction, shared frozen eligibility and actual pending-request input.
- Preserved existing shot semantics after review identified model-getter reference mutation; lasers wait for fresh native render calibration.
- Both products compiled; all four offline groups passed. Static artifact verifier confirms ABI4 and24 required hook exports. No game/host/Wine/headset runtime executed.
- Multiplayer Astra review found the proposed RPC header was reversed: native chat is H0 with J derived from its runtime descriptor. Native metadata enqueue/routing and headless bootstrap remain integration requirements. No multicast transport or alternate damage engine introduced.

### 2026-10-03: multiplayer implementation review and native menu pointing

Astra implementation review caught outbound handle mapping, pause recovery, lost/duplicate reliable presses, partial hook activation and incoherent native muzzle calibration. Main verified the native mapper caller and adopted explicit rollback, dependency pinning, headless symbol separation and fresh native muzzle references. The split pose/edge attempt was reopened under the architecture discipline; Astra selected an ordered native reliable snapshot reference with consumption credit before further cross-stream policy.

The native menu cursor was traced through actual input polling and command dispatch. Added ray-to-quad pointing, visible cursor, native mouse-command routing and suppression of duplicate Enter confirmation. Menu images and pointers correlate by a local generation; ABI5 appends48 bytes to Shared. The game/host/server products cross-compiled and six offline groups passed at the UI integration checkpoint; subsequent model-tree checks and multiplayer changes require a fresh final build. Native model-record globals, final draw consumption and affine conventions are now verified for remote weapon presentation. No deployed files, settings, runtime or network session were changed/executed.

### 2026-10-03: six-DOF and multiplayer/UI integration checkpoint

Integrated native reliable ordered snapshots, post-full-simulation consumption credit, immutable per-hand short taps, capability recovery/lifecycle invalidation, native authoritative equip/muzzle/fire queries, dedicated module loading and identified remote weapon model subtrees. Main corrected the initial four-current-capability credit bug, stale ACK liveness, per-hand remote eligibility and partial activation guards. Matched left/right native tool names were verified in installed asset metadata without copying assets.

Native menu ray coordinates and trigger now share the submitted input sample and exact visual-frame sequence; changed frames/focus/identity fail closed. ABI6 Shared83,887,512 bytes, Input224/Request352/Ui352 unchanged. Explicit math checks preserve XYZ head translation, pitch/yaw/roll and physical stereo separation. Seven offline groups pass and both architectures/server compile. Static fingerprints/export/internal entries and IPC layouts agree; no runtime was executed. Package0.2.0-dev contains the current extension and retains the old0.1.0 artifact.

Roomscale audit rejected unchecked RepositionPuppet and local-only checked placement. Normal player intent routes through native RPC-aware controls and the existing mechanism/joint physics pipeline; exact caller vector units and accepted-displacement origin consumption are still being traced. Full immersive equivalence remains unfinished.

### Post0.2.0 follow-up

Preserved the owned sniper's native gun/hand model in XR eyes instead of the stock zoom branch's flat orthographic replacement. Desktop native zoom remains unchanged. Strict isolated native compilation passes; immersive optics remain unfinished and this source follow-up is not in0.2.0. Normal player movement caller/basis/normalization closure is recorded in NATIVE_MOVEMENT_AUDIT; Astra/xhigh model+effort verified for the roomscale request/result/origin architecture review.

Owned native gun models now hide consistently in both XR eyes on lost/nonfinite hand tracking, using immutable per-pair request validity. This prevents native camera-attached visual fallback while tracking is lost; desktop rendering is preserved.

### 2026-10-03: coherent six-DOF crouching rig

User requires Astra for reviews. Updated AGENTS accordingly and ran a fresh Astra/xhigh source/design review with independently verified effective settings, followed by a second Astra verification after fixes. The shared rig correction now preserves deep crouch height, eye separation and physical head-to-gun offsets while maintaining separate horizontal/vertical tracking bounds. Local cameras/muzzles/models and wire production share the mapping.

Astra confirmed the retained-tap/newer-head codec defect and caught the cleared-pulse relay requirement. Active grips use the reachable head-volume envelope, inactive hands remain canonical, and wire4 rejects mixed older VR clients. A production codec/retention/consumption/relay/ACK regression covers independent hands and newer head motion. All seven offline groups pass; x86 proxy/server and x64 host/loader compile. Native static verification confirms34 exported hooks, three internal entries and matching ABI6 layouts. No runtime or installed-game mutation occurred.

Expanded native movement evidence through actual ProcessPlayerControls payload, input producer boundaries, body/support correction, native sequence gates and joint consumer. Corrected misleading nearest-export labels and the nominal-velocity accessor name. Body roomscale attribution, head obstruction, optics, vehicles, complete overlays, physical melee and remote head bones remain unfinished.

Packaged0.2.1-dev separately: archive SHA25683b741f8c53376178540e3f5f69cbebc253249f0f22e1fc271983723913a3697,8,650,698 bytes,112 entries and15 payload files. All archived payload hashes match the manifest (IPC6/wire4, runtime_verified=false). Installed-game read-only preflight passed with no collisions or writes. Earlier archives are preserved.

### 2026-10-03: sampled head-lean comfort follow-up

Astra/xhigh (effective settings independently verified) reviewed the proposed native query adapter, required default-disabled operation and exact request/body/head matching, then reviewed the implemented source and a final performance amendment. Added an opt-in native thick-ray head-lean fade, independent of lasers and weapon/hand/wheel availability, using the existing audited query cleanup phase. Both completed world eyes share one frozen visibility and one256-entry color table; alpha stays opaque and host HUD/wheels remain visible. No camera/body displacement, IPC extension or multiplayer protocol change was introduced.

Astra found no blocking source defect and signed off on the lookup amendment. All seven offline groups pass; x86 proxy/server and x64 host/loader build; final artifact verification confirms existing34 exported hooks, three internal seams and matching IPC6 layouts. Query availability, moving-geometry freshness, overlap and full eye enclosure remain unproven; missing/mismatched samples leave the world clear. This is not complete head protection or roomscale body collision.

The walking audit closed the native desired-speed/joint mapping and post-physics readback, but did not establish accepted physical-motion attribution. The optics audit records native zoom callbacks/timing and the unresolved lens/input/frame path. No game, host, Wine, headset or network session was executed. No installed files/settings were changed.

Packaged0.2.2-dev separately: archive SHA256181f8be84b946cea2da80ad613e4fe67cfe23f2d2b2bfd7777df74f26250edcb,8,668,647 bytes,115 entries and15 payload files. Archive payload hashes match the IPC6/wire4 manifest, runtime_verified=false, and the staged INI keeps head fade disabled. Installed-game read-only preflight passed with no collisions or writes. Earlier archives are preserved. Astra reviewer and the separate native investigation worker are closed.

### 2026-10-03: native sniper reference and immersive binding closure

Restored pinned build dependencies inside ignored deps/vendor after prior temporary checkouts disappeared. Native code proves sniper zoom switches shooter damage, and stock alternative input does not expose dual-sniper zoom. Added a fingerprint-bound owned-mesh cap audit with finite/bounds/framing, exact slice round-trips, connected manifold boundary, winding/circularity/coverage checks; no proprietary mesh was extracted or committed.

Corrected tracked sniper bullet/laser reference selection: use the base native attachment once for an accepted context, retaining native zoom damage. Astra/xhigh effective settings were verified; its source reviews caught weaker fallback acceptance and post-getter pointer-read ordering. One validated transient context now governs selection and retargeting, with rejected/nested placement unchanged. Eight offline groups pass and both architectures/server/official loader build; static native entries and IPC6/wire4 remain unchanged. No game, host, Wine, headset or network session was executed.

Astra approved the muzzle/parser source and selected a native scope-capture/aperture design, with actual resource/lens placement, cap treatment, weapon-depth, color-stage and zoom-authority gates. Head investigation found the native instance-specific skinning path; main corrected runtime-ID/array-index assumptions and proved shaBoneMatrices retains its pointer until subsequent draw/clear. Those remaining immersive features are still active, not implemented or advertised as complete.

Packaged0.2.3-dev separately: SHA256270b5db7cbea3068846db808050ad2da2b4f3a226ad036be1aa53d93d5f6785e,8,687,749 bytes,122 entries and15 payload files. All archive payload hashes match; IPC6/wire4, runtime_verified=false and head fade disabled. Installed-game dry-run preflight passed with no writes/collisions. Earlier packages remain unchanged.

### 2026-10-03: native remote-head and stereo consistency checkpoint

Astra/xhigh effective settings verified; design review selected one post-DDE30 temporary palette adapter shared by CPU/GPU skinning instead of duplicate consumers/global pointer swaps. Implemented full XYZ/quaternion Head/owned-descendant delta about native body eye, preserving animation and affine deformation. Native runtime IDs, exact body/mesh/draw/map ownership, sentinels, non-alias and atomic prepare/write gates replace guessed slots. Head option remains explicitly default-disabled because unhooked worker/model/deletion lifetime and appearance are unverified; registered physics tasks do join before Step returns, and native main-thread/owner gates enforce supported entry behavior.

Remote weapon/head pair admission now freezes age once, retains immutable eligibility, detects incompatible invalidation recovery with local history, checks current recipient capability, and holds binding/MP guards through Ready. Astra source review found four defects; main fixed remote-thread poisoning of local native-only pairs, server frozen-pose revision relabelling, unrelated unskinned draw rejection, and producer nesting/original exception handling. Astra follow-up approved all four fixes.

Nine offline groups pass; x86 proxy/server and x64 OpenXR host/official loader build. Static artifact verification records34 exported hooks, four internal entries including optional producer, and matching IPC6/wire4. Owned weapon-depth audit now proves native depth-cache/viewport callback and the requirement for matching world near/far planes. No game, host, Wine, headset, runtime or network session was launched. No installed files/settings changed. Full immersive scope remains active and unfinished.

Packaged0.2.4-dev separately: SHA256c3e4c879d92489c80bb015272ffa8781a3bdcf26d4d77a05417196cfdb54a718,8,747,905 bytes,135 entries and15 payload files. All archive payload hashes match; IPC6/wire4, runtime_verified=false, RemoteHeadTracking=0 and head fade disabled. Installed-game read-only preflight passed without collisions/writes. Earlier packages are preserved. Astra reviewer and native investigation worker are closed.

### Physical gun projection/depth follow-up

Fresh explicit Astra/xhigh (effective settings independently verified) corrected the prepared-projection design: native root execution can revise clip distances. Main integrated initial execution raw P/view/depth capture and caller-scoped owned gun adaptation while preserving original authored camera/mesh/hand animation and gameplay. Same invocation requires finite frozen hand placement; missing stages/restoration/nesting receive bounded cleanup and sticky pair rejection. No IPC/wire change. Full cross-build and10 offline groups pass; compiled x86 camera-copy/return-cleanup and35exported/5internal static gates pass. Astra source review caught stage0 failed-admission/native-fallback cleanup responsibility and the ABI verifier's incomplete unchanged-word assertion. Main fixed both, added a production-helper regression and tightened exact destination-store verification; bounded Astra follow-up gives GO. Final build/checks pass; these checks do not establish GPU wall occlusion or runtime playability.

Independent static investigations close loaded Scope name translation/resource-section association and native overlay ownership boundaries without installing guessed asset/UI hooks. Exact content/lifetime admission and complete overlays/scopes remain unfinished. No game/host/Wine/headset/network launch or installed-game mutation occurred.

Packaged0.2.5-dev separately: SHA256439f5268b7a964d6032c83f93c45e5217cd464ffd8cb29c67582e2c0052bc24a,8,779,699 bytes,147entries and15payloadfiles. All archived payload hashes match; IPC6/wire4, runtime_verified=false, RemoteHeadTracking=0 and head fade disabled. Installed-game read-only preflight passed without collisions/writes. Earlier archives are preserved. Astra physical-gun reviewer is closed; native scoped-optic lifecycle investigation continues separately. Full immersive equivalence remains active and incomplete.

### Ordinary scope pose observation follow-up

Corrected the stock skeleton to its actual Sniper root and proved packed skin binding, identity inverse bind and exact cap triangle range[901,923). The owned audit reports four slice digests without retaining asset bytes. Main caught and corrected a preliminary native-parameter/entity field conflation: weaponBC is the right-hand flag, assigned by native creation, so left sniper interpolation is skipped in stock behavior. Native release does not deactivate zoom. Native query resets shared renderer state; finished scope images inserted post-root would receive final HDR/fade again. Recorded loaded-buffer/draw boundaries without adding guessed cap hooks.

Fresh Astra/xhigh (effective settings verified) selected passive ordinary-draw observation before an additional prepass. Implemented accepted physical-placement, exact frozen request/owner/weapon/model and bounded native model→mesh→draw→palette admission, copying full evaluated affine without losing native mirror/stretch. Reused one producer hook independently of the default-disabled head mutator. Samples remain contentVerified=false and clear at both eye boundaries; no query, extra render, cap mutation or scope image is implemented. Astra source review caught the mutating observer thread guard and stale successful-only publication. Main added nonmutating pre-read ownership checks, unconditional supersession, pair-fault/model-replacement gates and production-helper regressions; bounded Astra follow-up gives GO.

Full cross-build passed, affected x86 proxy/server final incremental rebuild passed, all11 offline groups pass, artifact/compiled x86 ABI checks retain IPC6/wire4 and35 exported/5 internal gates. No game/host/Wine/headset/network execution or installed-game mutation occurred. The0.2.5 archive remains unchanged; this source follow-up is not packaged and full immersive equivalence remains active.

### Native sniper input and retained zoom-context follow-up (source0.2.6)

Fixed native left-primary input reaching the right sniper alternative-press handler when no left gun exists. Suppression is exact native caller/main-thread/live VR ownership, returns the native handled-event value, and leaves all other native calls intact. Astra caught pre-first-freeze capability identity being inferred from an empty pose; authority now copies live peer avatar/incarnation under its existing lock while retaining frozen pose validity. Bounded Astra source follow-up gives GO.

Source wire5 adds held zoom and immutable per-primary-shot zoom context to the existing ordered codec/retention. Historical unzoomed shots override newly held zoom just as zoomed shots override release. Failed sends retain context. Astra caught rejected pulses altering desired zoom; freeze now discards rejected historical context while preserving latest held intent. Matching-generation capability-reset and fresh historical expiry checks were strengthened. Native producer still sends zero zoom bits: this is causal transport support, not native zoom/input or rendered optics. IPC remains6; immutable0.2.5 archive remains wire4 and unchanged.

Native GameInfo constructor/getter proof exposed a spare bookkeeping slot being passed to an unchecked17-slot native getter. A narrow0..16 guard prevents GameStats from being treated as a player brain; separate Astra/xhigh source review gives GO. The native local pre-weapon scheduling adapter and shared compile-only predicate ABI follow-ups are separately reviewed. No game/host/Wine/headset/network execution or installed-game mutation. Full immersive equivalence remains active and incomplete.

Astra final source GO covers all-mode pre-weapon preparation after correcting failed-beginTick authority fallback and removing nonlocal client double-update dependency. Compile-only shared-entry final GO covers explicit GCC/MinGW/Windows/x86 guard, EAX/ECX/EDI emission and source/object provenance. Affected x86 proxy/server build and refreshed artifact/compiled ABI checks pass (36exported/5internal, IPC6). Previous full cross-build and11 offline groups passed; final scheduling-only rebuild required no changed portable behavior tests. The shared entries still install no native predicates, and source follow-ups remain unpackaged.

### Copied-authority intent invalidation

Astra/xhigh source GO closes four copied-Authority cancellation sites: affected-hand invalidation now clears primary pulse, desired zoom and historical zoom together with fire, using the existing shared helper. Raw release/held witnesses and opposite hand remain. Final affected x86 proxy/server builds and refreshed artifact/ABI checks pass (IPC6,36exported/5internal). No runtime launched.

Astra's next causality decision requires raw eligible zoom release plus per-hand server-owned epochs echoed through existing ACK/Pose, and a genuinely post-ACK input sample before rearming. Desired-zero or raw serial alone cannot distinguish a delayed same-type replacement. Conditional design GO only; native zoom enablement remains NO-GO until connected/tested offline with lifecycle gates. Source stillwire5/zerozoom/IPC6 and unpackaged.

### Action provenance and per-hand admission (source0.2.6 IPC7/wire6)

Connected OpenXR primary/zoom action activity, independent logical stream generations and same-sample Sprint/Jump zoom provenance through host IPC and native consumers. Presented menu pointers correlate the primary stream generation, and inactive triggers cannot synthesize release. Existing native ACK/Pose now carry per-hand weapon-intent epochs; changed-hand ACKs cancel existing intent and require a genuinely later input sequence/timestamp. Existing transport, credits, scheduler, inventory and damage remain native.

Terra implemented only the portable network codec/admission slice; main integrated host/menu/native boundaries. Explicit Astra/xhigh effective settings were independently verified for every review. Astra reproduced two defects: dead-band hysteresis laundering a cumulative pre-ACK release and muzzle checks comparing two stale echoes. Main added explicit current-neutral evidence and separate live validation metadata with final post-native-callback checks. A bounded follow-up exposed ungated client held/native fire; main reused existing Pending admission at submit, Snapshot, native weapon and every cached command consumer, preserving exact cached-value provenance without a second admission service. Final bounded Astra source follow-up gives GO; main spot-checked all load-bearing paths. See INPUT_INTENT_REVIEW_DISPOSITION.md.

Final full x86 proxy/server and x64 host/official-loader cross-build passes; all eleven offline groups pass. Combined actual portable producer→ACK→retention→server/client-use traces cover both hands and stream/epoch changes, fresh-neutral recovery and intervening ACK revocation. Refreshed artifact/compiled ABI records match products and retain36 exported/5 internal seams; IPC7 mapping83887640 bytes agrees across architectures. Archive0.2.5 hash is unchanged. Current source is unpackaged and desired zoom stillzero; full immersive equivalence remains active and incomplete. No game/host/Wine/headset/network/runtime launch or installed-game mutation.

Separate native investigations establish the destructive attachment-query boundary, a smaller mounted-anchor candidate using the existing native rider body pose, and exact native input-clamp/ClientAction dispatch. Completed sniper callback ownership and scope-color reviews are persisted; the mounted proposal is explicitly unreviewed and no vehicle/optic adapter is implied by the evidence.

### Mounted native tracking/control checkpoint (2026-10-04)

Implemented the Astra-reviewed minimal two-hook mounted adapter with shared native seat/body anchor. Preserves native eye position/height, full XYZ/quaternion tracked head/hands and independent eyes; right-hand aim enters original native look clamp/ClientAction, preserving native movement, vehicle physics/fire and RPC. Exact root-avatar query943B6 suppresses local body in XR; native handheld injectionFDA0B and desktop mounted camera remain native. Shared remote rider identity/anchor suppresses mounted handheld retargeting. Mode/seat transitions reuse existing origin/gates/wheels/calibration/generations, without a new controller/protocol/cache.

Explicit/effective Astra/xhigh initial source review found five bugs (retained equip, releases, newer hand loss, post-getter output, mounted handgun deletion); main adopted all fixes. Follow-up caught per-hand previous command history crossing an action stream/ACK epoch; production history helper clears only the affected5right/6left cache, preserving mounted ACK independence. Final bounded source GO closes all findings. Removed second native clamp retry after proving read-only original chain. Compiled wrapper sibling-tail transfer now explicitly proves original reference/ECX/ESP and audited native4-byte cleanup.

Final full x86 proxy/server+x64 host/official-loader build, all11 offline groups, artifact fingerprints/IPC7 mapping layout and both compiled ABI checks pass. New meaningful traces cover full rotated seats, head/hand axes/IPD, compatible captured input, ordinary release, coalesced streams/handheld epochs/mounted ACK independence, stale equip cancellation and getter lifecycle rejection. No game/host/Wine/headset/network execution or installed-game mutation. Source and review dispositions are recorded; new separate0.2.6 package follows. Full goal active: actual vehicle-muzzle lasers, roomscale collision, scopes/native zoom, full overlays, physical melee and complete remote-head enablement/lifetime remain.

Independent overlay inspection found native RenderView's postlude registers sound/vibration listeners. A simple repeated parent would duplicate them. Revised minimal candidate retains current Render3D capture and calls original native marker helpers only after each eye's completed color, preserving single native desktop postlude/listener ownership; design remains unreviewed. No overlay is implied by this research.

Packaged0.2.6-dev separately: SHA2567687f055e739149dbdcfb50e21931bcc4b70bb7728c1d2f11a6201fbcf13092f,8,977,200 bytes,218entries/15payloads. Every payload and reviewed source hash matches; IPC7/wire6/runtime_verified=false, optional head retarget/fade remain disabled. Read-only installed-game preflight passed with matching fingerprints and no collisions; no files changed.0.2.5 archive hash unchanged. See mounted-package-verification.json. Packaging is a bounded development checkpoint, not full-equivalence completion.

### Native world markers (0.2.7 checkpoint)

Added native navigation/objective marker draws after each admitted eye's complete original Render3D and before readback, borrowing copied executed full view/P and validated native drawport dimensions/fade. Exact native parent caller admits the optional phase; nested/fallback/desktop paths remain original. Original ortho/blend/raster/native draw helpers retain marker content and native graphics ownership. Actual eye color/depth/viewport and live frame/rider/world-info/drawport continuity checks reject incomplete/partial pairs. No retired-root activation, parent replay, listener hook, quest model or protocol change.

Same independently verified Astra/xhigh reviewer gave conditional design GO then bounded source GO, with no production blocker. Main corrected an unfinished opposite-eye test fixture and preserved independent manual argument provenance for reviewed artifact hashes; the automated ABI tool explicitly documents its narrower shape coverage. Final full x86 proxy/server+x64 host/official-loader cross-build/all12 offline groups and artifact/both compiled marker caller checks pass;38 exported/5 internal seams and IPC7/wire6 unchanged. Source acceptance/dispositions and structured hashes are in NATIVE_WORLD_OVERLAY_SOURCE_REVIEW.md and world-marker-source-checks.json.

Separate bounded investigations preserve remaining-work accuracy. Native flat overlay updates fade timing and invokes element callbacks, so it cannot be replayed per eye without demonstrated behavioral change; transparent native HUD alpha/fade/correlation still need a concrete reviewed adapter. Vehicle firing metadata distinguishes authored blast attachment from direction body's origin. Main caught the worker's missing-model claim: Cannon.mdl and stock Turret_Cannon.ep exist in All_PC_02.gro with recorded hashes; their actual attachment/executor placement still needs proof. No vehicle-muzzle laser or complete flat UI is implied. Goal remains active/incomplete; no game/host/Wine/headset/network runtime or installed-game mutation.

Separate0.2.7-dev package verified: SHA256c1217d75cb17ca3b0185ae99c1b6d1c8a6822bbeb9afd1bab946e0ec9de91729,9,009,640 bytes,232entries/15payloads,8 reviewed-source hash matches. IPC7/wire6/runtime_verified=false; optional head/fade defaults remain disabled. Installed-game dry-run preflight passes with no collisions/writes. Earlier0.2.5/0.2.6 archive hashes unchanged. See world-marker-package-verification.json. This is an immutable bounded checkpoint, not full goal completion.

Source-only main vehicle follow-up after packaging closes the actual CShooter virtual1D0 identity and original purpose1 model-attachment route. Corrects virtual594 direction label using actual turret vtable base; confirms stock Barrel01 file-local names without treating them as runtime IDs or decoded member values. Pure idle process/blast selection and borrowed current model getter lifetime remain narrow gates. NATIVE_TURRET_EXECUTOR_FOLLOWUP.md is investigation, not reviewed/implemented vehicle lasers; archive unchanged.

### Native turret aiming laser (0.2.8 checkpoint)

Explicitly selected/effective Astra xhigh design review identified destructive model scratch versus restored child-traversal flag and non-world low-level attachment matrices. Main adopted original purpose1 placement with explicit existence; added native main-thread/actual idle-scratch admission and bounded original active lookup preflight. Reused the existing query/sample bank/native ray/frozen two-eye/native line path. Exact turret table gate, rider/seat/model/resource/config/action/attachment/mechanism and request continuity reject changed samples; native direction feeds the collision ray directly and vehicle cache reuse is omitted. Both native mounted fire commands, gameplay/RPC and IPC7/wire6 remain unchanged.

Bounded Astra source GO found no blocker, justified the necessary native checks and narrowed test claims. Main preserved exact source/compiled artifact hashes and independent manual caller provenance. Full cross-build/all13 offline groups/artifact and compiled x86 caller checks pass. Tests exercise production pose, scratch-idle and frozen-source admission; native callbacks/COW/missing lookup/gameplay were not executed. Shared readableMemory moved unchanged for reuse. Native projectile clamp/scatter remains original, not predicted.

Additional independent native UI follow-up demonstrates mutating conversation text/time, two actual uniform full-drawport fade fills and the current pre-overlay Ready commit. It identifies necessary once-owner/correlated-image work without implementing replay or another UI policy. Full immersion goal remains active/incomplete; no game/host/Wine/XR/network launch or installed mutation. Packaging is a separate development checkpoint, not full completion.

Separate0.2.8-dev package verified: SHA256094a3e34737062810171d048a31b1f44c358ddd8516d61b29884774067e5715e,9,054,932bytes,243entries/15payloads,6 reviewed-source hash matches. IPC7/wire6/runtime_verified=false; optional head/fade remain disabled. Read-only installed-game dry-run preflight passes without collisions/writes.0.2.5/0.2.6/0.2.7 hashes unchanged. See vehicle-laser-package-verification.json. Package contains source4084924; subsequent documentation-only once-owner clarification is not repackaged.

### Complete flat UI design and native clipping follow-up

Main proved actual native physical viewport/scissor mapping, builtin device
program object records, adjusted projection upload and two actual device draw
families. Main also found dynamic glyphXYZ and depth-enable, correcting the
assumption that HUD depth-disable made all flat UI depth-independent. Original
depth enable does not replace comparison; native default is LESSEQUAL, with
current runtime state still unproved.

Explicit Astra/xhigh, effective independently verified for all three reviewer
turns, gave consolidated conditional design GO for once-only native callbacks/
immediate GPU duplication through the existing owner/pair transaction. It
withheld source GO for the matrix-only/fresh-depth variant and caught pre-UI
CPU-only world dimming as a material composition-order boundary. Main adopted
the source-depth/clip gate, minimal two draw hooks, actual object admission,
post-original state restoration, exact fade scopes and existing comfort owner;
no renderer/state-manager rewrite. Review and disposition are persisted and
reviewer closed. No source adapter was added, no runtime/install was run, and
immutable0.2.8 remains unchanged. Full immersive goal active and incomplete.


## Accepted flat native UI geometry (2026-10-04)

Astra/xhigh gave bounded source GO for the flat panel transform and tests. The
adapter retains original source clipping via six transported halfspaces and
synthetic output clip-Z, while preserving flat eye X/Y/W. Added VR UI draws will
disable destination depth tests/writes for legibility; source fog or depth-dependent
shading is not admitted. Original program objects, UV/color, blending and native
callbacks remain the integration reference. Physical thickness, replacement
shaders and scratch masks were rejected.

Final production-helper checks pass 1658 boundary witnesses, all14 offline
groups and Windows x86/x64 compile-only checks. These verify geometry, not native
GPU draws, raster appearance, callback coverage or frame completion. The helper
is not yet connected; the next step is the once-only owner/draw/pair adapter,
actual compiled position semantics and pre-UI dimming/fade ordering. Full goal
remains active. No runtime or installed-game changes; immutable0.2.8 unchanged.
See NATIVE_FLAT_UI_CLIP_SOURCE_REVIEW.md and native-ui-clip-source-checks.json.


## Native UI cleanup and admission preparation (2026-10-04)

Astra/xhigh approved the bounded native-finally boundary and actual program,
adjusted-eye-projection and creation-thread/device gates. Main caught a mistaken
HLSL-route claim; the verified native assembly route let us delete a proposed
shader parser. Native SEH differs from MinGW DWARF2, so one small MSVC-target C
finally object isolates cleanup while retaining the surrounding toolchain. GNU
callbacks realign locally and contain direct errors to a failurebool; they never
rethrow across the foreign boundary. Native exceptions remain native. Future
resource ownership/reentry and reset-only failure policy still require integration.

Full cross-build/all14 offline groups and compiled ABI/artifact checks pass.
The actual final PE code, scope/funclet/IAT identity, four HIGHLOW relocations and
GNU callback alignment are checked; no exception, Windows code or game was run.
The installed ProtonHotfix32-bit API-set/schema/provider export chain was inspected
read-only. Runtime selection, unwinding, GPU appearance and performance remain
unverified. IPC7/wire6/version0.2.8 and immutable archives remain unchanged.

Complete UI is still not connected. Next extend the existing Rendering pair
through the once-only original brain/overlay owner, add closed-domain immediate
GPU duplication with explicit cleanup/state restoration, pre-UI alpha-preserving
dimming and frozen existing comfort geometry, then final presented-pair metadata.
Keep existing world-only behavior where the complete-UI owner is not admitted.
No new queue/renderer/protocol service. Full immersion goal remains active;
roomscale collision, scopes/native zoom, physical melee and remote-head lifetime/
enablement remain required. See NATIVE_FLAT_UI_NATIVE_BOUNDARY_REVIEW.md.


## Current0.2.9 connected native UI checkpoint

Astra/xhigh SOURCE GO closes the once-only native UI integration after four
corrected defects: raster point/line and point/wireframe-fill admission, retained
failed-unlock ownership/reset quarantine, and session loss during moved fallback
HUD upload. Original brain/overlay/fade callbacks run once, immediate admitted
filled triangle draws preserve native programs/resources/blend/alpha/order in
both existing eye RTs. Exact native fade calls cover whole eyes. Actual adjusted
P/source constants and viewport/scissor clipping supply flat physical panels;
source fog/stencil/userplanes and unsupported color output reject the pair.
Nonlockable/actual swapchain identity, creation/main thread, current hooked device
and raw-resource/frame generations gate compatibility. No new renderer, shader
parser, HUD state model, frame queue or XR layer.

Existing Rendering ownership now extends through original overlay AND enclosing
owner return; Ready publishes only both final images with matching immutable
geometry. Pre-UI world dimming preserves alpha via existing pitched readback/
UpdateSurface, then final export only makes alpha opaque. The existing35°/8°/
90°s comfort anchor updates once and freezes full-canvas panel size/aspect; stats
fallback is suppressed only for a final surviving completed UI pair, including
reuse and post-wait revalidation. Final loss/quit clears EVERY layer. Failed raw
restore/unlock halts adaptation until successful reset/new creation. Nativefinally
cleanup covers explicit resource and commit-lock ownership, not arbitrary crashes.

Full x86 proxy/server+x64host/official-loader cross-build/all14 portable groups,
refreshed41-export/5-internal-seam artifacts and native-finally ABI checks pass.
IPC8/wire6/version0.2.9 products must match; previous archives remain immutable.
NATIVE_FLAT_UI_CONNECTED_REVIEW.md and native-ui-connected-source-checks.json
record exact source/artifact evidence. Compiled/offline acceptance does not prove
native callbacks/GPU/foreign unwind/provider behavior, stock callback/device/thread
opportunity, appearance/deadlines/performance or runtime/headset compatibility.
No game, host, Wine, XR/server/network session, installed-game/config changes or
publication. Roomscale body collision, native zoom/optic images, physical swing
melee, broader vehicles and complete remote-head lifetime/enablement remain
required; full goal is active, teleport excluded.

Separate0.2.9 package created from accepted source08a0504; archive SHA256
a5836e8da4b4844f2a26ae4e93a835f5dd271dc5d991854cb7aeb26f287b99f3,
9,234,076bytes/264entries/15payloads. CRC/all payload hashes and installed-game
fingerprint/collision dry-run pass, with no installed writes or runtime execution.
IPC8/wire6/runtime_verified=false. Old archives unchanged. Exact package record:
docs/native-ui-package-checks.json. Full goal remains active.


## Current0.2.10 input and multiplayer checkpoint

Astra/xhigh SOURCE GO accepts native input interval/TLS retirement and exact
credit/discard ownership after three review fixes: allocating lifecycle lock
scope, transport handoff credit/ACK retention, and cached local neutral rearming.
The original simulation/entity/player callbacks and existing scheduler, token,
neutral gates and reliable protocol remain. Root interruption restores TLS,
invalidates existing tracking/intent epochs, retains actual inactive awaiting
slots for later normal discard, and excludes pre-interruption local samples by
sequence/time/session/reference/producer. Snapshot resets preserve that boundary.
No native send, game callback, allocation, new ACK queue or protocol on abort.

A related actual compiler warning exposed the relay Sample revision landing in
liveIntentEpoch[0], leaving presentationRevision0 and preventing admitted remote
VR presentation. Corrected exact aggregate positioning and made narrowing a
build error for both x86 products. Full remote-head lifetime/enablement is separate.

Final x86 proxy/server+x64 host/official-loader build, all14 portable groups,
artifact/native-finally ABI and pinned8-window static planning checks pass.
Direct scan completeness, indirect incoming paths and actual HDE continuation/
installed-hook behavior remain unknown. Exact evidence/dispositions:
NATIVE_INPUT_INTERVAL_REVIEW.md and native-input-source-checks.json. No game,
Windows program, host, Wine, XR/server/network session, installed-game/config/save
changes or publication. IPC8/wire6/version0.2.10; prior archives immutable.

This is a real input/replication source fix, not native zoom or optical image
integration. Native zoom producer remains zero. Next connect callback-owned
native zoom/predicates only after actual borrowed cleanup lifetime, operation
reentry and production-entry admission close; preserve native damage/timing/
scheduler and historical shot context. Full roomscale collision, optic images,
physical swing melee, broader vehicles and complete remote-head support remain
required. Full goal active; teleport excluded.

Separate0.2.10 archive created from accepted source5d088b3;
SHA256efe34e42446b0f9c67a9b076cb0b07fbb2fc52cd147e459ea34968b9bd11e839, 9,265,351bytes/270entries/
15payloads. CRC and every payload digest pass. Installed-game
fingerprint/collision dry-run passes with no writes. IPC8/wire6/runtime=false.
Earlier archives remain immutable. Exact record: native-input-package-checks.json.
Full immersion goal remains active; native zoom and optics are the next scope.


## Current0.2.11 native per-hand zoom checkpoint

Native zoom input is connected end to end: contextual per-hand OpenXR action,
later neutral admission, local snapshot and existing wire6 intent, actual
caller-selected sniper Step/Fire, original native held/Activate/Deactivate,
field-free conditional reads and original PutDown/Delete. Native flags, timers,
progress, damage, ammo, recoil and sound remain native. Existing active zoom
survives native cooldown4/recoil8; holster/bringup cannot activate. Right-only
selection leaves left intent intact. Exact right-delete cross-call preservation
uses current ephemeral owner/BC routing independently of older cleanup claims.

Astra/xhigh source GO follows six interaction fixes plus the reassociated-owner
routing correction. One bounded per-source provenance store remains outside
copied snapshots/authority, with cleanup-only lifetime and no live eviction.
Revoked effects cannot restart, but same-source native Stop remains available;
source deletion/reuse prevents stale outer accesses. External native activation
retains stock ownership and restoration, conservatively declining another
adapter takeover until source deletion. 128-record exhaustion declines new zoom.
No deferred pointers, raw native state writes, scheduler or protocol change.

Final x86 proxy/server and x64 host/official loader cross-build and all15 offline
groups pass. Actual linked entries/callback return ABI, native-finally compiled
shape,47 exported/11 internal seams and matching IPC8 layouts pass. Unchanged
pinned HDE32 host decoding accepts18 stolen+8 continuation instructions. These
checks do not execute native callbacks, MinHook installation, Windows or VR.
Exact review/evidence: SNIPER_BORROWED_ZOOM_SOURCE_REVIEW.md and
native-zoom-source-checks.json. No installed Bin/config/save changes.

This completes native zoom controls, not optical rendering. Native owner FOV
is shared; the surrounding XR world preserves runtime eye FOV. Magnified optic
images, roomscale body collision, physical swing melee, broader vehicles and
complete remote-head lifetime/enablement remain required. Teleport stays excluded.

Separate0.2.11 archive created from accepted sourcefea524c; SHA256e72d51f0c57f3bc788dca969fd9e83c05ede1c4a4eeeee98509a8ed016e335ba, 9319806bytes/277entries/15payloads. CRC, every payload hash and installed fingerprint/collision
dry-run pass without writes. IPC8/wire6/runtime=false. Earlier packages
remain immutable. Exact record: native-zoom-package-checks.json. Full immersion
goal remains active; magnified optic images are the next implementation slice.

## Scope cap selection and owned geometry source follow-up

Explicit/effective Astra/xhigh rejected late finished-color lens replacement
and narrowed the next native image proof to the real cap material pass/root
suffix plus existing source-buffer ownership. Implemented shared exact
model/mesh/draw/palette selection and selected-surface layout observation in the
existing native palette callback; only values are retained per eye.
`contentVerified` stays false. No buffer traversal, lock, shader replacement,
resource cache or new native hook is enabled.

Added owned-byte exact cap extraction/full affine transformation and a private
fingerprint-bound stock mesh checker. Actual stock24 vertices/66indices and
reflection pass. Fixed in-place output aliasing and explicitly refuse NDEBUG
checker builds. Astra gave bounded SOURCE GO after both fixes. Final cross-build
and all16 portable groups/artifact/compiled-finally checks pass. No game, Windows,
XR, server or network runtime; installed files untouched;0.2.11 archive unchanged.
This advances scope association/geometry, not magnified image rendering or full
immersion completion. Actual cap color pass and inherited native buffer lifetime
remain the next gates; roomscale/melee/broader vehicles/remote head remain active.


## Actual Scope GPU geometry source follow-up

Implemented the minimal reader at the existing exact native DIP, with actual
current physical Scope raster/full affine, canonical bound COM identities,
static managed metadata/declaration/ranges and all four pinned slice hashes.
Geometry commits after successful original draw to the existing per-eye bank;
full contentVerified remains false. No new native hooks, image/shader, resource
cache, gameplay/protocol or recovery state machine.

Astra/xhigh required process-lifetime fatal draw containment for uncertain native
buffer Unlock, then caught incomplete hook coverage and stale same-invocation
geometry. All adopted: mandatory existing routing completeness, recognized
rejection retirement/no revival, unrelated surfaces preserved, explicit lock
ledger/native-finally cleanup and one uncertain reference retained until exit.
Reset cannot clear managed-buffer uncertainty. Ordinary declines retain native
forwarding. Source GO after both fixes and query-ordering refinement.

Final x86 proxy/server+x64 host/official loader cross-build/all17 portable groups
PASS; owned actual24/66 cap, artifact/IPC, compiled actual DIP argument/caller and
native-finally checks PASS. Runtime/game/XR/Windows/network execution absent.
Installed files untouched, immutable0.2.11 archive unchanged. Exact review and
source/product evidence: SCOPE_GPU_GEOMETRY_REVIEW.md/scope-gpu-source-checks.json.
Magnified source capture/color pass remains next; full immersive goal active.

## Optical frame and image shader preparation

Implemented current actual-cap-derived proper rigid optical camera/radii within
the existing scope observation. Full animated XYZ/quaternion/affine deformation
and left reflection stay intact. Astra caught translation cancellation, ray-scale
admission, malformed public records/duplicated FOV state and checker weaknesses;
all fixed with meaningful counterexample checks. Exact owned cap/UV0 mapping is
audited without retaining proprietary data.

Added mod-owned UV3/COLOR0 image shader and native Linux compilation/audit tool.
Generated151 slots require PS2-extended; baseline PS2 output was rejected. The
shader preserves original texture alpha × native fade/fog alpha and retains the
original VS interface. It is experimental and neither linked nor enabled.

Astra/xhigh design conditional GO/source preparation GO; final x86/x64 builds,
all18 offline groups, actual24/66cap/rigid frame and artifact/compiled DIP/finally
checks PASS. No native runtime, installed changes, protocol/hook changes or new
archive. Source capture/live color/UV admission/native image rendering remain
next. Full immersion goal is active, including roomscale/melee/broader vehicles/
remote head; teleport remains excluded. Exact record: scope-optics-source-checks.json.

## Live raw scope UV and bounded reconstruction

Extended the existing actual-DIP reader with mandatory bound stream3 FLOAT2/
TEXCOORD3, canonical same-VB metadata and the pinned7072-byte UV slice/hash.
Both COM snapshots include UV independently of optional weight stream6. The
fifth sequential copy retains the existing position-owned lock ledger/fatal
containment, with all unlocks/releases before hashing and successful native draw
before publication. Raw UV and positions share exact remapped source indices;
no new hooks, hasUv flag, cache, bank, generation or recovery policy.

Added centered UV-to-optic affine reconstruction with conditioning/local-fit/
float-reference error gates. Astra required final float validation, caught the
finite native UV offset4096 counterexample and removed optional fixture-driven
production state; all adopted. Owned checks verify every pair and every cap
triangle with unequal-W interiors. Identity defaults are mathematical fixtures;
actual native shader/constants/color purpose remain required before image use.

Astra/xhigh design conditional GO and bounded SOURCE GO. Final x86/x64 build,
all19 portable groups, fingerprinted five-slice checker, artifact/IPC, compiled
DIP forwarding and native-finally checks pass. Only Linux offline execution;
no actual COM/Windows/game/XR/network session. Magnified imagery remains absent,
contentVerified=false, full immersive goal active. No installed changes or new
archive. Evidence: SCOPE_LIVE_UV_REVIEW.md/scope-live-uv-source-checks.json.

User now requires Astra for investigations as well as reviews; AGENTS.md updated.
The active Terra investigation was stopped and handed off to Astra immediately.

## Astra native follow-up and roomscale design disposition

Astra investigated current native model/material routes, final modifier arguments,
actual shader/constants and generated input declarations. The ordinary preset
return7814C distinguishes the demonstrated fallback/visualization routes;
cached native VS handles/constants cannot certify successful device bindings.
Source preprocessing and actual declaration builder both join TEXCOORD3 to v3;
25 source/weight families retain c8/c9-to-oT3, while device-dependent bytecode
precludes invented exact hashes. A UV-only structural recognizer does not prove
whole skin/position/alpha or root COLOR ownership. No image enablement yet.

Astra's roomscale investigation verified ordinary native input/physics/RPC seams,
then senior-reviewed a submitted-request settlement candidate. Conditional GO
only for bounded prototype, production NO-GO pending action duration/units and
coherent pose/origin publication. Main rejects default promotion because native
walking response could consume physical horizontal travel under saturation or
acceleration. Proper6DOF remains the target; no roomscale placeholder was added.
Original native collision/movement must be retained. Next investigation seeks a
narrow native relative-collision movement/result seam before any physics rewrite.

Two effective Astra/xhigh turns verified for each follow-up agent; both closed
when complete. Exact records: SCOPE_NATIVE_COLOR_ADMISSION.md,
ROOMSCALE_INPUT_DESIGN_BRIEF.md and ROOMSCALE_INPUT_DESIGN_REVIEW.md. These are
native research/design results, not additional runtime-tested features. Existing
code acceptance8029af5/all19 offline groups remains; full goal active.

## Actual shader and UV row admission

The existing live scope reader now acquires the actual immutable VS and finite
c8/c9 rows, completely decodes a bounded conservative VS1.1 UV interface before
locking buffers, and rechecks shader identity/row bytes after all copies. Actual
rows derive current image coordinates only after native DIP success and fresh
raster validation. Existing observation retirement/reset clears both derived
fields. No new hook, cache, bank, generation, protocol or gameplay mutation.

Astra/xhigh design GO and bounded SOURCE GO; legacy full-mask examples added and
fixture evidence label narrowed per review. Full x86/x64 builds, all20 portable
groups, mod-owned independent direct fixture decode/compiled fixture rejection,
owned five-slice cap and artifact/compiled DIP/native-finally checks pass. Source
and products: scope-program-source-checks.json; review: SCOPE_PROGRAM_ADMISSION_REVIEW.md.
No native runtime, installed changes or new package. Actual native shader-family
compatibility, material/COLOR-alpha and source imagery remain unadmitted;
contentVerified=false and magnified imagery remains absent. Full goal active.

Astra's separate roomscale audit found checked placement skips small collision
queries and does not slide; kinematic/constraint targets do not return attributable
accepted player displacement. Solver storage is worker-owned. These APIs are not
promoted; native physics remains intact. See ROOMSCALE_COLLISION_API_AUDIT.md.

## Native scope alpha and opaque shader preparation

Astra verified PP and PP2 sample different alpha interpolants; experimental
COLOR0 main is now explicitly PP-only/unadmitted. Added a separate opaque RGB
entry without native color/diffuse-alpha inputs and entry-specific compiler
audits requiring onlyoC0 output. Both disabled entries compile/disassemble on
Linux: main151slots/opaque146slots,4temps. No native image consumer is enabled.

Senior review retracted the initially proposed late-overlay GO: equal stored
depth cannot identify cap ownership, and the final five triangles could be
overwritten. Main rejects that layout. Ordered native901/native22/RGB22/native5
is conditional on originalLESSEQUAL and actual PS/material/root/query/raster/
source-resource admission; the transaction must admit beforeprefix and reject
uncertain execution beforepairpublication. No extra draw is implemented.
SOURCE GO after correcting the alpha comment and rejecting depth/MRT shader
outputs; exact evidence: SCOPE_OPAQUE_IMAGE_REVIEW.md/scope-opaque-source-checks.json.

Astra traced CircularSaw native cadence/three-ray hit/damage authority and
existing tracked shooting placement coverage. Swing-to-fireMask alone lacks
proved native held/edge/release coherence. Narrow follow-up investigates whether
existing native commands suffice before adding any operator adapter. No combat
FSM/damage/physics/network replacement or gesture implementation is added.
See PHYSICAL_MELEE_NATIVE_AUDIT.md. Full immersive goal remains active.

## Actual opaque scope observation reader

Astra/xhigh reviewed the smaller device-property route and removed proposed
material hooks, shader-bank association and otherwise-unused module pinning.
Implemented a bounded complete PS2.0 property predicate, actual retained-eye
target/depth/viewport and opaque raster reads in the existing TLS owner. Both
snapshots are outside native buffer locks; partial COM outputs clean up above
NativeFinally. Color failure preserves admitted geometry. The new scalar means
matching pre-draw observations only; it is not continuing DIP/image permission.

Final Astra SOURCE GO after POW exponent-alias correction, inactive-scissor
simplification and positive observation-bank lifetime/reset checks. Full x86
client/server/x64 host/loader builds and all21 offline groups pass; owned cap,
mod-owned independent PS fixture decode and compiled DIP/native-finally checks
pass. Exact source/product evidence: scope-color-source-checks.json. No native
runtime, installed changes or new archive; contentVerified=false and magnified
imagery remains absent. Full immersive objective stays active.

Native primary review approves a five-site read-only scalar join, rejecting the
brain lease and preserving native history/blocking/mapping/FSM/RPC. EDI hook-entry
support is prepared and independently compile/disassembly checked; production
join/motion producer remain required. Owned typed patch resources permit combo2,
but live selection remains unknown; stock menu YES and actual getter/dual state
are the supported uncoupled route. No raw9BC/resource edits/new toggle loop.
See PHYSICAL_MELEE_JOIN_DESIGN_REVIEW.md/NATIVE_COMBO_CAPABILITY_AUDIT.md.

## Scope ordinary-command/query boundary

Actual physical weapon/sniper wrappers capture the pinned ordinary command
return4BF3A in the existing invocation. Both optional color snapshots require
fresh current Scope ownership and native Engine query index==-1. No query tracker,
Issue hook, bank or new protocol. Native count thresholds/flare ratios rule out
the simpler “already-visible coverage means irrelevant” argument. Bookkeeping
is explicitly not an all-issuers/failure-total GPU query getter.

Astra/xhigh bounded SOURCE GO; full client/server/host builds/all21 portable
groups and artifact/compiled DIP/native-finally checks pass. Exact fingerprints:
scope-query-source-checks.json. Geometry/native forwarding remain independent;
no split/image transaction, game execution or new package. Full goal stays active.

Further Astra evidence rejects native action replay for primary-only multiplayer:
ClientAction adds RPC and can overwrite outstanding movement correction. Compare
immutable read substitution before expanding native command/correction policy.
Native full-volume overlap lacks finite-move acceptance, and the thick-triangle
kernel has a concrete edge miss plus whole-hull continuation/floor-contact gaps.
A narrow scoped mathematical-kernel adaptation is under senior review; no body
move, solver replacement or movement protocol is implemented by those findings.
See PRIMARY_NATIVE_ACTION_AUDIT.md/ROOMSCALE_KERNEL_ADAPTER_REVIEW_BRIEF.md.

## Bounded native model-query source vertical

Added the scoped model-triangle adapter using original native TOI/traversal,
per-triangle initial-contact bounds and existing registrar/native-finally. No
hook is activated and no player displacement or movement protocol is added.
Astra/xhigh source review found a dot-order cancellation defect; main fixed it
with exact binary32 products and a native-order interval facing certificate.
The follow-up received bounded SOURCE GO.446 portable checks pass in optimized,
UBSan and reviewed x87 precision variants; compile-only actual detour/fixture
ABI passes. See ROOMSCALE_MODEL_QUERY_SOURCE_REVIEW.md and its source seal.

The native solver join is verified, but public ray state includes cleanup
callbacks and visited-hull flags. Scalar save/restore is not safe query-lifecycle
ownership. Primitive initial contact, TOI numerical conservatism, actual body
coverage, checked placement and authoritative settlement remain required.

Scope first-gun capture has a demonstrated flare omission. Astra identified a
native contextual collection predicate that can omit self-weapons, then selected
a source-only native callback command ranked between visibility and bloom as a
consistent proposed capture cut. This preserves native suffix/cleanup; source
camera/target/color-stage and command lifetime proof still precede image work.
See SCOPE_SCENE_SOURCE_AUDIT.md. No scope image was enabled; no runtime was run.

## Native firing operator integration

Replaced the high-level native fire-query override with five audited current-bit
reads in the original operator/held paths. The existing simulation interval
captures one native-mapped value from existing input/authority, with immutable
same-pawn inheritance and no new history/action/ACK state machine. Native blocking,
flip, callbacks, inventory, weapon FSM, damage and RPC remain original.

Independent Astra/xhigh review found and closed desktop-startup and native-carry
ownership regressions. Local ownership now requires existing initialized tracking
and exact player/handle identity; a local transport nonce cannot claim it. Live
carried objects retain original primary commands before validation. Carry revokes
prepared handgun high for that interval while admitted copies stay immutable.
No vehicle-control replacement is added. Final bounded SOURCE GO is recorded in
PRIMARY_JOIN_SOURCE_REVIEW.md and primary-join-source-checks.json.

Client/server/host/loader builds, all23 offline groups, final linked predicates/
wrapper returns/native-finally/zero-call helper checks and pinned artifact checks
pass.48 exported native hooks/16 internal hooks; IPC8/wire6 unchanged. This
implements native consumer wiring, not physical gesture production, full body
collision or immersive scope images. No game/Windows/XR/network session was run
and no installation/deployment or archive replacement occurred.

## Physical primary worktree and carry architecture reopening

Added LOCAL XYZ/quaternion motion with actual grip provenance and explicit fresh
quiet evidence, plus a small game-side logical sample wired through existing
gate, packet, ACK, command and native primary consumers. Raw menu/render/XR input
is unchanged. No new combat, movement, transport or arming state machine.

Astra/xhigh design/source reviews caught neutral laundering, grip fallback,
carry manual-history loss and retired-result revival. Main corrected producer
and command-cache cases; current client/server compile and all25 portable groups
pass. Native puppet348 versus raw manual carry history is now reopened under
Astra Max: the physical-primary worktree remains unaccepted and unpackaged.
See PHYSICAL_PRIMARY_SOURCE_REVIEW.md and PHYSICAL_CARRY_ARCHITECTURE_REOPEN.md.

Independent Astra scopes audit rejects public bone-query ownership/partial-setup
recovery at this entry. Main selects a completed native first-eye preview as
the next source-pose proof candidate; no scope image is enabled by this evidence.
No game/Windows/XR/network execution, installed-file changes or archive mutation.

## Astra Max native carry-history counterexample

Fresh Astra/max confirmed that a gesture-only high can be committed to native
puppet348, then reach native ThrowObject after carry admission with manual input
low. The command-cache correction does not repair this native history. Current
physical-primary source remains NO-GO; compiled/portable producer checks do not
constitute acceptance of its native carry behavior.

The preferred incremental candidate keeps original command/RPC input manual-only
and records the two raw manual bits at the actual native history commit, while
existing logical pose intents continue feeding the handheld join. Separate Astra
proofs are pending for native lifetime/copy/commit ownership and carry consumers/
multiplayer observer behavior. No candidate hooks have been installed. See
PHYSICAL_CARRY_MAX_DISPOSITION.md. The full immersive goal remains active.

The follow-up Astra/max lifetime investigation identifies the exact completed
native store epilogue8E760 and corrects F29E to copy construction, but leaves
integration NO-GO: the current copied18-row Authority storage cannot own native
history without registration/copy/capacity and stale-save proof. No additional
native hooks were added. See PHYSICAL_CARRY_LIFETIME_GATE.md. Main's portable
production-helper trace independently confirms the cached/native release split.

Implemented the previously reviewed angular scope zoom conversion as a pure
bounded optical helper. Analytic half-angle, inverse magnification, malformed
angles and optical-projection integration checks pass, including UBSan. No
native zoom read, source-camera render, cap replacement or scope image is added
by this helper; those end-to-end integration requirements remain open.

Fresh Astra/xhigh carry/observer proof identifies late carry auto-throw at9B86C
and confirms that manual-only native replication would leave gesture-only
observer weapons idle: tracked grip placement alone does not replicate their
firing state/animation queue. ThrowObject veto alone also loses genuine manual
release under gesture-high. Source remains NO-GO. The expanded manual-history/
carry-selector/observer candidate materially exceeds the earlier join estimate,
so main reopened it under Astra/max to seek necessary boundaries and deletion.
See PHYSICAL_CARRY_OBSERVER_GATE.md and PHYSICAL_PRIMARY_SIMPLIFICATION_BRIEF.md.

Astra/max simplification review finds a real smaller saw candidate: its held
query starts native firing without primary0 DoAttack starting the weapon.
Held-only remains NO-GO because state4 can fire before reading low; canonical
release/bookkeeping/sound and manual-release isolation must be preserved.
One bounded native-state stop-equivalence proof is active, not an added history
implementation. See PHYSICAL_SAW_LEVEL_REVIEW.md.

Separate Astra/max registration evidence locates default player registration
after its348 zero store and verifies CoreB370 as known-handle validation.
Fresh copy-constructor destination identity remains unproved; EntityID/+9A0
copies alias source identity and cannot stand in for the native registry.
See PHYSICAL_CARRY_REGISTRATION_GATE.md. No new native hook, history registry,
protocol, package or installed-file mutation; immersive objective stays active.

Astra/xhigh saw-state proof rejects the concrete state4/sound1-or2 stop guard:
a new pulse during already-released state8/E8=3 can lack another start/fire
sound request and lose canonical release bookkeeping. Actual38 release value
is1. Main's production producer/gate trace admits20ms inputs with release20ms,
new high40ms and release60ms after designated recoil start; native actor timing
remains the bounded missing edge. Ending after cooldown does not by itself
repair the pre-step guard. See PHYSICAL_SAW_STOP_GATE.md. Native integration
remains unaccepted; no new hook or runtime execution.

Astra/xhigh closes the saw scheduling edge: native active simulation has a1ms
minimum and admits5ms/20ms variable steps. Exact entity/timer order supplies a
reachable state8/E8=3 second-release miss. The proposed state-only stop adapter
is rejected, not merely unproved. See PHYSICAL_SAW_SCHEDULE_PROOF.md.

Separate Astra/xhigh gives bounded design GO for a fresh, exact-weapon observer
held consumer preserving161 and manual160/348. Existing50ms latest-only relay
does not prove every short hold reaches consumption. Before adding retention,
main investigates native firing notification/state delivery. No observer source
or new transport is enabled. See PHYSICAL_OBSERVER_CONSUMER_REVIEW.md.

Fresh Astra/xhigh resolves that delivery gate: the firing helper is AI hearing;
ordinary dynamic replication excludes saw state and model animation queues, and
the saw receive callback is a no-op. This bounded absence does not deny other
replicated damage/world effects. A separate native-byte proof rejects proposed
bits2/3/4 metadata: bit2 is the stock grenade command and the operator processes
four lanes. It also identifies reset348 at98B41. No encoding or protocol change
was added. See PHYSICAL_NATIVE_DELIVERY_AND_BITS_GATE.md.

Main reopens the senior comparison around weapon-bound consumed logical-level
metadata versus durable manual command-history storage. B0/E8 alone is refuted,
but that does not prove a player-lifetime registry is necessary. The candidate
preserves manual command/history, native combat and carry; exact canonical
release/suppression, consumption/lifetime and observer delivery remain review
gates. See PHYSICAL_SAW_CONSUMPTION_REVIEW_BRIEF.md. Physical source is still
unaccepted and unpackaged; the full immersive objective remains active.

Astra/xhigh establishes stock fresh-ray init/configure/check/consume ordering
and both collision/model deferred-cleanup registrations at the worker-joined
physics boundary. It isolates one remaining ownership edge: resource replacement
can pump queued post-load tasks through Core46A2D during world/model traversal.
No nested query is demonstrated yet, but callback non-reentry is unproved.
Native query unwind only destroys a profile sample; rayInit is not recovery for
an interrupted traversal. No query activation/body move was added. See
ROOMSCALE_FRESH_QUERY_GATE.md. Next inspect the exact task registration/target
set rather than treating worker exclusion as complete ownership.

Astra/max selects the narrower saw-bound consumed logical-input record with
manual160/348, conditional on exact native press/release dispatch, consumption
ordering and lifecycle proof. This avoids an unearned mirrored player-history
registry and its four carry selectors. The review adds copied-active-saw state
and complete post-flip dispatch targeting to the gates. See
PHYSICAL_SAW_CONSUMPTION_MAX_DISPOSITION.md. Native reconciliation is not enabled.

Implemented the review's concrete sender-loss correction in the existing pose
freeze/relay path: an optional relay view retains accepted one-use pulses while
keeping ordinary fire separate from historical expansion; pulse snapshots can
emit immediately through the existing relay path. No wire version, ACK/credit
policy or receiver retention layer changed. Production freeze→relay codec checks
cover released two-hand pulses, pulse-plus-physical-hold with ordinary fire0/1,
one-use clearing and expiry. All25 offline groups and x86 game/server/x64 host
builds pass. Fresh Astra/xhigh gives sender-only SOURCE GO; additional focused
historical-zoom/revocation/expiry checks pass. Read-only compiled artifact
verification and network UBSan pass. See OBSERVER_PULSE_SENDER_SOURCE_REVIEW.md.
Latest-value receiver overwrites,
unreliable transport and native observer consumption remain unfinished; these
checks do not establish complete multiplayer melee or runtime behavior.

Astra/xhigh resolves the post-load registration set to five native task types,
but the global queue includes reflected deletion and indirect graphics/sound/
model callbacks whose full non-reentry is unproved. No actual nested ray was
demonstrated. Main reopens ownership design before expanding into a whole-program
callback audit; readiness/owned-query alternatives require native proof, not
guessed flags or queue suppression. See ROOMSCALE_POSTLOAD_TASK_GATE.md.

The next bounded Astra saw proof targets exact canonical post-flip dispatch and
receipt/commit ordering, considering completion of bookkeeping/base stop before
sound/resource reentry. It runs against the existing stable uniquely mapped saw
reference. No native reconciliation hook, copy/lifetime policy or new history
registry is enabled. See PHYSICAL_SAW_DISPATCH_COMMIT_PROOF_BRIEF.md.

Astra/xhigh establishes canonical semantic0/1 target sets and a completed
base-release receipt at165498 before cutting-sound processing. The preferred
candidate wraps original48FF0 instead of relocating the soundcall. Source is
still gated: state4→1 performs animation work before B0 is written; actual
same-binding reentry on that earlier path remains unproved. Next inspect that
specific animation/get-idle closure, not a generic hypothetical callback tree.
See PHYSICAL_SAW_DISPATCH_COMMIT_GATE.md. No native hook/state write was added.

Astra/xhigh rejects a resource-ready getter shortcut: concrete World/model
IsReplacementReady always returns1, proxy event readiness doesn'tclear loaded
resource flags, and refcount/change-count/queue observations don'tpin every
dynamicreceiver throughaquery. No actual nestedray isdemonstrated. Main reopens
theownershiparchitecture around a narrow unavailable-result path for the extra
mod-owned query at actual replacement branches, preserving allstockquery
callbacks andnormalcleanup. This candidate remains unverified; no body/query
hook or resource-pointer replacement isenabled. See
ROOMSCALE_RESOURCE_ADMISSION_GATE.md andROOMSCALE_QUERY_REJECTION_REVIEW_BRIEF.md.

## 2026-10-05 — User pause and project handoff

The user explicitly requested a pause and documentation for another agent.
Goal status is paused. Added docs/HANDOFF.md and entry links in README,
AGENTS and implementation status. Preserved all seven unaccepted physical-melee
worktree paths and immutable0.2.11 archive; no source edits, builds, runtime
tests, packaging or installed-game changes were performed for the handoff.

Recorded both final read-only Astra results and closed their completed agents:
Astra/xhigh proves the saw state4-to1 animation/get-idle normal allocation-success
path non-reentrant, but failure allocator -> fatal conExit callback closure
remains unproved. Astra/Max rejects two-site roomscale cancellation coverage:
deeper model-prep skeleton replacement at E1810 reaches a concrete global pump.
General purpose-bound cancellation remains conditional on real prep/model/world
cleanup and invocation provenance. Neither result enables a native hook or
completes an immersive feature. Handoff lists the exact bounded next proofs,
accepted observer sender fix, remaining integration and verification limits.

## 2026-10-05 — Root native inspection and inactive receipt candidate

Privately supplied Core/Engine/Sam2Game/Sam2 and 41 additional DLLs were read,
never run or published. Pinned fatal-exit checker now reproduces fatal/pre/list/
post callback sites and MSVCR71 exit identity; registration closure remains
unproved. Added inactive noncopyable consumed-level receipt helper, with explicit
stable lifetime-epoch ownership and nested-completion revision fencing. Native
integration, carry isolation and observer delivery remain gated. All25 portable
groups pass Debug/Release; receipt ASan/UBSan passes with unsupported leak checking
disabled, network UBSan passes. Original PC files and unaccepted WIP unchanged.

## 2026-10-05 — Cloud toolchain and independent gesture preparation

Root rebuilt x86 game/server and x64 host/loader using native-TLS GNU MinGW16.2.
Configure now checks the compiler's actual TLS object and rejects emulated TLS;
linked primary predicates remain call-free. Corrected the stale camera wrapper
verifier to prove twelve matrix words plus separate native caller provenance,
with eight negative/control cases. Added a bounded GNU16 vehicle receiver proof
and explicit private-game path to roomscale verification. Targeted linked/fixture
ABI checks pass; no check was disabled to accept unsafe compiler output.

Recovered only motion math/tests from preserved WIP and added an independent
physical gesture source that never mutates manual input/history. All28 portable
groups pass Debug/Release; gesture/receipt sanitizers and both Windows compile-only
variants pass. Helpers remain inactive pending native ownership/dispatch and
multiplayer integration. User requires root-only work through full completion;
there is no outside review step, no runtime authorization, and no completion claim.

## 2026-10-05 — Root-owned magnified-scope source integration

Connected per-hand native zoom/base-angle observation, a distinct first-eye
preview, gun-free native source views and a pre-bloom capture command. The
actual admitted opaque Scope draw now executes native901/native22/image22/
native5 with unchanged native VS/geometry, RGB-only image writes and verified
restoration of touched GPU state. Images remain under the same frozen native
frame/resource owner. Unknown target/shader/geometry/correspondence declines
optional imagery; partial image transactions reject the pair without replay.

This is an implemented narrow direct-UNORM subset, not runtime validation or a
claim of compatibility with arbitrary HDR/postprocess/modded content. No game,
Windows, headset or multiplayer runtime was executed. Roomscale body collision,
physical melee and remaining vehicle/head requirements remain open; broader
research and the requested full-project self-review still follow implementation.

## 2026-10-06 — Windows/Proton startup hardening

Bounded UTF-16 module/system path reads replace unchecked MAX_PATH buffers.
Host launch now uses an explicit image path with inherited prefix/environment;
system D3D9 loading rejects proxy aliases and absent mandatory exports. Added
Wine/DXVK OpenXR failure diagnostics and documented both target platforms.
All36 Debug/Release groups, both architecture builds, artifact/affected ABI
checks, path sanitizers and Windows compile-only checks pass. Runtime support
remains unverified under the no-runtime-testing constraint.

## 2026-10-06 — Cross-platform installer failure recovery

Installer preflight rejects Windows device/path aliases and case/file-directory
collisions even on Linux. Reserved receipt paths cannot enter the payload.
A copied payload is rehashed before receipt creation; copy-integrity failures
roll back generated files and newly-created empty folders, allowing retry.
Synthetic regression covers corrupted copies, retry and platform path hazards;
all37 offline groups pass Debug/Release. No installed game was modified.

## 2026-10-06 — Public offline CI

Added read-only, commit-pinned Linux CI for Debug/Release portable helpers and
publication metadata, with an explicit required Python-test inventory. A fresh
local Release configure/build passes all37 groups. This workflow does not build
or run the Windows mod and has no private game inputs. Remote results are not
claimed until the published workflow completes.

The first public CI run exposed two runner-compiler fixture assignments that GCC14/16
accepted. Made the OrderedPosePolicy reset type explicit without changing the
production policy or assertions. Also advanced the pinned official checkout
action to its Node24-based v6 to remove the runner deprecation warning. Both
local configurations still pass all37 tests; remote compatibility has not been
rechecked because the owner subsequently requested Actions disabled.


## 2026-10-06 — Owner-requested local-only checks

Removed the Actions workflow after the owner requested CI disabled for this
repository. Continue local offline checks and anonymous source publishing. No
GitHub Actions rerun is requested; the fixture reset fix remains verified only
with the local compilers. Game/device testing remains the owner's responsibility
after implementation, per their explicit renewed no-runtime instruction.

## 2026-10-06 — Optional roomscale resource-cancellation boundary

Implemented and cross-built inactive native gates for ten replacement branches
and five cancellation propagation points. Static inspection found two additional
material-getter callbacks beyond the prior model-preparer inventory. No hook is
queued by engine attach; body movement/query ownership remain unfinished. The
ABI verifier checks pinned native prefixes/cleanup and compiled save/TLS/stack
boundaries; all38 portable groups pass Debug/Release, with decision sanitizers
and both Windows compile-only fixtures. No runtime was executed.

## 2026-10-06 — Primitive contact portion of roomscale query

Added an inactive primitive-kernel adapter for bounded nondeepening initial
contact on native sphere/box/capsule/cylinder descriptors. Preserves original
unscoped calls and hidden-output ABI; unsupported/deepening/uncertain contacts
invalidate the entire optional query. All39 local groups pass Debug/Release;
8000 generated geometry cases, ASan/UBSan, both Windows compile-only fixtures,
pinned native/compiled ABI and x86 product builds pass. Full roomscale query
ownership, placement and multiplayer settlement remain unfinished; no runtime.

Added a bounded, outward-rounded local capsule cover using the copied actual
primitive dimensions, with a caller-supplied query inflation budget and at most
34 spheres. It covers continuous volume rather than sparse point samples;
unsupported shapes/budgets fail closed. All40 local groups and cover sanitizers/
both Windows compile-only checks pass. Native body movement is still unconnected.

## 2026-10-06 — Physical model-hull cancellation coverage

Static inspection found the distinct CModelHull→CBF90→CB3B0 collision path and
its eight resource-replacement branches. Added exact optional-query gates and
native false-return cleanup, bringing the inactive adapter to23 bindings. The
pinned native/compiler verifier, x86 product builds and all40 local Debug/Release
groups pass. No hook activation, native query or body movement is claimed.

Combined resource/triangle/primitive scope entry now shares one sticky failure
flag instead of requiring the eventual caller to join two independent results.
It requires both math bindings, preserves native-finally ownership, and remains
inactive until the actual query owner is connected. Existing compiled ABI and
all40 local Debug/Release groups pass with a resource-to-math failure regression.

## 2026-10-06 — Inactive body geometry reader

Added a bounded single-hybrid/single-primitive body reader with generation and
owner comparisons, separate model/hull placement and fail-closed graph/shape
validation. The 41st portable group checks synthetic memory and per-read failure;
all41 Debug/Release groups, reader ASan/UBSan and both Windows compile-only checks
pass. No live reader, query owner, placement transaction or movement is enabled.

The primitive-hull hit-material path also conditionally replaces a smart object
after its math result. Added the24th inactive resource/propagation gate, retaining
the native false-return cleanup and invalidating the whole optional query. Native
and compiled ABI checks plus x86 products pass. Dummy hulls use a thin kernel;
complete thick-query hull admission remains open rather than silently ignored.

Bounded optional native hull dispatch to four exact inspected class tables and
unchanged virtual targets. Unknown/dummy/derived targets cancel the whole query
through native traversal continuation rather than disappearing from collision.
The25th inactive gate passes native/compiled ABI checks, both x86 builds and
all41 local Debug/Release groups. Native query ownership remains unconnected.

Upright capsule-cover placement now accounts conservatively for native world
float-grid quantization and refuses a radius-budget overflow. Zero-offset sphere
placement remains exact. All41 local groups plus cover sanitizer and both Windows
compile-only fixtures pass. This is geometry support, not enabled body movement.

## 2026-10-06 — Actual player collision assets and two-hull capture

Verified the privately supplied player model and its mechanism/skeleton/mesh
inputs. The native Swimming profile has two capsules, one rotated; the previous
single-hull reader intentionally rejected it. Extended bounded geometry capture
to retain both complete shapes without admitting an upright-only query for a
rotated shape. Added pinned asset/profile audit and failure/cycle/extra-shape
regressions. Body movement, native ownership and transformed-cover integration
remain inactive and unfinished; no runtime was launched.

Added an outward-bounded affine image cover for rotated hulls, including centre
quantization and a caller-owned radius budget. Rotation/shear boundary checks,
sanitizers, both Windows compile-only fixtures and all41 Debug/Release groups
pass. This is mathematical cover support; actual native collision-frame binding
and movement remain unconnected.

Bounded the actual raw native quaternion expansion and its float spills rather
than using normalized presentation math. The cover includes finite-precision
forward/inverse-transpose geometry and now prepares both captured hulls as one
all-or-nothing result. Two thousand independent reconstruction cases and all41
local groups pass, alongside body/cover sanitizers, x86/x64 compile-only and
pinned native layout checks. Native query ownership, movement and settlement
remain unfinished; no runtime or hook activation.

Added mandatory whole-path clearance mode to the combined body-query scope.
Initially separated candidates need an outward finite-path supporting-plane
certificate; an unproved native miss cannot authorize movement. False-miss
controls, generated primitive oracles, all41 local groups, x86 builds, kernel
ABI checks, sanitizers and both Windows compile-only fixtures pass. This remains
inactive pending native query ownership and checked movement integration.

## 2026-10-06 — Optional immersive swimming and joystick baseline

Connected a caller-scoped native water-input adapter. Head-directed joystick
swimming is the default; `[Swimming] Immersive=1` enables bounded arm pulls while
joystick/jump input is idle. The native control/RPC/physics path remains in use,
with unchanged look/fire, no body writes and no new wire fields. Strokes reset
on focus/tracking/menu/wheel/reference/identity/water-mode interruptions. Exact
native pose records, movement basis, RPC vector and compiled calling convention
have new static checks. See SWIMMING_CONTROLS.md. Runtime behavior and comfort
are unverified on Windows and Proton; no game or headset session was launched.

All42 portable groups pass Debug/Release; production swimming helpers pass
ASan/UBSan. Both x86 products and the x64 host build. Native/compiled ABI checks
pass normally and under Python -O; a wrong-cleanup object is rejected. Artifact,
export and matching IPC-layout checks pass. No runtime verification is claimed.

Bounded the physical-model vertex-buffer provider route for optional roomscale
queries. A new inactive pre-lock gate admits only unlocked system-memory reads
and cancels before unknown/GPU providers, preserving native unlock/cleanup.
All42 portable groups and both x86 builds pass; the expanded26-gate native and
compiled-register/TLS/FP verifier passes normally and with Python optimization.
Whole-query ownership/allocation and placement remain open.

Added same-thread native ray-extent observation and native-finally cleanup around
query initialization/check/continuation/model queries. Existing laser/head
probes now reject nested native-query entry and are quarantined after abort,
without trying to repair native traversal or suppress stock queries. This
connects an ownership observation boundary, not roomscale movement. Also pinned
the fatal-report MessageBox/paint-lock path as additional static evidence;
shutdown and saw-consumption closure remain unproved.

Both x86 products, all42 Debug/Release groups, native-finally product checks,
artifact/export/IPC checks and normal/-O production query-observer ABI checks
pass. A deliberately corrupted model-argument slot is rejected. The swimming
compiled ABI still passes after the observer change. No runtime was executed.

## 2026-10-06 — Checked-placement pre-write boundary

Added an inactive typed native placement adapter and two cancellation gates.
Its collision-check callback sees the actual composed root target before any
root write; refusal returns through the original checked-setter failure suffix
before joint/model callbacks. Commit entry is tracked separately from successful
return so an uncertain mutation cannot be blindly retried. Arming and each
request verify installed guard JMPs, including relocated CALL provenance.

All43 portable groups pass Debug/Release, the production progress helper passes
ASan/UBSan, both x86 products build, and normal/-O native/compiled placement ABI,
resource, native-finally and artifact checks pass. A wrong cancellation-frame
fixture is rejected. The component remains inactive until actual swept geometry,
query ownership and origin/replication settlement are connected. See
ROOMSCALE_CHECKED_PLACEMENT_BOUNDARY.md; no runtime was executed.

## 2026-10-06 — Isolated extra-query arithmetic mode

Added an inactive bounded FP frame for roomscale math/query work. It saves the
caller FX image, installs explicit x87/SSE controls, checks entry/exit modes and
restores through native-finally cleanup. Native commit and ordinary gameplay
must remain outside it. All44 Debug/Release groups, FP-helper ASan/UBSan, both
x86 builds and normal/-O compiled boundary checks pass; a restore-as-save
negative fixture is rejected. See ROOMSCALE_MATH_FRAME.md. No runtime testing
or roomscale activation is claimed.
