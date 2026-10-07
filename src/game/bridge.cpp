#include "common/color.hpp"
#include "common/controls.hpp"
#include "common/winpath.hpp"
#include "common/frame_policy.hpp"
#include "common/native_ui.hpp"
#include "common/native_ui_finish.hpp"
#include "common/world_markers.hpp"
#include "game.hpp"
#include "menu.hpp"
#include "native_finally.hpp"
#include "scope_gpu.hpp"
#include "scope_source_gpu.hpp"
#include "common/scope_capture_command.hpp"
#include "shaders/scope_image_opaque.hpp"
#include "scope_observer.hpp"
#include <MinHook.h>
#include <atomic>
#include <bcrypt.h>
#include <cstdarg>
#include <cstdio>
#include <string>
#include <vector>
namespace ss2vr::game {
Channel channel;
static SRWLOCK logLock = SRWLOCK_INIT;
static FILE *logfile = nullptr;
void log(const char *fmt, ...) {
    AcquireSRWLockExclusive(&logLock);
    if (!logfile) {
        try {
            std::wstring directory;
            if (moduleDirectory(nullptr,directory)) logfile=_wfopen((directory+L"SS2VR.log").c_str(),L"a");
        } catch (...) {} // Logging failure must not strand the lock or cross native callbacks.
    }
    if (logfile) {
        va_list a;
        va_start(a, fmt);
        vfprintf(logfile, fmt, a);
        va_end(a);
        fputc('\n', logfile);
        fflush(logfile);
    }
    ReleaseSRWLockExclusive(&logLock);
}
static bool matches(HMODULE module, const char *hash) {
    std::wstring path;
    if (!modulePath(module,path)) return false;
    HANDLE f = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (f == INVALID_HANDLE_VALUE)
        return false;
    BCRYPT_ALG_HANDLE alg = nullptr;
    BCRYPT_HASH_HANDLE h = nullptr;
    bool ok = false;
    std::vector<unsigned char> obj;
    DWORD n = 0, size = 0;
    unsigned char digest[32];
    if (BCryptOpenAlgorithmProvider(&alg, BCRYPT_SHA256_ALGORITHM, nullptr, 0) >= 0 &&
        BCryptGetProperty(alg, BCRYPT_OBJECT_LENGTH, reinterpret_cast<PUCHAR>(&size), sizeof(size), &n, 0) >=
            0) {
        obj.resize(size);
        if (BCryptCreateHash(alg, &h, obj.data(), size, nullptr, 0, 0) >= 0) {
            unsigned char buf[32768];
            ok = true;
            while (true) {
                if (!ReadFile(f, buf, sizeof(buf), &n, nullptr)) {
                    ok = false;
                    break;
                }
                if (!n)
                    break;
                if (BCryptHashData(h, buf, n, 0) < 0) {
                    ok = false;
                    break;
                }
            }
            if (BCryptFinishHash(h, digest, 32, 0) < 0)
                ok = false;
            char hex[65]{};
            for (int i = 0; i < 32; i++)
                sprintf(hex + 2 * i, "%02x", digest[i]);
            ok = ok && std::string(hex) == hash;
        }
    }
    if (h)
        BCryptDestroyHash(h);
    if (alg)
        BCryptCloseAlgorithmProvider(alg, 0);
    CloseHandle(f);
    return ok;
}
bool supported(bool headless) {
    struct Item {
        const wchar_t *file;
        const char *hash;
    };
    const Item items[] = {
        {nullptr, headless ? "bfa9d628483bb6e9cb39338e075848a6e02328ccb612f701d87dd909d4f87b57"
                           : "727901f161133ff653fcdc196858335b991b743e67deb448c03c808e5b33e28b"},
        {L"Engine.dll", "da6efc9f72637eb3b6f48eadca2107be89b09c00618b6e72d5d3632938a7d851"},
        {L"Sam2Game.dll", "5628b4ed30a966f10c8e8ea46bf0789257a35ea80127bacacebe28b1ce5303df"},
        {L"Core.dll", "7a1bd56b9bfa3edfbb23f4d3c96490e40a7c0b031b85e797e1af91b2ba3cf207"},
        {L"GfxD3D.dll", "88749b79be36f0c0dccb623c4f1712685b5af603b451e25ed3c5029bedcdf3ed"}};
    for (auto i : items) {
        if (headless && i.file && std::wstring(i.file) == L"GfxD3D.dll")
            continue;
        auto m = GetModuleHandleW(i.file);
        if (!m || !matches(m, i.hash)) {
            log("Build fingerprint rejected; desktop retained (%ls)", i.file ? i.file : L"Sam2.exe");
            return false;
        }
    }
    return true;
}
static IDirect3DDevice9 *device = nullptr, *creationDevice = nullptr;
static DWORD creationThread = 0;
static IDirect3DSurface9 *eye[2]{}, *depth[2]{}, *readback[2]{};
static uint32_t width = 0, height = 0;
static D3DFORMAT depthFormat = D3DFMT_UNKNOWN, colorFormat = D3DFMT_UNKNOWN;
static bool hasStencil = false;
static std::atomic<bool> stopping = false;
static int deferredSlot = -1;
static uint64_t deferredSequence = 0;
static void retireDeferred() {
    if (deferredSlot < 0 || !channel.shared)
        return;
    Lock l(channel);
    if (l) {
        auto &slot = channel.shared->slot[deferredSlot];
        if (slot.state == SlotState::Rendering && slot.request.sequence == deferredSequence)
            slot.state = SlotState::Empty;
        deferredSlot = -1;
    }
}
thread_local int activeEye = -1;
thread_local bool eyeInvalid = false;
static thread_local bool scopeScratch = false;
static std::atomic<bool> scopeSourcesHalted{false};
thread_local IDirect3DSurface9 *desktopTarget = nullptr;
thread_local IDirect3DSurface9 *desktopDepth = nullptr;
thread_local Request renderRequest;
extern bool beginStereo(void *, const Request &);
extern bool headFramePrepared(void *,const Request &) noexcept;
extern float frozenWorldVisibility();
extern bool commitStereo(void *, const Request &, Slot &);
extern void beginEye(void *, const Request &, int);
extern void endEye();
static bool sameSurface(IDirect3DSurface9 *a, IDirect3DSurface9 *b) {
    if (a == b)
        return true;
    if (!a || !b)
        return false;
    IUnknown *x = nullptr, *y = nullptr;
    bool same = SUCCEEDED(a->QueryInterface(IID_IUnknown, reinterpret_cast<void **>(&x))) &&
                SUCCEEDED(b->QueryInterface(IID_IUnknown, reinterpret_cast<void **>(&y))) && x == y;
    if (x)
        x->Release();
    if (y)
        y->Release();
    return same;
}
using SetRT = HRESULT(WINAPI *)(IDirect3DDevice9 *, DWORD, IDirect3DSurface9 *);
static SetRT originalSetRT = nullptr;
using SetDepth = HRESULT(WINAPI *)(IDirect3DDevice9 *, IDirect3DSurface9 *);
static SetDepth originalSetDepth = nullptr;
using Stretch = HRESULT(WINAPI *)(IDirect3DDevice9 *, IDirect3DSurface9 *, const RECT *, IDirect3DSurface9 *,
                                  const RECT *, D3DTEXTUREFILTERTYPE);
using ReadTarget = HRESULT(WINAPI *)(IDirect3DDevice9 *, IDirect3DSurface9 *, IDirect3DSurface9 *);
using Fill = HRESULT(WINAPI *)(IDirect3DDevice9 *, IDirect3DSurface9 *, const RECT *, D3DCOLOR);
static Stretch originalStretch = nullptr;
static ReadTarget originalReadTarget = nullptr;
static Fill originalFill = nullptr;
// One existing Rendering slot, held through its native once-only owner. These
// explicit owners precede foreign calls: native SEH does not unwind GNU RAII.
static std::atomic<uint64_t> resourceGeneration{1};
uint64_t graphicsResourceGeneration() noexcept { return resourceGeneration.load(std::memory_order_acquire); }
static std::atomic<bool> uiHalted{false};
static bool uiRoutingReady = false;
bool scopeGpuRoutingCurrent(IDirect3DDevice9 *d) noexcept {
    return uiRoutingReady && d == device && d == creationDevice && creationThread == GetCurrentThreadId();
}
static IDirect3DSurface9 *uiQuarantinedReadback = nullptr;
struct NativeUiFrame {
    IDirect3DDevice9 *device = nullptr;
    IDirect3DSwapChain9 *chain = nullptr;
    IDirect3DSurface9 *color = nullptr, *ds = nullptr, *backbuffer = nullptr, *extra = nullptr,
        *target[2]{}, *z[2]{}, *read[2]{}, *locked = nullptr, *probeColor = nullptr, *probeDepth = nullptr;
    D3DVIEWPORT9 viewport{};
    RECT scissor{};
    Request request;
    Matrix44 projection[2];
    Pose panel[2];
    std::vector<uint8_t> pixels[2];
    void *player = nullptr;
    uint64_t generation = 0;
    int slot = -1;
    bool restore = false, ui = false, fault = false, world = false;
    bool overlay = false, overlaySeen = false, complete = false, fade = false;
    ScopeSourceView scopeView[2];
    IDirect3DTexture9 *scopeImage[2]{};
    IDirect3DPixelShader9 *scopeShader = nullptr;
    bool scopeReady[2]{};
};
static thread_local NativeUiFrame uiFrame;
static thread_local bool scopeTransaction = false, scopeInterference = false;
static thread_local uint64_t scopeTransactionGeneration = 0;
// Written once by the first deviceCreated diagnostic, then immutable. This
// snapshot correlates startup events; it never establishes current ownership.
static struct {
    IDirect3DDevice9 *device = nullptr;
    DWORD thread = 0;
} startupCreation;
static std::atomic<bool> startupCreationPublished{false};
// Startup observations only: no COM/native calls or shared-memory dereferences.
// Callback observations use arguments, immutable metadata, TLS and atomics.
// Local idle excludes cross-thread deferred state; unavailable fields are -1.
static void logStartup(const char *event, IDirect3DDevice9 *d, bool ready,
                       int routingReady = -1, int channelPresent = -1) {
    const DWORD currentThread = GetCurrentThreadId();
    const bool published = startupCreationPublished.load(std::memory_order_acquire);
    auto *firstDevice = published ? startupCreation.device : nullptr;
    const DWORD firstThread = published ? startupCreation.thread : 0;
    const int ownerMatch = published ? currentThread == firstThread && d == firstDevice : -1;
    const bool idleLocal = uiFrame.slot < 0 && activeEye < 0 && !scopeScratch && !scopeTransaction;
    log("Startup %s device=%p firstCreationDevice=%p firstCreationThread=%lu thread=%lu "
        "firstOwnerMatch=%d idleLocal=%d hooksReady=%d routingReady=%d channel=%d "
        "stopping=%d halted=%d eye=%d uiSlot=%d scratch=%d transaction=%d",
        event, static_cast<void *>(d), static_cast<void *>(firstDevice), firstThread, currentThread,
        ownerMatch, idleLocal, ready, routingReady, channelPresent, stopping.load(), uiHalted.load(),
        activeEye, uiFrame.slot, scopeScratch, scopeTransaction);
}
bool startupDeviceOwner(IDirect3DDevice9 *d) noexcept {
    return startupCreationPublished.load(std::memory_order_acquire) &&
        d == startupCreation.device && GetCurrentThreadId() == startupCreation.thread;
}
uint32_t traceChainPresent(IDirect3DSwapChain9 *chain, uintptr_t caller, HWND overrideWindow) noexcept {
    // Diagnostic only: immutable startup metadata is not current admission.
    // Foreign threads forward without COM introspection. References never span
    // the original Present, so reset/replacement cannot inherit probe ownership.
    if (!chain || !startupCreationPublished.load(std::memory_order_acquire) ||
        GetCurrentThreadId() != startupCreation.thread) return 0;
    const bool ready = hooksReady.load(std::memory_order_acquire);
    // Only the published creation thread writes these saturating counters.
    static unsigned observed[2]{}, tickets = 0;
    if (observed[ready] >= (ready ? 8u : 2u)) return 0;
    const unsigned ordinal = observed[ready]++;
    uint32_t ticket = 0;
    IDirect3DDevice9 *d = nullptr;
    IDirect3DSurface9 *bb = nullptr, *rt = nullptr, *z = nullptr;
    withNativeFinally([&] {
        const HRESULT deviceResult = chain->GetDevice(&d);
        if (FAILED(deviceResult) || d != startupCreation.device) return;
        ticket = ++tickets;
        D3DPRESENT_PARAMETERS params{};
        D3DSURFACE_DESC color{}, target{}, depthDesc{};
        const HRESULT paramsResult = chain->GetPresentParameters(&params);
        const HRESULT bbResult = chain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &bb);
        const HRESULT colorResult = bb ? bb->GetDesc(&color) : D3DERR_NOTFOUND;
        const HRESULT rtResult = d->GetRenderTarget(0, &rt);
        if (rt) rt->GetDesc(&target);
        const HRESULT zResult = d->GetDepthStencilSurface(&z);
        if (z) z->GetDesc(&depthDesc);
        const auto gfx = reinterpret_cast<uintptr_t>(GetModuleHandleW(L"GfxD3D.dll"));
        const uintptr_t rva = gfx && caller >= gfx ? caller - gfx : 0;
        const HWND destination = overrideWindow ? overrideWindow : params.hDeviceWindow;
        log("Presentation probe ticket=%u ordinal=%u completed=0 hooksReady=%d thread=%lu "
            "chain=%p device=%p caller=%p gfxRva=%08lx window=%p foreground=%p "
            "paramsHr=%08lx bbHr=%08lx descHr=%08lx bb=%p size=%ux%u format=%u msaa=%u "
            "rtHr=%08lx rt=%p rtSize=%ux%u depthHr=%08lx depth=%p depthSize=%ux%u depthFormat=%u",
            ticket, ordinal, ready, GetCurrentThreadId(),
            static_cast<void *>(chain), static_cast<void *>(d), reinterpret_cast<void *>(caller),
            static_cast<unsigned long>(rva), static_cast<void *>(destination),
            static_cast<void *>(GetForegroundWindow()), static_cast<unsigned long>(paramsResult),
            static_cast<unsigned long>(bbResult), static_cast<unsigned long>(colorResult),
            static_cast<void *>(bb), color.Width, color.Height, static_cast<unsigned>(color.Format),
            static_cast<unsigned>(color.MultiSampleType), static_cast<unsigned long>(rtResult),
            static_cast<void *>(rt), target.Width, target.Height, static_cast<unsigned long>(zResult),
            static_cast<void *>(z), depthDesc.Width, depthDesc.Height, static_cast<unsigned>(depthDesc.Format));
    }, [&](bool) noexcept {
        for (auto *surface : {bb, rt, z}) if (surface) surface->Release();
        bb = rt = z = nullptr;
        if (d) { d->Release(); d = nullptr; }
    });
    return ticket;
}
void scopeGpuOutput(IDirect3DDevice9 *d) noexcept {
    if (scopeTransaction && d == uiFrame.device) scopeInterference = true;
}
bool scopeGpuTransactionBegin(IDirect3DDevice9 *d) noexcept {
    if (scopeTransaction) { scopeInterference = true; return false; }
    if (!scopeGpuRoutingCurrent(d) || uiFrame.device != d || uiFrame.slot < 0 ||
        uiFrame.fault || uiHalted || stopping) return false;
    scopeTransactionGeneration = graphicsResourceGeneration();
    scopeInterference = false; scopeTransaction = true;
    return true;
}
bool scopeGpuTransactionCurrent(IDirect3DDevice9 *d) noexcept {
    return scopeTransaction && !scopeInterference && scopeGpuRoutingCurrent(d) && d == uiFrame.device &&
        uiFrame.slot >= 0 && !uiFrame.fault && !uiHalted && !stopping &&
        scopeTransactionGeneration == graphicsResourceGeneration() &&
        uiFrame.generation == graphicsResourceGeneration();
}
void scopeGpuTransactionEnd() noexcept { scopeTransaction = false; }
bool scopeGpuImage(IDirect3DDevice9 *d,unsigned hand,ScopeSourceView &view,
                   IDirect3DTexture9 *&texture,IDirect3DPixelShader9 *&shader) noexcept {
    if (texture || shader || hand >= 2 || scopeScratch || activeEye < 0 || activeEye > 1 ||
        !scopeGpuRoutingCurrent(d) || d != uiFrame.device || uiFrame.slot < 0 ||
        uiFrame.fault || uiHalted || stopping || scopeSourcesHalted ||
        uiFrame.generation != graphicsResourceGeneration() || !uiFrame.scopeReady[hand] ||
        !uiFrame.scopeImage[hand] || !uiFrame.scopeShader || uiFrame.overlay) return false;
    view = uiFrame.scopeView[hand];
    texture = uiFrame.scopeImage[hand]; texture->AddRef();
    shader = uiFrame.scopeShader; shader->AddRef();
    return true;
}
bool scopeGpuEyeOwner(IDirect3DDevice9 *d,IUnknown *&color,IUnknown *&z,UINT &w,UINT &h) noexcept {
    if(color || z || !scopeGpuRoutingCurrent(d) || activeEye<0 || activeEye>1 ||
        uiFrame.slot<0 || uiFrame.device!=d || uiFrame.fault || uiFrame.overlay || stopping || uiHalted ||
        uiFrame.generation!=graphicsResourceGeneration() ||
        renderRequest.sequence!=uiFrame.request.sequence ||
        renderRequest.input.sequence!=uiFrame.request.input.sequence ||
        !uiFrame.target[activeEye] || !uiFrame.z[activeEye]) return false;
    w=uiFrame.request.width; h=uiFrame.request.height;
    return w && h &&
        SUCCEEDED(uiFrame.target[activeEye]->QueryInterface(IID_IUnknown,reinterpret_cast<void **>(&color))) && color &&
        SUCCEEDED(uiFrame.z[activeEye]->QueryInterface(IID_IUnknown,reinterpret_cast<void **>(&z))) && z;
}
static thread_local bool uiBypass = false;
static void releaseUiFrame() noexcept;
static bool restoreUiFrame() noexcept;
void nativeUiFault() noexcept { if (uiFrame.slot >= 0 && uiFrame.ui) uiFrame.fault = true; }
void scopeGpuFault() noexcept {
    eyeInvalid=true;
    uiFrame.fault=true; // Includes world-only pairs; nativeUiFault is UI-conditional.
    uiHalted=true;
}
static bool uiCurrent() {
    return uiFrame.slot >= 0 && uiFrame.ui && !uiFrame.fault && !stopping && !uiHalted &&
        uiFrame.generation == resourceGeneration.load() && nativeUiDeviceCurrent(uiFrame.device) &&
        nativeUiFrameCurrent(uiFrame.player, uiFrame.request);
}
static bool observingUi(IDirect3DDevice9 *d) {
    return !uiBypass && uiFrame.overlay && uiFrame.ui && d == uiFrame.device;
}
// No resource owner is left on an inner callback stack. Restoration is limited
// to state actually modified; native programs/textures/blends/streams stay bound.
struct NativeUiDrawState {
    IDirect3DSurface9 *color = nullptr, *extra = nullptr;
    IDirect3DSwapChain9 *chain = nullptr;
    IDirect3DVertexShader9 *vertex = nullptr;
    IDirect3DPixelShader9 *pixel = nullptr;
    IDirect3DVertexDeclaration9 *declaration = nullptr;
    IDirect3DVertexBuffer9 *positions = nullptr;
    D3DVIEWPORT9 viewport{};
    RECT scissor{};
    Matrix44 constants;
    float planes[6][4]{};
    DWORD state[4]{};
    DWORD fill = 0;
    bool busy = false, captured = false, changed = false;
};
static thread_local NativeUiDrawState uiDraw;
static constexpr D3DRENDERSTATETYPE uiStates[]{D3DRS_ZENABLE, D3DRS_ZWRITEENABLE,
    D3DRS_SCISSORTESTENABLE, D3DRS_CLIPPLANEENABLE};
