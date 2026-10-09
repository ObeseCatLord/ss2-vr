#pragma once
#include <cstdint>
#include <cstring>
#include <span>

namespace ss2vr {
// The pinned eight-byte virtual-call prefix is relocated intact by x86 MinHook,
// followed by its jump back. Return the actual nested caller, never the old PC.
inline uint32_t rideModelTrampolineReturn(std::span<const uint8_t> code,
                                         uint32_t base,uint32_t continuation) noexcept {
    constexpr uint8_t prefix[]={0x8b,0x01,0xff,0x90,0xbc,0,0,0};
    if(code.size()!=13 || !base || base>UINT32_MAX-13 ||
       std::memcmp(code.data(),prefix,8) || code[8]!=0xe9)return 0;
    uint32_t displacement=0;std::memcpy(&displacement,code.data()+9,4);
    if(uint32_t(base+13+displacement)!=continuation)return 0;
    return base+8;
}
// One synchronous native getter borrow. Numeric result tokens must never seed
// a later render association or become dereferenceable retained pointers.
struct RideModelJoin {
    uintptr_t receiver=0,expectedCaller=0;
    uint32_t thread=0,classRva=0,handle=0,invocation=0,calls=0;
    uintptr_t renderable=0,instance=0;
    bool busy=false,declined=false,innerReturned=false,outerReturned=false,aborted=false;
    void invalidate() noexcept {declined=true;renderable=instance=0;}
    bool enter(uintptr_t object,uintptr_t caller,uint32_t currentThread) noexcept {
        if(calls!=UINT32_MAX)++calls;
        if(busy || calls!=1 || !receiver || !expectedCaller || !thread ||
           object!=receiver || caller!=expectedCaller || currentThread!=thread || declined) {
            invalidate();return false;
        }
        busy=true;return true;
    }
    void copyRenderable(uintptr_t value) noexcept {
        if(!busy || declined || !value) {invalidate();return;}
        renderable=value;
    }
    void finishInner(bool abnormal) noexcept {
        if(abnormal) {aborted=true;invalidate();}
        else innerReturned=true;
        busy=false;
    }
    void copyInstance(uintptr_t value,uint32_t liveClass,uint32_t liveHandle) noexcept {
        if(declined || busy || !innerReturned || calls!=1 || !renderable || !value ||
           liveClass!=classRva || liveHandle!=handle ||
           (classRva!=0x2a8558 && classRva!=0x2b8420)) {invalidate();return;}
        instance=value;
    }
    void finishOuter(bool abnormal) noexcept {
        if(abnormal) {aborted=true;invalidate();}
        else outerReturned=true;
    }
    bool publishable() const noexcept {
        return calls==1 && renderable && instance && innerReturned && outerReturned &&
               !busy && !declined && !aborted;
    }
};
}
