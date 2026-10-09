#include "common/idle_weapon_trace.hpp"
#include <cassert>
#include <string_view>
using namespace ss2vr;
int main() {
    assert(idleProbeWeaponId(L"1")==1 && idleProbeWeaponId(L"13")==13);
    for(auto s:{L"",L"0",L"2",L"01",L"013",L"13 ",L" 13",L"130",L"1,13",L"*"})
        assert(idleProbeWeaponId(s)==-1);
    const IdleDrawIdentity identity{1,2,3,4,5,6,0,1};
    for(int id:{1,13}) {
        IdleWeaponTrace trace;
        assert(trace.admit(identity,id) && trace.nativeId==id);
        assert(!trace.admit(identity,id)); // Same invocation cannot change ownership.
    }
    for(int id:{-1,0,2,12,14,17}) {
        IdleWeaponTrace trace;
        assert(!trace.admit(identity,id) && !trace.admitted);
    }
    IdleWeaponTrace historical;
    assert(historical.admit(identity) && historical.nativeId==1);
}
