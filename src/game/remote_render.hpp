#pragma once

#include "common/protocol.hpp"
#include "common/scope_pose.hpp"
#include "common/idle_weapon_trace.hpp"
#include <cstdint>
#include <windows.h>

namespace ss2vr::game::remote_render {

using HookInstallerRva = bool (*)(HMODULE, uint32_t, void *, void **);

bool initialize(HMODULE engine, HMODULE core, HMODULE sam, HookInstallerRva install, bool enableHeadTracking,
                bool observeRideControl=false);

// Pin before native simulation; check lifecycle entries before native mutation.
void noteSimulationThread();
bool checkNativeThread();
// Passive observers decline foreign/unknown ownership without poisoning it.
bool ownsNativeThread();
// Borrow only within a validated native placement or out-of-render muzzle
// extent. Returns copied values; no model pointer or allocation is retained.
bool copyModelConfigurationStretch(void *instance,uint32_t expectedConfigurationVtable,
                                   IdleConfigIdentity &identity,Vec3 &stretch);
bool idleProjectionConfigured() noexcept;
bool copyIdleRaster(void *instance,IdleRasterCopy &out);
// ID2-only policy selected by the current submission owner; copies raw values
// without changing the single-affine raster/geometry admission policy.
bool copyIdlePalette(void *instance,IdlePaletteCopy &out);
ScopeRasterStatus copyScopeRaster(void *instance, Matrix34 &affine, ScopeSurfaceLayout &layout);

// Called on the simulation thread after the original CPlayerOnStep.
void observePlayer(void *player);
void invalidatePlayer(void *player);

// One exact native world invocation owns the existing bank. Stereo keeps its
// token through UI publication/cleanup; mono keeps it through its original draw.
uint32_t freezePair(uint32_t localPlayer=0);
void useFrozenPair(bool enabled);
uint32_t beginMonoPresentation(uint32_t localPlayer=0);
// Historical copied world observation, only on a normal original mono return.
// Retirement remains unconditional and separate, including diagnostic failure.
void completeMonoPresentation(uint32_t owner,bool completed);
void retirePresentation(uint32_t owner) noexcept;
bool suppressNestedPresentation() noexcept;
bool presentationSuppressionCurrent() noexcept;
void invalidatePresentationForReset(bool activeDraw) noexcept;
void restorePresentationSuppression(bool previous) noexcept;
// Caller owns IPC slot and snapshot lock; validates remote identity through Ready.
bool commitPair(Slot &slot, const Request &request, bool localEligible, uint32_t owner);

} // namespace ss2vr::game::remote_render
