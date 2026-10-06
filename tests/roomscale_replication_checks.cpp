#include "common/roomscale_replication.hpp"
#include <cassert>
#include <cstring>
#include <random>
using namespace ss2vr;
using namespace ss2vr::roomscale;
static bool close(Vec3 a,Vec3 b,double tolerance=.000002) {
    return std::hypot(double(a.x)-b.x,double(a.y)-b.y,double(a.z)-b.z)<=tolerance;
}
int main() {
    OriginLedger<4> ledger;
    StageOffset offset{9,8,7};
    assert(!ledger.lookup(1,0,offset)&&offset.x==9);
    assert(!ledger.canAppend(1,0));
    ledger.reset(9);
    assert(ledger.lookup(9,0,offset)&&offset.x==0&&ledger.size()==1);
    assert(ledger.append(9,0,{.1,0,0},.2,2));
    assert(!ledger.append(9,0,{.1,0,0},.2,2)); // Stale outer completion.
    assert(!ledger.append(8,1,{.1,0,0},.2,2)); // Another origin epoch.
    assert(!ledger.append(9,1,{NAN,0,0},.2,2));
    assert(!ledger.append(9,1,{.3,0,0},.2,2));
    assert(!ledger.append(9,1,{.1,0,0},INFINITY,2));
    assert(!ledger.append(9,1,{.1,0,0},.2,.15));
    assert(ledger.revision()==1&&ledger.size()==2);
    assert(ledger.append(9,1,{.1,0,0},.2,2));
    assert(ledger.append(9,2,{.1,0,0},.2,2));
    assert(!ledger.canAppend(9,3)&&!ledger.append(9,3,{.1,0,0},.2,2));
    assert(!ledger.retireBefore(9,4)&&!ledger.retireBefore(8,2));
    assert(ledger.retireBefore(9,2)&&ledger.size()==2);
    assert(!ledger.lookup(9,1,offset)&&ledger.lookup(9,2,offset));
    assert(ledger.append(9,3,{-.1,0,0},.2,2));
    assert(ledger.lookup(9,4,offset)&&std::abs(offset.x-.2)<1e-15);
    assert(ledger.retireBefore(9,4)&&ledger.size()==1);
    assert(!ledger.lookup(9,3,offset));

    // A coordinate helper must use the supplied capture basis, never silently
    // borrow another pose's basis. The existing input policy separately cancels
    // retained taps when snap turn changes trackingGeneration; this helper
    // cannot authorize replay across that boundary.
    ledger.reset(11);
    assert(ledger.append(11,0,{.12,.01,-.07},.2,2));
    const Pose hand{yaw(.6f),{.3f,-.2f,-.4f}};
    Pose moved{};
    OriginFrame oldHand{yaw(.3f),0},newHead{yaw(1.2f),0};
    assert(rebaseBodyPose(ledger,11,oldHand,hand,moved));
    assert(close(moved.p,hand.p-rotate(oldHand.stageToBody,{.12f,.01f,-.07f})));
    assert(std::memcmp(&hand.q,&moved.q,sizeof(Quat))==0);
    assert(!close(moved.p,hand.p-rotate(newHead.stageToBody,{.12f,.01f,-.07f}),.001));
    Pose already=moved;
    assert(rebaseBodyPose(ledger,11,OriginFrame{oldHand.stageToBody,1},already,moved));
    assert(close(already.p,moved.p,0));
    Pose untouched=moved;
    assert(!rebaseBodyPose(ledger,12,oldHand,hand,moved));
    assert(!rebaseBodyPose(ledger,11,OriginFrame{{0,0,0,2},0},hand,moved));
    assert(!rebaseBodyPose(ledger,11,OriginFrame{yaw(0),2},hand,moved));
    assert(std::memcmp(&untouched,&moved,sizeof(Pose))==0);
    ledger.reset(12);
    assert(!ledger.lookup(11,1,offset)&&ledger.lookup(12,0,offset));
    ledger.reset();assert(ledger.size()==0&&!ledger.lookup(12,0,offset));

    std::mt19937 rng(72013);
    std::uniform_real_distribution<float> unit(-1,1);
    for(unsigned i=0;i<10000;++i) {
        OriginLedger<8> history;history.reset(1);
        const Vec3 first{unit(rng)*.04f,unit(rng)*.04f,unit(rng)*.04f};
        const Vec3 second{unit(rng)*.04f,unit(rng)*.04f,unit(rng)*.04f};
        assert(history.append(1,0,{first.x,first.y,first.z},.1,1));
        assert(history.append(1,1,{second.x,second.y,second.z},.1,1));
        const Quat basis=normalize({unit(rng),unit(rng),unit(rng),unit(rng)});
        Pose captured{yaw(unit(rng)),{unit(rng),unit(rng),unit(rng)}};
        Pose output;
        assert(rebaseBodyPose(history,1,OriginFrame{basis,0},captured,output));
        assert(close(output.p,captured.p-rotate(basis,first+second)));
        assert(rebaseBodyPose(history,1,OriginFrame{basis,1},captured,output));
        assert(close(output.p,captured.p-rotate(basis,second)));
        assert(std::memcmp(&output.q,&captured.q,sizeof(Quat))==0);
    }
}
