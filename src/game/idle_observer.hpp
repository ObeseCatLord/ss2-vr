#pragma once
#include "common/idle_weapon_trace.hpp"
#include "scope_observer.hpp"
namespace ss2vr::game {
// Valid only inside the original selected ID1 ordinary gun invocation.
bool currentIdleRaster(IdleRasterCopy &,IdleWeaponTrace *&);
bool currentIdleDraw(ScopeDrawBinding &,IdleDrawIdentity &,IdleWeaponTrace *&);
// Boolean-only association check for an already rejected diagnostic. Never
// provides an admitted raster/geometry output or changes rejection policy.
bool idleRejectedRasterCurrent(const IdleRasterCopy &,const IdleWeaponTrace *);
}
