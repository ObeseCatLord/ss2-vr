#pragma once
#include <string_view>
namespace ss2vr {
// Exact private collector selectors. No native inventory/equip operation.
inline int idleProbeWeaponId(std::wstring_view value) {
    return value==L"1"?1:value==L"13"?13:-1;
}
inline bool idleProbeWeaponSupported(int id) {return id==1 || id==13;}
}
