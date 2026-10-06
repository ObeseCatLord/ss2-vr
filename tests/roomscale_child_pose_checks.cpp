#include "common/roomscale_child_pose.hpp"
#include <cassert>
#include <random>
using namespace ss2vr;
using namespace ss2vr::roomscale;
#include "support/roomscale_child_oracle.hpp"
using namespace ss2vr_test;
namespace {
void check(Pose p,Pose r) {
    const auto bounds=nativeChildPoseEnvelope(p,r);assert(bounds.valid);
    for(auto arithmetic: {Arithmetic{-1,false},Arithmetic{0,false},Arithmetic{1,false},Arithmetic{0,true}}) {
        const auto values=oracle(p,r,arithmetic);
        for(unsigned i=0;i<7;++i) {
            const auto interval=i<4?bounds.q[i]:bounds.p[i-4];
            assert(interval.lo<=values[i] && interval.hi>=values[i]);
        }
    }
}
}
int main() {
    check(Pose{{0,0,0,1},{1,2,3}},Pose{{0,0,0,1},{0,1.1f,0}});
    check(Pose{{0,.70710677f,0,.70710677f},{10000,0,-10000}},
          Pose{{.70710677f,0,0,.70710677f},{0,1.1f,0}});
    std::mt19937 gen(17291);std::uniform_real_distribution<float> unit(-1,1),world(-20000,20000);
    for(unsigned sample=0;sample<3000;++sample) {
        Pose p{normalize({unit(gen),unit(gen),unit(gen),unit(gen)}),{world(gen),world(gen),world(gen)}};
        Pose r{normalize({unit(gen),unit(gen),unit(gen),unit(gen)}),{unit(gen)*2,unit(gen)*2,unit(gen)*2}};
        check(p,r);
    }
    check(Pose{{0,0,0,1},{1e-40f,0,0}},Pose{{0,0,0,1},{-1e-40f,0,0}});
    Pose bad{{0,0,0,0},{0,0,0}},good{{0,0,0,1},{0,0,0}};
    assert(!nativeChildPoseEnvelope(bad,good).valid);
    good.p.x=INFINITY;assert(!nativeChildPoseEnvelope(Pose{},good).valid);
    assert(!detail::valid(child_pose_detail::rounded({-INFINITY,INFINITY})));
    assert(!detail::valid(child_pose_detail::rounded({double(std::numeric_limits<float>::max())*2,double(std::numeric_limits<float>::max())*2})));
}
