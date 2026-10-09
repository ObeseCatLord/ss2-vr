#pragma once
#include <cstdint>
#include <limits>
namespace ss2vr {
inline uint32_t idleProjectionSlotsSource(uintptr_t base,uintptr_t caller,uintptr_t table) noexcept {
    if(!base || base>std::numeric_limits<uintptr_t>::max()-0x2834c)return 0;
    if(caller==base+0xf4ff && table==base+0x2834c)return 1;
    if(caller==base+0x856b && table==base+0x27ccc)return 2;
    return 0;
}
inline uint32_t idleProjectionFogSource(uintptr_t base,uintptr_t caller) noexcept {
    if(!base || base>std::numeric_limits<uintptr_t>::max()-0xfc8a)return 0;
    return caller==base+0xfc8a?1u:caller==base+0x8ca7?2u:0u;
}
}
