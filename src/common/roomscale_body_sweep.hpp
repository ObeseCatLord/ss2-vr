#pragma once
#include "roomscale_child_pose.hpp"
#include "roomscale_hull_transform.hpp"

namespace ss2vr::roomscale {
struct BodySweepCover {
    BodyCover body{};
    Vector direction{};
    float maximumParameter=0;
    bool valid=false;
};
// Geometry preparation only. A caller must prove that the candidate is the
// checked native root target, that captures remain owned, and that native
// traversal sees all relevant objects. This does not authorize placement.
inline BodySweepCover coverBodyRootSweep(const BodyGeometry& body,const Pose& candidate,
                                        float relativeRadiusBudget) {
    using namespace detail;
    BodySweepCover result;
    if(!arithmeticSupported()||!finite(candidate)||!finite(body.rootPose)||
       !bodyUnitQuaternion(candidate.q)||!bodyUnitQuaternion(body.rootPose.q)||
       !body.hullCount||body.hullCount>BodyGeometry::MaximumHulls||
       !std::isfinite(relativeRadiusBudget)||relativeRadiusBudget<=0||relativeRadiusBudget>1)return result;
    const std::array<float,3> start{body.rootPose.p.x,body.rootPose.p.y,body.rootPose.p.z};
    const std::array<float,3> end{candidate.p.x,candidate.p.y,candidate.p.z};
    IntervalVector delta{};
    for(unsigned c=0;c<3;++c)delta[c]=minus(exact(end[c]),exact(start[c]));
    const auto distance=length(delta);
    if(!valid(distance)||distance.lo<=0)return result;
    const float parameter=outwardFloat(distance.hi);
    if(!parameter)return result;
    std::array<float,3> direction{};
    IntervalVector endpointError{};
    for(unsigned c=0;c<3;++c) {
        direction[c]=static_cast<float>((double(end[c])-double(start[c]))/double(parameter));
        if(!std::isfinite(direction[c]))return {};
        endpointError[c]=minus(delta[c],times(exact(direction[c]),exact(parameter)));
    }
    const auto endpointRadius=length(endpointError);
    if(!valid(endpointRadius)||!(double(direction[0])*direction[0]+
        double(direction[1])*direction[1]+double(direction[2])*direction[2]>0))return {};
    const auto a=body.rootPose.q,b=candidate.q;
    const std::array<Interval,4> parentQ{{{std::min(a.x,b.x),std::max(a.x,b.x)},
        {std::min(a.y,b.y),std::max(a.y,b.y)},{std::min(a.z,b.z),std::max(a.z,b.z)},
        {std::min(a.w,b.w),std::max(a.w,b.w)}}};
    for(unsigned hull=0;hull<body.hullCount;++hull) {
        const auto& captured=body.hulls[hull];
        auto shape=captured.primitive;
        if(shape.kind==0){shape.kind=2;shape.height=shape.width;}
        if(shape.kind!=2||!std::isfinite(shape.width)||shape.width<=0)return {};
        const double nativeRadius=double(shape.width)*.5;
        const auto budget=plus(exact(nativeRadius),times(exact(nativeRadius),exact(relativeRadiusBudget)));
        if(!valid(budget)||budget.lo<=0||budget.lo>std::numeric_limits<float>::max())return {};
        float maximum=static_cast<float>(budget.lo);
        if(double(maximum)>budget.lo)maximum=std::nextafter(maximum,0.f);
        const auto local=coverCapsule(shape,relativeRadiusBudget*.5f);
        const auto actual=nativeHullTransformEnvelope(captured.pose);
        auto recomposed=childPoseFromParentBounds(parentQ,{exact(0),exact(0),exact(0)},captured.relativePose);
        if(!local.valid||!actual.valid||!recomposed.valid)return {};
        // Separate the linear root displacement from the native rounded child
        // offset. The final add/store may use a different FP mode at commit;
        // two outer float spacings enclose that add and binary32 storage for
        // every parent coordinate between the original and candidate roots.
        for(unsigned c=0;c<3;++c) {
            const Interval parentRange{std::min(start[c],end[c]),std::max(start[c],end[c])};
            const auto range=plus(parentRange,recomposed.p[c]);
            if(!valid(range)||range.lo < -std::numeric_limits<float>::max()||
               range.hi > std::numeric_limits<float>::max())return {};
            const float lo=static_cast<float>(range.lo),hi=static_cast<float>(range.hi);
            const double spacing=std::max(double(lo)-double(std::nextafter(lo,-INFINITY)),
                double(std::nextafter(hi,INFINITY))-double(hi));
            const double error=up(spacing*2);
            if(!std::isfinite(error))return {};
            recomposed.p[c]=plus(plus(exact(start[c]),recomposed.p[c]),{-error,error});
        }
        const auto swept=hullTransformFromPoseBounds(recomposed.q,recomposed.p);
        if(!swept.valid)return {};
        AffineEnvelope unionFrame{};
        for(unsigned i=0;i<12;++i)unionFrame[i]={std::min(actual.matrix[i].lo,swept.matrix[i].lo),
                                                          std::max(actual.matrix[i].hi,swept.matrix[i].hi)};
        auto cover=placeAffineEnvelope(local,unionFrame,maximum);
        if(!cover.valid)return {};
        for(unsigned i=0;i<cover.count;++i) {
            const auto needed=plus(exact(cover.sphere[i].radius),exact(endpointRadius.hi));
            const float radius=valid(needed)?outwardFloat(needed.hi):0;
            if(!radius||radius>maximum)return {};
            cover.sphere[i].radius=radius;
        }
        result.body.hulls[hull]=cover;
        ++result.body.hullCount;result.body.queryCount+=cover.count;
    }
    result.body.valid=true;result.direction={direction[0],direction[1],direction[2]};
    result.maximumParameter=parameter;result.valid=true;
    return result;
}
} // namespace ss2vr::roomscale
