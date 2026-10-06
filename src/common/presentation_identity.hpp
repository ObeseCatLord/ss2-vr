#pragma once
#include "network.hpp"
namespace ss2vr {
inline bool validFrozenPresentationRevision(uint64_t frozenRevision, uint64_t currentRevision) {
    return frozenRevision && frozenRevision == currentRevision;
}
template<class OwnerAllowed>
inline bool eligiblePresentationPair(bool active, bool samePairThread, bool admitted, bool invalid,
                                     OwnerAllowed &&ownerAllowed) {
    if (!active || !samePairThread)
        return false;
    if (!admitted)
        return true; // Deliberate native-only fallback has no remote-object reads.
    return !invalid && ownerAllowed();
}
// Age is evaluated once at admission. A newer compatible pose waits for the
// next pair; lifecycle/tracking/weapon changes must reject the current pair.
inline bool samePresentationIdentity(uint32_t avatar, uint32_t incarnation,
                                     const network::PosePacket &frozen, uint32_t currentAvatar,
                                     uint32_t currentIncarnation, const network::PosePacket &current,
                                     bool negotiated, bool valid) {
    return negotiated && valid && avatar && incarnation && avatar == currentAvatar &&
           incarnation == currentIncarnation && frozen.clientNonce == current.clientNonce &&
           frozen.serverNonce == current.serverNonce &&
           frozen.trackingGeneration == current.trackingGeneration &&
           frozen.validMask == current.validMask &&
           frozen.nativeWeaponId[0] == current.nativeWeaponId[0] &&
           frozen.nativeWeaponId[1] == current.nativeWeaponId[1];
}
} // namespace ss2vr
