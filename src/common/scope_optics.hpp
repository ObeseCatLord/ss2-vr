#pragma once
#include "scope_geometry.hpp"

namespace ss2vr {
// Copied native sniper interpolation values. These are presentation evidence,
// not a second zoom timer, activation request or proof of native completion.
struct ScopeNativeZoom {
    float startMultiplier = 0, endMultiplier = 0, progress = 0;
    bool valid = false;
};
inline ScopeNativeZoom scopeNativeZoom(uint32_t active, uint32_t alternativeHeld,
                                      float start, float end, float progress) noexcept {
    if (active != 1 || alternativeHeld != 1 || !std::isfinite(start) || !std::isfinite(end) ||
        !std::isfinite(progress) || start <= 0 || end <= 0 || start > 1 || end > 1 ||
        progress < 0 || progress > 1) return {};
    return {start, end, progress, true};
}
// The pinned CPuppet projection clamps the unzoomed horizontal angle to
// 45..135 degrees, then multiplies by Player::GetFOVMultiplier and pi/180.
// Recover only that base angle from the actual frustum argument. The shared
// multiplier is a divisor here, NEVER the per-hand scope magnification.
inline float scopeNativeBaseFov(float renderedRadians, float playerMultiplier) noexcept {
    if (!std::isfinite(renderedRadians) || renderedRadians <= 0 ||
        !std::isfinite(playerMultiplier) || playerMultiplier <= 0 || playerMultiplier > 1) return 0;
    constexpr double radians = 3.14159265358979323846 / 180;
    const double base = double(renderedRadians) / playerMultiplier;
    if (!std::isfinite(base) || base < 45*radians-1e-6 || base > 135*radians+1e-6) return 0;
    return float(base);
}
struct ScopeOpticalFrame {
    Matrix34 camera{}, inverseCamera{}; // Proper rigid camera; -Z is forward.
    float radiusX = 0, radiusY = 0;
    bool valid = false;
};
inline bool scopeUnitVector(Vec3 v, Vec3 &out) {
    const float n2 = dot(v,v);
    if (!std::isfinite(n2) || n2 < 1e-12f) return false;
    out = v*(1/std::sqrt(n2));
    return true;
}
inline Vec3 scopeTransformPoint(const Matrix34 &m, Vec3 p) {
    return {m.m[0]*p.x+m.m[1]*p.y+m.m[2]*p.z+m.m[3],
            m.m[4]*p.x+m.m[5]*p.y+m.m[6]*p.z+m.m[7],
            m.m[8]*p.x+m.m[9]*p.y+m.m[10]*p.z+m.m[11]};
}
// Geometry and affine must already belong to the admitted current native draw.
// This calculates an optical frame only; it does not admit a material/pass or
// imply that a source image exists. Keep reflected winding in the cap itself.
inline bool scopeOpticalFrame(const ScopeCapGeometry &cap, const Matrix34 &affine,
                              ScopeOpticalFrame &out) {
    out = {};
    Matrix34 inv;
    if (!cap.valid || !affineInverse(affine,inv)) return false;
    Vec3 center{}, normal{};
    for (const auto p : cap.positions) {
        if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z)) return false;
        center = center+p;
    }
    center = center*(1.f/float(ScopeCapVertices));
    for (size_t i=0;i<ScopeCapIndices;i+=3) {
        const auto a=cap.indices[i], b=cap.indices[i+1], c=cap.indices[i+2];
        if (a>=ScopeCapVertices || b>=ScopeCapVertices || c>=ScopeCapVertices) return false;
        const auto n=cross(cap.positions[b]-cap.positions[a],cap.positions[c]-cap.positions[a]);
        // The pinned stock rear cap has consistent outward +Z winding. A
        // different/degenerate/nonplanar cap cannot supply an optical camera.
        if (!std::isfinite(n.z) || n.z<=1e-12f || std::abs(n.x)>n.z*1e-4f ||
            std::abs(n.y)>n.z*1e-4f) return false;
        normal = normal+n;
    }
    for (const auto p : cap.positions)
        if (std::abs(p.z-center.z)>1e-5f) return false;
    // Inverse transpose transports the outward normal even under reflection.
    // Crossing reflected edges instead would invert the optical direction.
    Vec3 back, up, right;
    if (!scopeUnitVector({inv.m[0]*normal.x+inv.m[4]*normal.y+inv.m[8]*normal.z,
                          inv.m[1]*normal.x+inv.m[5]*normal.y+inv.m[9]*normal.z,
                          inv.m[2]*normal.x+inv.m[6]*normal.y+inv.m[10]*normal.z},back)) return false;
    up={affine.m[1],affine.m[5],affine.m[9]};
    if (!scopeUnitVector(up-back*dot(up,back),up) || !scopeUnitVector(cross(up,back),right)) return false;
    up=cross(back,right);
    const Vec3 origin=scopeTransformPoint(affine,center);
    ScopeOpticalFrame next;
    next.camera={{right.x,up.x,back.x,origin.x, right.y,up.y,back.y,origin.y,
                  right.z,up.z,back.z,origin.z}};
    if (!affineInverse(next.camera,next.inverseCamera)) return false;
    for (const auto p : cap.positions) {
        // Measure A*(p-center) in the rigid basis. Introducing world translation
        // here loses small aperture offsets far from the native world origin.
        const Vec3 delta=p-center;
        const Vec3 offset{affine.m[0]*delta.x+affine.m[1]*delta.y+affine.m[2]*delta.z,
                          affine.m[4]*delta.x+affine.m[5]*delta.y+affine.m[6]*delta.z,
                          affine.m[8]*delta.x+affine.m[9]*delta.y+affine.m[10]*delta.z};
        const Vec3 local{dot(right,offset),dot(up,offset),dot(back,offset)};
        if (!std::isfinite(local.x) || !std::isfinite(local.y) || !std::isfinite(local.z) ||
            std::abs(local.z)>1e-4f) return false;
        next.radiusX=std::max(next.radiusX,std::abs(local.x));
        next.radiusY=std::max(next.radiusY,std::abs(local.y));
    }
    if (!(next.radiusX>1e-5f && next.radiusY>1e-5f && next.radiusX<1 && next.radiusY<1)) return false;
    next.valid=true; out=next; return true;
}
struct ScopeOpticalProjection {
    float tangentX = 0, tangentY = 0, magnification = 0;
    bool valid = false;
};
// Native zoom scales an angular FOV. Convert two admitted angles in radians
// into slope magnification; reciprocal angle scaling is only a small-angle
// approximation. The caller owns native zoom sampling and its frozen lifetime.
inline bool scopeAngularMagnification(float baseFovRadians, float zoomFovRadians,
                                     float &out) {
    out=0;
    constexpr double pi=3.14159265358979323846;
    if (!std::isfinite(baseFovRadians) || !std::isfinite(zoomFovRadians) ||
        baseFovRadians<=0 || double(baseFovRadians)>=pi || zoomFovRadians<=0 ||
        zoomFovRadians>baseFovRadians) return false;
    const double magnification=std::tan(double(baseFovRadians)*.5)/
                               std::tan(double(zoomFovRadians)*.5);
    if (!std::isfinite(magnification) || magnification<1 || magnification>100) return false;
    out=float(magnification);
    return std::isfinite(out) && out>=1 && out<=100;
}
inline bool scopeNativeMagnification(float baseRadians, const ScopeNativeZoom &zoom, float &out) noexcept {
    out = 0;
    // Revalidate copied inputs rather than trusting a caller-set valid bit.
    if (!zoom.valid || !scopeNativeZoom(1,1,zoom.startMultiplier,zoom.endMultiplier,zoom.progress).valid)
        return false;
    const float multiplier = zoom.startMultiplier +
        (zoom.endMultiplier - zoom.startMultiplier) * zoom.progress;
    return scopeAngularMagnification(baseRadians, baseRadians * multiplier, out);
}
inline bool scopeOpticalProjectionValid(const ScopeOpticalProjection &p) {
    return p.valid && std::isfinite(p.magnification) && p.magnification>=1 && p.magnification<=100 &&
        std::isfinite(p.tangentX) && std::isfinite(p.tangentY) &&
        p.tangentX>=1e-4f && p.tangentY>=1e-4f && p.tangentX<=10 && p.tangentY<=10 &&
        std::isfinite(p.magnification*p.tangentX) && std::isfinite(p.magnification*p.tangentY);
}
inline bool scopeOpticalFov(const ScopeOpticalProjection &p, Fov &out) {
    out={};
    if (!scopeOpticalProjectionValid(p)) return false;
    const float x=std::atan(p.tangentX), y=std::atan(p.tangentY);
    out={-x,x,y,-y}; return true;
}
inline bool scopeOpticalFrameValid(const ScopeOpticalFrame &f) {
    if (!f.valid || !finiteMatrix(f.camera) || !finiteMatrix(f.inverseCamera)) return false;
    Vec3 x{f.camera.m[0],f.camera.m[4],f.camera.m[8]};
    Vec3 y{f.camera.m[1],f.camera.m[5],f.camera.m[9]};
    Vec3 z{f.camera.m[2],f.camera.m[6],f.camera.m[10]};
    if (std::abs(dot(x,x)-1)>1e-4f || std::abs(dot(y,y)-1)>1e-4f || std::abs(dot(z,z)-1)>1e-4f ||
        std::abs(dot(x,y))>1e-4f || std::abs(dot(x,z))>1e-4f || std::abs(dot(y,z))>1e-4f ||
        std::abs(dot(cross(x,y),z)-1)>1e-4f) return false;
    for (unsigned r=0;r<3;++r)
        for (unsigned c=0;c<3;++c)
            if (std::abs(f.inverseCamera.m[4*r+c]-f.camera.m[4*c+r])>1e-4f) return false;
    return true;
}
// Relief and magnification are explicit optical parameters, not native state
// guesses. Native zoom timing/progress is consumed by the caller, never written.
inline bool scopeOpticalProjection(const ScopeOpticalFrame &frame, float relief,
                                   float magnification, ScopeOpticalProjection &out) {
    out={};
    if (!scopeOpticalFrameValid(frame) || !std::isfinite(relief) || relief<.01f || relief>1 ||
        !std::isfinite(magnification) || magnification<1 || magnification>100 ||
        !std::isfinite(frame.radiusX) || !std::isfinite(frame.radiusY) ||
        frame.radiusX<=0 || frame.radiusY<=0) return false;
    const float tx=frame.radiusX/(relief*magnification), ty=frame.radiusY/(relief*magnification);
    if (!std::isfinite(tx) || !std::isfinite(ty) || tx<1e-4f || ty<1e-4f || tx>10 || ty>10) return false;
    out={tx,ty,magnification,true}; return true;
}
// Current world-eye ray (not a cropped XR image). Angular compression makes
// narrowed source content visibly magnified across the physical aperture.
// Backwards/parallel/malformed rays are rejected before UV sampling.
inline bool scopeImageUv(const ScopeOpticalFrame &frame, const ScopeOpticalProjection &p,
                          Vec3 worldRay, std::array<float,2> &out) {
    out={};
    if (!scopeOpticalFrameValid(frame) || !scopeOpticalProjectionValid(p) ||
        !std::isfinite(worldRay.x) || !std::isfinite(worldRay.y) || !std::isfinite(worldRay.z)) return false;
    const float scale=std::max({std::abs(worldRay.x),std::abs(worldRay.y),std::abs(worldRay.z)});
    if (!(scale>0)) return false;
    // Component division remains defined even for subnormal finite directions;
    // computing a reciprocal first could overflow and change admission.
    worldRay={worldRay.x/scale,worldRay.y/scale,worldRay.z/scale};
    const auto &m=frame.inverseCamera.m;
    const Vec3 ray{m[0]*worldRay.x+m[1]*worldRay.y+m[2]*worldRay.z,
                   m[4]*worldRay.x+m[5]*worldRay.y+m[6]*worldRay.z,
                   m[8]*worldRay.x+m[9]*worldRay.y+m[10]*worldRay.z};
    if (!std::isfinite(ray.x) || !std::isfinite(ray.y) || !std::isfinite(ray.z) || ray.z>=-1e-6f) return false;
    const float x=.5f+.5f*(ray.x/-ray.z)/(p.magnification*p.tangentX);
    const float y=.5f-.5f*(ray.y/-ray.z)/(p.magnification*p.tangentY);
    if (!std::isfinite(x) || !std::isfinite(y)) return false;
    out={x,y}; return true;
}
} // namespace ss2vr
