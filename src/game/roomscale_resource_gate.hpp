#pragma once
#include <windows.h>
#include <cstdint>

namespace ss2vr::game {
using RoomscaleInternalHook = bool (*)(HMODULE, uint32_t, void *, void **);
using RoomscaleResourceBody = void (__cdecl *)(void *);
// Experimental optional-query boundary. These functions do not activate body
// movement or prove collision fractions. The owner must fingerprint Engine,
// successfully enable every queued gate before running a scope, exclude foreign
// query workers, provide fresh native query cleanup, and make
// the entire result unusable if unavailable becomes true.
bool queueRoomscaleResourceGates(HMODULE engine, RoomscaleInternalHook registrar);
void resetRoomscaleResourceGatesAfterRemoval() noexcept;
bool runRoomscaleResourceScope(bool &unavailable, DWORD simulationThread,
                               RoomscaleResourceBody body, void *context) noexcept;
// Only the checked-placement adapter may end filtering immediately before its
// first approved native write. Subsequent native commit callbacks stay native.
bool finishRoomscaleResourceScopeForCommit(bool &unavailable) noexcept;
} // namespace ss2vr::game
