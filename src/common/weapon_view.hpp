#pragma once
#include "model_tree.hpp"
#include <algorithm>
#include <cmath>
namespace ss2vr {
struct WeaponWorldView {
    Matrix34 view{};
    Matrix44 projection{};
    float nearDepth = 0, farDepth = 1;
    bool valid = false;
};
inline bool finiteProjection(const Matrix44 &p) {
    for (float v : p.m)
        if (!std::isfinite(v)) return false;
    return p.m[0] > 0 && p.m[5] > 0 && p.m[14] == -1 && p.m[15] == 0;
}
inline bool validDepthRange(float nearDepth, float farDepth) {
    return std::isfinite(nearDepth) && std::isfinite(farDepth) &&
           nearDepth >= 0 && farDepth <= 1 && farDepth > nearDepth;
}
inline bool sameWeaponView(const Matrix34 &a, const Matrix34 &b) {
    for (unsigned i = 0; i != 12; ++i)
        if (!std::isfinite(a.m[i]) || !std::isfinite(b.m[i]) ||
            std::abs(a.m[i] - b.m[i]) > .001f) return false;
    return true;
}
inline bool sameProjectionXY(const Matrix44 &a, const Matrix44 &b) {
    // Native execution may revise Z clipping. Every other coefficient must
    // retain this exact prepared asymmetric eye's projection provenance.
    for (unsigned i = 0; i != 16; ++i) {
        if (i == 10 || i == 11) continue;
        if (!std::isfinite(a.m[i]) || !std::isfinite(b.m[i]) ||
            std::abs(a.m[i] - b.m[i]) > .00001f) return false;
    }
    return true;
}
inline WeaponWorldView executedWeaponView(const WeaponWorldView &prepared,
                                          const Matrix34 &view, const Matrix44 &projection,
                                          float nearDepth, float farDepth) {
    const bool valid = prepared.valid && finiteMatrix(view) && finiteProjection(projection) &&
                       validDepthRange(nearDepth, farDepth) &&
                       sameWeaponView(prepared.view, view) &&
                       sameProjectionXY(prepared.projection, projection) &&
                       nearDepth == prepared.nearDepth && farDepth == prepared.farDepth;
    return {view, projection, nearDepth, farDepth, valid};
}
// Ephemeral native Render invocation ordering, not simulation/gameplay state.
struct WeaponViewPass {
    WeaponWorldView world;
    unsigned stage = 0;
    bool failed = false;
    bool graphicsSetupReached = false;
    void graphicsSetup() { graphicsSetupReached = true; }
    bool advance(unsigned expected) {
        if (failed || stage != expected) {
            failed = true;
            return false;
        }
        ++stage;
        return true;
    }
    bool projection(Matrix44 &out) {
        graphicsSetup(); // Native caller installs even a rejected/fallback result.
        if (!world.valid || !advance(0)) { failed = true; return false; }
        out = world.projection;
        return true;
    }
    bool view(Matrix34 &out) {
        graphicsSetup();
        if (!advance(1)) return false;
        out = world.view;
        return true;
    }
    bool depth(float &nearDepth, float &farDepth) {
        graphicsSetup();
        if (!advance(2)) return false;
        nearDepth = world.nearDepth;
        farDepth = world.farDepth;
        return true;
    }
    bool placed(bool success) {
        if (!success) { failed = true; return false; }
        return advance(3);
    }
    bool restored() { return advance(4); }
    bool complete() const { return !failed && (stage == 0 || stage == 5); }
    bool cleanupRequired() const { return graphicsSetupReached && !complete(); }
};
// Used by the render wrappers, including suppression barriers for unowned
// nested calls. Native exceptions are not caught by this context guard.
template<class T> struct ScopedWeaponContext {
    T *&slot;
    T *previous;
    ScopedWeaponContext(T *&slot, T *current) : slot(slot), previous(slot) { slot = current; }
    ~ScopedWeaponContext() { slot = previous; }
    ScopedWeaponContext(const ScopedWeaponContext &) = delete;
};
} // namespace ss2vr
