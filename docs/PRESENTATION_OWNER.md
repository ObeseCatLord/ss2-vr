# Native presentation owner — PC continuation

The focused IPC10 desktop test rendered the menu but never entered the current
device-Present hook. The pinned native windowed renderer creates a16x16 implicit
buffer and presents a WindowCanvas additional swapchain. Existing allocation,
menu readback and stereo admission independently use implicit buffer zero; a
callback hook alone cannot correct these consumers.

Astra Max reviewed the actual source and owned native binary. Effective
current-turn settings were verified as Astra/max for1/1 contexts. Its conditional
design approval is not source/runtime acceptance of an implemented adapter.
Main spot-checks confirm the pinned graphics fingerprint, native additional-chain
creation and the separate chain/device callsites. Private native-derived evidence
remains outside source.

| Recommendation | Disposition |
|---|---|
| Observe additional-chain creation and chain Present | Adopt first as a bounded read-only native probe; production admission awaits live ownership evidence. |
| Retain current device-Present coverage | Adopt; both original routes remain forwarded. |
| Intercept every GetSwapChain | Omit; seed implicit zero at creation/successful reset and observe native additional-chain creation. |
| Carry actual canvas identity to all three buffer consumers | Adopt for the subsequent adapter, after focused WindowCanvas correlation. |
| Separate menu color readback from stereo depth readiness | Adopt for the subsequent adapter; rendererReady must still certify stereo resources. |
| Cache persistent backbuffers or allocate on reset success | Reject; reacquire at admitted boundaries, release resources before reset. |
| New renderer/transport, unconditional worker-thread host boot | Reject; no demonstrated incompatibility requires these changes. |

## Tracing-only checkpoint

The probe hooks two explicitly observed COM method implementations at most,
using separate typed trampolines. Unknown methods remain untouched and are
reported once. It is limited to the immutable first device/creation thread;
that diagnostic snapshot does not establish current canvas ownership. No chain
registry, capture owner, input policy or host-start path is added.

Chain Present is x86 WINAPI/stdcall with this plus five arguments including the
trailing DWORD flags (24stack bytes). The callback preserves all arguments,
calls the original once and returns its HRESULT. A bounded before/after trace
records native caller, effective window, actual chain/device/backbuffer,
RT0/depth descriptors, readiness and HRESULT. Temporary COM references are
released through the existing NativeFinally boundary before original Present.
The probe does not retain resources across presentation, reset or replacement.

The tracing source received a separate Astra/xhigh review. Initial NO-GO found
completion-time COM queries, unbounded failed-hook diagnostics and independent
creation snapshots that could disagree. All were corrected: completion logs
scalars only, failures are latched, discovery/tracing share one immutable
identity, and observation counters saturate. The follow-up approved the bounded
desktop probe; effective Astra/xhigh current-turn settings were verified. This
does not approve a production capture adapter.

Compiled callback inspection must confirm the24-byte stack cleanup. Native
testing must correlate recurring callbacks after hooksReady with the focused
game output and pinned presentation provenance. A stock-looking desktop menu,
successful hook installation or returned HRESULT alone is insufficient evidence
of VR capture or OpenXR submission.

The owner-executed native probe now confirms eight recurring post-readiness
calls from the pinned native chain presentation site. The first creation
device/thread agree, effective destination equals the foreground game window,
and the1280x720 X8R8G8B8 non-MSAA backbuffer is RT0. All original Present returns
are successful. The currently bound native D24S8 depth is3440x1440; compatible
stereo/viewport admission remains separate. A verified focused-window capture
shows the rendered startup scene, not a newly accepted menu/VR image. No host
was started by the tracing-only game path. The lab PID was stopped and evidence
preserved privately. This closes the recurring native route discovery gate,
not capture/resource/lifecycle acceptance.

## Subsequent adapter acceptance

Use one admitted owner beside existing resource lifetime: device/chain/window,
descriptor and resource generation. Reject foreign outputs before IPC/input/GPU
work. Prove native main-thread and creation ownership, native callsite/window
correlation, full canvas extent and source properties. Reacquire chain buffer
zero per capture; keep native Render3D/UI classification and CPU transport.

Menu needs matching non-MSAA color/readback, independent of stereo depth.
Stereo still requires compatible native RT0/depth/viewport and existing owner
gates. Distinguish device-to-chain delegation from genuinely nested native
presentation, preserve native exception propagation, and explicitly clean
references/locks/TLS across unwinds. Reuse the current channel/host body only
after owner admission.

Invalidate pending frames/menu freshness on owner replacement/loss, including
same-size replacement. Separate target reallocation from owner retirement.
Release frame-retained and added references before original Reset, quarantine
failed-reset callbacks and reacquire only at a later admitted boundary after
native chain recreation. Resize/reset/device-replacement proof precedes gameplay
stereo acceptance.

Smallest vertical proof: focused native menu → admitted actual chain → one host
launch → real-sized CPU menu image matching visible composition, while desktop
behavior remains correct. OpenXR graphical simulation and actual headset/input
acceptance remain separate later gates.

Contracts: [chain Present](https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3dswapchain9-present),
[readback](https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-getrendertargetdata),
[Reset](https://learn.microsoft.com/en-us/windows/win32/api/d3d9/nf-d3d9-idirect3ddevice9-reset).
