#include "common/roomscale_primitive_query.hpp"
#include "common/optional_query.hpp"
#include <cassert>
#include <limits>
#include <random>
using namespace ss2vr::roomscale;
int main() {
    const Primitive sphere{0,4,0,0},box{1,4,4,4},capsule{2,4,8,0},cylinder{3,4,4,0};
    auto classify=[](Primitive shape,Vector p,Vector d,double t=1.) {
        QueryScope scope{t,1e-6,false,false};
        return classifyPrimitive(scope,{p,d},shape,1);
    };
    for(auto shape:{sphere,box,capsule,cylinder}) {
        assert(classify(shape,{4,0,0},{-1,0,0}).decision==Decision::nativeToi);
        assert(classify(shape,{3,0,0},{0,0,1}).decision==Decision::ignoreNondeepening);
        assert(classify(shape,{2.5f,0,0},{1,0,0}).decision==Decision::ignoreNondeepening);
        assert(classify(shape,{2.5f,0,0},{-1,0,0}).decision==Decision::invalid);
    }
    // Deep initial sphere overlap used to produce a negative entry rejected by
    // the engine. It may now be skipped only with a whole-travel certificate.
    assert(classify(sphere,{0,0,0},{1,0,0}).decision==Decision::ignoreNondeepening);
    assert(classify(sphere,{.5f,0,0},{-1,0,0}).decision==Decision::invalid);
    assert(classify(box,{0,3,0},{1,0,0},100).decision==Decision::ignoreNondeepening);
    assert(classify(capsule,{3,0,0},{0,1,0},100).decision==Decision::ignoreNondeepening);
    assert(classify(cylinder,{0,3,0},{1,0,0},100).decision==Decision::ignoreNondeepening);
    // A corner path toward another face is not a reason to ignore the hull.
    assert(classify(box,{2.5f,2.5f,0},{-1,-1,0}).decision==Decision::invalid);
    assert(classify(cylinder,{2.5f,2.5f,0},{-1,-1,0}).decision==Decision::invalid);
    for(auto bad:{Primitive{4,4,4,4},Primitive{0,0,0,0},Primitive{2,4,3,0},
                  Primitive{1,4,-1,4},Primitive{3,4,0,0}}) {
        QueryScope scope{1,1e-6,false,false};
        assert(classifyPrimitive(scope,{{3,0,0},{1,0,0}},bad,1).decision==Decision::invalid&&scope.failed);
    }
    QueryScope invalid{1,1e-6,false,false};
    assert(classifyPrimitive(invalid,{{3,0,0},{0,0,0}},sphere,1).decision==Decision::invalid&&invalid.failed);
    QueryScope nan{1,1e-6,false,false};
    assert(classifyPrimitive(nan,{{std::numeric_limits<float>::quiet_NaN(),0,0},{1,0,0}},sphere,1).decision==Decision::invalid&&nan.failed);

    unsigned calls=0;
    PrimitiveInterval out{};
    auto original=[&](PrimitiveInterval* result,const Ray&,const Primitive&,float) {
        ++calls;*result={2,4};return result;
    };
    const Ray separated{{4,0,0},{-1,0,0}},tangent{{3,0,0},{0,0,1}};
    assert(dispatchPrimitive(nullptr,original,&out,separated,sphere,1)==&out&&calls==1&&out.entry==2);
    QueryScope good{1,1e-6,false,false};
    assert(dispatchPrimitive(&good,original,&out,separated,sphere,1)==&out&&calls==2&&!good.failed);
    QueryScope skip{1,1e-6,false,false};
    assert(dispatchPrimitive(&skip,original,&out,tangent,sphere,1)==&out&&calls==2&&!skip.failed&&out.entry>out.exit);
    QueryScope reject{1,1e-6,false,false};
    assert(dispatchPrimitive(&reject,original,&out,{{2.5f,0,0},{-1,0,0}},sphere,1)==&out&&calls==2&&reject.failed);
    QueryScope negative{1,1e-6,false,false};
    auto overlap=[](PrimitiveInterval* result,const Ray&,const Primitive&,float) { *result={-2,4};return result; };
    dispatchPrimitive(&negative,overlap,&out,separated,box,1);
    assert(negative.failed&&out.entry>out.exit);

    // Independent long-double signed distances check every admitted skip along
    // sampled finite paths. This is a regression oracle, not a native TOI proof.
    auto distance=[](Primitive shape,long double x,long double y,long double z) {
        const long double r=static_cast<long double>(shape.width)/2;
        if(shape.kind==0) return std::sqrt(x*x+y*y+z*z)-r;
        if(shape.kind==2) {
            const long double spine=static_cast<long double>(shape.height)/2-r;
            y-=std::clamp(y,-spine,spine);
            return std::sqrt(x*x+y*y+z*z)-r;
        }
        if(shape.kind==1) {
            const long double a=std::abs(x)-r,b=std::abs(y)-shape.height/2.L,c=std::abs(z)-shape.depth/2.L;
            const long double aa=std::max(0.L,a),bb=std::max(0.L,b),cc=std::max(0.L,c);
            return std::sqrt(aa*aa+bb*bb+cc*cc)+std::min(0.L,std::max({a,b,c}));
        }
        const long double a=std::sqrt(x*x+z*z)-r,b=std::abs(y)-shape.height/2.L;
        const long double aa=std::max(0.L,a),bb=std::max(0.L,b);
        return std::sqrt(aa*aa+bb*bb)+std::min(0.L,std::max(a,b));
    };
    std::mt19937 random(0x52564d);
    std::uniform_real_distribution<float> position(-5,5),direction(-2,2);
    unsigned skipped=0,certified=0;
    for(auto shape:{sphere,box,capsule,cylinder}) for(unsigned trial=0;trial<2000;++trial) {
        const Vector p{position(random),position(random),position(random)};
        const Vector d{direction(random),direction(random),direction(random)};
        QueryScope scope{2,1e-6,false,false};
        const auto plan=classifyPrimitive(scope,{p,d},shape,1);
        QueryScope whole{2,1e-6,false,true};
        const auto proof=classifyPrimitive(whole,{p,d},shape,1);
        if(proof.decision==Decision::nativeToi) {
            ++certified;
            for(unsigned sample=0;sample<=64;++sample) {
                const long double t=2.L*sample/64;
                assert(distance(shape,p.x+t*d.x,p.y+t*d.y,p.z+t*d.z)>1.L);
            }
        }
        if(plan.decision!=Decision::ignoreNondeepening) continue;
        ++skipped;
        const auto start=distance(shape,p.x,p.y,p.z);
        for(unsigned sample=0;sample<=64;++sample) {
            const long double t=2.L*sample/64;
            const auto at=distance(shape,p.x+t*d.x,p.y+t*d.y,p.z+t*d.z);
            assert(at>=start-1.00001e-6L);
        }
    }
    assert(skipped>100&&certified>100);
    const int previous=std::fegetround();
    assert(std::fesetround(FE_DOWNWARD)==0);
    QueryScope rounding{1,1e-6,false,false};
    assert(classifyPrimitive(rounding,tangent,sphere,1).decision==Decision::invalid&&rounding.failed);
    assert(std::fesetround(previous)==0);

    QueryScope resourceCancelled{1,1e-6,false,false};
    assert(ss2vr::optionalQueryDecision(&resourceCancelled.failed,true,0)==ss2vr::OptionalQueryDecision::cancel);
    const unsigned before=calls;
    dispatchPrimitive(&resourceCancelled,original,&out,separated,sphere,1);
    assert(resourceCancelled.failed&&calls==before&&out.entry>out.exit);
    unsigned strictCalls=0;
    auto falseMiss=[&](PrimitiveInterval* output,const Ray&,const Primitive&,float) {
        ++strictCalls;*output={NativeMiss,-NativeMiss};return output;
    };
    QueryScope strictFree{1,1e-6,false,true};
    dispatchPrimitive(&strictFree,falseMiss,&out,{{5,0,0},{-1,0,0}},sphere,1);
    assert(!strictFree.failed&&strictCalls==1);
    QueryScope strictCrossing{5,1e-6,false,true};
    dispatchPrimitive(&strictCrossing,falseMiss,&out,{{5,0,0},{-1,0,0}},sphere,1);
    assert(strictCrossing.failed&&strictCalls==1&&out.entry>out.exit);

}
