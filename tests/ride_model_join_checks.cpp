#include "common/ride_model_join.hpp"
#include <cassert>
#include <initializer_list>
#include <array>
#ifdef NDEBUG
#error Join checks require assertions
#endif
using namespace ss2vr;
static RideModelJoin fixture(uint32_t table=0x2a8558) {
    return {123,456,7,table,8,1};
}
static void inner(RideModelJoin &row) {
    assert(row.enter(123,456,7));row.copyRenderable(111);row.finishInner(false);
}
int main() {
    std::array<uint8_t,13> trampoline{0x8b,0x01,0xff,0x90,0xbc,0,0,0,0xe9,0,0,0,0};
    const uint32_t base=0x20000000,continuation=0x100450b8;
    const uint32_t displacement=continuation-(base+13);
    std::memcpy(trampoline.data()+9,&displacement,4);
    assert(rideModelTrampolineReturn(trampoline,base,continuation)==base+8);
    assert(!rideModelTrampolineReturn(trampoline,base,continuation+1));
    assert(!rideModelTrampolineReturn(trampoline,0,continuation));
    assert(!rideModelTrampolineReturn(trampoline,UINT32_MAX-8,continuation));
    assert(!rideModelTrampolineReturn(std::span(trampoline).first(12),base,continuation));
    for(auto index:{0u,3u,4u,8u}) {
        auto changed=trampoline;changed[index]^=1;
        assert(!rideModelTrampolineReturn(changed,base,continuation));
    }
    for(auto table:{0x2a8558u,0x2b8420u}) {
        auto row=fixture(table);inner(row);assert(!row.publishable());
        row.copyInstance(222,table,8);assert(!row.publishable());row.finishOuter(false);
        assert(row.publishable() && row.renderable==111 && row.instance==222);
    }
    for(unsigned failure=0;failure<10;++failure) {
        auto row=fixture();
        if(failure<3) {
            assert(!row.enter(failure==0?124:123,failure==1?457:456,failure==2?8:7));
        } else if(failure==3) {
            assert(row.enter(123,456,7));assert(!row.enter(123,456,7));
        } else if(failure==4) {
            assert(row.enter(123,456,7));row.copyRenderable(0);row.finishInner(false);
        } else {
            inner(row);
            if(failure==5)assert(!row.enter(123,456,7));
            if(failure==6)row.finishInner(true);
            row.copyInstance(failure==7?0:222,0x2a8558,failure==8?9:8);
            row.finishOuter(failure==9);
        }
        assert(!row.publishable());
    }
    auto changedClass=fixture();inner(changedClass);
    changedClass.copyInstance(222,0x2b8420,8);changedClass.finishOuter(false);assert(!changedClass.publishable());
    auto unsupported=fixture(0x2a7ea0);inner(unsupported);
    unsupported.copyInstance(222,0x2a7ea0,8);unsupported.finishOuter(false);assert(!unsupported.publishable());
    auto nested=fixture();inner(nested);nested.invalidate();
    nested.copyInstance(222,0x2a8558,8);nested.finishOuter(false);assert(!nested.publishable());
    auto missing=fixture();missing.copyInstance(222,0x2a8558,8);missing.finishOuter(false);assert(!missing.publishable());
}
