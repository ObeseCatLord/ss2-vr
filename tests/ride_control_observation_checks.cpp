#include "common/ride_control_observation.hpp"
#include <cassert>
#include <initializer_list>
#ifdef NDEBUG
#error Observation checks require assertions
#endif
using namespace ss2vr;
static RideControlObservation fixture() {return {123,456,7};}
int main() {
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
