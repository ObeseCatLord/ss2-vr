#pragma once
#include "common/roomscale_triangle_query.hpp"
#include <windows.h>

#if !defined(__i386__) || !defined(__MINGW32__)
#error Roomscale native query boundary requires the supported MinGW x86 target
#endif
namespace ss2vr::game {
using RoomscaleTriangleKernel=float(__cdecl *)(const roomscale::Ray&,const roomscale::Vector&,
    const roomscale::Vector&,const roomscale::Vector&,const roomscale::Vector&,float);
using RoomscaleQueueHook=bool(*)(HMODULE,const char*,void*,void**);
using RoomscaleQueryBody=void(__cdecl *)(void*);
inline constexpr char RoomscaleTriangleExport[]=
    "?mthIntersectThickRayTriangle@SeriousEngine@@YAMABVRay3f@1@ABVVector3f@1@111M@Z";

// Caller has already fingerprinted Core. Pass the existing transactional hook()
// registrar: it owns create/queue/disable/remove, INCLUDING partial failure.
// This function neither enables hooks nor initiates a query. A false result
// requires the caller's existing rollback, not freeing the trampoline here.
bool queueRoomscaleTriangleHook(HMODULE core,RoomscaleQueueHook registrar);
// Only after all detours are disabled and removed, with callers quiescent.
void resetRoomscaleTriangleHookAfterRemoval() noexcept;

// Explicit experimental admission, NOT proof of shared native scratch ownership.
// The caller must separately establish exclusive query scratch / stopped worker
// use, matching ray t parameter, minDistance==0 and positive actual ray radius.
// It must restore native scratch itself; this wrapper only restores mod TLS.
// recognizedSimulationThread comes from the caller's verified native owner.
// The callback's dynamic extent must contain only the admitted model query;
// nested/reentrant unrelated triangle calls cannot be distinguished by this ABI.
// A failed scope invalidates the entire query result; never accept its fraction.
// Returning true means no adapter fault, NOT a certified movement fraction.
// scope/context must live above this call and survive native unwinding.
bool runRoomscaleModelQueryScope(roomscale::QueryScope& scope,DWORD recognizedSimulationThread,
                                RoomscaleQueryBody body,void* context) noexcept;
extern "C" float __cdecl ss2vrRoomscaleTriangleQuery(const roomscale::Ray&,const roomscale::Vector&,
    const roomscale::Vector&,const roomscale::Vector&,const roomscale::Vector&,float) noexcept;
} // namespace ss2vr::game
