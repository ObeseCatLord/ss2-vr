#pragma once
#include <windows.h>
namespace ss2vr::game {
using RoomscaleMathBody=bool(__cdecl *)(void *);
// Isolate only additional query/math work. The owner must exclude gameplay and
// commit callbacks, and must not call a native body setter inside this frame.
// Full caller FX state is restored before returning or crossing native unwind.
// A nested attempt fails both frames; this is not a general engine FP policy.
bool runRoomscaleMathFrame(bool &failed,DWORD simulationThread,
                           RoomscaleMathBody body,void *context) noexcept;
} // namespace ss2vr::game
