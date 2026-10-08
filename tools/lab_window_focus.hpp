// Private diagnostic of borrowed Win32 windows. No game/native-state writes.
#pragma once
#include <windows.h>
#include <cwchar>
#include <cstdint>

namespace ss2vr::lab {
struct WindowObservation {
    HWND window{};
    DWORD owner{}, thread{};
    bool titleMatches{}, rootMatches{}, visible{}, iconic{};
};
inline WindowObservation observeWindow(HWND window, const wchar_t *title) {
    WindowObservation out{};out.window=window;
    if (!window) return out;
    out.thread=GetWindowThreadProcessId(window,&out.owner);
    wchar_t actual[64]{};
    const int length=GetWindowTextW(window,actual,64);
    out.titleMatches=length>0 && std::wcscmp(actual,title)==0;
    out.rootMatches=GetAncestor(window,GA_ROOT)==window;
    out.visible=IsWindowVisible(window)!=FALSE;
    out.iconic=IsIconic(window)!=FALSE;
    return out;
}
struct FocusObservation {
    WindowObservation before{}, after{};
    bool showCalled{}, showRequested{}, foregroundRequested{}, foregroundObserved{};
};
inline FocusObservation requestBorrowedForeground(HWND window, bool iconic, const wchar_t *title) {
    FocusObservation out{};
    out.before=observeWindow(GetForegroundWindow(),title);
    if (iconic) {out.showCalled=true;out.showRequested=ShowWindowAsync(window,SW_RESTORE)!=FALSE;}
    out.foregroundRequested=SetForegroundWindow(window)!=FALSE;
    out.after=observeWindow(GetForegroundWindow(),title);
    out.foregroundObserved=out.after.window==window;
    return out;
}
inline unsigned long long windowValue(HWND window) {
    return static_cast<unsigned long long>(reinterpret_cast<std::uintptr_t>(window));
}
} // namespace ss2vr::lab
