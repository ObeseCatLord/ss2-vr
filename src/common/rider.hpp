#pragma once
#include "math.hpp"

namespace ss2vr {
// Native lifecycle identity only. The engine owns seats, transitions and drive.
struct RiderIdentity {
    uint32_t player = 0, ride = 0, seat = 0, state = 0;
    bool seatValid = false;
    bool operator==(const RiderIdentity &) const = default;
    bool seated() const { return player && ride && state == 3 && seatValid; }
    bool handheld() const { return player && !ride && state == 0; }
};
inline bool sameHandheldRig(const RiderIdentity &captured, const RiderIdentity &live,
                            uint32_t generation, uint32_t liveGeneration) {
    return captured.handheld() && captured == live && generation && generation == liveGeneration;
}
inline bool trackedHandCurrent(const Input &captured, const Input &live, unsigned hand) {
    return hand < 2 && captured.handValid[hand] && live.handValid[hand] &&
           finite(captured.hand[hand]) && finite(live.hand[hand]);
}
inline bool validNativeBodyPose(const Pose &pose) {
    if (!finite(pose))
        return false;
    for (float value : {pose.q.x, pose.q.y, pose.q.z, pose.q.w, pose.p.x, pose.p.y, pose.p.z})
        if (std::bit_cast<uint32_t>(value) == 0x7f61b1e6)
            return false;
    const auto &q = pose.q;
    const float length2 = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
    return std::isfinite(length2) && length2 >= .95f * .95f && length2 <= 1.05f * 1.05f;
}
inline bool compatibleRiderInput(const RiderIdentity &captured, const RiderIdentity &live,
                                  const Input &input, const Input &latest,
                                  uint32_t generation, uint32_t liveGeneration, uint64_t now) {
    const auto usable = [now](const Input &value) {
        return value.session && value.focused && value.headValid && finite(value.head) &&
               now >= value.tickMs && now - value.tickMs < 200;
    };
    return captured.player && captured == live && generation && generation == liveGeneration &&
           input.session == latest.session && input.reference == latest.reference &&
           latest.sequence >= input.sequence && usable(input) && usable(latest);
}
inline bool nativeRiderAnchor(const RiderIdentity &identity, Pose view, uint32_t heightBits,
                             const Pose &absoluteBody, Pose &out) {
    if (!identity.player || !applyNativeViewHeight(view, heightBits) || !validNativeBodyPose(view))
        return false;
    if (identity.state == 3) {
        if (!identity.seated() || !validNativeBodyPose(absoluteBody))
            return false;
        // Position retains native seat/view-height interpolation; operator aim
        // never becomes the seated head/hand rig's orientation.
        view.q = absoluteBody.q;
    }
    out = view;
    return true;
}
} // namespace ss2vr
