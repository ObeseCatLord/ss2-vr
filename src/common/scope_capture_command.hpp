#pragma once
#include <cstdint>

namespace ss2vr {
// Adapter-selected rank, not an invented engine enum. The native comparator
// uses the low 24 bits. Preserve flares/visibility before capture and bloom,
// post-render callback and text after it, with the same cut on every source.
inline constexpr uint32_t ScopeCaptureRank = 0xaffff;
inline constexpr int32_t ScopeCaptureMaxCommands = 16384;
static_assert(0xa0000 < ScopeCaptureRank && ScopeCaptureRank < 0xb0000);
// Per-native-invocation duplicate latch. A reentrant callback permanently
// rejects the candidate even if the outer copy subsequently returns success.
struct ScopeCaptureOnce {
    bool called = false, rejected = false;
    bool enter() noexcept {
        if (called) { rejected = true; return false; }
        called = true;
        return !rejected;
    }
    bool complete(bool nativeFinished, bool copied) const noexcept {
        return called && !rejected && nativeFinished && copied;
    }
};
struct ScopeCommandAppend {
    uintptr_t root = 0, currentRoot = 0, parent = 0;
    uintptr_t beforeArray = 0, afterArray = 0, command = 0, last = 0;
    int32_t beforeCount = -1, afterCount = -1;
};
// A normal-return membership check, not allocation recovery or a lifetime pin.
// No executing array is changed by this helper. A failed check must leave the
// already-linked command harmless and decline the source, never retry/remove.
inline bool scopeCommandAppendConfirmed(const ScopeCommandAppend &a) noexcept {
    return a.root && a.root == a.currentRoot && a.root == a.parent &&
        a.beforeArray && a.beforeArray == a.afterArray && a.command && a.command == a.last &&
        a.beforeCount >= 0 && a.beforeCount < ScopeCaptureMaxCommands &&
        a.afterCount == a.beforeCount + 1;
}
} // namespace ss2vr
