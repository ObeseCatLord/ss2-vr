#pragma once
#include "common/idle_weapon_trace.hpp"
#include "scope_observer.hpp"
namespace ss2vr::game {
// Valid only inside the original selected ID1 ordinary gun invocation.
bool currentIdleDraw(ScopeDrawBinding &,IdleDrawIdentity &,IdleWeaponTrace *&);
}
