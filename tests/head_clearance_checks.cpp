#include "common/head_clearance.hpp"
#include <cassert>
using namespace ss2vr;
int main() {
    Request image;image.session=1;image.reference=2;
    for(auto& f:image.fov)f={-.8f,.8f,.8f,-.8f};
    Input latest;latest.headValid=latest.focused=1;latest.session=1;latest.reference=2;
    Pose eyes[2];eyes[0].p.x=-.032f;eyes[1].p.x=.032f;
    HeadClearance c{1000,{},.3f,.1f,.05f,HeadClearanceMode::Clear,0};
    auto allows=[&]{return headClearanceAllows(c,image,latest,eyes,1050);};
    assert(validHeadClearance(c)&&allows());
    latest.head.p.x=.15f;eyes[0].p.x+=.15f;eyes[1].p.x+=.15f;assert(allows());
    latest.head.p.x=.23f;eyes[0].p.x+=.08f;eyes[1].p.x+=.08f;assert(!allows());
    latest.head.p={};eyes[0].p={0,0,-.032f};eyes[1].p={0,0,.032f};assert(allows());
    eyes[0].q=yaw(2);eyes[1].q=yaw(-2);assert(allows()); // Sphere covers arbitrary eye rotation.
    assert(!headClearanceAllows(c,image,latest,eyes,999));
    assert(!headClearanceAllows(c,image,latest,eyes,1101));
    ++latest.reference;assert(!allows());--latest.reference;
    latest.headValid=0;assert(!allows());latest.headValid=1;
    latest.focused=0;assert(!allows());latest.focused=1;
    image.fov[0].left=-1.4f;image.fov[0].right=1.4f;assert(!allows());
    image.fov[0]={-.8f,.8f,.8f,-.8f};assert(allows());
    image.input.head.p.x=.001f;assert(!allows());image.input.head.p.x=0;
    image.input.tickMs=1001;assert(!allows());image.input.tickMs=0;
    c.reserved=1;assert(!validHeadClearance(c)&&!allows());c.reserved=0;
    c.clearRadius=NAN;assert(!allows());c.clearRadius=.3f;
    c.nearZ=INFINITY;assert(!allows());c.nearZ=.05f;
    c.headRadius=.4f;assert(!allows());c.headRadius=.1f;
    c.mode=HeadClearanceMode(999);assert(!allows());
    c={};assert(validHeadClearance(c)&&allows());
    c.mode=HeadClearanceMode::Opaque;assert(validHeadClearance(c)&&allows());
    c.tickMs=1000;assert(!validHeadClearance(c)&&!allows());
}
