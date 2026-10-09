#pragma once
#include <cstdint>
namespace ss2vr {
struct RideControlScalars {
    uint32_t classRva=0,mode=0,executionAbilities=0,movementAbilities=0;
    uint32_t parameterToken=0,renderableToken=0;
};
// Private scalar evidence from a borrowed native callback. Tokens are equality
// keys, not retained entities/resources or a certified operated-seat binding.
struct RideControlObservation {
    uintptr_t rideToken=0,lookToken=0;
    uint32_t thread=0,calls=0;
    bool callbackBusy=false,declined=false,copied=false,returned=false,aborted=false;
    RideControlScalars values{};
    bool enter(uintptr_t ride,uintptr_t look,uint32_t currentThread) noexcept {
        if(calls!=UINT32_MAX)++calls;
        if(callbackBusy || calls!=1 || !rideToken || !lookToken || !thread ||
           ride!=rideToken || look!=lookToken || currentThread!=thread) {
            declined=true;copied=false;return false;
        }
        callbackBusy=true;return true;
    }
    void copy(RideControlScalars row) noexcept {
        if(!callbackBusy || declined || (row.classRva!=0x2a8558 && row.classRva!=0x2b8420)) {
            declined=true;copied=false;return;
        }
        values=row;copied=true;
    }
    void finishCallback(bool abnormal) noexcept {
        if(abnormal) {aborted=true;copied=false;}
        else returned=true;
        callbackBusy=false;
    }
    void finishOuter(bool abnormal) noexcept {
        if(abnormal) {aborted=true;copied=false;}
    }
    bool publishable() const noexcept {
        return calls==1 && copied && returned && !callbackBusy && !declined && !aborted;
    }
};
}
