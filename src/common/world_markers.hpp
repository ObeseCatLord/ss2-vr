#pragma once
#include "weapon_view.hpp"

namespace ss2vr {
struct WorldMarkerDimensions {
    int32_t width = 0, height = 0;
};
inline bool worldMarkerDimensions(const int32_t (&rectangle)[4], uint32_t width, uint32_t height,
                                  WorldMarkerDimensions &out) {
    const int64_t w = int64_t(rectangle[2]) - rectangle[0],
                  h = int64_t(rectangle[3]) - rectangle[1];
    if (rectangle[0] || rectangle[1] || w <= 0 || h <= 0 ||
        w > INT32_MAX || h > INT32_MAX || uint64_t(w) != width || uint64_t(h) != height)
        return false;
    out = {int32_t(w), int32_t(h)};
    return true;
}
inline bool worldMarkerViewValid(const WeaponWorldView &world, Pose eye, Fov fov,
                                 WorldMarkerDimensions dimensions, uint32_t width, uint32_t height) {
    return width && height && dimensions.width > 0 && dimensions.height > 0 &&
           uint32_t(dimensions.width) == width && uint32_t(dimensions.height) == height &&
           world.valid && finite(eye) && finiteMatrix(world.view) && finiteProjection(world.projection) &&
           validDepthRange(world.nearDepth, world.farDepth) &&
           sameWeaponView(world.view, matrix(inverse(eye))) &&
           sameProjectionXY(world.projection, projection(fov));
}
// Used only by the admitted eye loop. Never replay this phase from an original
// render callback, recursive fallback, or desktop restoration.
template <class CheckTarget, class PostRender>
bool finishEyeRender(bool rendered, CheckTarget checkTarget, PostRender postRender) {
    return rendered && checkTarget() && postRender() && checkTarget();
}
} // namespace ss2vr
