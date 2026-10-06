# Connected native UI: Astra review and disposition

Baseline7295b76, working integration. Explicit/effective gpt-6-astra/xhigh reviewer
Turing the3rd. Read-only senior/source review, no delegates or runtime execution.
Connected source brief: NATIVE_FLAT_UI_CONNECTED_BRIEF.md. Main owns edits and
spot-checks. Initial SOURCE NO-GO; final bounded follow-up SOURCE GO below.

| Astra finding | Main disposition |
|---|---|
| Point primitives have an untransported pixel footprint | Adopt and broaden narrowly to triangle-only panel admission. Points AND lines have device pixel footprints outside the accepted triangle homography; preserve original desktop/offscreen output and reject unsupported main-target topology before duplication/complete metadata. Actual D3D enum values statically asserted against local header; no reconstruction/renderer added. |
| Unlock failure loses ownership and permits readback reuse | Adopt. Explicit additional lock reference survives until confirmed UnlockRect. Failure faults pair and halts adaptation; bounded cleanup retries once then transfers unresolved reference to reset quarantine. Both stereo AND menu readback respect halt. deviceLost retires quarantined reference before successful-reset/new-creation recovery. |
| Loss during moved fallback HUD upload can survive final submission | Adopt. Final current quit/loss guard after all possible HUD upload/waits clears every projection/wheel/HUD layer and clears surviving-world accounting before submit. World-only eligibility is insufficient for other layers. |
| Triangle topology alone still admits point/wireframe fill | Adopt. Query actual D3DRS_FILLMODE, require SOLID with actual topology in the same production gate; add all admitted triangle families crossed with point/wireframe/unknown-fill rejection. Fill state is untouched, so no restoration state is added. |
| Focused offline failure regression coverage | Adopt. Tiny stateless production boundary helpers share topology, explicit lock-retirement and final layer retirement with portable checks. They own no frame/resource/queue/policy state. Checks cover unsupported raster families, retained lock owner on failed unlock and repeated cleanup, halt persistence after recovery, and newly reported loss after fallback upload including HUD-only frames. Native callbacks/GPU/driver/XR are not executed or mocked as proof of full behavior. |

Main independently confirmed each initial defect in actual source. The topology
gate follows successful original desktop draw and actual target classification;
offscreen draws retain native rendering. Failed normal unlock previously cleared
the pointer unconditionally; the fix keeps a separately retained COM lock owner.
The host's prior final loss check preceded new HUD waits; the new last guard
covers loss even when no world is present. No human decision/approval was needed.

Astra otherwise verified persistent eye-target references and empty-overlay
actual-eye panel admission; modified-state restoration and halt behavior; explicit
snapshot/binding/MP lock retirement with original binding→MP order; actual
nonlockable backbuffer and unsupported alternate-backbuffer output gates; explicit
frame/draw/observer swapchain ownership. Provider GetContainer errors beyond
E_NOINTERFACE reject conservatively; header confirmation is not provider execution.
It independently confirmed corrected shader evidence: all builtin vertex sources
remainVS1.1; the optional parser upgrade is ps.1., not a vertex declaration route.
No shader parser is required. Original m4x4/c1..4 assembler proof is unchanged.

Offline source checks/builds are bounded evidence. Stock callback/device/thread
opportunity, GPU appearance, performance/deadline, native unwind and runtime/headset
behavior remain unverified. Full immersive equivalence remains active: roomscale,
native zoom/optic images, swing melee, broader vehicles and remote-head lifetime/
enablement are still required. No game/host/Wine/XR/server/network session, installed
changes or publication. Older archives remain immutable.


## Final bounded source verdict

Astra independently checked topology AND actual solid-fill admission, retained
unlock ownership/quarantine/halt for world and menus, and last current-loss
retirement of every layer before accounting/submission. **SOURCE GO**; no blocking
ownership/submission regression. Effective Astra/xhigh verified for all reviewer
turns; reviewer closed. The small shared boundary helpers own no persistent state
or extra pipeline. Main spot-checked the load-bearing claims and actual compiled
brain/overlay/fill argument forwarding; x86 overlay wrapperret4, brain/fillret0
and local GNU native-finally callback alignment remain correct for accepted
artifacts. No arbitrary native crash recovery is claimed.

Final x86 proxy/server+x64host/official loader full cross-build, all14 portable
groups, refreshed41-export/5-internal-seam ABI8/wire6 artifacts and actual linked
native-finally checks pass. New failure regressions include solid-vs-point/
wireframe fill, explicit retained failed-lock ownership and late session loss.
These are offline production-helper/source/compiled checks, not native callback,
GPU, provider or runtime verification. native-ui-connected-source-checks.json
records exact source/artifact hashes and bounded evidence.
