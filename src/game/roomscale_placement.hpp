#pragma once
#include "common/math.hpp"
#include "roomscale_query.hpp"
#include "roomscale_resource_gate.hpp"
namespace ss2vr::game {
using RoomscalePlacementCheck=bool(__cdecl *)(void *,const Pose &) noexcept;
struct RoomscalePlacementOutcome {
    bool completed=false;
    bool mayHaveMoved=false;
    int nativeResult=0;
};
// Inactive until the owner queues/enables every component and supplies an
// exclusive post-simulation extent. The check sees the ACTUAL proposed root
// pose before its first native write; it must copy any data it retains.
bool queueRoomscalePlacementHooks(HMODULE engine,RoomscaleQueueHook exports,
                                  RoomscaleInternalHook internals);
bool armRoomscalePlacementAfterEnable() noexcept;
void resetRoomscalePlacementHooksAfterRemoval() noexcept;
// Caller owns body/lifetime/worker proof and all final pose/origin settlement.
// A false completion with mayHaveMoved must never be retried as an unconsumed
// displacement. This API does not install hooks or provide collision geometry.
RoomscalePlacementOutcome runCheckedRoomscalePlacement(void *mechanism,void *part,void *root,
    const Pose &target,DWORD simulationThread,RoomscalePlacementCheck check,void *context) noexcept;
} // namespace ss2vr::game