static void cleanupUiDraw(bool aborted) noexcept {
    if (aborted) { nativeUiFault(); uiHalted = true; }
    bool ok = true;
    if (uiDraw.changed && uiDraw.captured && uiFrame.device) {
        uiBypass = true;
        auto *d = uiFrame.device;
        ok = SUCCEEDED(originalSetRT(d, 0, uiDraw.color)) && ok;
        ok = SUCCEEDED(d->SetVertexShaderConstantF(1, uiDraw.constants.m, 4)) && ok;
        for (unsigned i=0;i<4;++i) ok = SUCCEEDED(d->SetRenderState(uiStates[i], uiDraw.state[i])) && ok;
        for (unsigned i=0;i<6;++i) ok = SUCCEEDED(d->SetClipPlane(i, uiDraw.planes[i])) && ok;
        // SetRenderTarget resets viewport. Both raster rectangles are restored last.
        ok = SUCCEEDED(d->SetViewport(&uiDraw.viewport)) && ok;
        ok = SUCCEEDED(d->SetScissorRect(&uiDraw.scissor)) && ok;
        uiBypass = false;
    }
    if (!ok) { nativeUiFault(); uiHalted = true; }
    uiBypass = false;
    if (uiDraw.chain) uiDraw.chain->Release();
    if (uiDraw.extra) uiDraw.extra->Release();
    if (uiDraw.color) uiDraw.color->Release();
    if (uiDraw.vertex) uiDraw.vertex->Release();
    if (uiDraw.pixel) uiDraw.pixel->Release();
    if (uiDraw.declaration) uiDraw.declaration->Release();
    if (uiDraw.positions) uiDraw.positions->Release();
    uiDraw = {};
}
static bool singleUiTarget(IDirect3DDevice9 *d,IDirect3DSurface9 *&extra) {
    D3DCAPS9 caps{};
    if (FAILED(d->GetDeviceCaps(&caps))) return false;
    for (DWORD i=1;i<caps.NumSimultaneousRTs;++i) {
        const HRESULT hr=d->GetRenderTarget(i,&extra);
        const bool active=extra!=nullptr;
        if (extra) extra->Release();
        extra=nullptr;
        if ((FAILED(hr) && hr!=D3DERR_NOTFOUND) || active) return false;
    }
    return true;
}
static bool uiOffscreen(IDirect3DSurface9 *target,IDirect3DSwapChain9 *&chain) {
    if (!target) return false;
    const HRESULT result=target->GetContainer(__uuidof(IDirect3DSwapChain9),reinterpret_cast<void **>(&chain));
    // Another backbuffer is another output, not an intermediate composition.
    if (chain) chain->Release();
    chain=nullptr;
    return result==E_NOINTERFACE;
}
static bool captureUiDraw(IDirect3DDevice9 *d) {
    if (!uiCurrent() || FAILED(d->GetRenderTarget(0,&uiDraw.color))) return false;
    if (!singleUiTarget(d,uiDraw.extra)) { nativeUiFault(); return false; }
    if (!sameSurface(uiDraw.color,uiFrame.color)) {
        if (!uiOffscreen(uiDraw.color,uiDraw.chain)) nativeUiFault();
        return false;
    }
    DWORD fog=0, stencil=0, clipping=0;
    if (FAILED(d->GetRenderState(D3DRS_FOGENABLE,&fog)) || fog ||
        FAILED(d->GetRenderState(D3DRS_STENCILENABLE,&stencil)) || stencil ||
        FAILED(d->GetRenderState(D3DRS_CLIPPING,&clipping)) || !clipping ||
        FAILED(d->GetRenderState(D3DRS_FILLMODE,&uiDraw.fill)) ||
        FAILED(d->GetVertexShader(&uiDraw.vertex)) || FAILED(d->GetPixelShader(&uiDraw.pixel)) ||
        !nativeUiProgramsCurrent(uiDraw.vertex,uiDraw.pixel) ||
        FAILED(d->GetVertexDeclaration(&uiDraw.declaration)) || !uiDraw.declaration) return false;
    D3DVERTEXELEMENT9 elements[MAXD3DDECLLENGTH+1]{};
    UINT count=MAXD3DDECLLENGTH+1, positionBytes=0, positionOffset=0;
    if (FAILED(uiDraw.declaration->GetDeclaration(elements,&count)) || count>MAXD3DDECLLENGTH+1) return false;
    for (unsigned i=0;i<count && elements[i].Stream!=0xff;++i) {
        const auto &e=elements[i];
        if (e.Usage == D3DDECLUSAGE_POSITIONT) return false;
        if (e.Usage != D3DDECLUSAGE_POSITION || e.UsageIndex != 0) continue;
        if (positionBytes || e.Stream || e.Method != D3DDECLMETHOD_DEFAULT ||
            (e.Type != D3DDECLTYPE_FLOAT3 && e.Type != D3DDECLTYPE_FLOAT4)) return false;
        positionBytes = e.Type == D3DDECLTYPE_FLOAT3 ? 12 : 16;
        positionOffset = e.Offset;
    }
    // Exact builtin VS1.1 position v0; legacy POSITION0 maps to register0.
    UINT offset=0,stride=0,frequency=0;
    if (!positionBytes || FAILED(d->GetStreamSource(0,&uiDraw.positions,&offset,&stride)) ||
        !uiDraw.positions || stride < positionOffset+positionBytes ||
        FAILED(d->GetStreamSourceFreq(0,&frequency)) || frequency!=1 ||
        FAILED(d->GetVertexShaderConstantF(1,uiDraw.constants.m,4)) ||
        FAILED(d->GetViewport(&uiDraw.viewport)) || FAILED(d->GetScissorRect(&uiDraw.scissor))) return false;
    for (unsigned i=0;i<4;++i) if (FAILED(d->GetRenderState(uiStates[i],&uiDraw.state[i]))) return false;
    if (uiDraw.state[3]) return false; // Do not overwrite an original user-plane policy.
    for (unsigned i=0;i<6;++i) if (FAILED(d->GetClipPlane(i,uiDraw.planes[i]))) return false;
    uiDraw.captured = true;
    return true;
}
using Draw = HRESULT(WINAPI *)(IDirect3DDevice9 *,D3DPRIMITIVETYPE,UINT,UINT);
using DrawIndexed = HRESULT(WINAPI *)(IDirect3DDevice9 *,D3DPRIMITIVETYPE,INT,UINT,UINT,UINT,UINT);
static Draw originalDraw = nullptr;
static DrawIndexed originalDrawIndexed = nullptr;
static_assert(D3DPT_TRIANGLELIST==4 && D3DPT_TRIANGLESTRIP==5 && D3DPT_TRIANGLEFAN==6);
static_assert(D3DFILL_POINT==1 && D3DFILL_WIREFRAME==2 && D3DFILL_SOLID==3);
template<class Call> static HRESULT uiDrawCall(IDirect3DDevice9 *d,D3DPRIMITIVETYPE topology,Call call) noexcept {
    if (!scopeGpuForwardingAllowed()) return D3DERR_INVALIDCALL;
    if (!observingUi(d)) return call();
    if (uiDraw.busy) { nativeUiFault(); return call(); }
    uiDraw.busy = true;
    HRESULT result = D3DERR_INVALIDCALL;
    const bool contained = withNativeFinally([&] {
        result = call(); // Desktop first, exactly once; preserve its HRESULT/state effects.
        if (FAILED(result)) { nativeUiFault(); return; }
        if (!captureUiDraw(d)) {
            // Offscreen drawing stays native; only eventual desktop composition is adapted.
            if (!uiDraw.color || sameSurface(uiDraw.color,uiFrame.color)) nativeUiFault();
            return;
        }
        if (!nativeUiTriangleTopology(topology,uiDraw.fill)) { nativeUiFault(); return; }
        NativeUiProjection projected[2];
        for (unsigned i=0;i<2;++i) {
            if (uiFrame.fade) {
                const auto &v=uiDraw.viewport;
                if (v.X || v.Y || v.Width!=uiFrame.request.width || v.Height!=uiFrame.request.height ||
                    v.MinZ!=0 || v.MaxZ!=1 || uiDraw.state[2]) { nativeUiFault(); return; }
                projected[i].constants = uiDraw.constants;
            } else {
                const auto &v=uiDraw.viewport;
                const NativeUiViewport vp{v.X,v.Y,v.Width,v.Height,v.MinZ,v.MaxZ};
                const auto &r=uiDraw.scissor;
                if (!nativeUiProjection(uiDraw.constants,uiFrame.projection[i],uiFrame.panel[i],
                    uiFrame.request.uiWidth,uiFrame.request.uiHeight,uiFrame.request.width,
                    uiFrame.request.height,vp,uiDraw.state[2]!=0,{r.left,r.top,r.right,r.bottom},projected[i])) {
                    nativeUiFault(); return;
                }
            }
        }
        uiBypass = true;
        uiDraw.changed = true; // Any partial mutation must attempt every restore.
        for (unsigned i=0;i<2;++i) {
            if (!uiCurrent()) { nativeUiFault(); break; }
            if (projected[i].empty) continue;
            bool ok = SUCCEEDED(originalSetRT(d,0,uiFrame.target[i]));
            const D3DVIEWPORT9 full{0,0,uiFrame.request.width,uiFrame.request.height,0,1};
            ok = SUCCEEDED(d->SetViewport(&full)) && ok;
            ok = SUCCEEDED(d->SetRenderState(D3DRS_ZENABLE,D3DZB_FALSE)) && ok;
            ok = SUCCEEDED(d->SetRenderState(D3DRS_ZWRITEENABLE,FALSE)) && ok;
            ok = SUCCEEDED(d->SetRenderState(D3DRS_SCISSORTESTENABLE,FALSE)) && ok;
            ok = SUCCEEDED(d->SetVertexShaderConstantF(1,projected[i].constants.m,4)) && ok;
            if (!uiFrame.fade)
                for (unsigned j=0;j<6;++j) ok = SUCCEEDED(d->SetClipPlane(j,projected[i].planes[j].data())) && ok;
            ok = SUCCEEDED(d->SetRenderState(D3DRS_CLIPPLANEENABLE,uiFrame.fade ? 0 : 0x3f)) && ok;
            if (!ok || FAILED(call())) { nativeUiFault(); break; }
        }
        uiBypass = false;
    }, [&](bool aborted) noexcept { cleanupUiDraw(aborted); });
    if (!contained) nativeUiFault();
    return result;
}
static HRESULT WINAPI draw(IDirect3DDevice9 *d,D3DPRIMITIVETYPE type,UINT start,UINT count) {
    scopeGpuOutput(d);
    return uiDrawCall(d,type,[&]{ return scopeGpuForwardingAllowed() ? originalDraw(d,type,start,count) : D3DERR_INVALIDCALL; });
}
static HRESULT forwardIndexed(IDirect3DDevice9 *d,D3DPRIMITIVETYPE type,INT base,UINT minimum,
                               UINT vertices,UINT start,UINT count) {
    return uiDrawCall(d,type,[&]{ return scopeGpuForwardingAllowed() ?
        originalDrawIndexed(d,type,base,minimum,vertices,start,count) : D3DERR_INVALIDCALL; });
}
static HRESULT WINAPI drawIndexed(IDirect3DDevice9 *d,D3DPRIMITIVETYPE type,INT base,UINT minimum,
                                  UINT vertices,UINT start,UINT count) {
#ifdef _MSC_VER
    const auto caller=reinterpret_cast<uintptr_t>(_ReturnAddress());
#else
    const auto caller=reinterpret_cast<uintptr_t>(__builtin_return_address(0));
#endif
    return scopeGpuDraw(d,type,base,minimum,vertices,start,count,caller,forwardIndexed);
}
// Complete output domain for IDirect3DDevice9 color writes: only the two above
// draw families are duplicated. Other desktop writes reject the entire pair.
static thread_local IDirect3DSurface9 *uiObservedTarget = nullptr, *uiObservedExtra = nullptr;
static thread_local IDirect3DSwapChain9 *uiObservedChain = nullptr;
static thread_local bool uiObserverBusy = false;
template<class Call> static HRESULT observeUiOutput(IDirect3DDevice9 *d, Call call,
                                                    IDirect3DSurface9 *target, bool current) noexcept {
    if (!scopeGpuForwardingAllowed()) return D3DERR_INVALIDCALL;
    if (!observingUi(d)) return call();
    if (uiObserverBusy) { nativeUiFault(); return call(); }
    uiObserverBusy = true;
    HRESULT result = D3DERR_INVALIDCALL;
    withNativeFinally([&] {
        result = call();
        if (current) {
            if (!singleUiTarget(d,uiObservedExtra)) { nativeUiFault(); return; }
            if (FAILED(d->GetRenderTarget(0,&uiObservedTarget))) { nativeUiFault(); return; }
            target = uiObservedTarget;
        }
        if (!uiCurrent() || !target || sameSurface(target,uiFrame.color) ||
            !uiOffscreen(target,uiObservedChain)) nativeUiFault();
    },[&](bool aborted) noexcept {
        if (aborted) { nativeUiFault(); uiHalted = true; }
        if (uiObservedChain) uiObservedChain->Release();
        uiObservedChain=nullptr;
        if (uiObservedExtra) uiObservedExtra->Release();
        uiObservedExtra=nullptr;
        if (uiObservedTarget) uiObservedTarget->Release();
        uiObservedTarget = nullptr;
        uiObserverBusy = false;
    });
    return result;
}
using Clear = HRESULT(WINAPI *)(IDirect3DDevice9 *,DWORD,const D3DRECT *,DWORD,D3DCOLOR,float,DWORD);
using DrawUp = HRESULT(WINAPI *)(IDirect3DDevice9 *,D3DPRIMITIVETYPE,UINT,const void *,UINT);
using DrawIndexedUp = HRESULT(WINAPI *)(IDirect3DDevice9 *,D3DPRIMITIVETYPE,UINT,UINT,UINT,const void *,D3DFORMAT,const void *,UINT);
using DrawRect = HRESULT(WINAPI *)(IDirect3DDevice9 *,UINT,const float *,const D3DRECTPATCH_INFO *);
using DrawTri = HRESULT(WINAPI *)(IDirect3DDevice9 *,UINT,const float *,const D3DTRIPATCH_INFO *);
using Update = HRESULT(WINAPI *)(IDirect3DDevice9 *,IDirect3DSurface9 *,const RECT *,IDirect3DSurface9 *,const POINT *);
static Clear originalClear = nullptr;
static DrawUp originalDrawUp = nullptr;
static DrawIndexedUp originalDrawIndexedUp = nullptr;
static DrawRect originalDrawRect = nullptr;
static DrawTri originalDrawTri = nullptr;
static Update originalUpdate = nullptr;
static HRESULT WINAPI clear(IDirect3DDevice9 *d,DWORD count,const D3DRECT *rects,DWORD flags,D3DCOLOR color,float z,DWORD stencil) {
    scopeGpuOutput(d);
    const auto call=[&]{ return originalClear(d,count,rects,flags,color,z,stencil); };
    return flags&D3DCLEAR_TARGET ? observeUiOutput(d,call,nullptr,true) : call();
}
static HRESULT WINAPI drawUp(IDirect3DDevice9 *d,D3DPRIMITIVETYPE type,UINT count,const void *vertices,UINT stride) {
    scopeGpuOutput(d);
    return observeUiOutput(d,[&]{ return originalDrawUp(d,type,count,vertices,stride); },nullptr,true);
}
static HRESULT WINAPI drawIndexedUp(IDirect3DDevice9 *d,D3DPRIMITIVETYPE type,UINT minimum,UINT vertices,UINT count,
                                    const void *indices,D3DFORMAT format,const void *data,UINT stride) {
    scopeGpuOutput(d);
    return observeUiOutput(d,[&]{ return originalDrawIndexedUp(d,type,minimum,vertices,count,indices,format,data,stride); },nullptr,true);
}
static HRESULT WINAPI drawRect(IDirect3DDevice9 *d,UINT handle,const float *segments,const D3DRECTPATCH_INFO *info) {
    scopeGpuOutput(d);
    return observeUiOutput(d,[&]{ return originalDrawRect(d,handle,segments,info); },nullptr,true);
}
static HRESULT WINAPI drawTri(IDirect3DDevice9 *d,UINT handle,const float *segments,const D3DTRIPATCH_INFO *info) {
    scopeGpuOutput(d);
    return observeUiOutput(d,[&]{ return originalDrawTri(d,handle,segments,info); },nullptr,true);
}
static HRESULT WINAPI update(IDirect3DDevice9 *d,IDirect3DSurface9 *source,const RECT *rect,IDirect3DSurface9 *target,const POINT *point) {
    scopeGpuOutput(d);
    return observeUiOutput(d,[&]{ return originalUpdate(d,source,rect,target,point); },target,false);
}
static IDirect3DSurface9 *eyeSurface(IDirect3DSurface9 *surface) {
    if ((activeEye >= 0 || scopeScratch)) {
        if (sameSurface(surface, desktopTarget))
            return uiFrame.slot >= 0 ? uiFrame.target[scopeScratch ? 0 : activeEye] : eye[scopeScratch ? 0 : activeEye];
        if (sameSurface(surface, desktopDepth))
            return uiFrame.slot >= 0 ? uiFrame.z[scopeScratch ? 0 : activeEye] : depth[scopeScratch ? 0 : activeEye];
    }
    return surface;
}
static HRESULT WINAPI stretch(IDirect3DDevice9 *d, IDirect3DSurface9 *source, const RECT *sourceRect,
                              IDirect3DSurface9 *target, const RECT *targetRect,
                              D3DTEXTUREFILTERTYPE filter) {
    scopeGpuOutput(d);
    HRESULT result =
        observeUiOutput(d, [&] { return originalStretch(d, eyeSurface(source), sourceRect, eyeSurface(target), targetRect, filter); }, target, false);
    if ((activeEye >= 0 || scopeScratch) && FAILED(result))
        eyeInvalid = true;
    return result;
}
static HRESULT WINAPI readTarget(IDirect3DDevice9 *d, IDirect3DSurface9 *source, IDirect3DSurface9 *target) {
    scopeGpuOutput(d);
    HRESULT result = originalReadTarget(d, eyeSurface(source), target);
    if ((activeEye >= 0 || scopeScratch) && FAILED(result))
        eyeInvalid = true;
    return result;
}
static HRESULT WINAPI fill(IDirect3DDevice9 *d, IDirect3DSurface9 *target, const RECT *rect, D3DCOLOR color) {
    scopeGpuOutput(d);
    HRESULT result = observeUiOutput(d, [&] { return originalFill(d, eyeSurface(target), rect, color); }, target, false);
    if ((activeEye >= 0 || scopeScratch) && FAILED(result))
        eyeInvalid = true;
    return result;
}
static HRESULT WINAPI setDepth(IDirect3DDevice9 *d, IDirect3DSurface9 *target) {
    scopeGpuOutput(d);
    if ((activeEye >= 0 || scopeScratch) && sameSurface(target, desktopDepth))
        target = uiFrame.slot >= 0 ? uiFrame.z[scopeScratch ? 0 : activeEye] : depth[scopeScratch ? 0 : activeEye];
    HRESULT result = originalSetDepth(d, target);
    if ((activeEye >= 0 || scopeScratch) && FAILED(result))
        eyeInvalid = true;
    return result;
}
static HRESULT WINAPI setRT(IDirect3DDevice9 *d, DWORD index, IDirect3DSurface9 *target) {
    scopeGpuOutput(d);
    if ((activeEye >= 0 || scopeScratch) && index > 0 && target)
        eyeInvalid = true;
    if ((activeEye >= 0 || scopeScratch) && index == 0 && sameSurface(target, desktopTarget))
        target = uiFrame.slot >= 0 ? uiFrame.target[scopeScratch ? 0 : activeEye] : eye[scopeScratch ? 0 : activeEye];
    HRESULT result = originalSetRT(d, index, target);
    if ((activeEye >= 0 || scopeScratch) && FAILED(result))
        eyeInvalid = true;
    if (index == 0 && target && activeEye < 0 && !scopeScratch && SUCCEEDED(result)) {
        static std::atomic<bool> first{false}, firstReady{false};
        const bool ready = hooksReady.load(std::memory_order_acquire);
        if (!first.load(std::memory_order_relaxed) && !first.exchange(true, std::memory_order_relaxed))
            logStartup("first ordinary RT0", d, ready);
        if (ready && !firstReady.load(std::memory_order_relaxed) &&
            !firstReady.exchange(true, std::memory_order_relaxed))
            logStartup("first ordinary RT0 after hooksReady", d, ready);
    }
    return result;
}
void deviceLost() {
    ++resourceGeneration;
    nativeUiFault();
    if (uiQuarantinedReadback) uiQuarantinedReadback->Release();
    uiQuarantinedReadback=nullptr;
    invalidateRenderer();
    for (int i = 0; i < 2; i++) {
        for (auto p : {eye[i], depth[i], readback[i]})
            if (p)
                p->Release();
        eye[i] = depth[i] = readback[i] = nullptr;
    }
    width = height = 0;
    if (channel.shared) {
        Lock l(channel);
        if (l) {
            channel.shared->rendererReady = 0;
            for (auto &s : channel.shared->slot) {
                if (s.state == SlotState::Requested || s.state == SlotState::Ready)
                    s.state = SlotState::Empty;
                else if (s.state == SlotState::Rendering)
                    s.cancelled = 1;
            }
        }
    }
}
static bool allocate() {
    if (!device)
        return false;
    IDirect3DSurface9 *bb = nullptr;
    if (FAILED(device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &bb)))
        return false;
    D3DSURFACE_DESC desc{};
    bb->GetDesc(&desc);
    bb->Release();
    if (desc.Width > MaxDimension || desc.Height > MaxDimension || !desc.Width || !desc.Height ||
        (desc.Format != D3DFMT_A8R8G8B8 && desc.Format != D3DFMT_X8R8G8B8))
        return false;
    IDirect3DSurface9 *nativeDepth = nullptr;
    D3DSURFACE_DESC depthDesc{};
    if (FAILED(device->GetDepthStencilSurface(&nativeDepth)) || !nativeDepth)
        return false;
    nativeDepth->GetDesc(&depthDesc);
    nativeDepth->Release();
    if (depthDesc.MultiSampleType != D3DMULTISAMPLE_NONE || desc.MultiSampleType != D3DMULTISAMPLE_NONE)
        return false;
    if (eye[0] && desc.Width == width && desc.Height == height && depthDesc.Format == depthFormat &&
        desc.Format == colorFormat) {
        Lock l(channel);
        if (l)
            channel.shared->rendererReady = 1;
        return true;
    }
    deviceLost();
    width = desc.Width;
    height = desc.Height;
    depthFormat = depthDesc.Format;
    colorFormat = desc.Format;
    hasStencil = depthFormat == D3DFMT_D24S8 || depthFormat == D3DFMT_D15S1 || depthFormat == D3DFMT_D24X4S4;
    for (int i = 0; i < 2; i++) {
        if (FAILED(device->CreateRenderTarget(width, height, colorFormat, D3DMULTISAMPLE_NONE, 0, FALSE,
                                              &eye[i], nullptr)) ||
            FAILED(device->CreateDepthStencilSurface(width, height, depthFormat, D3DMULTISAMPLE_NONE, 0, TRUE,
                                                     &depth[i], nullptr)) ||
            FAILED(device->CreateOffscreenPlainSurface(width, height, colorFormat, D3DPOOL_SYSTEMMEM,
                                                       &readback[i], nullptr))) {
            deviceLost();
            return false;
        }
    }
    Lock l(channel);
    if (l) {
        channel.shared->width = width;
        channel.shared->height = height;
        channel.shared->rendererReady = 1;
    }
    log("CPU stereo targets %ux%u; performance unverified", width, height);
    return true;
}
template <class T> static bool deviceHook(void *address, void *detour, T &original, const char *name) {
    static void *boundAddress = nullptr;
    if (original)
        return address == boundAddress;
    static std::atomic<bool> createOk{false}, createFailed{false}, enableOk{false}, enableFailed{false};
    const auto created = MH_CreateHook(address, detour, reinterpret_cast<void **>(&original));
    auto &createReported = created == MH_OK ? createOk : createFailed;
    if (!createReported.exchange(true, std::memory_order_relaxed))
        log("Startup routing hook=%s operation=create status=%d thread=%lu", name,
            static_cast<int>(created), GetCurrentThreadId());
    if (created == MH_OK) {
        const auto enabled = MH_EnableHook(address);
        auto &enableReported = enabled == MH_OK ? enableOk : enableFailed;
        if (!enableReported.exchange(true, std::memory_order_relaxed))
            log("Startup routing hook=%s operation=enable status=%d thread=%lu", name,
                static_cast<int>(enabled), GetCurrentThreadId());
        if (enabled == MH_OK) {
            boundAddress = address;
            return true;
        }
        MH_RemoveHook(address);
        original = nullptr;
    }
    return false;
}
// Component admission only: the native main-thread/owner/resource-epoch checks
// remain required. A Reset reuses creation ownership; deviceReady cannot invent
// that ownership. MULTITHREADED offers per-call safety, not a whole transaction.
bool nativePresentationIdleForBodyMove() noexcept {
    // uiFrame spans both eyes, their gap, scope previews and deferred native UI.
    // eyeIndex alone would incorrectly admit a body move between two eyes.
    return creationThread && GetCurrentThreadId()==creationThread &&
        device && device==creationDevice && eye[0] && eye[1] && !stopping.load() &&
        uiFrame.slot<0 && deferredSlot<0 && activeEye<0 && !scopeScratch && !scopeTransaction;
}
bool nativeUiDeviceCurrent(IDirect3DDevice9 *d) {
    if (!d || d != device || d != creationDevice || !creationThread ||
        GetCurrentThreadId() != creationThread) return false;
    D3DDEVICE_CREATION_PARAMETERS creation{};
    if (FAILED(d->GetCreationParameters(&creation)) ||
        (creation.BehaviorFlags & (D3DCREATE_PUREDEVICE | D3DCREATE_MULTITHREADED))) return false;
    D3DCAPS9 caps{};
    return SUCCEEDED(d->GetDeviceCaps(&caps)) && caps.MaxUserClipPlanes >= 6;
}
void deviceCreated(IDirect3DDevice9 *d) {
    creationDevice = d;
    creationThread = GetCurrentThreadId();
    uiHalted = false; // A newly created device is a new admitted resource lifetime.
    static std::atomic<bool> first{false};
    if (!first.exchange(true, std::memory_order_relaxed)) {
        startupCreation.device = d;
        startupCreation.thread = GetCurrentThreadId();
        startupCreationPublished.store(true, std::memory_order_release);
        logStartup("deviceCreated", d, hooksReady.load(std::memory_order_acquire));
    }
    deviceReady(d);
}
void deviceReady(IDirect3DDevice9 *d) {
    if (device && device != d) {
        deviceLost();
        device->Release();
    }
    if (device != d) {
        device = d;
        device->AddRef();
    }
    auto v = *reinterpret_cast<void ***>(d);
    uiRoutingReady = true;
    uiRoutingReady = deviceHook(v[37], reinterpret_cast<void *>(setRT), originalSetRT, "SetRenderTarget") && uiRoutingReady;
    uiRoutingReady = deviceHook(v[39], reinterpret_cast<void *>(setDepth), originalSetDepth, "Depth") && uiRoutingReady;
    uiRoutingReady = deviceHook(v[34], reinterpret_cast<void *>(stretch), originalStretch, "Canvas copy") && uiRoutingReady;
    uiRoutingReady = deviceHook(v[32], reinterpret_cast<void *>(readTarget), originalReadTarget, "Canvas read") && uiRoutingReady;
    uiRoutingReady = deviceHook(v[35], reinterpret_cast<void *>(fill), originalFill, "Canvas fill") && uiRoutingReady;
    uiRoutingReady = deviceHook(v[81], reinterpret_cast<void *>(draw), originalDraw, "Native UI draw") && uiRoutingReady;
    uiRoutingReady = deviceHook(v[82], reinterpret_cast<void *>(drawIndexed), originalDrawIndexed, "Native UI indexed draw") && uiRoutingReady;
    uiRoutingReady = deviceHook(v[43], reinterpret_cast<void *>(clear), originalClear, "Native UI clear observer") && uiRoutingReady;
    uiRoutingReady = deviceHook(v[83], reinterpret_cast<void *>(drawUp), originalDrawUp, "Native UI UP observer") && uiRoutingReady;
    uiRoutingReady = deviceHook(v[84], reinterpret_cast<void *>(drawIndexedUp), originalDrawIndexedUp, "Native UI indexed UP observer") && uiRoutingReady;
    uiRoutingReady = deviceHook(v[115], reinterpret_cast<void *>(drawRect), originalDrawRect, "Native UI rectangle observer") && uiRoutingReady;
    uiRoutingReady = deviceHook(v[116], reinterpret_cast<void *>(drawTri), originalDrawTri, "Native UI triangle observer") && uiRoutingReady;
    uiRoutingReady = deviceHook(v[30], reinterpret_cast<void *>(update), originalUpdate, "Native UI update observer") && uiRoutingReady;
    const bool ready = hooksReady.load();
    static std::atomic<bool> firstWaiting{false}, firstReady{false};
    auto &reported = ready ? firstReady : firstWaiting;
    if (!reported.exchange(true, std::memory_order_relaxed))
        logStartup("deviceReady", d, ready, uiRoutingReady);
    if (!ready)
        return;
    if (!channel.shared) {
        wchar_t token[64];
        swprintf(token, 64, L"%lu-%llu", GetCurrentProcessId(), GetTickCount64());
        if (!channel.open(token, true)) {
            static std::atomic<bool> failed{false};
            if (!failed.exchange(true, std::memory_order_relaxed))
                logStartup("IPC create success=0", d, ready);
            return;
        }
        static std::atomic<bool> opened{false};
        if (!opened.exchange(true, std::memory_order_relaxed))
            logStartup("IPC create success=1", d, ready, -1, 1);
        std::wstring root;
        if (!moduleDirectory(nullptr,root)) {
            log("Startup OpenXR host creation skipped: directory unavailable");
            return;
        }
        const auto executable=root+L"SS2VR\\ss2vr_host.exe";
        auto cmd=L"\""+executable+L"\" --channel "+token;
        STARTUPINFOW si{};
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi{};
        std::vector<wchar_t> arg(cmd.begin(), cmd.end());
        arg.push_back(0);
        // Explicit image path prevents whitespace-based executable search.
        // Null environment preserves Steam/Proton's prefix and runtime context.
        if (CreateProcessW(executable.c_str(), arg.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr,
                           root.c_str(), &si, &pi)) {
            log("Startup OpenXR host created=1 process=%lu thread=%lu", pi.dwProcessId, GetCurrentThreadId());
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
        } else {
            const DWORD error = GetLastError();
            log("Startup OpenXR host created=0 error=%lu thread=%lu", error, GetCurrentThreadId());
        }
    }
    allocate();
}
static uint64_t lastMenuCopy = 0, menuSequence = 0;
static MenuNavigation menuNavigation;
static WORD pendingKey = 0;
static uint64_t releaseKeyAt = 0;
static void keyEvent(WORD key, bool up) {
    INPUT event{};
    event.type = INPUT_KEYBOARD;
    event.ki.wScan = static_cast<WORD>(MapVirtualKeyW(key, MAPVK_VK_TO_VSC));
    event.ki.dwFlags = KEYEVENTF_SCANCODE | (up ? KEYEVENTF_KEYUP : 0);
    if (key == VK_UP || key == VK_DOWN || key == VK_LEFT || key == VK_RIGHT)
        event.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
    SendInput(1, &event, sizeof(INPUT));
}
static void keyTap(WORD key) {
    if (pendingKey)
        keyEvent(pendingKey, true);
    keyEvent(key, false);
    pendingKey = key;
    releaseKeyAt = GetTickCount64() + 75;
}
bool foregroundGame() {
    if (!device)
        return false;
    D3DDEVICE_CREATION_PARAMETERS params{};
    if (FAILED(device->GetCreationParameters(&params)) || !params.hFocusWindow)
        return false;
    HWND foreground = GetForegroundWindow();
    return foreground && GetAncestor(foreground, GA_ROOT) == GetAncestor(params.hFocusWindow, GA_ROOT);
}
void present(IDirect3DDevice9 *d) {
    static std::atomic<bool> first{false};
    if (!first.load(std::memory_order_relaxed) && !first.exchange(true, std::memory_order_relaxed))
        logStartup("first device Present", d, hooksReady.load(std::memory_order_acquire));
    nativeUiFault(); // A nested presentation cannot reuse the retained eye resources.
    if (pendingKey && GetTickCount64() >= releaseKeyAt) {
        keyEvent(pendingKey, true);
        pendingKey = 0;
    }
    retireDeferred();
    if (!device || !hooksReady || uiHalted)
        return;
    if (!channel.shared)
        deviceReady(device);
    if (!channel.shared || !allocate())
        return;
    Input input{};
    Ui ui{};
    {
        Lock l(channel);
        if (!l)
            return;
        input = channel.shared->latest;
        ui = channel.shared->ui;
    }
    uint64_t now = GetTickCount64();
    bool gameplay = ui.gameplay && now >= ui.tickMs && now - ui.tickMs < 250;
    bool menuVisible = menus::active() || !gameplay;
    bool valid = input.focused && input.headValid && now >= input.tickMs && now - input.tickMs < 200;
    bool foreground = foregroundGame();
    const MenuAction action =
        menuNavigation.sample(input, menuVisible, valid && foreground, menus::pointerAvailable());
    WORD key = 0;
    switch (action) {
    case MenuAction::Back:
        key = VK_ESCAPE;
        break;
    case MenuAction::Confirm:
        key = VK_RETURN;
        break;
    case MenuAction::Up:
        key = VK_UP;
        break;
    case MenuAction::Down:
        key = VK_DOWN;
        break;
    case MenuAction::Left:
        key = VK_LEFT;
        break;
    case MenuAction::Right:
        key = VK_RIGHT;
        break;
    default:
        break;
    }
    if (key)
        keyTap(key);
    if ((!valid || !foreground) && pendingKey) {
        keyEvent(pendingKey, true);
        pendingKey = 0;
    }
    {
        Lock l(channel);
        if (l)
            channel.shared->menu.visible = menuVisible ? 1 : 0;
    }
    if (!menuVisible || now - lastMenuCopy < 33)
        return;
    uint32_t menuGeneration = menus::captureGeneration();
    lastMenuCopy = now;
    IDirect3DSurface9 *backbuffer = nullptr;
    D3DLOCKED_RECT locked{};
    if (FAILED(device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer)))
        return;
    bool ok = SUCCEEDED(device->StretchRect(backbuffer, nullptr, eye[0], nullptr, D3DTEXF_NONE));
    backbuffer->Release();
    if (ok)
        ok = SUCCEEDED(device->GetRenderTargetData(eye[0], readback[0]));
    if (ok)
        ok = SUCCEEDED(readback[0]->LockRect(&locked, nullptr, D3DLOCK_READONLY));
    if (!ok)
        return;
    if (locked.Pitch >= int(width * 4)) {
        Lock l(channel);
        if (l) {
            auto &m = channel.shared->menu;
            for (uint32_t y = 0; y < height; y++)
                memcpy(m.pixels + size_t(y) * width * 4,
                       static_cast<uint8_t *>(locked.pBits) + size_t(y) * locked.Pitch, width * 4);
            for (size_t pixel = 3; pixel < size_t(width) * height * 4; pixel += 4)
                m.pixels[pixel] = 255;
            m.width = width;
            m.height = height;
            m.tickMs = now;
            m.sequence = ++menuSequence;
            m.interactionGeneration = menuGeneration;
            m.visible = 1;
        }
    }
    readback[0]->UnlockRect();
}
static bool eyeTargetCurrent(int index) {
    D3DVIEWPORT9 viewport{};
    const bool matches=index>=0 && index<2 && activeEye==index && uiFrame.device &&
        SUCCEEDED(uiFrame.device->GetRenderTarget(0,&uiFrame.probeColor)) &&
        SUCCEEDED(uiFrame.device->GetDepthStencilSurface(&uiFrame.probeDepth)) &&
        SUCCEEDED(uiFrame.device->GetViewport(&viewport)) &&
        sameSurface(uiFrame.probeColor,uiFrame.target[index]) && sameSurface(uiFrame.probeDepth,uiFrame.z[index]) &&
        viewport.X==0 && viewport.Y==0 && viewport.Width==uiFrame.request.width && viewport.Height==uiFrame.request.height;
    if(uiFrame.probeColor) uiFrame.probeColor->Release();
    if(uiFrame.probeDepth) uiFrame.probeDepth->Release();
    uiFrame.probeColor=uiFrame.probeDepth=nullptr;
    return matches;
}
static bool restoreUiFrame() noexcept {
    if (!uiFrame.restore) return true;
    bool ok = true;
    uiBypass = true;
    auto *d=uiFrame.device;
    ok = SUCCEEDED(originalSetRT(d,0,uiFrame.color)) && ok;
    ok = SUCCEEDED(originalSetDepth(d,uiFrame.ds)) && ok;
    ok = SUCCEEDED(d->SetViewport(&uiFrame.viewport)) && ok;
    ok = SUCCEEDED(d->SetScissorRect(&uiFrame.scissor)) && ok;
    uiBypass = false;
    uiFrame.restore = false;
    if (!ok) { uiFrame.fault = true; uiHalted = true; }
    return ok;
}
static void releaseUiFrame() noexcept {
    retireNativeUiLock(uiFrame.locked,
        [](IDirect3DSurface9 *surface) {
            if (FAILED(surface->UnlockRect())) return false;
            surface->Release(); // Explicit lock reference, separate from frame/global refs.
            return true;
        },[] { uiFrame.fault=true; uiHalted=true; });
    if (uiFrame.locked) {
        // Halt already prevents world AND menu readback reuse. Transfer the lock
        // reference for retirement at deviceLost, never assume it was unlocked.
        uiQuarantinedReadback=uiFrame.locked;
        uiFrame.locked=nullptr;
    }
    if (uiFrame.probeColor) uiFrame.probeColor->Release();
    if (uiFrame.probeDepth) uiFrame.probeDepth->Release();
    uiFrame.probeColor=uiFrame.probeDepth=nullptr;
    if (uiFrame.backbuffer) uiFrame.backbuffer->Release();
    if (uiFrame.extra) uiFrame.extra->Release();
    if (uiFrame.chain) uiFrame.chain->Release();
    uiFrame.chain=nullptr;
    uiFrame.backbuffer=uiFrame.extra=nullptr;
    if (uiFrame.color) uiFrame.color->Release();
    if (uiFrame.ds) uiFrame.ds->Release();
    for (unsigned i=0;i<2;++i) {
        if (uiFrame.target[i]) uiFrame.target[i]->Release();
        if (uiFrame.z[i]) uiFrame.z[i]->Release();
        if (uiFrame.read[i]) uiFrame.read[i]->Release();
        uiFrame.target[i]=uiFrame.z[i]=uiFrame.read[i]=nullptr;
        uiFrame.pixels[i].clear(); // Retain capacity; cleanup never allocates.
    }
    for (unsigned i=0;i<2;++i) {
        auto *texture = uiFrame.scopeImage[i]; uiFrame.scopeImage[i] = nullptr;
        if (texture) texture->Release();
        uiFrame.scopeReady[i] = false; uiFrame.scopeView[i] = {};
    }
    auto *shader = uiFrame.scopeShader; uiFrame.scopeShader = nullptr;
    if (shader) shader->Release();
    if (uiFrame.device) uiFrame.device->Release();
    uiFrame.device=nullptr;
    uiFrame.color=uiFrame.ds=nullptr;
    uiFrame.player=nullptr;
    uiFrame.slot=-1;
    uiFrame.ui=uiFrame.fault=uiFrame.world=uiFrame.restore=false;
    uiFrame.overlay=uiFrame.overlaySeen=uiFrame.complete=uiFrame.fade=false;
}
void nativeUiEndOwner(bool aborted) noexcept {
    if (uiFrame.slot<0) return;
    if (aborted) { uiFrame.fault=true; uiHalted=true; }
    if (scopeScratch) scopeSourcesHalted = true; // Any early owner retirement leaves native cleanup unproved.
    scopeScratch = false;
    scopeGpuTransactionEnd();
    cleanupUiDraw(aborted);
    activeEye=-1;
    eyeInvalid=false;
    endEye();
    desktopTarget=desktopDepth=nullptr;
    restoreUiFrame();
    deferredSlot=uiFrame.slot;
    deferredSequence=uiFrame.request.sequence;
    releaseUiFrame();
    retireDeferred();
}
bool nativeUiBeginOverlay(void *player,bool admitted) {
    if (uiFrame.slot<0 || !uiFrame.ui) return false;
    if (!admitted || player!=uiFrame.player || !uiFrame.world || uiFrame.overlaySeen || !uiCurrent()) {
        nativeUiFault(); return false;
    }
    uiFrame.overlaySeen=true;
    uiFrame.overlay=true;
    return true;
}
void nativeUiEndOverlay(bool completed) noexcept {
    if (uiFrame.slot<0 || !uiFrame.ui) return;
    if (!completed || !uiFrame.overlay || uiFrame.fade || uiDraw.busy) nativeUiFault();
    uiFrame.complete=completed && uiFrame.overlay && !uiFrame.fault;
    uiFrame.overlay=false;
    uiFrame.fade=false;
}
bool nativeUiBeginFade() {
    if (!uiFrame.overlay || !uiFrame.ui) return false;
    if (uiFrame.fade || !uiCurrent()) { nativeUiFault(); return false; }
    uiFrame.fade=true;
    return true;
}
void nativeUiEndFade(bool completed) noexcept {
    if (!completed) nativeUiFault();
    uiFrame.fade=false;
}
static bool readUiEye(unsigned i,bool dim,float visibility) {
    auto *d=uiFrame.device;
    auto *read=uiFrame.read[i];
    if (FAILED(d->GetRenderTargetData(uiFrame.target[i],read))) return false;
    D3DLOCKED_RECT locked{};
    if (FAILED(read->LockRect(&locked,nullptr,dim ? 0 : D3DLOCK_READONLY))) return false;
    uiFrame.locked=read;
    read->AddRef();
    bool ok=locked.pBits && locked.Pitch>=int(uiFrame.request.width*4);
    if (ok) {
        const size_t rowBytes=size_t(uiFrame.request.width)*4;
        if (!dim) uiFrame.pixels[i].resize(rowBytes*uiFrame.request.height);
        const auto dimming=makeOpaqueDimming(visibility);
        for (uint32_t row=0;row<uiFrame.request.height;++row) {
            auto *source=static_cast<uint8_t *>(locked.pBits)+size_t(row)*locked.Pitch;
            if (dim) dimRgbPixels(source,uiFrame.request.width,dimming);
            else memcpy(uiFrame.pixels[i].data()+size_t(row)*rowBytes,source,rowBytes);
        }
    }
    ok = retireNativeUiLock(uiFrame.locked,
        [](IDirect3DSurface9 *surface) {
            if (FAILED(surface->UnlockRect())) return false;
            surface->Release();
            return true;
        },[] { uiFrame.fault=true; uiHalted=true; }) && ok;
    if (dim && ok) ok=SUCCEEDED(originalUpdate(d,read,nullptr,uiFrame.target[i],nullptr));
    return ok;
}
static bool publishUiFrame(uint32_t presentation) {
    if (!channel.shared || !uiFrame.world || uiFrame.fault ||
        uiFrame.generation!=resourceGeneration.load()) return false;
    // No native rendering while IPC is held. commitStereo retains the existing
    // native snapshot/remote binding admission through the Ready transition.
    if (!channel.lock(2)) return false;
    bool accepted=false;
    withNativeFinally([&] {
        auto &slot=channel.shared->slot[uiFrame.slot];
        if (!slot.cancelled && slot.state==SlotState::Rendering && sameRequest(slot.request,uiFrame.request)) {
            for (unsigned i=0;i<2;++i) memcpy(slot.pixels[i],uiFrame.pixels[i].data(),uiFrame.pixels[i].size());
            slot.presentation=presentation;
            slot.presentationReserved=0;
            accepted=commitStereo(uiFrame.player,uiFrame.request,slot);
            if (accepted) SetEvent(channel.ready);
            else slot.state=SlotState::Empty;
        }
    },[&](bool aborted) noexcept {
        if (aborted) { uiFrame.fault=true; uiHalted=true; }
        channel.unlock();
    });
    return accepted;
}
void nativeUiFinishOwner() {
    if (uiFrame.slot<0 || !uiFrame.ui) return;
    if (!uiFrame.complete || uiFrame.overlay || uiFrame.fade || !uiCurrent()) {
        nativeUiFault(); return;
    }
    // UI and original full-field fades have now finished in BOTH owned eye RTs.
    bool ok=readUiEye(0,false,1) && readUiEye(1,false,1);
    if (ok) {
        const auto opaque=makeOpaqueDimming(1);
        for (auto &image:uiFrame.pixels) dimOpaquePixels(image.data(),image.size()/4,opaque);
    }
    if (!ok || !uiCurrent() || !publishUiFrame(NativeUiComplete)) nativeUiFault();
}
struct ScopeCaptureFrame {
    unsigned hand = 2;
    uint64_t generation = 0;
    bool active = false, copied = false;
    ScopeCaptureOnce once;
};
// Never recycled after an abnormal source invocation. A stale native callback
// first checks the process-lifetime halt and cannot reach released resources.
static thread_local ScopeCaptureFrame scopeCaptureFrame;
static bool scopeCaptureCurrent(void *opaque) noexcept {
    const auto *capture = static_cast<ScopeCaptureFrame *>(opaque);
    return !scopeSourcesHalted && capture == &scopeCaptureFrame && capture->active && !capture->once.rejected &&
        capture->hand < 2 && scopeScratch && activeEye == -1 && uiFrame.slot >= 0 &&
        !uiFrame.fault && !eyeInvalid && !uiHalted && !stopping &&
        capture->generation == graphicsResourceGeneration() &&
        uiFrame.generation == capture->generation;
}
static HRESULT copyScopePixels(IDirect3DDevice9 *d,IDirect3DSurface9 *from,IDirect3DSurface9 *to) noexcept {
    return originalStretch(d,from,nullptr,to,nullptr,D3DTEXF_NONE);
}
static bool scopeCaptureTransferCurrent(void *opaque) noexcept {
    return scopeCaptureCurrent(opaque) && scopeSourceExecuting() && scopeGpuTransactionCurrent(uiFrame.device);
}
static __attribute__((force_align_arg_pointer)) void __cdecl captureNativeScope(void *opaque) noexcept {
    if (!scopeCaptureCurrent(opaque) || !scopeSourceExecuting()) return;
    if (!scopeCaptureFrame.once.enter()) { scopeCaptureFrame.copied = false; return; }
    if (!scopeGpuTransactionBegin(uiFrame.device)) return;
    IDirect3DDevice9 *retainedDevice = nullptr;
    IDirect3DSurface9 *retainedSource = nullptr;
    IDirect3DTexture9 *retainedDestination = nullptr;
    const UINT width = uiFrame.request.width, height = uiFrame.request.height;
    // Independent invocation references survive even a reentrant early frame
    // retirement. All outputs live above the native finally which releases them.
    withNativeFinally([&] {
        retainedDevice = uiFrame.device; retainedDevice->AddRef();
        if (!scopeCaptureTransferCurrent(opaque)) return;
        retainedSource = uiFrame.target[0]; retainedSource->AddRef();
        if (!scopeCaptureTransferCurrent(opaque)) return;
        retainedDestination = uiFrame.scopeImage[scopeCaptureFrame.hand]; retainedDestination->AddRef();
        if (!scopeCaptureTransferCurrent(opaque)) return;
        scopeCaptureFrame.copied = captureScopeSource(retainedDevice,retainedSource,
            retainedDestination,width,height,copyScopePixels,scopeCaptureTransferCurrent,opaque);
    },[&](bool aborted) noexcept {
        if (aborted) { scopeSourcesHalted = true; scopeCaptureFrame.copied = false; }
        if (retainedDestination) { auto *p=retainedDestination; retainedDestination=nullptr; p->Release(); }
        if (retainedSource) { auto *p=retainedSource; retainedSource=nullptr; p->Release(); }
        if (retainedDevice) { auto *p=retainedDevice; retainedDevice=nullptr; p->Release(); }
        if (!scopeCaptureTransferCurrent(opaque)) scopeCaptureFrame.copied = false;
        scopeGpuTransactionEnd();
    });
}
static bool prepareScopeScratch() {
    auto *d = uiFrame.device;
    const D3DVIEWPORT9 vp{0,0,uiFrame.request.width,uiFrame.request.height,0,1};
    return SUCCEEDED(originalSetRT(d,0,uiFrame.target[0])) && SUCCEEDED(originalSetDepth(d,uiFrame.z[0])) &&
        SUCCEEDED(d->SetViewport(&vp)) && SUCCEEDED(d->SetScissorRect(&uiFrame.scissor)) &&
        SUCCEEDED(d->Clear(0,nullptr,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|
            (hasStencil ? D3DCLEAR_STENCIL : 0),0xff000000,1,0));
}
static bool renderScopeSources(void *puppet,void(__thiscall *original)(void *),const Request &request) {
    if (scopeSourcesHalted || !scopePreviewNeeded(puppet,request)) return true;
    auto *d = uiFrame.device;
    D3DCAPS9 caps{};
    constexpr DWORD required = D3DPS20CAPS_ARBITRARYSWIZZLE | D3DPS20CAPS_GRADIENTINSTRUCTIONS |
        D3DPS20CAPS_PREDICATION | D3DPS20CAPS_NODEPENDENTREADLIMIT | D3DPS20CAPS_NOTEXINSTRUCTIONLIMIT;
    if (FAILED(d->GetDeviceCaps(&caps)) || caps.PixelShaderVersion < D3DPS_VERSION(2,0) ||
        !(caps.TextureFilterCaps & D3DPTFILTERCAPS_MINFLINEAR) ||
        !(caps.TextureFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR) ||
        caps.PS20Caps.NumTemps < 22 || caps.PS20Caps.NumInstructionSlots < 512 ||
        (caps.PS20Caps.Caps & required) != required ||
        (colorFormat != D3DFMT_A8R8G8B8 && colorFormat != D3DFMT_X8R8G8B8)) return true;
    if (FAILED(d->CreatePixelShader(reinterpret_cast<const DWORD *>(ScopeImageOpaqueProgram),&uiFrame.scopeShader)) ||
        !uiFrame.scopeShader) return true;
    activeEye = 0; eyeInvalid = false; renderRequest = request;
    beginScopePreview(puppet,request);
    bool ok = prepareScopeScratch();
    if (ok) original(puppet);
    ok = ok && !eyeInvalid && eyeTargetCurrent(0);
    bool candidate[2]{};
    if (ok) for (unsigned hand=0;hand<2;++hand) candidate[hand] = copyScopeSourceView(hand,uiFrame.scopeView[hand]);
    endEye(); activeEye = -1;
    for (unsigned hand=0;hand<2 && ok;++hand) {
        if (!candidate[hand]) continue;
        if (FAILED(d->CreateTexture(request.width,request.height,1,D3DUSAGE_RENDERTARGET,colorFormat,
                                   D3DPOOL_DEFAULT,&uiFrame.scopeImage[hand],nullptr)) || !uiFrame.scopeImage[hand]) continue;
        scopeCaptureFrame = {hand,uiFrame.generation,true,false,{}};
        scopeScratch = true; eyeInvalid = false;
        const bool begun = beginScopeSource(puppet,request,uiFrame.scopeView[hand],captureNativeScope,
                                            scopeCaptureCurrent,&scopeCaptureFrame);
        if (begun) {
            ok = prepareScopeScratch();
            if (ok) original(puppet);
            const bool finished = endScopeSource(ok && !eyeInvalid);
            uiFrame.scopeReady[hand] = scopeCaptureFrame.once.complete(finished,scopeCaptureFrame.copied) &&
                scopeCaptureCurrent(&scopeCaptureFrame) && nativeUiFrameCurrent(puppet,request);
        }
        scopeCaptureFrame.active = false;
        scopeScratch = false;
        ok = ok && !eyeInvalid && !uiFrame.fault && !scopeSourcesHalted;
    }
    return ok;
}
void stereo(void *puppet,void(__thiscall *original)(void *),EyePostRender postRender) {
    if (uiFrame.slot>=0) {
        nativeUiFault();
        withNativeFinally([&]{ original(puppet); },[&](bool aborted) noexcept { if(aborted) uiHalted=true; });
        return;
    }
    retireDeferred();
    if (deferredSlot>=0 || activeEye>=0 || stopping || uiHalted || !device || !channel.shared || !originalSetRT ||
        !originalSetDepth || !originalStretch || !originalReadTarget || !originalFill || !allocate()) {
        withNativeFinally([&]{ original(puppet); },[&](bool aborted) noexcept { if(aborted) uiHalted=true; });
        return;
    }
    Request request{};
    int chosen=-1;
    {
        Lock l(channel);
        if(l) for(int i=0;i<2;++i) {
            auto &slot=channel.shared->slot[i];
            const auto now=GetTickCount64();
            if(slot.state==SlotState::Requested && !slot.cancelled && slot.request.width==width &&
                slot.request.height==height && slot.request.input.focused && slot.request.input.headValid &&
                now>=slot.request.input.tickMs && now-slot.request.input.tickMs<200 &&
                headFramePrepared(puppet,slot.request)) {
                chosen=i; request=slot.request; slot.state=SlotState::Rendering; break;
            }
        }
    }
    if(chosen<0) {
        withNativeFinally([&]{ original(puppet); },[&](bool aborted) noexcept { if(aborted) uiHalted=true; });
        return;
    }
    uiFrame.slot=chosen;
    uiFrame.request=request;
    uiFrame.player=puppet;
    uiFrame.generation=resourceGeneration.load();
    uiFrame.device=device;
    device->AddRef();
    for(unsigned i=0;i<2;++i) {
        uiFrame.target[i]=eye[i]; eye[i]->AddRef();
        uiFrame.z[i]=depth[i]; depth[i]->AddRef();
        uiFrame.read[i]=readback[i]; readback[i]->AddRef();
    }
    bool retained=false;
    withNativeFinally([&] {
        auto *d=uiFrame.device;
        bool ok=SUCCEEDED(d->GetRenderTarget(0,&uiFrame.color)) &&
            SUCCEEDED(d->GetDepthStencilSurface(&uiFrame.ds)) &&
            SUCCEEDED(d->GetViewport(&uiFrame.viewport)) && SUCCEEDED(d->GetScissorRect(&uiFrame.scissor));
        // Backbuffer proof uses the retained known original desktop target rather
        // than retaining any native intermediate target after its invocation.
        ok=SUCCEEDED(d->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&uiFrame.backbuffer)) && ok;
        ok=ok && sameSurface(uiFrame.color,uiFrame.backbuffer) && uiFrame.ds;
        if(uiFrame.backbuffer) uiFrame.backbuffer->Release();
        uiFrame.backbuffer=nullptr;
        const auto &vp=uiFrame.viewport;
        ok=ok && vp.X==0 && vp.Y==0 && vp.Width==width && vp.Height==height;
        D3DSURFACE_DESC cd{},zd{};
        if(uiFrame.color) ok=SUCCEEDED(uiFrame.color->GetDesc(&cd)) && ok;
        if(uiFrame.ds) ok=SUCCEEDED(uiFrame.ds->GetDesc(&zd)) && ok;
        ok=ok && cd.Format==colorFormat && cd.MultiSampleType==D3DMULTISAMPLE_NONE &&
            zd.Format==depthFormat && zd.MultiSampleType==D3DMULTISAMPLE_NONE;
        ok=singleUiTarget(d,uiFrame.extra) && ok;
        uiFrame.restore=ok;
        D3DPRESENT_PARAMETERS presentation{};
        const bool directWritesExcluded=ok &&
            SUCCEEDED(uiFrame.color->GetContainer(__uuidof(IDirect3DSwapChain9),reinterpret_cast<void **>(&uiFrame.chain))) &&
            uiFrame.chain && SUCCEEDED(uiFrame.chain->GetPresentParameters(&presentation)) &&
            !(presentation.Flags&D3DPRESENTFLAG_LOCKABLE_BACKBUFFER);
        uiFrame.ui=ok && postRender && request.uiRequested==1 && !uiHalted && uiRoutingReady &&
            originalDraw && originalDrawIndexed && originalClear && originalDrawUp && originalDrawIndexedUp &&
            originalDrawRect && originalDrawTri && originalUpdate && directWritesExcluded &&
            nativeUiDeviceCurrent(d) && nativeUiOwnerCurrent(puppet);
        if(uiFrame.ui) for(unsigned i=0;i<2;++i)
            uiFrame.panel[i]=compose(inverse(request.eye[i]),request.uiPanel);
        desktopTarget=uiFrame.color;
        desktopDepth=uiFrame.ds;
        if(ok) ok=beginStereo(puppet,request);
        const float visibility=ok ? frozenWorldVisibility() : 1;
        if (ok) ok = renderScopeSources(puppet,original,request);
        for(int i=0;i<2 && ok;++i) {
            activeEye=i; eyeInvalid=false; renderRequest=request;
            beginEye(puppet,request,i);
            ok=SUCCEEDED(originalSetRT(d,0,uiFrame.target[i])) && SUCCEEDED(originalSetDepth(d,uiFrame.z[i]));
            const D3DVIEWPORT9 ev{0,0,width,height,0,1};
            ok=SUCCEEDED(d->SetViewport(&ev)) && ok;
            ok=SUCCEEDED(d->SetScissorRect(&uiFrame.scissor)) && ok;
            ok=ok && SUCCEEDED(d->Clear(0,nullptr,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER|
                (hasStencil ? D3DCLEAR_STENCIL : 0),0xff000000,1,0));
            if(ok) original(puppet);
            if(uiFrame.ui && ok) {
                ok=copyExecutedUiProjection(puppet,request,i,uiFrame.projection[i]);
                Matrix44 identity{};
                identity.m[0]=identity.m[5]=identity.m[10]=identity.m[15]=1;
                NativeUiProjection admission;
                ok=ok && nativeUiProjection(identity,uiFrame.projection[i],uiFrame.panel[i],
                    request.uiWidth,request.uiHeight,request.width,request.height,
                    {0,0,request.width,request.height,0,1},false,{},admission) && !admission.empty;
            }
            ok=finishEyeRender(ok && !eyeInvalid,
                [&]{ return !eyeInvalid && eyeTargetCurrent(i); },
                [&]{ return !postRender || postRender(puppet,request,i); });
            endEye(); activeEye=-1;
            if(ok && (!uiFrame.ui || visibility!=1)) ok=readUiEye(i,uiFrame.ui,visibility);
        }
        activeEye=-1; endEye(); desktopTarget=desktopDepth=nullptr;
        ok=restoreUiFrame() && ok;
        // Native desktop rebuild remains exactly once; never replay brain/overlay.
        original(puppet);
        uiFrame.world=ok && !uiFrame.fault && uiFrame.generation==resourceGeneration.load();
        if(uiFrame.ui) {
            if(!uiCurrent()) nativeUiFault();
            retained=true; // Even a partial UI pair remains owned until enclosing cleanup.
        } else if(uiFrame.world) {
            const auto dimming=makeOpaqueDimming(visibility);
            for(auto &image:uiFrame.pixels) dimOpaquePixels(image.data(),image.size()/4,dimming);
            publishUiFrame(0);
        }
    },[&](bool aborted) noexcept {
        if(aborted || !retained) nativeUiEndOwner(aborted);
    });
}
void deviceResetSucceeded(IDirect3DDevice9 *d) {
    if (d==creationDevice && GetCurrentThreadId()==creationThread) uiHalted=false;
    deviceReady(d);
}
void shutdown() {
    stopping = true;
    if (channel.shared) {
        Lock l(channel);
        if (l)
            channel.shared->shutdown = 1;
    }
    deviceLost();
    if (device)
        device->Release();
    device = nullptr;
}
} // namespace ss2vr::game
