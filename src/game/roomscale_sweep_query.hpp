#pragma once
#include "common/roomscale_body_sweep.hpp"
#include <windows.h>
namespace ss2vr::game {
using RoomscaleQueryOwnerCurrent=bool(__cdecl *)(void *) noexcept;
// Call only after Engine fingerprint validation; this resolves native exports
// and does not install hooks, start a query, or move anything.
bool configureRoomscaleSweepQueries(HMODULE engine) noexcept;
void resetRoomscaleSweepQueriesAfterQuiescence() noexcept;
// Requires the enclosing checked-placement resource scope and math frame, plus
// enabled triangle/primitive/resource hooks. ownerCurrent must establish phase,
// worker/lifetime/scratch ownership and an inspected normal ray-cleanup list.
// All copied body/candidate data must remain current through final commit.
// Every sphere must finish unobstructed. Native results are copied before the
// ordinary next rayInit; abnormal native unwind is never "repaired" here.
bool runRoomscaleSweepQueries(const roomscale::BodyGeometry&,const roomscale::BodySweepCover&,
    float contactDepthBudget,DWORD simulationThread,RoomscaleQueryOwnerCurrent ownerCurrent,
    void *ownerContext,bool &failed) noexcept;
} // namespace ss2vr::game
