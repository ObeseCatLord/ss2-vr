#pragma once
#include "roomscale_body_geometry.hpp"

namespace ss2vr::roomscale {
struct OriginSettlement {
    Pose origin{};
    double headPositionError=0;
    bool valid=false;
};
// Translate the tracking origin by the ACTUAL accepted anchor displacement.
// Intended for the translation-only body controller: a changed native anchor
// orientation is not silently absorbed as a tracking recenter. The controller
// owns publication/rollback policy; invalid does not mean the body never moved.
inline OriginSettlement settleTranslatedAnchor(const Pose& oldOrigin,float turn,const Pose& head,
    const Pose& before,const Pose& after,float maximumDisplacement,float errorBudget) {
    OriginSettlement result;
    if(!arithmeticSupported()||!finite(oldOrigin)||!finite(head)||!finite(before)||!finite(after)||
       !bodyUnitQuaternion(oldOrigin.q)||!bodyUnitQuaternion(before.q)||!bodyUnitQuaternion(after.q)||
       !std::isfinite(turn)||!std::isfinite(maximumDisplacement)||maximumDisplacement<=0||
       !std::isfinite(errorBudget)||errorBudget<0||
       before.q.x!=after.q.x||before.q.y!=after.q.y||before.q.z!=after.q.z||before.q.w!=after.q.w)return result;
    const auto unclamped=[](Vec3 p) {
        const auto bounded=boundHeadTranslation(p);
        return p.x==bounded.x && p.y==bounded.y && p.z==bounded.z;
    };
    const auto relative=relativeTracking(oldOrigin,head);
    // A clamped physical offset needs separate obstruction/recenter policy.
    // Do not consume a larger physical displacement than the native move.
    if(!finite(relative)||!unclamped(relative.p))return result;
    const Vec3 delta=after.p-before.p;
    const double distance=std::sqrt(double(delta.x)*delta.x+double(delta.y)*delta.y+double(delta.z)*delta.z);
    if(!std::isfinite(distance)||distance>maximumDisplacement)return result;
    const auto local=rotate(inverse(before.q),delta);
    const auto stage=rotate(oldOrigin.q,rotate(yaw(-turn),local));
    Pose origin=oldOrigin;origin.p=origin.p+stage;
    if(!finite(origin))return result;
    const auto nextRelative=relativeTracking(origin,head);
    if(!finite(nextRelative)||!unclamped(nextRelative.p))return result;
    const auto first=worldHeadTracking(before,oldOrigin,turn,head);
    const auto second=worldHeadTracking(after,origin,turn,head);
    if(!finite(first)||!finite(second))return result;
    const double dx=double(first.p.x)-second.p.x,dy=double(first.p.y)-second.p.y,dz=double(first.p.z)-second.p.z;
    const double error=std::sqrt(dx*dx+dy*dy+dz*dz);
    if(!std::isfinite(error)||error>errorBudget)return result;
    result.origin=origin;result.headPositionError=error;result.valid=true;
    return result;
}
} // namespace ss2vr::roomscale
