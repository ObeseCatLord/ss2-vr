#pragma once
#include "math.hpp"
#include <cstring>

namespace ss2vr {
// Fixed virtual handle centre for the matched stock ID1 assembly, in native
// pre-stretch model coordinates. Reference: V27 request31/eye0/right idle pose.
// This does not define physical skin contact, a rest pose or another weapon.
constexpr Vec3 Id1ModelHandleReference{-.00960000045598f, -.00474976426398f, -.536467618906f};
// Stock ID13 virtual handle-section centre in the authored native Idle model
// frame. Both inspected variants and own consumed Black draws agree on identity.
// This convention does not define skin contact or another weapon's reference.
constexpr Vec3 Id13ModelHandleReference{0.f, .003122344828160041f, .006718217565474383f};
constexpr bool weaponAlignmentSupported(uint32_t nativeId) {
    return nativeId==1 || nativeId==13;
}
// Binding freshness is independent of having an authored grip correction.
// The installed native inventory has seventeen slots; slot fourteen is unused.
constexpr bool weaponModelBindingSupported(uint32_t nativeId) {
    return nativeId < WeaponCount && nativeId != 14;
}
inline bool weaponModelHandleReference(uint32_t nativeId,Vec3 &out) {
    out={};
    if(nativeId==1)out=Id1ModelHandleReference;
    else if(nativeId==13)out=Id13ModelHandleReference;
    else return false;
    return true;
}

inline bool alignmentBorrowPhase(unsigned renderDepth, bool placement) {
    return renderDepth == (placement ? 1u : 0u);
}
// Stack-owned wrapper provenance, not a cached native object or new ownership
// policy. A desktop sniper's original render delegates to the original base.
struct WeaponRenderRoute {
    uintptr_t weapon=0;
    unsigned depth=0;
    bool sniper=false,desktop=false,originalActive=false;
    const WeaponRenderRoute *parent=nullptr;
};
inline bool nativeDesktopSniperPlacement(const WeaponRenderRoute *route,uintptr_t weapon,
                                         int eye,bool nativePlacementCaller) {
    if(!route || !route->parent || !weapon || eye!=-1 || !nativePlacementCaller)return false;
    const auto &parent=*route->parent;
    return route->weapon==weapon && parent.weapon==weapon && route->depth==2 && parent.depth==1 &&
        !route->sniper && parent.sniper && route->desktop && parent.desktop &&
        route->originalActive && parent.originalActive;
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
    uint32_t nativeId=0;
    bool operator==(const WeaponAlignmentBinding &other) const {
        return nativeId==other.nativeId && owner==other.owner && weapon==other.weapon && model==other.model &&
            instance==other.instance && selector==other.selector && hand==other.hand &&
            configuration==other.configuration && file==other.file && resource==other.resource &&
            std::memcmp(&baseStretch,&other.baseStretch,sizeof(Vec3))==0;
    }
};
inline bool weaponModelCalibrationMatches(const WeaponAlignmentBinding &retained,
                                          const WeaponAlignmentBinding &current,
                                          bool alignmentApplied) {
    return weaponModelBindingSupported(retained.nativeId) &&
           alignmentApplied == weaponAlignmentSupported(retained.nativeId) && retained == current;
}
} // namespace ss2vr
