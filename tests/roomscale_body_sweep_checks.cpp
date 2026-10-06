#include "common/roomscale_body_sweep.hpp"
#include "support/roomscale_child_oracle.hpp"
#include <cassert>
#include <random>
using namespace ss2vr;
using namespace ss2vr::roomscale;
namespace {
Pose child(Pose root,Pose relative) {
    const auto value=ss2vr_test::oracle(root,relative,{});
    return {{float(value[0]),float(value[1]),float(value[2]),float(value[3])},
            {float(value[4]),float(value[5]),float(value[6])}};
}
bool contains(const CapsuleCover& cover,Vector direction,float length,double t,Vec3 point) {
    for(unsigned i=0;i<cover.count;++i) {
        const auto& s=cover.sphere[i];
        const double x=point.x-(double(s.centre.x)+double(direction.x)*length*t);
        const double y=point.y-(double(s.centre.y)+double(direction.y)*length*t);
        const double z=point.z-(double(s.centre.z)+double(direction.z)*length*t);
        if(x*x+y*y+z*z<=double(s.radius)*s.radius)return true;
    }
    return false;
}
Vec3 rawTransform(Pose pose,Vec3 p) {
    // Wide independent raw-quaternion transform; no normalize(Pose) helper.
    const double x=pose.q.x,y=pose.q.y,z=pose.q.z,w=pose.q.w;
    return {float((1-2*(y*y+z*z))*p.x+2*(x*y-z*w)*p.y+2*(x*z+y*w)*p.z+pose.p.x),
            float(2*(x*y+z*w)*p.x+(1-2*(x*x+z*z))*p.y+2*(y*z-x*w)*p.z+pose.p.y),
            float(2*(x*z-y*w)*p.x+2*(y*z+x*w)*p.y+(1-2*(x*x+y*y))*p.z+pose.p.z)};
}
}
int main() {
    std::mt19937 rng(88172);std::uniform_real_distribution<float> angle(-Pi,Pi),unit(-1,1);
    for(unsigned trial=0;trial<200;++trial) {
        BodyGeometry body;body.hullCount=trial%2+1;
        body.rootPose={yaw(angle(rng)),{unit(rng)*50,unit(rng)*10,unit(rng)*50}};
        for(unsigned h=0;h<body.hullCount;++h) {
            auto& hull=body.hulls[h];
            hull.relativePose={h?Quat{.70710677f,0,0,.70710677f}:Quat{0,0,0,1},{0,1.1f,0}};
            hull.primitive={2,1,2.2f,0};hull.pose=child(body.rootPose,hull.relativePose);
        }
        Pose target=body.rootPose;target.p.x+=.025f;target.p.z-=.01f;
        if(trial%3==0)target.q=multiply(yaw(.0001f),target.q);
        const auto cover=coverBodyRootSweep(body,target,.04f);assert(cover.valid);
        assert(cover.body.hullCount==body.hullCount && cover.body.queryCount<=68);
        for(unsigned step=0;step<=4;++step) {
            const double t=step*.25;Pose root=body.rootPose;
            root.q={float(root.q.x+(double(target.q.x)-root.q.x)*t),
                    float(root.q.y+(double(target.q.y)-root.q.y)*t),
                    float(root.q.z+(double(target.q.z)-root.q.z)*t),
                    float(root.q.w+(double(target.q.w)-root.q.w)*t)};
            root.p.x=float(double(root.p.x)+(double(target.p.x)-root.p.x)*t);
            root.p.z=float(double(root.p.z)+(double(target.p.z)-root.p.z)*t);
            for(unsigned h=0;h<body.hullCount;++h) {
                const auto pose=child(root,body.hulls[h].relativePose);
                for(unsigned sample=0;sample<100;++sample) {
                    const float azimuth=angle(rng),vertical=unit(rng);
                    const float r=.5f*std::sqrt(1-vertical*vertical);
                    const Vec3 local{r*std::cos(azimuth),.5f*vertical+(vertical<0?-.6f:.6f),r*std::sin(azimuth)};
                    assert(contains(cover.body.hulls[h],cover.direction,cover.maximumParameter,t,rawTransform(pose,local)));
                }
            }
        }
        auto invalid=body;invalid.hulls[body.hullCount-1].relativePose.q={0,0,0,0};
        assert(!coverBodyRootSweep(invalid,target,.04f).valid);
        assert(!coverBodyRootSweep(body,target,0).valid);
        assert(!coverBodyRootSweep(body,body.rootPose,.04f).valid);
        Pose nonfinite=target;nonfinite.p.x=NAN;
        assert(!coverBodyRootSweep(body,nonfinite,.04f).valid);
        auto oversized=body;oversized.hullCount=3;
        assert(!coverBodyRootSweep(oversized,target,.04f).valid);
    }
    BodyGeometry remote;remote.hullCount=1;
    remote.rootPose={{0,0,0,1},{1e8f,0,0}};
    remote.hulls[0].relativePose={{0,0,0,1},{0,1.1f,0}};
    remote.hulls[0].primitive={2,1,2.2f,0};
    remote.hulls[0].pose=child(remote.rootPose,remote.hulls[0].relativePose);
    auto destination=remote.rootPose;destination.p.x+=8;
    // World-grid uncertainty must not consume an arbitrarily enlarged radius.
    assert(!coverBodyRootSweep(remote,destination,.04f).valid);

}
