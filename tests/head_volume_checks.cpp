#include "common/head_volume.hpp"
#include <cassert>
#include <random>
using namespace ss2vr;
int main() {
    roomscale::BodyGeometry body;body.hullCount=2;
    auto still=coverHeadVolumeSweep(body,{1,2,3},{1,2,3},.12f,.001f);
    assert(still.valid&&still.body.hullCount==2&&still.body.queryCount==2);
    assert(still.maximumParameter>0&&still.direction.x==1);
    assert(still.body.hulls[0].sphere[0].radius>=.12f);
    assert(!coverHeadVolumeSweep(body,{},{4,0,0},.12f,.001f).valid);
    assert(!coverHeadVolumeSweep(body,{},{NAN,0,0},.12f,.001f).valid);
    assert(!coverHeadVolumeSweep(body,{},{},0,.001f).valid);
    assert(!coverHeadVolumeSweep(body,{},{},.12f,0).valid);
    assert(!coverHeadVolumeSweep(body,{},{},.12f,.12f).valid);
    assert(!coverHeadVolumeSweep(body,{},{},INFINITY,.001f).valid);
    body.hullCount=0;assert(!coverHeadVolumeSweep(body,{},{},.12f,.001f).valid);
    body.hullCount=3;assert(!coverHeadVolumeSweep(body,{},{},.12f,.001f).valid);
    body.hullCount=2;
    Request request{};request.sequence=1;request.trackingGeneration=2;request.session=3;request.reference=4;
    request.input.sequence=5;request.input.session=3;request.input.reference=4;
    request.input.focused=request.input.headValid=1;
    Pose anchor{},head{};head.p.x=.1f;
    HeadVolumeObservation observation{anchor,head,6,2,3,4,1,5,7,8,1000,.12f,.05f,.002f,true,true};
    auto visible=[&] {return headVolumeVisible(observation,anchor,head,6,request,7,8,1050);};
    assert(visible());
    observation.clear=false;assert(!visible());observation.clear=true;
    observation.sampled=false;assert(!visible());observation.sampled=true;
    ++observation.simulationRevision;assert(!visible());--observation.simulationRevision;
    ++observation.rigRevision;assert(!visible());--observation.rigRevision;
    ++request.sequence;assert(!visible());--request.sequence;
    ++request.input.sequence;assert(!visible());--request.input.sequence;
    ++request.trackingGeneration;assert(!visible());--request.trackingGeneration;
    ++request.input.reference;assert(!visible());--request.input.reference;
    request.input.headValid=0;assert(!visible());request.input.headValid=1;
    request.input.focused=0;assert(!visible());request.input.focused=1;
    head.p.x+=.001f;assert(!visible());head=observation.head;
    anchor.p.y=.001f;assert(!visible());anchor=observation.body;
    observation.radius=NAN;assert(!visible());observation.radius=.12f;
    assert(!headVolumeVisible(observation,anchor,head,6,request,7,8,999));
    assert(!headVolumeVisible(observation,anchor,head,6,request,7,8,1101));
    assert(!headVolumeVisible(observation,anchor,head,7,request,7,8,1050));
    assert(visible());

    std::mt19937 random(15973);
    std::uniform_real_distribution<float> point(-1000,1000),step(-1,1),unit(0,1);
    for(unsigned i=0;i<10000;++i) {
        const Vec3 from{point(random),point(random),point(random)};
        const Vec3 to=from+Vec3{step(random),step(random),step(random)};
        const float radius=.05f+unit(random)*.2f;
        const auto cover=coverHeadVolumeSweep(body,from,to,radius,.001f);
        assert(cover.valid);
        const auto sphere=cover.body.hulls[0].sphere[0];
        assert(sphere.radius>=radius&&double(sphere.radius)-radius<=.001);
        assert(sphere.centre.x==from.x&&sphere.centre.y==from.y&&sphere.centre.z==from.z);
        for(unsigned sample=0;sample<=20;++sample) {
            const long double f=static_cast<long double>(sample)/20;
            const long double trueX=from.x+f*(static_cast<long double>(to.x)-from.x);
            const long double trueY=from.y+f*(static_cast<long double>(to.y)-from.y);
            const long double trueZ=from.z+f*(static_cast<long double>(to.z)-from.z);
            const long double time=f*cover.maximumParameter;
            const long double rayX=from.x+time*cover.direction.x;
            const long double rayY=from.y+time*cover.direction.y;
            const long double rayZ=from.z+time*cover.direction.z;
            const long double error=std::hypot(trueX-rayX,trueY-rayY,trueZ-rayZ);
            assert(error+radius<=sphere.radius);
        }
    }
}
