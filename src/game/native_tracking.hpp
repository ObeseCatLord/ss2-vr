#pragma once
#include "common/rider.hpp"

namespace ss2vr::game {
// Borrow the native player/ride only for the current native callback. No query
// cache, physics mutation or retained native pointer belongs to this adapter.
bool readNativeRider(void *player, RiderIdentity &out);
bool nativeRiderCurrent(void *player, const RiderIdentity &expected);
bool nativeTrackingAnchor(void *player, Pose &out, const RiderIdentity *expected = nullptr);
} // namespace ss2vr::game
