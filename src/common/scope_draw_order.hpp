#pragma once
#include "scope_geometry.hpp"
#include <cstdint>
#include <type_traits>

namespace ss2vr {
struct ScopeDrawRange {
    uint32_t firstIndex = 0, triangles = 0;
    bool operator==(const ScopeDrawRange &) const = default;
};
inline constexpr ScopeDrawRange ScopePrefixDraw{0, 901};
inline constexpr ScopeDrawRange ScopeCapDraw{901 * 3, 22};
inline constexpr ScopeDrawRange ScopeSuffixDraw{923 * 3, 5};
static_assert(ScopePrefixDraw.triangles + ScopeCapDraw.triangles +
              ScopeSuffixDraw.triangles == ScopeTriangles);

// Ordered execution only, NOT image or native-state admission. The caller must
// already own every source/resource/state needed by the entire transaction.
// The native callback retains the original DIP's topology/base/minimum/vertices;
// only StartIndex and PrimitiveCount change. All indices here are INDEX units.
// The image callback draws this same cap with RGB-only writes and must restore
// all touched state before returning true. An uncertain/partial result is false.
// A foreign unwind requires the caller's existing NativeFinally owner; this
// helper does not catch native exceptions or own COM resources.
// There is deliberately no full-draw fallback, retry, or persistent phase: once
// invoked, a failure invalidates the owning pair even if no prefix completed.
// The native scope GPU adapter uses this only after source-image admission.
template<class Native, class Image, class Healthy>
bool executeScopeCapDraw(Native &&native, Image &&image, Healthy &&healthy) noexcept {
    static_assert(std::is_nothrow_invocable_r_v<bool, Native &, ScopeDrawRange>);
    static_assert(std::is_nothrow_invocable_r_v<bool, Image &, ScopeDrawRange>);
    static_assert(std::is_nothrow_invocable_r_v<bool, Healthy &>);
    if (!healthy() || !native(ScopePrefixDraw) || !healthy()) return false;
    if (!native(ScopeCapDraw) || !healthy()) return false;
    if (!image(ScopeCapDraw) || !healthy()) return false;
    return native(ScopeSuffixDraw) && healthy();
}
} // namespace ss2vr
