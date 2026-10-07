#include "common/head_query_binding.hpp"
#include <cassert>
#include <cstring>
using namespace ss2vr;
using namespace ss2vr::roomscale;
struct Memory {
    std::array<unsigned char,0x6000> bytes{};
    unsigned reads=0,resolves=0,failRead=0,failResolve=0;
    template<class T>void put(unsigned at,const T& value) {std::memcpy(bytes.data()+at,&value,sizeof(value));}
    bool read(uint32_t at,void* out,size_t n) {
        if(++reads==failRead||at>bytes.size()||n>bytes.size()-at)return false;
        std::memcpy(out,bytes.data()+at,n);return true;
    }
    uint32_t resolve(uint32_t handle) {
        if(++resolves==failResolve)return 0;
        return handle==11?0x1000:handle==22?0x2000:handle==33?0x3000:0;
    }
    Memory() {
        put(0x1000,uint32_t(0x12345678));put(0x1010,uint32_t(0));
        put(0x147c,uint32_t(0x4000));put(0x1120,uint32_t(33));put(0x1114,uint32_t(22));
        put(0x2034,uint32_t(0));put(0x2038,uint32_t(33));put(0x302c,Pose{yaw(.2f),{1,2,3}});
    }
    bool capture(HeadQueryBinding& result) {
        return readHeadQueryBinding([&](uint32_t at,void* to,size_t n){return read(at,to,n);},
            [&](uint32_t h){return resolve(h);},11,0x12345678,91,result);
    }
};
int main() {
    Memory good;HeadQueryBinding captured;
    assert(good.capture(captured));
    const unsigned reads=good.reads,resolves=good.resolves;
    assert(captured.subject.avatar==0x1000&&captured.subject.mechanism==0x2000);
    assert(captured.subject.categoryCount==1&&captured.subject.categories[0]==91);
    assert(captured.poseSourceHandle==33&&captured.poseSource==0x3000);
    for(unsigned i=1;i<=reads;++i) {Memory m;m.failRead=i;HeadQueryBinding result;assert(!m.capture(result));}
    for(unsigned i=1;i<=resolves;++i) {Memory m;m.failResolve=i;HeadQueryBinding result;assert(!m.capture(result));}
    for(auto [offset,value]: {std::pair{0x1000u,0u},{0x1010u,2u},{0x147cu,0u},{0x1114u,0u},{0x2038u,0u}}) {
        Memory m;m.put(offset,value);HeadQueryBinding result;assert(!m.capture(result));
    }
    Memory mounted;mounted.put(0x2034,uint32_t(44));
    HeadQueryBinding seat;assert(mounted.capture(seat));
    assert(seat.parentMechanism==44&&!sameHeadQueryBinding(captured,seat));
    // Parent presence is allowed, not treated as a proven seat association.
    // The native caller still owns its exact RiderIdentity and phase checks.
    Memory moved;moved.put(0x302c,Pose{yaw(.2f),{1,2.01f,3}});
    HeadQueryBinding changed;assert(moved.capture(changed)&&!sameHeadQueryBinding(captured,changed));
    Memory replaced;replaced.put(0x147c,uint32_t(0x4500));
    assert(replaced.capture(changed)&&!sameHeadQueryBinding(captured,changed));
    Memory invalid;invalid.put(0x302c,Pose{{0,0,0,2},{}});assert(!invalid.capture(changed));
    BodyGeometry body;body.playerHandle=11;body.player=0x1000;body.mechanism=0x2000;body.hullCount=2;
    body.hulls[0].category=71;body.hulls[1].category=81;
    const auto subject=sphereSubjectForBody(body);
    assert(subject.valid()&&subject.avatar==body.player&&subject.avatar!=body.playerHandle&&
           subject.categories[0]==71&&subject.categories[1]==81&&subject.categoryCount==2);
    body.hullCount=3;assert(!sphereSubjectForBody(body).valid());
}
