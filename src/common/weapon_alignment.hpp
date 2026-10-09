#pragma once
#include "math.hpp"
#include <cstring>

namespace ss2vr {
// Fixed virtual handle centre for the matched stock ID1 assembly, in native
// pre-stretch model coordinates. Reference: V27 request31/eye0/right idle pose.
// This does not define physical skin contact, a rest pose or another weapon.
constexpr Vec3 Id1ModelHandleReference{-.00960000045598f, -.00474976426398f, -.536467618906f};

inline bool alignmentBorrowPhase(unsigned renderDepth, bool placement) {
    return renderDepth == (placement ? 1u : 0u);
}
inline bool id1RenderedStretch(uint32_t selector, Vec3 base, Vec3 &out) {
    if (!std::isfinite(base.x) || !std::isfinite(base.y) || !std::isfinite(base.z)) return false;
    out = base;
    if (!selector) {
        // Exact native SetStretch floor, including signed zero and nonuniform
        // scale. The native branch flips X, floors all components, then renders.
        const auto floor = [](float x) {
            constexpr float epsilon = std::bit_cast<float>(uint32_t{0x3727c5ac});
            return std::copysign(std::max(std::fabs(x),epsilon),x);
        };
        out = {floor(-base.x), floor(base.y), floor(base.z)};
    }
    return true;
}
// Value-only identities. The copied instance address is never dereferenced from
// a cache; consumption resolves the current native handle and copies it anew.
struct WeaponAlignmentBinding {
    uint32_t owner=0, weapon=0, model=0, instance=0, selector=0, hand=2;
    uint32_t configuration=0, file=0;
    int32_t resource=-1;
    Vec3 baseStretch{};
    bool operator==(const WeaponAlignmentBinding &other) const {
        return owner==other.owner && weapon==other.weapon && model==other.model &&
            instance==other.instance && selector==other.selector && hand==other.hand &&
            configuration==other.configuration && file==other.file && resource==other.resource &&
            std::memcmp(&baseStretch,&other.baseStretch,sizeof(Vec3))==0;
    }
};
} // namespace ss2vr
