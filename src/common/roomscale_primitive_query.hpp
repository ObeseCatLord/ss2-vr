#pragma once
#include "roomscale_triangle_query.hpp"
#include <cstdint>

namespace ss2vr::roomscale {
// Pinned Core mthIntersectThickRayPrimitive descriptor, not a guessed avatar
// shape. Width is diameter for sphere/capsule/cylinder, full X size for box;
// height/depth are full sizes. Type 4's exact shape is not admitted here.
struct Primitive { uint32_t kind; float width, height, depth; };
static_assert(sizeof(Primitive)==16 && offsetof(Primitive,width)==4 &&
              offsetof(Primitive,height)==8 && offsetof(Primitive,depth)==12);
struct PrimitivePlan {
    Decision decision=Decision::invalid;
    double startLower=0, startUpper=0, travelLower=0;
};
namespace primitive_detail {
using namespace detail;
inline Interval positive(Interval x) { return {std::max(0.,x.lo),std::max(0.,x.hi)}; }
inline Interval negative(Interval x) { return {std::min(0.,x.lo),std::min(0.,x.hi)}; }
inline Interval maximum(Interval a,Interval b) { return {std::max(a.lo,b.lo),std::max(a.hi,b.hi)}; }
inline Interval signedDistance(const IntervalVector& offsets) {
    IntervalVector outside{positive(offsets[0]),positive(offsets[1]),positive(offsets[2])};
    return plus(length(outside),negative(maximum(maximum(offsets[0],offsets[1]),offsets[2])));
}
inline double component(DoubleVector v,unsigned i) { return i==0?v.x:i==1?v.y:v.z; }
inline DoubleVector unit(unsigned i,double sign) { return i==0?DoubleVector{sign,0,0}:i==1?DoubleVector{0,sign,0}:DoubleVector{0,0,sign}; }
inline double sign(double x) { return x<0?-1.:1.; }
}

// Certifies only a bounded initial-contact skip against one exact convex
// primitive. A supporting plane of the WHOLE primitive bounds every point of
// the finite ray, so tangent contact cannot hide a later face of that primitive.
// Separated starts retain native TOI (whose numerical conservatism is separate).
// Deepening/uncertain starts invalidate the optional query; no fake native hit,
// hull continuation, overlap escape, body displacement or physics is invented.
inline PrimitivePlan classifyPrimitive(QueryScope& scope,const Ray& ray,
                                       const Primitive& shape,float radius) {
    using namespace detail;
    using namespace primitive_detail;
    auto invalid=[&]() { scope.failed=true; return PrimitivePlan{}; };
    const auto p=convert(ray.origin),d=convert(ray.direction);
    if (!arithmeticSupported()||!validScope(scope)||!finite(p)||!finite(d)||
        !std::isfinite(radius)||radius<=0||scope.contactDepthBudget>=radius||
        shape.kind>3||!std::isfinite(shape.width)||shape.width<=0||
        !(dot(d,d)>0)||!finite(scale(d,scope.maxRayParameter))) return invalid();
    const double r=double(shape.width)*.5;
    DoubleVector axis{};
    Interval distance{},support{};
    if (shape.kind==0 || shape.kind==2) {
        Interval spine=exact(0);
        if (shape.kind==2) {
            if (!std::isfinite(shape.height)||shape.height<shape.width) return invalid();
            spine=minus(exact(double(shape.height)*.5),exact(r));
            spine.lo=std::max(0.,spine.lo);spine.hi=std::max(0.,spine.hi);
        }
        // This point is definitely on the actual spine. Its distance supplies
        // an upper bound even when the rounded half-spine is uncertain.
        const DoubleVector point{0,std::clamp(p.y,-spine.lo,spine.lo),0};
        axis=sub(p,point);
        if (!(dot(axis,axis)>0)) axis=d; // Centre: any finite support direction.
        const auto n=interval(axis); const auto norm=length(n);
        distance=minus(length(difference(interval(p),interval(point))),exact(r));
        support=plus(times(exact(r),norm),times(spine,exact(std::abs(axis.y))));
    } else if (shape.kind==1) {
        if (!std::isfinite(shape.height)||shape.height<=0||
            !std::isfinite(shape.depth)||shape.depth<=0) return invalid();
        const DoubleVector half{r,double(shape.height)*.5,double(shape.depth)*.5};
        const DoubleVector closest{std::clamp(p.x,-half.x,half.x),
            std::clamp(p.y,-half.y,half.y),std::clamp(p.z,-half.z,half.z)};
        axis=sub(p,closest);
        IntervalVector offsets;
        for (unsigned i=0;i<3;++i)
            offsets[i]=minus(exact(std::abs(component(p,i))),exact(component(half,i)));
        distance=signedDistance(offsets);
        if (!(dot(axis,axis)>0)) {
            unsigned nearest=0;
            for (unsigned i=1;i<3;++i)
                if (std::abs(component(p,i))-component(half,i)>
                    std::abs(component(p,nearest))-component(half,nearest)) nearest=i;
            axis=unit(nearest,sign(component(p,nearest)));
        }
        support=inner(interval({std::abs(axis.x),std::abs(axis.y),std::abs(axis.z)}),interval(half));
    } else {
        if (!std::isfinite(shape.height)||shape.height<=0) return invalid();
        const double half=double(shape.height)*.5;
        const auto radial=length(interval({p.x,0,p.z}));
        const auto dr=minus(radial,exact(r)),dy=minus(exact(std::abs(p.y)),exact(half));
        distance=plus(length({positive(dr),positive(dy),exact(0)}),negative(maximum(dr,dy)));
        const double lengthXZ=std::hypot(p.x,p.z);
        const DoubleVector radialAxis=lengthXZ>0?DoubleVector{p.x/lengthXZ,0,p.z/lengthXZ}:DoubleVector{1,0,0};
        const double outsideR=std::max(0.,lengthXZ-r),outsideY=std::max(0.,std::abs(p.y)-half);
        axis=add(scale(radialAxis,outsideR),{0,sign(p.y)*outsideY,0});
        if (!(dot(axis,axis)>0))
            axis=lengthXZ-r>std::abs(p.y)-half?radialAxis:DoubleVector{0,sign(p.y),0};
        support=plus(times(exact(r),length(interval({axis.x,0,axis.z}))),
                     times(exact(half),exact(std::abs(axis.y))));
    }
    const auto n=interval(axis); const auto norm=length(n);
    if (!finite(axis)||!valid(norm)||norm.lo<=0||!valid(distance)||!valid(support)) return invalid();
    const auto separation=dividePositive(minus(inner(interval(p),n),support),norm);
    const auto derivative=dividePositive(inner(interval(d),n),norm);
    const auto travel=plus(exact(separation.lo),times({std::min(0.,derivative.lo),0},exact(scope.maxRayParameter)));
    if (!valid(separation)||!valid(derivative)||!valid(travel)||distance.hi<separation.lo) return invalid();
    PrimitivePlan plan{Decision::nativeToi,separation.lo,distance.hi,travel.lo};
    if (separation.lo>radius) return plan;
    if (up(distance.hi-separation.lo)>scope.contactDepthBudget) return invalid();
    if (travel.lo>=up(distance.hi-scope.contactDepthBudget)) {
        plan.decision=Decision::ignoreNondeepening;
        return plan;
    }
    return invalid();
}

struct PrimitiveInterval { float entry, exit; };
static_assert(sizeof(PrimitiveInterval)==8);
// A native miss is represented by an empty interval. Rejection invalidates the
// whole optional query, not just this hull. Preserve the original hidden-output
// ABI; never return a temporary pointer or invent a zero-distance hit/normal.
template<class Original>
inline PrimitiveInterval* dispatchPrimitive(QueryScope* scope,Original original,
    PrimitiveInterval* out,const Ray& ray,const Primitive& shape,float radius) {
    if (!scope) return original(out,ray,shape,radius);
    if (!out) { scope->failed=true; return nullptr; }
    const auto plan=classifyPrimitive(*scope,ray,shape,radius);
    if (plan.decision!=Decision::nativeToi) {
        *out={NativeMiss,-NativeMiss};
        return out;
    }
    auto* result=original(out,ray,shape,radius);
    if (result!=out||!std::isfinite(out->entry)||!std::isfinite(out->exit)||
        (out->entry<=0&&out->exit>=0)) {
        // Expanded native box/cylinder bounds can report an initial overlap
        // even when the exact convex support proves a separated sphere. The
        // engine's negative-entry rejection could then hide a later collision.
        scope->failed=true;
        *out={NativeMiss,-NativeMiss};
    }
    return out;
}
} // namespace ss2vr::roomscale
