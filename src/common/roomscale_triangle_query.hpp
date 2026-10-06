#pragma once
#include <algorithm>
#include <array>
#include <cfenv>
#include <cmath>
#include <cstddef>
#include <limits>
#include <type_traits>

// A model-query experiment, NOT a body controller or a certified native sweep.
// Geometry bounds assume IEEE binary64, default round-to-nearest, no fast-math.
#if defined(__FAST_MATH__)
#error Roomscale contact bounds require IEEE arithmetic without fast-math
#endif
namespace ss2vr::roomscale {
struct Vector { float x, y, z; };
struct Ray { Vector origin, direction; };
static_assert(sizeof(Vector) == 12 && sizeof(Ray) == 24);
static_assert(offsetof(Ray, direction) == 12);
static_assert(std::numeric_limits<double>::is_iec559);
static_assert(std::numeric_limits<float>::is_iec559 && std::numeric_limits<float>::digits==24);
static_assert(std::numeric_limits<double>::digits==53 && std::numeric_limits<double>::radix==2);
// Core 1D3EF, binary32 0x7f61b1e6. Not infinity/NaN.
inline constexpr float NativeMiss = 3.0e38f;

// Caller owns this lexical value above the native-finally frame. Budget is a
// maximum additional contact depth in the SAME units as the supplied geometry.
// maxRayParameter is t in origin + t*direction; direction is never normalized.
struct QueryScope {
    double maxRayParameter;
    double contactDepthBudget;
    bool failed;
};
static_assert(std::is_trivial_v<QueryScope> && std::is_standard_layout_v<QueryScope>);
enum class Decision { nativeToi, ignoreNondeepening, block, invalid };
struct DoubleVector { double x, y, z; };
struct Plan {
    Decision decision = Decision::invalid;
    bool reverseWinding = false;
    DoubleVector closestFromA{};
    double distanceLower = 0, distanceUpper = 0;
    double derivativeLower = 0, derivativeUpper = 0;
    double travelDistanceLower = 0;
};

namespace detail {
inline DoubleVector convert(Vector a) { return {a.x, a.y, a.z}; }
inline DoubleVector add(DoubleVector a, DoubleVector b) { return {a.x+b.x,a.y+b.y,a.z+b.z}; }
inline DoubleVector sub(DoubleVector a, DoubleVector b) { return {a.x-b.x,a.y-b.y,a.z-b.z}; }
inline DoubleVector scale(DoubleVector a, double s) { return {a.x*s,a.y*s,a.z*s}; }
inline double dot(DoubleVector a, DoubleVector b) { return a.x*b.x+a.y*b.y+a.z*b.z; }
inline DoubleVector cross(DoubleVector a, DoubleVector b) {
    return {a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};
}
inline bool finite(DoubleVector a) { return std::isfinite(a.x)&&std::isfinite(a.y)&&std::isfinite(a.z); }
inline double down(double a) { return std::nextafter(a,-std::numeric_limits<double>::infinity()); }
inline double up(double a) { return std::nextafter(a,std::numeric_limits<double>::infinity()); }

// Every operation encloses its exact real result, including underflow. The
// approximate closest point proposes an axis; only these support-plane bounds
// authorize a skip. They do not assume a rounded barycentric test is exact.
struct Interval { double lo, hi; };
inline Interval exact(double a) { return {a,a}; }
inline Interval plus(Interval a, Interval b) { return {down(a.lo+b.lo),up(a.hi+b.hi)}; }
inline Interval minus(Interval a, Interval b) { return {down(a.lo-b.hi),up(a.hi-b.lo)}; }
inline Interval times(Interval a, Interval b) {
    const std::array<double,4> p{a.lo*b.lo,a.lo*b.hi,a.hi*b.lo,a.hi*b.hi};
    return {down(*std::min_element(p.begin(),p.end())),up(*std::max_element(p.begin(),p.end()))};
}
inline Interval square(Interval a) {
    double lo = a.lo<=0 && a.hi>=0 ? 0 : std::min(a.lo*a.lo,a.hi*a.hi);
    return {std::max(0.0,down(lo)),up(std::max(a.lo*a.lo,a.hi*a.hi))};
}
inline bool valid(Interval a) { return std::isfinite(a.lo)&&std::isfinite(a.hi)&&a.lo<=a.hi; }
inline Interval dividePositive(Interval a, Interval b) {
    return times(a,{down(1/b.hi),up(1/b.lo)});
}
using IntervalVector = std::array<Interval,3>;
inline IntervalVector interval(DoubleVector a) { return {exact(a.x),exact(a.y),exact(a.z)}; }
inline IntervalVector difference(IntervalVector a, IntervalVector b) {
    return {minus(a[0],b[0]),minus(a[1],b[1]),minus(a[2],b[2])};
}
inline IntervalVector sum(IntervalVector a, IntervalVector b) {
    return {plus(a[0],b[0]),plus(a[1],b[1]),plus(a[2],b[2])};
}
inline IntervalVector product(IntervalVector a, Interval s) {
    return {times(a[0],s),times(a[1],s),times(a[2],s)};
}
inline Interval inner(IntervalVector a, IntervalVector b) {
    return plus(plus(times(a[0],b[0]),times(a[1],b[1])),times(a[2],b[2]));
}
inline Interval length(IntervalVector a) {
    auto s=plus(plus(square(a[0]),square(a[1])),square(a[2]));
    return {std::max(0.0,down(std::sqrt(std::max(0.0,s.lo)))),up(std::sqrt(s.hi))};
}

// The native facing gate evaluates (y*ny + z*nz) + x*nx. Each binary32
// product is exact in binary64 and in either admitted x87 precision (53/64).
// Its exponent is also within binary64's normal range. Enclose each native
// addition's rounded result; an uncertain sign rejects rather than preserve
// the native positive-facing early miss. Exact opposite products cancel under
// both precisions, so the remaining product gives an exact sign, including
// tiny nonzero directions that a different addition order could discard.
inline bool nativeFacing(DoubleVector direction,DoubleVector normal,bool& reverse) {
    const double x=direction.x*normal.x,y=direction.y*normal.y,z=direction.z*normal.z;
    if (y==-z) { reverse=x>0; return true; }
    const auto facing=plus(plus(exact(y),exact(z)),exact(x));
    if (!valid(facing)) return false;
    if (facing.hi<=0) { reverse=false; return true; }
    if (facing.lo>0) { reverse=true; return true; }
    return false;
}

// q = (1-v)*u*(B-A) + v*(C-A), u,v in [0,1]. This is a feasible
// convex combination even when a conventional rounded u+v would exceed 1.
struct Projection { double u=0, v=0, distance2=std::numeric_limits<double>::infinity(); DoubleVector q{}; };
inline Projection project(DoubleVector ab, DoubleVector ac, DoubleVector ap, DoubleVector n, double n2) {
    Projection best;
    auto offer=[&](double u,double v) {
        u=std::clamp(u,0.0,1.0); v=std::clamp(v,0.0,1.0);
        auto q=add(scale(ab,(1-v)*u),scale(ac,v));
        auto d=sub(ap,q); double d2=dot(d,d);
        if (std::isfinite(d2)&&d2<best.distance2) best={u,v,d2,q};
    };
    offer(dot(ap,ab)/dot(ab,ab),0);
    offer(0,dot(ap,ac)/dot(ac,ac));
    auto bc=sub(ac,ab);
    offer(1,dot(sub(ap,ab),bc)/dot(bc,bc));
    const double u=dot(cross(ap,ac),n)/n2;
    const double v=dot(cross(ab,ap),n)/n2;
    if (std::isfinite(u)&&std::isfinite(v)&&u>=0&&v>=0&&u+v<=1&&v<1)
        offer(u/(1-v),v);
    return best;
}
} // namespace detail

inline bool validScope(const QueryScope& s) {
    return !s.failed && std::isfinite(s.maxRayParameter) && s.maxRayParameter>0 &&
        std::isfinite(s.contactDepthBudget) && s.contactDepthBudget>=0;
}

// Read only: never change the engine's FP environment. nextafter bounds are
// invalid under reduced x87 precision or flush-to-zero; unmasked traps also
// make even outward rounding at zero unsafe. Refuse that scope instead.
inline bool arithmeticSupported() {
    if (std::fegetround()!=FE_TONEAREST) return false;
#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
    unsigned short control=0;
    __asm__ volatile("fnstcw %0" : "=m"(control));
    if ((control&0x3f)!=0x3f || (control&0xc00)!=0 || (control&0x300)<0x200) return false;
#if defined(__SSE__)
    unsigned int mxcsr=0;
    __asm__ volatile("stmxcsr %0" : "=m"(mxcsr));
    if ((mxcsr&0x1f80)!=0x1f80 || (mxcsr&0xe040)!=0) return false;
#endif
#endif
    return true;
}

inline Plan classify(QueryScope& scope,const Ray& ray,const Vector& a,const Vector& b,
                     const Vector& c,const Vector& normal,float radius) {
    using namespace detail;
    auto invalid=[&]() { scope.failed=true; return Plan{}; };
    const auto p=convert(ray.origin),d=convert(ray.direction),aa=convert(a),bb=convert(b),cc=convert(c);
    const auto supplied=convert(normal);
    if (!arithmeticSupported()||!validScope(scope)||!finite(p)||!finite(d)||!finite(aa)||!finite(bb)||!finite(cc)||
        !finite(supplied)||!std::isfinite(radius)||radius<=0||scope.contactDepthBudget>=radius)
        return invalid();
    const auto ab=sub(bb,aa),ac=sub(cc,aa),ap=sub(p,aa),bc=sub(cc,bb),n=cross(ab,ac);
    const double edge2=std::max({dot(ab,ab),dot(ac,ac),dot(bc,bc)}),n2=dot(n,n);
    const double d2=dot(d,d),supplied2=dot(supplied,supplied);
    // Deliberately reject ill-conditioned/degenerate input, rather than add a
    // second point/segment TOI kernel. This is not a default body-size policy.
    if (!(edge2>0)||!std::isfinite(n2)||n2<=64*std::numeric_limits<double>::epsilon()*edge2*edge2||
        !(d2>0)||!std::isfinite(d2)||!finite(scale(d,scope.maxRayParameter))) return invalid();
    // Native CAE80 supplies a binary32 unit normal consistent with winding.
    // This validation is not a bound on native TOI's float error.
    constexpr double normalSlack=16*std::numeric_limits<float>::epsilon();
    const auto mismatch=cross(n,supplied);
    if (std::abs(supplied2-1)>normalSlack||dot(n,supplied)<=0||
        dot(mismatch,mismatch)>normalSlack*normalSlack*n2*supplied2) return invalid();
    auto projection=project(ab,ac,ap,n,n2);
    if (!std::isfinite(projection.distance2)||projection.distance2<=0) return invalid();
    const auto axis=sub(ap,projection.q);
    const auto ai=interval(aa),bi=interval(bb),ci=interval(cc),pi=interval(p),ni=interval(axis);
    const auto norm=length(ni);
    if (!valid(norm)||norm.lo<=0) return invalid();
    auto start=dividePositive(inner(difference(pi,ai),ni),norm);
    for (auto vertex : {bi,ci}) {
        const auto candidate=dividePositive(inner(difference(pi,vertex),ni),norm);
        if (!valid(candidate)) return invalid();
        start.lo=std::min(start.lo,candidate.lo);
    }
    const auto w=times(minus(exact(1),exact(projection.v)),exact(projection.u));
    const auto qi=sum(product(difference(bi,ai),w),product(difference(ci,ai),exact(projection.v)));
    const auto distance=length(difference(difference(pi,ai),qi));
    const auto derivative=dividePositive(inner(interval(d),ni),norm);
    if (!valid(start)||!valid(distance)||!valid(derivative)||start.lo<=0||distance.hi<start.lo)
        return invalid();
    const auto travel=plus(exact(start.lo),times({std::min(0.0,derivative.lo),0},exact(scope.maxRayParameter)));
    if (!valid(travel)) return invalid();
    Plan plan{Decision::nativeToi,false,projection.q,start.lo,distance.hi,
              derivative.lo,derivative.hi,travel.lo};
    if (start.lo>radius) { // Proven initially separated; native TOI will run.
        if (!nativeFacing(d,supplied,plan.reverseWinding)) return invalid();
        return plan;
    }
    // Bounds must locate the initial contact to within the explicit budget.
    // Otherwise an approximate projection cannot authorize a skip or disguise
    // an unknown start as a normal native miss.
    if (up(distance.hi-start.lo)>scope.contactDepthBudget) return invalid();
    if (derivative.hi<0) { plan.decision=Decision::block; return plan; }
    // For every point on the finite ray the supporting plane gives this lower
    // distance. Comparing to an UPPER start distance bounds extra penetration,
    // including projection/normal/rounding uncertainty and near-zero derivative.
    if (travel.lo>=up(distance.hi-scope.contactDepthBudget)) {
        plan.decision=Decision::ignoreNondeepening;
        return plan;
    }
    return invalid();
}

// The actual game detour and portable forwarding tests share this adapter.
// The original native TOI is still binary32 and has NO proven conservative
// numerical bound. A successful return here is not full-volume certification.
template<class Original>
inline float dispatch(QueryScope* scope,Original original,const Ray& ray,const Vector& a,
                      const Vector& b,const Vector& c,const Vector& normal,float radius) {
    if (!scope) return original(ray,a,b,c,normal,radius); // Forward without geometry checks.
    const auto plan=classify(*scope,ray,a,b,c,normal,radius);
    if (plan.decision==Decision::invalid||plan.decision==Decision::block) return 0;
    if (plan.decision==Decision::ignoreNondeepening) return NativeMiss;
    const Vector opposite{-normal.x,-normal.y,-normal.z};
    const float t=plan.reverseWinding ? original(ray,a,c,b,opposite,radius) : original(ray,a,b,c,normal,radius);
    if (!std::isfinite(t)||t<0) { scope->failed=true; return 0; }
    // Negative native entries are not reinterpreted as safe exits. Native float
    // uncertainty/degeneracy remains a gate, not a guessed epsilon correction.
    return t;
}
} // namespace ss2vr::roomscale
