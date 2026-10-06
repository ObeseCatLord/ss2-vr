#pragma once
#include <array>
#include <cstdint>

namespace ss2vr {
// Classification of the audited canonical player callback's weapon targets,
// AFTER its player bookkeeping. Inputs are already native post-flip semantic
// indices and resolved selected handles, not left/right hand enum values.
// This helper calls no native code and grants no lifetime/dispatch permission.
struct NativePrimaryReleaseTargets {
    std::array<uint32_t, 2> primary{};
    unsigned count = 0;
    uint32_t alternative = 0;
    bool primaryLane = false;
};
constexpr NativePrimaryReleaseTargets nativePrimaryReleaseTargets(
    unsigned semantic, uint32_t right, uint32_t left, bool uncoupled) noexcept {
    NativePrimaryReleaseTargets result;
    if (semantic > 1) return result;
    result.primaryLane = true;
    if (semantic == 0) {
        if (right) result.primary[result.count++] = right;
        if (!uncoupled && left) result.primary[result.count++] = left;
    } else if (left) {
        result.primary[result.count++] = left;
    } else {
        result.alternative = right;
    }
    return result;
}
constexpr bool nativePrimaryPressHasWeaponCallback(unsigned semantic, uint32_t right,
                                                    uint32_t left) noexcept {
    return semantic > 1 || (semantic == 1 && right && !left);
}
// The initial connected scope is deliberately one unique primary target. A
// multi-target/alternative/manual callback must not be swallowed on behalf of
// just one saw. Resolve identity, mapping and mode again before native use.
constexpr bool nativePrimarySingleTarget(unsigned semantic, uint32_t right, uint32_t left,
                                         bool uncoupled, uint32_t exactWeapon) noexcept {
    if (!exactWeapon || (right && right == left) ||
        nativePrimaryPressHasWeaponCallback(semantic, right, left)) return false;
    const auto release = nativePrimaryReleaseTargets(semantic, right, left, uncoupled);
    return release.primaryLane && !release.alternative && release.count == 1 &&
           release.primary[0] == exactWeapon;
}
} // namespace ss2vr
