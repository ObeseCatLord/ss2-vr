#include "common/roomscale_capsule_cover.hpp"
#include <cassert>
#include <limits>
using namespace ss2vr::roomscale;
int main() {
    for (float width:{.01f,.2f,.6f,2.f,1000.f}) for (float ratio:{1.f,1.5f,3.f,5.f}) {
        Primitive shape{2,width,width*ratio,0};
        const auto cover=coverCapsule(shape,.02f);
        assert(cover.valid&&cover.count>=1&&cover.count<=34);
        const long double r=static_cast<long double>(shape.width)/2;
        const long double extent=static_cast<long double>(shape.height)/2-r;
        for(unsigned s=0;s<=1024;++s) {
            const long double y=-extent-r+2*(extent+r)*s/1024;
            const long double cap=std::max(0.L,std::abs(y)-extent);
            const long double radial2=std::max(0.L,r*r-cap*cap);
            bool covered=false;
            for(unsigned i=0;i<cover.count;++i) {
                const auto sphere=cover.sphere[i];
                assert(sphere.centre.x==0&&sphere.centre.z==0&&sphere.radius>0);
                assert(sphere.radius<=r*(1+static_cast<long double>(.02f)));
                const long double dy=y-sphere.centre.y;
                covered |= dy*dy+radial2<=static_cast<long double>(sphere.radius)*sphere.radius;
            }
            assert(covered);
        }
    }
    assert(coverCapsule({2,2,2,0},0).valid); // Exact spherical capsule, one sphere.
    assert(!coverCapsule({2,2,4,0},0).valid); // Finite cells need radial allowance.
    assert(!coverCapsule({2,.01f,1000,0},.01f).valid); // Never exceed bounded work.
    for(auto bad:{Primitive{0,2,2,0},Primitive{2,-2,2,0},Primitive{2,2,1,0}})
        assert(!coverCapsule(bad,.02f).valid);
    assert(!coverCapsule({2,2,4,0},-1).valid);
    assert(!coverCapsule({2,2,4,0},std::numeric_limits<float>::quiet_NaN()).valid);
    const auto tiny=coverCapsule({2,std::numeric_limits<float>::denorm_min(),std::numeric_limits<float>::denorm_min(),0},0);
    assert(!tiny.valid); // Cannot round the radius down to zero.
    const auto spherical=coverCapsule({2,2,2,0},0);
    const auto placedSphere=placeUprightCover(spherical,{1,1e9f,3},1);
    assert(placedSphere.valid&&placedSphere.sphere[0].radius==1);
    const Primitive avatar{2,.6f,1.8f,0};
    const auto local=coverCapsule(avatar,.02f);
    for(float origin:{0.f,100.f,-100.f,100000.f,-100000.f}) {
        const auto world=placeUprightCover(local,{19,origin,-27},.32f);
        assert(world.valid&&world.count==local.count);
        for(unsigned i=0;i<world.count;++i) {
            const long double actualY=static_cast<long double>(origin)+local.sphere[i].centre.y;
            const long double displacement=std::abs(actualY-world.sphere[i].centre.y);
            assert(displacement+local.sphere[i].radius<=world.sphere[i].radius);
            assert(world.sphere[i].centre.x==19&&world.sphere[i].centre.z==-27);
        }
    }
    // World-grid rounding is not allowed to spend an unapproved radius budget.
    assert(!placeUprightCover(local,{0,10000000.f,0},.32f).valid);
    auto malformed=local;malformed.count=35;
    assert(!placeUprightCover(malformed,{0,0,0},1).valid);
    malformed=local;malformed.sphere[0].centre.x=1;
    assert(!placeUprightCover(malformed,{0,0,0},1).valid);
    assert(!placeUprightCover(local,{0,std::numeric_limits<float>::infinity(),0},1).valid);
    assert(!placeUprightCover(local,{0,0,0},0).valid);

}
