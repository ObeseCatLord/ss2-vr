#pragma once
#include "roomscale_body_geometry.hpp"

namespace ss2vr::roomscale {
struct ChildPoseEnvelope {
    std::array<detail::Interval,4> q{};
    detail::IntervalVector p{};
    bool valid=false;
};
namespace child_pose_detail {
using detail::Interval;
// Enclose either a binary32 store or an x87 operation with 24/53/64-bit
// significand precision, under any rounding direction. The enclosing float
// grid is deliberately wider than the extended-exponent x87 value. Overflow
// refuses the envelope instead of certifying an unbounded placement.
inline Interval rounded(Interval value) {
    if(!detail::valid(value)||value.lo < -std::numeric_limits<float>::max()||
       value.hi > std::numeric_limits<float>::max()) return {NAN,NAN};
    volatile float lower=static_cast<float>(value.lo),upper=static_cast<float>(value.hi);
    const float lo=std::nextafter(float(lower),-std::numeric_limits<float>::infinity());
    const float hi=std::nextafter(float(upper),std::numeric_limits<float>::infinity());
    return {double(lo),double(hi)};
}
inline Interval add(Interval a,Interval b) { return rounded(detail::plus(a,b)); }
inline Interval sub(Interval a,Interval b) { return rounded(detail::minus(a,b)); }
inline Interval mul(Interval a,Interval b) { return rounded(detail::times(a,b)); }
inline Interval twice(Interval a) { return add(a,a); }
} // namespace child_pose_detail

// Bound the pinned CAspect::OnMoved child recomposition. The native routine
// writes each child's absolute pose from the NEW parent pose and the child's
// stored relative pose. It does not just add the parent's displacement to the
// old absolute child pose. No native storage is changed here.
inline ChildPoseEnvelope childPoseFromParentBounds(
    const std::array<detail::Interval,4>& parentQ,const detail::IntervalVector& parentP,
    const Pose& relative) {
    using namespace child_pose_detail;
    using detail::exact;
    ChildPoseEnvelope result;
    if(!arithmeticSupported()||!finite(relative)||!bodyUnitQuaternion(relative.q))return result;
    for(auto v:parentQ)if(!detail::valid(v))return result;
    for(auto v:parentP)if(!detail::valid(v))return result;
    const auto x=parentQ[0],y=parentQ[1],z=parentQ[2],w=parentQ[3];
    const auto lx=exact(relative.q.x),ly=exact(relative.q.y),lz=exact(relative.q.z),lw=exact(relative.q.w);
    result.q={
        sub(add(add(mul(w,lx),mul(y,lz)),mul(x,lw)),mul(z,ly)),
        add(add(sub(mul(w,ly),mul(x,lz)),mul(z,lx)),mul(y,lw)),
        add(sub(add(mul(x,ly),mul(w,lz)),mul(y,lx)),mul(z,lw)),
        sub(sub(sub(mul(w,lw),mul(x,lx)),mul(y,ly)),mul(z,lz))};
    const auto a=twice(mul(x,x)),b=twice(mul(y,x)),c=twice(mul(z,x));
    const auto d=twice(mul(y,y)),e=twice(mul(z,y)),f=twice(mul(z,z));
    const auto g=twice(mul(w,x)),h=twice(mul(w,y)),i=twice(mul(z,w));
    const std::array<detail::IntervalVector,3> m{{
        {sub(exact(1),add(f,d)),sub(b,i),add(h,c)},
        {add(i,b),sub(exact(1),add(a,f)),sub(e,g)},
        {sub(c,h),add(g,e),sub(exact(1),add(d,a))}}};
    const auto px=exact(relative.p.x),py=exact(relative.p.y),pz=exact(relative.p.z);
    result.p={
        add(add(add(mul(m[0][2],pz),mul(m[0][0],px)),mul(m[0][1],py)),parentP[0]),
        add(add(add(mul(m[1][0],px),mul(m[1][2],pz)),mul(m[1][1],py)),parentP[1]),
        add(add(add(mul(m[2][0],px),mul(m[2][2],pz)),mul(m[2][1],py)),parentP[2])};
    for(auto q:result.q)if(!detail::valid(q))return {};
    for(auto p:result.p)if(!detail::valid(p))return {};
    result.valid=true;
    return result;
}
inline ChildPoseEnvelope nativeChildPoseEnvelope(const Pose& parent,const Pose& relative) {
    using detail::exact;
    if(!finite(parent)||!bodyUnitQuaternion(parent.q))return {};
    return childPoseFromParentBounds({exact(parent.q.x),exact(parent.q.y),exact(parent.q.z),exact(parent.q.w)},
                                    {exact(parent.p.x),exact(parent.p.y),exact(parent.p.z)},relative);
}
} // namespace ss2vr::roomscale
