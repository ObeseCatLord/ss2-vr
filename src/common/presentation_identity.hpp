#pragma once
#include "network.hpp"
#include <atomic>
namespace ss2vr {
constexpr bool nativeRenderOwnsOuterScope(uint32_t enclosingDepth) noexcept {
    return enclosingDepth == 0;
}
constexpr bool frozenPresentationResetSuppressed(bool wasSuppressed, bool activeDraw,
                                                 uint32_t localOwner) noexcept {
    return wasSuppressed || (activeDraw && localOwner != 0);
}
constexpr bool frozenPresentationOwnerMatches(uint32_t global, uint32_t local, uint32_t captured) noexcept {
    return captured && global == captured && local == captured;
}
inline uint32_t nextFrozenPresentationOwner(uint32_t &serial) noexcept {
    if (serial == UINT32_MAX) return 0;
    return ++serial;
}
// Invalidation precedes owner release. The caller keeps its TLS token until
// its own cleanup, preventing reentry from acquiring another bank mid-unwind.
inline void invalidateFrozenPresentationOwner(std::atomic<uint32_t> &global,
                                              std::atomic<bool> &invalid,
                                              uint32_t local, uint32_t captured) noexcept {
    if (!frozenPresentationOwnerMatches(global.load(std::memory_order_acquire), local, captured)) return;
    invalid.store(true, std::memory_order_release);
    global.compare_exchange_strong(captured, 0, std::memory_order_acq_rel);
}
inline void retireFrozenPresentationOwner(std::atomic<uint32_t> &global,
                                          std::atomic<bool> &invalid,
                                          uint32_t &local, bool &enabled,
                                          uint32_t captured) noexcept {
    if (!captured || local != captured) return;
    invalidateFrozenPresentationOwner(global, invalid, local, captured);
    enabled = false;
    local = 0;
}
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
