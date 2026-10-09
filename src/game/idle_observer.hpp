#pragma once
#include "common/idle_weapon_trace.hpp"
#include "scope_observer.hpp"
namespace ss2vr::game {
// Valid only inside the original selected ID1 ordinary gun invocation.
bool currentIdleRaster(IdleRasterCopy &,IdleWeaponTrace *&);
bool currentIdleDraw(ScopeDrawBinding &,IdleDrawIdentity &,IdleWeaponTrace *&);
IdleWeaponTrace *idleSubmissionOwner() noexcept;
bool copyIdleSubmissionMetadata(const IdleWeaponTrace *,IdleSubmissionMetadata &) noexcept;
void retireIdleSubmissionOwner(IdleWeaponTrace *) noexcept;
// Pure invocation metadata borrow. The caller separately establishes native
// thread ownership before original entry; DIP revalidates the native binding.
IdleWeaponTrace *idleProjectionOwner() noexcept;
// Boolean-only association check for an already rejected diagnostic. Never
// provides an admitted raster/geometry output or changes rejection policy.
bool idleRejectedRasterCurrent(const IdleRasterCopy &,const IdleWeaponTrace *);
}
