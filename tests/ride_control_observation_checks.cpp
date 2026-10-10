#include "common/ride_control_observation.hpp"
#include "common/ride_render_observation.hpp"
#include <cassert>
#include <initializer_list>
#ifdef NDEBUG
#error Observation checks require assertions
#endif
using namespace ss2vr;
static RideControlObservation fixture() {return {123,456,7};}
static void mainMappings() {
    std::array<PaletteMap,15> maps;
    for(unsigned i=0;i<maps.size();++i)maps[i]={2,int32_t(100+i)};
    uint32_t slot=0;
    const auto owned=[](int32_t bone) {return bone>=100 && bone<115;};
    assert(selectRideMainMapping(maps,15,2,0,15,103,owned,slot) && slot==3); // Not authored slot10.
    auto duplicate=maps;duplicate[10].bone=103;
    assert(!selectRideMainMapping(duplicate,15,2,0,15,103,owned,slot) && slot==UINT32_MAX);
    auto bad=maps;bad[14].bone=200;
    assert(!selectRideMainMapping(bad,15,2,0,15,103,owned,slot));
    bad=maps;bad[5].draw=3;assert(!selectRideMainMapping(bad,15,2,0,15,103,owned,slot));
    assert(!selectRideMainMapping(maps,14,2,0,15,103,owned,slot));
    assert(!selectRideMainMapping(maps,15,2,1,15,103,owned,slot));
    assert(!selectRideMainMapping(maps,15,2,-1,15,103,owned,slot));
    assert(!selectRideMainMapping(maps,15,2,0,0,103,owned,slot));
    assert(!selectRideMainMapping(maps,15,2,0,33,103,owned,slot));
    assert(!selectRideMainMapping(maps,15,2,0,15,3,owned,slot)); // Wrong relative/global index.
    RideRenderFrameCopy frame;frame.identity={1,2,3,4,0x2a8558,5,6,7};
    frame.configuration=8;frame.file=9;frame.resource=10;frame.modelRecord=1;
    frame.lod=12;frame.mainBone=103;frame.boneDefinition=14;frame.world[0]=0x3f800000;
    frame.main[0]=0x40000000; // Canonical differs; actual palette is the observation.
    RideMainDrawCopy draw;draw.identity=frame.identity;draw.instance=7;
    draw.configuration=8;draw.file=9;draw.resource=10;draw.modelRecord=1;draw.lod=12;
    draw.bone=103;draw.definition=14;draw.world=frame.world;draw.actualPalette[0]=0x40400000;
    draw.paletteCount=15;draw.localMainSlot=3;
    assert(rideMainDrawLinked(draw,frame));
    for(unsigned failure=0;failure<7;++failure) {
        auto crossed=draw;
        if(failure==0)++crossed.instance;
        if(failure==1)++crossed.modelRecord;
        if(failure==2)++crossed.lod;
        if(failure==3)++crossed.configuration;
        if(failure==4)++crossed.bone;
        if(failure==5)++crossed.definition;
        if(failure==6)crossed.localMainSlot=15;
        assert(!rideMainDrawLinked(crossed,frame));
    }
}
int main() {
    mainMappings();
    for(auto table:{0x2a8558u,0x2b8420u}) {
        auto row=fixture();assert(row.enter(123,456,7));
        row.copy({table,2,4,41,0,0}); // Null association tokens remain unassociated.
        assert(!row.publishable());row.finishCallback(false);row.finishOuter(false);
        assert(row.publishable() && row.values.movementAbilities==41 && row.values.executionAbilities==4);
    }
    for(unsigned failure=0;failure<7;++failure) {
        auto row=fixture();
        if(failure<3) {
            assert(!row.enter(failure==0?124:123,failure==1?457:456,failure==2?8:7));
        } else {
            assert(row.enter(123,456,7));row.copy({0x2a8558,2,4,41,1,2});
            if(failure==3)assert(!row.enter(123,456,7)); // Nested callback cannot lend its result.
            if(failure==4)row.copy({0x2a7ea0,2,4,41,1,2});
            row.finishCallback(failure==5);row.finishOuter(failure==6);
        }
        assert(!row.publishable());
    }
    auto duplicate=fixture();assert(duplicate.enter(123,456,7));duplicate.copy({0x2a8558,2,4,41,0,0});
    duplicate.finishCallback(false);assert(!duplicate.enter(123,456,7));assert(!duplicate.publishable());
    auto missing=fixture();missing.finishOuter(false);assert(!missing.publishable());
}
