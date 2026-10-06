// Portable geometry/forwarding checks. The native Core TOI is NOT executed.
#include "common/roomscale_triangle_query.hpp"
#include <bit>
#include <cstdint>
#include <cstdlib>
#include <iostream>

using namespace ss2vr::roomscale;
namespace {
unsigned checks=0;
void check(bool ok,const char* expression,int line) {
    ++checks;
    if (!ok) { std::cerr<<"line "<<line<<": "<<expression<<'\n'; std::exit(1); }
}
#define CHECK(x) check(bool(x),#x,__LINE__)
bool near(double a,double b,double epsilon=1e-12) { return std::abs(a-b)<=epsilon; }
constexpr Vector A{0,0,0},B{4,0,0},C{0,4,0},N{0,0,1};
QueryScope scope(double maximum=4,double budget=1e-9) { return {maximum,budget,false,false}; }
Plan plan(QueryScope& s,Ray ray,float radius=1) { return classify(s,ray,A,B,C,N,radius); }
void encloses(const Plan& p,double distance) {
    CHECK(p.distanceLower<=distance);
    CHECK(p.distanceUpper>=distance);
}
void projectionChecks() {
    struct Case { Vector origin; DoubleVector q; double distance; };
    const Case cases[]={{{1,1,2},{1,1,0},2},{{2,-1,1},{2,0,0},std::sqrt(2.)},
        {{-1,-1,1},{0,0,0},std::sqrt(3.)},{{3,3,1},{2,2,0},std::sqrt(3.)},
        {{5,-1,1},{4,0,0},std::sqrt(3.)},{{-1,5,1},{0,4,0},std::sqrt(3.)}};
    for (const auto& c:cases) {
        auto s=scope(); auto p=plan(s,{c.origin,{0,0,-1}},0.5f);
        CHECK(!s.failed&&p.decision==Decision::nativeToi);
        CHECK(near(p.closestFromA.x,c.q.x)); CHECK(near(p.closestFromA.y,c.q.y));
        CHECK(near(p.closestFromA.z,c.q.z)); encloses(p,c.distance);
    }
    auto s=scope();
    auto p=classify(s,{{101,-19,9},{0,0,-1}},{100,-20,7},{104,-20,7},{100,-16,7},N,1);
    CHECK(p.decision==Decision::nativeToi&&!s.failed);
    CHECK(near(p.closestFromA.x,1)&&near(p.closestFromA.y,1)); encloses(p,2);
}
void contactChecks() {
    auto s=scope(); auto tangent=plan(s,{{1,1,1},{1,0,0}});
    CHECK(tangent.decision==Decision::ignoreNondeepening&&!s.failed);
    CHECK(tangent.travelDistanceLower>=tangent.distanceUpper-s.contactDepthBudget);
    // Face -> edge BC -> vertex B. Independent distances for this right
    // triangle, sampled across both transitions, never fall below start=1.
    for (int i=0;i<=120;++i) {
        const double x=1+i/20.0,y=1;
        double distance=1;
        if (x+y>4&&x-y<4) distance=std::sqrt(1+(x+y-4)*(x+y-4)/2);
        else if (x-y>=4) distance=std::sqrt(1+(x-4)*(x-4)+y*y);
        CHECK(distance>=1);
    }
    s=scope(); CHECK(plan(s,{{1,1,1},{0,0,1}}).decision==Decision::ignoreNondeepening);
    s=scope(); CHECK(plan(s,{{1,1,1},{0,0,-1}}).decision==Decision::block&&!s.failed);
    s=scope(); CHECK(plan(s,{{1,1,0.75f},{0,0,1}}).decision==Decision::ignoreNondeepening);
    s=scope(); CHECK(plan(s,{{1,1,0.75f},{0,0,-1}}).decision==Decision::block);
    // Inside a sphere shell is supported; a center ON the triangle has no
    // separating axis and must invalidate, including an apparently escaping ray.
    s=scope(); CHECK(plan(s,{{1,1,0},{0,0,1}}).decision==Decision::invalid&&s.failed);
    s=scope(4,0); CHECK(plan(s,{{1,1,1},{1,0,0}}).decision==Decision::invalid&&s.failed);

    // Vertex -> edge AB, moving upward quickly enough that the initial
    // derivative is positive: (-1,-1,1) dot (1,0,2) = +1.
    s=scope(); auto transition=plan(s,{{-1,-1,1},{1,0,2}},2);
    CHECK(transition.decision==Decision::ignoreNondeepening);
    for (int i=0;i<=80;++i) {
        double t=i/20.0,x=-1+t,z=1+2*t;
        double distance=std::sqrt((x<0?x*x:0)+1+z*z);
        CHECK(distance>=std::sqrt(3.));
        CHECK(distance>=transition.travelDistanceLower);
    }
    // Same native model may contain BOTH triangles: skipping floor contact
    // cannot suppress a wall. Each decision is made without hull identity.
    s=scope(); CHECK(plan(s,{{1,1,1},{1,0,0}}).decision==Decision::ignoreNondeepening);
    auto wall=classify(s,{{1,1,1},{1,0,0}},{3,0,0},{3,4,0},{3,0,4},{1,0,0},1);
    CHECK(wall.decision==Decision::nativeToi&&!s.failed);
    wall=classify(s,{{2,1,1},{1,0,0}},{3,0,0},{3,4,0},{3,0,4},{1,0,0},1);
    CHECK(wall.decision==Decision::block&&!s.failed);
}
void parameterAndBudgetChecks() {
    auto a=scope(2),b=scope(1);
    auto pa=plan(a,{{1,1,1},{1,0,0}}),pb=plan(b,{{1,1,1},{2,0,0}});
    CHECK(pa.decision==Decision::ignoreNondeepening&&pb.decision==pa.decision);
    CHECK(near(pa.travelDistanceLower,pb.travelDistanceLower));
    // An almost tangent derivative straddles zero because the exact products
    // cancel. A finite short request fits its budget; a huge request cannot.
    const Ray cancellation{{-1,-1,1},{1,-1,0}};
    a=scope(1,1e-10); pa=plan(a,cancellation,2);
    CHECK(pa.decision==Decision::ignoreNondeepening);
    CHECK(pa.derivativeLower<0&&pa.derivativeUpper>0);
    CHECK(pa.travelDistanceLower>=pa.distanceUpper-a.contactDepthBudget);
    b=scope(1e15,1e-10); pb=plan(b,cancellation,2);
    CHECK(pb.decision==Decision::invalid&&b.failed);
    a=scope(1,1e-18); CHECK(plan(a,cancellation,2).decision==Decision::invalid&&a.failed);
    a=scope(); pa=plan(a,{{1,1,1},{1,0,-1e-7f}});
    CHECK(pa.decision==Decision::block); // No arbitrary tangent epsilon.
    a=scope(1e300); CHECK(plan(a,{{1,1,2},{1e30f,0,0}}).decision==Decision::invalid&&a.failed);
}
void invalidChecks() {
    const float nan=std::numeric_limits<float>::quiet_NaN(),inf=std::numeric_limits<float>::infinity();
    auto reject=[&](Ray ray,Vector a,Vector b,Vector c,Vector normal,float radius) {
        auto s=scope(); CHECK(classify(s,ray,a,b,c,normal,radius).decision==Decision::invalid&&s.failed);
        auto valid=plan(s,{{1,1,2},{0,0,-1}}); CHECK(valid.decision==Decision::invalid);
    };
    const Ray ray{{1,1,2},{0,0,-1}};
    reject(ray,A,A,C,N,1); reject(ray,A,B,{2,0,0},N,1);
    reject(ray,A,B,{2,1e-12f,0},N,1);
    reject(ray,A,B,C,{0,0,0},1); reject(ray,A,B,C,{0,0,-1},1);
    reject(ray,A,B,C,{0,0,2},1); reject(ray,A,B,C,{1,0,0},1);
    reject(ray,A,B,C,N,0); reject(ray,A,B,C,N,-1);
    reject(ray,A,B,C,N,nan); reject(ray,A,B,C,N,inf);
    reject({{nan,1,2},{0,0,-1}},A,B,C,N,1);
    reject({{1,1,2},{0,inf,-1}},A,B,C,N,1);
    reject({{1,1,2},{0,0,0}},A,B,C,N,1);
    reject(ray,{nan,0,0},B,C,N,1); reject(ray,A,B,C,{0,0,nan},1);
    for (double maximum:{0.,-1.,std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN()}) {
        auto s=scope(maximum); CHECK(plan(s,ray).decision==Decision::invalid&&s.failed);
    }
    for (double budget:{-1.,1.,std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN()}) {
        auto s=scope(1,budget); CHECK(plan(s,ray).decision==Decision::invalid&&s.failed);
    }
    const int rounding=std::fegetround(); CHECK(std::fesetround(FE_DOWNWARD)==0);
    auto s=scope(); CHECK(plan(s,ray).decision==Decision::invalid&&s.failed);
    CHECK(std::fesetround(rounding)==0);
#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
    unsigned short control=0; __asm__ volatile("fnstcw %0":"=m"(control));
    const unsigned short reduced=control&~0x300;
    __asm__ volatile("fldcw %0"::"m"(reduced));
    const bool rejected=!arithmeticSupported();
    __asm__ volatile("fldcw %0"::"m"(control)); CHECK(rejected);
#if defined(__SSE__)
    unsigned int original=0; __asm__ volatile("stmxcsr %0":"=m"(original));
    const unsigned int flushed=original|0x8000;
    __asm__ volatile("ldmxcsr %0"::"m"(flushed));
    const bool rejectedFlush=!arithmeticSupported();
    __asm__ volatile("ldmxcsr %0"::"m"(original)); CHECK(rejectedFlush);
#endif
#endif
}
void forwardingChecks() {
    const Vector a{0,-2,0},b{0,2,0},c{0,0,4},normal{1,0,0};
    const Ray ray{{0.125f,0,-1.125f},{0.6f,0,0.8f}};
    // Independent geometry: closest feature is interior AB (y=0); solve
    // (1/8+3t/5)^2+(-9/8+4t/5)^2=1. The initial/end rational
    // distances are 41/32 and 149/160. This is not a mock native proof.
    const long double linear=-33.L/20,constant=9.L/32;
    const double ideal=static_cast<double>((-linear-std::sqrt(linear*linear-4*constant))/2);
    CHECK(near(ideal,0.19303876701177303)); CHECK(41./32>1&&149./160<1);
    CHECK(ideal>0&&ideal<0.25);
    const double dx=ray.direction.x,dz=ray.direction.z;
    const double quadratic=dx*dx+dz*dz,coefficient=2*(0.125*dx-1.125*dz);
    const double actual=(-coefficient-std::sqrt(coefficient*coefficient-4*quadratic*(9./32)))/(2*quadratic);
    CHECK(near(actual,ideal,1e-7));
    CHECK(near(std::hypot(0.125+dx*actual,-1.125+dz*actual),1));
    CHECK(-1.125+dz*actual<0); // Beyond the face, beside the finite edge.
    CHECK(std::sqrt(1+4.)>1); // Both AB endpoints are farther away.
    unsigned calls=0;
    auto original=[&](const Ray& r,const Vector& aa,const Vector& bb,const Vector& cc,const Vector& n,float radius) {
        ++calls; CHECK(&r==&ray&&&aa==&a); CHECK(&bb==&c&&&cc==&b);
        CHECK(n.x==-1&&n.y==0&&n.z==0&&radius==1);
        CHECK(r.direction.x==ray.direction.x&&r.direction.z==ray.direction.z);
        return static_cast<float>(actual); // Forwarding witness only.
    };
    auto s=scope(.25); const float t=dispatch(&s,original,ray,a,b,c,normal,1);
    CHECK(calls==1&&!s.failed&&t==static_cast<float>(actual));
    s=scope(.25); auto reversed=classify(s,ray,a,c,b,{-1,0,0},1);
    CHECK(reversed.decision==Decision::nativeToi&&!reversed.reverseWinding);
    CHECK(near(reversed.distanceLower,std::sqrt(41./32)));
    s=scope(.25); CHECK(classify(s,ray,a,b,c,normal,1).reverseWinding);
    const Vector reversedNormal{-1,0,0};
    auto reversedOriginal=[&](const Ray& r,const Vector& aa,const Vector& bb,const Vector& cc,const Vector& n,float radius) {
        CHECK(&r==&ray&&&aa==&a&&&bb==&c&&&cc==&b&&&n==&reversedNormal&&radius==1);
        return static_cast<float>(actual);
    };
    CHECK(dispatch(&s,reversedOriginal,ray,a,c,b,reversedNormal,1)==t);
    const float payload=std::bit_cast<float>(std::uint32_t{0x7fc12345});
    auto unchanged=[&](const Ray& r,const Vector& aa,const Vector& bb,const Vector& cc,const Vector& n,float radius) {
        CHECK(&r==&ray&&&aa==&a&&&bb==&b&&&cc==&c&&&n==&normal);
        CHECK(std::bit_cast<std::uint32_t>(radius)==0x7fc12345); return -0.0f;
    };
    CHECK(std::signbit(dispatch(nullptr,unchanged,ray,a,b,c,normal,payload)));
    for (float returned:{NativeMiss,2.f,-1.f,std::numeric_limits<float>::infinity(),payload}) {
        s=scope(.25); auto stub=[&](auto&,...){ return returned; };
        const float out=dispatch(&s,stub,ray,a,b,c,normal,1);
        if (!std::isfinite(returned)||returned<0) CHECK(s.failed&&out==0);
        else CHECK(!s.failed&&out==returned); // No normalization/clamping of t.
    }
    auto never=[&](auto&,...){ CHECK(false); return 0.f; };
    s=scope(); const Ray tangent{{1,1,1},{1,0,0}};
    CHECK(dispatch(&s,never,tangent,A,B,C,N,1)==NativeMiss&&!s.failed);
    CHECK(std::bit_cast<std::uint32_t>(NativeMiss)==0x7f61b1e6);
    s=scope(); CHECK(dispatch(&s,never,{{1,1,1},{0,0,-1}},A,B,C,N,1)==0&&!s.failed);
    s=scope(); CHECK(dispatch(&s,never,{{1,1,0},{0,0,1}},A,B,C,N,1)==0&&s.failed);
}
void facingCancellationChecks() {
    constexpr float unit=0.577350269f;
    const Vector a{0,0,0},b{4,-4,0},c{4,0,-4},normal{unit,unit,unit};
    // Initial closest edge AB distance sqrt(1.5)>1; at t=.5 the distance
    // falls below1. The tiny X term survives native Y/Z-first cancellation.
    CHECK(std::sqrt(1.5)>1&&std::sqrt(.375)<1);
    auto cases=[&] {
        for (float x:{0x1p-80f,-0x1p-80f}) {
            const Ray ray{{1,-2,1},{x,1,-1}};
            auto s=scope(1); auto p=classify(s,ray,a,b,c,normal,1);
            CHECK(p.decision==Decision::nativeToi&&!s.failed);
            CHECK(p.reverseWinding==(x>0));
            unsigned calls=0;
            auto original=[&](const Ray& r,const Vector& aa,const Vector& bb,const Vector& cc,
                              const Vector& n,float radius) {
                ++calls; CHECK(&r==&ray&&&aa==&a&&radius==1);
                CHECK(&bb==(x>0?&c:&b)&&&cc==(x>0?&b:&c));
                CHECK(n.x==(x>0?-unit:unit)&&n.y==n.x&&n.z==n.x);
                return .25f; // Only verifies actual forwarding decisions.
            };
            CHECK(dispatch(&s,original,ray,a,b,c,normal,1)==.25f&&!s.failed&&calls==1);
        }
        // An unresolved sign is explicitly failed, never an unflagged miss.
        auto s=scope(1);
        CHECK(classify(s,{{1,-2,1},{1,-.5f,-.5f}},a,b,c,normal,1).decision==Decision::invalid&&s.failed);
    };
#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
    unsigned short saved=0;
    __asm__ volatile("fnstcw %0":"=m"(saved));
    for (unsigned short precision:{0x200,0x300}) {
        const unsigned short control=(saved&~0x300u)|precision;
        __asm__ volatile("fldcw %0"::"m"(control));
        cases();
    }
    __asm__ volatile("fldcw %0"::"m"(saved));
#else
    cases();
#endif
}
} // namespace
int main() {
    CHECK(arithmeticSupported());
    projectionChecks(); contactChecks(); parameterAndBudgetChecks(); invalidChecks(); forwardingChecks();
    facingCancellationChecks();
    unsigned strictCalls=0;
    auto miss=[&](const Ray&,const Vector&,const Vector&,const Vector&,const Vector&,float) {
        ++strictCalls;return NativeMiss;
    };
    QueryScope strictFree{1,1e-6,false,true};
    CHECK(dispatch(&strictFree,miss,{{1,1,3},{0,0,-1}},A,B,C,N,.5f)==NativeMiss);
    CHECK(!strictFree.failed&&strictCalls==1);
    QueryScope strictCrossing{4,1e-6,false,true};
    dispatch(&strictCrossing,miss,{{1,1,3},{0,0,-1}},A,B,C,N,.5f);
    CHECK(strictCrossing.failed&&strictCalls==1); // A hypothetical native miss cannot clear a crossing.

    for(unsigned k=1;k<=200;++k) {
        const double travel=double(k)/50;
        QueryScope whole{travel,1e-6,false,true};
        const auto proof=classify(whole,{{1,1,3},{0,0,-1}},A,B,C,N,.5f);
        if(travel<2.5) CHECK(proof.decision==Decision::nativeToi&&!whole.failed);
        else CHECK(whole.failed&&proof.decision==Decision::invalid);
    }

    std::cout<<checks<<" portable triangle checks passed; native TOI not executed\n";
}
