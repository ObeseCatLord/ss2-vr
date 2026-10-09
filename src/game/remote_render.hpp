#pragma once

#include "common/protocol.hpp"
#include "common/scope_pose.hpp"
#include "common/idle_weapon_trace.hpp"
#include <cstdint>
#include <windows.h>

namespace ss2vr::game::remote_render {

using HookInstallerRva = bool (*)(HMODULE, uint32_t, void *, void **);

bool initialize(HMODULE engine, HMODULE core, HMODULE sam, HookInstallerRva install, bool enableHeadTracking);

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
ScopeRasterStatus copyScopeRaster(void *instance, Matrix34 &affine, ScopeSurfaceLayout &layout);

// Called on the simulation thread after the original CPlayerOnStep.
void observePlayer(void *player);
void invalidatePlayer(void *player);

// The stereo caller freezes the presentation bank before its left eye and
// scopes this flag around each eye. Desktop draws read the latest bank.
void freezePair();
void useFrozenPair(bool enabled);
// Caller owns IPC slot and snapshot lock; validates remote identity through Ready.
bool commitPair(Slot &slot, const Request &request, bool localEligible);

} // namespace ss2vr::game::remote_render
