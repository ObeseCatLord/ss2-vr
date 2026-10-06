#pragma once
#include <windows.h>
#include <cstdint>

namespace ss2vr::game {
using ScopeCaptureCallback = void(__cdecl *)(void *);
using ScopeCaptureCurrent = bool (*)(void *) noexcept;
// Native module references only; configure after the existing Engine/Sam hash
// admission. Modules must remain loaded through all queued command cleanup.
class ScopeCaptureCommands {
public:
    bool configure(HMODULE engine, HMODULE sam) noexcept;
    // Call once, only at the actual FDA0B source-injection return under the
    // frozen source owner. current is a read-only, callback-free owner check.
    // callback/context must remain live through native source execution AND
    // collection/pile cleanup. This queues a command; it does not capture yet.
    // False may leave a harmless native-owned command. Never retry in this
    // view, remove list entries, or free native pile storage manually.
    bool queue(void *root, uintptr_t injectionCaller, ScopeCaptureCallback callback,
               ScopeCaptureCurrent current, void *context) const noexcept;
private:
    using Allocate = void *(__cdecl *)(uint32_t);
    using Construct = void(__thiscall *)(void *);
    uintptr_t engine_ = 0, injectionReturn_ = 0;
    Allocate allocate_ = nullptr;
    Construct construct_ = nullptr;
};
} // namespace ss2vr::game
