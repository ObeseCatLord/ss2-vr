#define WIN32_LEAN_AND_MEAN
#include "common/winproc.hpp"
#include "common/winpath.hpp"
#include "game.hpp"
#include <MinHook.h>
#include <atomic>
#include <d3d9.h>
#include <vector>
#include <windows.h>
using Create9 = IDirect3D9 *(WINAPI *)(UINT);
using Create9Ex = HRESULT(WINAPI *)(UINT, IDirect3D9Ex **);
using CreateDevice = HRESULT(WINAPI *)(IDirect3D9 *, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS *,
                                       IDirect3DDevice9 **);
using Reset = HRESULT(WINAPI *)(IDirect3DDevice9 *, D3DPRESENT_PARAMETERS *);
static HMODULE realDll = nullptr;
static Create9 create9 = nullptr;
static Create9Ex create9ex = nullptr;
static CreateDevice originalCreate = nullptr;
static Reset originalReset = nullptr;
using Present = HRESULT(WINAPI *)(IDirect3DDevice9 *, const RECT *, const RECT *, HWND, const RGNDATA *);
static Present originalPresent = nullptr;
// Diagnostic latches never gate hook installation or forwarding. Keep both
// first success and first failure visible if an existing caller retries.
static void logHook(const char *name, const char *operation, MH_STATUS status,
                    std::atomic<bool> &succeeded, std::atomic<bool> &failed) {
    auto &reported = status == MH_OK ? succeeded : failed;
    if (!reported.exchange(true, std::memory_order_relaxed))
        ss2vr::game::log("Startup hook=%s operation=%s status=%d thread=%lu", name, operation,
                        static_cast<int>(status), GetCurrentThreadId());
}
template <class T> static void proxyHook(void *address, void *detour, T &original, const char *name) {
    static std::atomic<bool> createOk{false}, createFailed{false}, enableOk{false}, enableFailed{false};
    const auto created = MH_CreateHook(address, detour, reinterpret_cast<void **>(&original));
    logHook(name, "create", created, createOk, createFailed);
    if (created == MH_OK) {
        const auto enabled = MH_EnableHook(address);
        logHook(name, "enable", enabled, enableOk, enableFailed);
    }
}
static HRESULT WINAPI present(IDirect3DDevice9 *d, const RECT *src, const RECT *dst, HWND window,
                              const RGNDATA *region) {
    ss2vr::game::present(d);
    return originalPresent(d, src, dst, window, region);
}
static INIT_ONCE initialize = INIT_ONCE_STATIC_INIT;
static bool vrEnabled = false;
static DWORD WINAPI hookThread(void *) {
    for (int i = 0; i < 600; i++) {
        if (GetModuleHandleW(L"Sam2Game.dll") && GetModuleHandleW(L"Engine.dll") &&
            GetModuleHandleW(L"Core.dll")) {
            ss2vr::game::attach();
            return 0;
        }
        Sleep(50);
    }
    ss2vr::game::log("Native modules unavailable; desktop retained");
    return 0;
}
static BOOL CALLBACK init(PINIT_ONCE, PVOID, PVOID *) {
    // Detours outlive graphics backend reloads; their owning DLL must stay mapped.
    HMODULE self = nullptr;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN,
                            reinterpret_cast<LPCWSTR>(&init), &self))
        return FALSE;
    std::wstring directory;
    if (!ss2vr::systemDirectory(directory)) return FALSE;
    try {
        const auto path=directory+L"\\d3d9.dll";
        realDll=LoadLibraryExW(path.c_str(),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
    } catch (...) { return FALSE; }
    if (!realDll) return FALSE;
    if (realDll==self) {
        FreeLibrary(realDll); realDll=nullptr;
        return FALSE; // Never recurse into this proxy after a loader/override alias.
    }
    create9 = ss2vr::loadProc<Create9>(realDll, "Direct3DCreate9");
    create9ex = ss2vr::loadProc<Create9Ex>(realDll, "Direct3DCreate9Ex");
    if (!create9) {
        FreeLibrary(realDll); realDll=nullptr; create9ex=nullptr;
        return FALSE;
    }
    const auto hookStatus = MH_Initialize();
    vrEnabled = hookStatus == MH_OK;
    ss2vr::game::log("Startup MinHook initialize status=%d", static_cast<int>(hookStatus));
    if (vrEnabled) {
        HANDLE thread = CreateThread(nullptr, 0, hookThread, nullptr, 0, nullptr);
        const DWORD error = thread ? ERROR_SUCCESS : GetLastError();
        ss2vr::game::log("Startup hook worker created=%d error=%lu", thread != nullptr, error);
        if (thread)
            CloseHandle(thread);
    }
    return TRUE;
}
static HRESULT WINAPI reset(IDirect3DDevice9 *d, D3DPRESENT_PARAMETERS *p) {
    ss2vr::game::deviceLost();
    HRESULT hr = originalReset(d, p);
    if (SUCCEEDED(hr))
        ss2vr::game::deviceResetSucceeded(d);
    return hr;
}
static HRESULT WINAPI createDevice(IDirect3D9 *api, UINT adapter, D3DDEVTYPE type, HWND window, DWORD flags,
                                   D3DPRESENT_PARAMETERS *p, IDirect3DDevice9 **out) {
    HRESULT hr = originalCreate(api, adapter, type, window, flags, p, out);
    static std::atomic<bool> createOk{false}, createFailed{false};
    auto &reported = SUCCEEDED(hr) ? createOk : createFailed;
    if (!reported.exchange(true, std::memory_order_relaxed))
        ss2vr::game::log("Startup CreateDevice hr=%08lx device=%p type=%u flags=%08lx thread=%lu",
                        static_cast<unsigned long>(hr),
                        SUCCEEDED(hr) && out ? static_cast<void *>(*out) : nullptr,
                        static_cast<unsigned>(type), flags, GetCurrentThreadId());
    if (SUCCEEDED(hr) && out && *out && type == D3DDEVTYPE_HAL) {
        auto table = *reinterpret_cast<void ***>(*out);
        if (!originalReset)
            proxyHook(table[16], reinterpret_cast<void *>(reset), originalReset, "Reset");
        if (!originalPresent)
            proxyHook(table[17], reinterpret_cast<void *>(present), originalPresent, "Present");
        ss2vr::game::deviceCreated(*out);
    }
    return hr;
}
extern "C" __declspec(dllexport) IDirect3D9 *WINAPI Direct3DCreate9(UINT version) {
    const BOOL initialized = InitOnceExecuteOnce(&initialize, init, nullptr, nullptr);
    IDirect3D9 *api = create9 ? create9(version) : nullptr;
    static std::atomic<bool> apiOk{false}, apiFailed{false};
    auto &reported = api ? apiOk : apiFailed;
    if (!reported.exchange(true, std::memory_order_relaxed))
        ss2vr::game::log("Startup Direct3DCreate9 initialized=%d api=%p hooksEnabled=%d thread=%lu",
                        static_cast<int>(initialized), static_cast<void *>(api), vrEnabled, GetCurrentThreadId());
    if (api && vrEnabled && !originalCreate) {
        auto v = *reinterpret_cast<void ***>(api);
        proxyHook(v[16], reinterpret_cast<void *>(createDevice), originalCreate, "CreateDevice");
    }
    return api;
}
extern "C" __declspec(dllexport) HRESULT WINAPI Direct3DCreate9Ex(UINT version, IDirect3D9Ex **out) {
    InitOnceExecuteOnce(&initialize, init, nullptr, nullptr);
    return create9ex ? create9ex(version, out) : D3DERR_NOTAVAILABLE;
}
#define FORWARD(name, ret, args, callargs, fallback)                                                         \
    extern "C" __declspec(dllexport) ret WINAPI name args {                                                  \
        InitOnceExecuteOnce(&initialize, init, nullptr, nullptr);                                            \
        auto f = realDll ? ss2vr::loadProc<ret(WINAPI *) args>(realDll, #name) : nullptr;                    \
        if (f)                                                                                               \
            return f callargs;                                                                               \
        return fallback;                                                                                     \
    }
FORWARD(D3DPERF_BeginEvent, int, (D3DCOLOR c, LPCWSTR s), (c, s), -1)
FORWARD(D3DPERF_EndEvent, int, (), (), -1)
FORWARD(D3DPERF_GetStatus, DWORD, (), (), 0)
FORWARD(D3DPERF_QueryRepeatFrame, BOOL, (), (), FALSE)
extern "C" __declspec(dllexport) void WINAPI D3DPERF_SetMarker(D3DCOLOR c, LPCWSTR s) {
    InitOnceExecuteOnce(&initialize, init, nullptr, nullptr);
    auto f = ss2vr::loadProc<void(WINAPI *)(D3DCOLOR, LPCWSTR)>(realDll, "D3DPERF_SetMarker");
    if (f)
        f(c, s);
}
extern "C" __declspec(dllexport) void WINAPI D3DPERF_SetRegion(D3DCOLOR c, LPCWSTR s) {
    InitOnceExecuteOnce(&initialize, init, nullptr, nullptr);
    auto f = ss2vr::loadProc<void(WINAPI *)(D3DCOLOR, LPCWSTR)>(realDll, "D3DPERF_SetRegion");
    if (f)
        f(c, s);
}
extern "C" __declspec(dllexport) void WINAPI D3DPERF_SetOptions(DWORD o) {
    InitOnceExecuteOnce(&initialize, init, nullptr, nullptr);
    auto f = ss2vr::loadProc<void(WINAPI *)(DWORD)>(realDll, "D3DPERF_SetOptions");
    if (f)
        f(o);
}
BOOL WINAPI DllMain(HINSTANCE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH)
        DisableThreadLibraryCalls(module);
    return TRUE;
}
