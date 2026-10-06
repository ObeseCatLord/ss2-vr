#pragma once
#include <vector>

namespace ss2vr {
// Keep every trampoline alive until ALL detours have been disabled. A surviving
// derived detour may call another original trampoline in the same transaction.
template <class Disable, class Remove>
bool rollbackHooks(std::vector<void *> &owned, Disable disable, Remove remove) {
    bool disabled = true;
    for (auto address : owned)
        disabled = disable(address) && disabled;
    if (!disabled)
        return false;
    bool removed = true;
    for (auto it = owned.begin(); it != owned.end();) {
        if (remove(*it))
            it = owned.erase(it);
        else {
            removed = false;
            ++it;
        }
    }
    return removed;
}
} // namespace ss2vr
