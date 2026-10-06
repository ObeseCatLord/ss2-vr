#pragma once
#include "roomscale_body_geometry.hpp"
#include "roomscale_capsule_cover.hpp"

namespace ss2vr::roomscale {
struct HullTransformEnvelope { AffineEnvelope matrix{}; bool valid=false; };

// Bound the pinned primitive-hull raw-quaternion expansion (Engine525CB..52674).
// Unlike presentation matrix(Pose), this NEVER normalizes the native quaternion.
// Intermediate d/e/g/h and final coefficients are binary32 stores in that code.
// The envelope includes both forward placement and inverse-transpose query
// geometry: finite-precision matrices are not assumed perfectly orthogonal.
// This is geometry only; the native owner still must freeze/revalidate the pose.
inline HullTransformEnvelope nativeHullTransformEnvelope(const ss2vr::Pose& pose) {
    using namespace detail;
    HullTransformEnvelope result;
    if (!arithmeticSupported()||!ss2vr::finite(pose)||!bodyUnitQuaternion(pose.q)) return result;
    auto twice=[](Interval a) { return plus(a,a); };
    auto stored=[](Interval a) {
        if(!valid(a))return Interval{NAN,NAN};
        // Force binary32 storage even on an x87 compiler with excess precision.
        volatile float lo=static_cast<float>(a.lo),hi=static_cast<float>(a.hi);
        return Interval{double(lo),double(hi)};
    };
    const auto x=exact(pose.q.x),y=exact(pose.q.y),z=exact(pose.q.z),w=exact(pose.q.w);
    const auto a=twice(times(x,x)),b=twice(times(y,x)),c=twice(times(z,x));
    const auto d=stored(twice(times(y,y))),e=stored(twice(times(z,y))),f=twice(times(z,z));
    const auto g=stored(twice(times(w,x))),h=stored(twice(times(w,y))),i=twice(times(w,z));
    const std::array<IntervalVector,3> m{{
        {stored(minus(exact(1),plus(f,d))),stored(minus(b,i)),stored(plus(h,c))},
        {stored(plus(i,b)),stored(minus(exact(1),plus(a,f))),stored(minus(e,g))},
        {stored(minus(c,h)),stored(plus(g,e)),stored(minus(exact(1),plus(d,a)))}}};
    for(const auto& row:m)for(auto value:row)if(!valid(value))return result;
    auto cross=[](const IntervalVector& u,const IntervalVector& v) -> IntervalVector {
        return {minus(times(u[1],v[2]),times(u[2],v[1])),
                minus(times(u[2],v[0]),times(u[0],v[2])),
                minus(times(u[0],v[1]),times(u[1],v[0]))};
    };
    const std::array<IntervalVector,3> cofactor{{cross(m[1],m[2]),cross(m[2],m[0]),cross(m[0],m[1])}};
    const auto determinant=inner(m[0],cofactor[0]);
    if(!valid(determinant)||determinant.lo<=0)return result;
    for(unsigned r=0;r<3;++r)for(unsigned col=0;col<3;++col) {
        // Native local-ray construction uses M^T(world-position). The inverse
        // image of the primitive is therefore M^-T, not an assumed exact M.
        const auto inverseTranspose=dividePositive(cofactor[r][col],determinant);
        if(!valid(inverseTranspose))return {};
        result.matrix[r*4+col]={std::min(m[r][col].lo,inverseTranspose.lo),
                                std::max(m[r][col].hi,inverseTranspose.hi)};
    }
    result.matrix[3]=exact(pose.p.x);result.matrix[7]=exact(pose.p.y);result.matrix[11]=exact(pose.p.z);
    result.valid=true;
    return result;
}
struct BodyCover {
    std::array<CapsuleCover,BodyGeometry::MaximumHulls> hulls{};
    unsigned hullCount=0,queryCount=0;
    bool valid=false;
};
// Build the complete copied body union, never a usable partial cover. Half the
// explicit radius allowance is reserved for frame/world-grid uncertainty; all
// final radii still satisfy the original caller's total allowance. No default
// physical dimensions, body resize, native call or movement policy is supplied.
inline BodyCover coverBodyGeometry(const BodyGeometry& body,float relativeRadiusBudget) {
    using namespace detail;
    BodyCover result;
    if(!arithmeticSupported()||!body.hullCount||body.hullCount>BodyGeometry::MaximumHulls||
        !std::isfinite(relativeRadiusBudget)||relativeRadiusBudget<=0||relativeRadiusBudget>1) return result;
    for(unsigned i=0;i<body.hullCount;++i) {
        auto shape=body.hulls[i].primitive;
        if(shape.kind==0) { shape.kind=2;shape.height=shape.width; }
        if(shape.kind!=2||!std::isfinite(shape.width)||shape.width<=0) return {};
        const double radius=double(shape.width)*.5;
        const auto budget=plus(exact(radius),times(exact(radius),exact(relativeRadiusBudget)));
        if(!valid(budget)||budget.lo<=0||budget.lo>std::numeric_limits<float>::max())return {};
        float maximum=static_cast<float>(budget.lo);
        if(double(maximum)>budget.lo)maximum=std::nextafter(maximum,0.f);
        const auto local=coverCapsule(shape,relativeRadiusBudget*.5f);
        const auto frame=nativeHullTransformEnvelope(body.hulls[i].pose);
        if(!local.valid||!frame.valid)return {};
        auto world=placeAffineEnvelope(local,frame.matrix,maximum);
        if(!world.valid)return {};
        result.hulls[i]=world;
        ++result.hullCount;
        result.queryCount+=world.count;
    }
    result.valid=true;
    return result;
}
} // namespace ss2vr::roomscale
