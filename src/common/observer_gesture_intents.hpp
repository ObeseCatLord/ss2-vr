#pragma once
#include "network.hpp"

namespace ss2vr::network {
// One retained physical edge per hand in the existing Remote owner. Latest pose
// replacement is not consumption. The owner supplies incarnation/recipient
// admission and resets this object when either changes.
class ObserverGestureIntents {
    struct Context {
        uint64_t clientNonce = 0, serverNonce = 0;
        uint32_t trackingGeneration = 0, intentEpoch = 0, gestureGeneration = 0;
        int16_t nativeWeaponId = -1;
        bool operator==(const Context &) const = default;
    };
    static Context context(const PosePacket &p, unsigned h) noexcept {
        return {p.clientNonce, p.serverNonce, p.trackingGeneration, p.intentEpoch[h],
                p.gestureGeneration[h], p.nativeWeaponId[h]};
    }
    struct Hand {
        PosePacket pulse{};
        uint64_t receivedMs = 0, seenSequence = 0;
        uint32_t generation = 0;
        bool pending = false;
    } hands_[2];
    static bool eligible(const PosePacket &p, unsigned h) noexcept {
        const auto bit = uint8_t(1u << h);
        return (p.validMask & 1) && (p.validMask & (bit << 1)) &&
            (p.gestureEligibleMask & bit) && p.gestureGeneration[h] && p.intentEpoch[h] &&
            p.nativeWeaponId[h] == 0 && !(p.wheelOrEquipBlockedMask & bit) && p.requestedWeapon[h] < 0;
    }
    static bool context(const PosePacket &a, const PosePacket &b, unsigned h) noexcept {
        return context(a, h) == context(b, h);
    }
  public:
    static constexpr bool level(const PosePacket &pose, unsigned hand, bool retained) noexcept {
        return hand < 2 && ((pose.gestureDownMask | (retained ? pose.gesturePulseMask : 0)) & (1u << hand));
    }
    struct Receipt {
        const ObserverGestureIntents *owner = nullptr;
        unsigned hand = 2;
        uint64_t sequence = 0, receivedMs = 0;
        // Source stamps and a millisecond receive time can coincide across an
        // intent replacement. Keep the exact hand context entered by native code.
        Context context{};
    };
    void cancel(uint8_t mask = HandMask) noexcept {
        for (unsigned h = 0; h < 2; ++h)
            if (mask & (1u << h)) { hands_[h].pending = false; hands_[h].pulse = {}; }
    }
    // Called only for a decoded/admitted same-incarnation relay. `latest` is
    // already the newest presentation sample; incoming may be an older reliable
    // pulse. A delayed pulse is legal only within its current per-hand context.
    void receive(const PosePacket &latest, const PosePacket &incoming, uint64_t now) noexcept {
        for (unsigned h = 0; h < 2; ++h) {
            auto &slot = hands_[h];
            if (!eligible(latest, h)) { slot.pending = false; slot.pulse = {}; continue; }
            if (slot.generation != latest.gestureGeneration[h] ||
                (slot.pulse.gestureGeneration[h] && !context(slot.pulse, latest, h))) {
                slot = {}; slot.generation = latest.gestureGeneration[h];
            }
            if (slot.pulse.gestureGeneration[h] && (!context(slot.pulse, latest, h) || now < slot.receivedMs ||
                now - slot.receivedMs > MaxPoseAgeMs)) { slot.pending = false; slot.pulse = {}; }
            const auto bit = uint8_t(1u << h);
            if (!(incoming.gesturePulseMask & bit) || !eligible(incoming, h) ||
                !context(latest, incoming, h) || incoming.gestureSequence[h] <= slot.seenSequence ||
                incoming.gestureSequence[h] > latest.gestureSequence[h] ||
                incoming.gestureTickMs[h] > latest.gestureTickMs[h]) continue;
            slot.seenSequence = incoming.gestureSequence[h];
            if (slot.pending) continue; // Capacity discard, never future pulse debt.
            slot.pulse = incoming; slot.receivedMs = now; slot.pending = true;
        }
    }
    bool sample(const PosePacket &latest, unsigned h, uint64_t now,
                PosePacket &out, Receipt &receipt) noexcept {
        receipt = {};
        if (h >= 2) return false;
        auto &slot = hands_[h];
        if (!slot.pending) return false;
        if (!eligible(latest, h) || !context(slot.pulse, latest, h) || now < slot.receivedMs ||
            now - slot.receivedMs > MaxPoseAgeMs) {
            slot.pending = false; slot.pulse = {}; return false;
        }
        out = slot.pulse;
        receipt = {this, h, slot.pulse.gestureSequence[h], slot.receivedMs,
                   context(slot.pulse, h)};
        return true;
    }
    // Native high completion or an explicit rejected/discarded probe retires
    // precisely this retained edge. Held/low latest poses and sender ACKs do not.
    bool finish(const Receipt &receipt) noexcept {
        if (receipt.owner != this || receipt.hand >= 2) return false;
        auto &slot = hands_[receipt.hand];
        if (!slot.pending || slot.generation != receipt.context.gestureGeneration ||
            context(slot.pulse, receipt.hand) != receipt.context ||
            slot.pulse.gestureSequence[receipt.hand] != receipt.sequence ||
            slot.receivedMs != receipt.receivedMs) return false;
        slot.pending = false;
        return true;
    }
    // Consuming the retained edge does not revoke an already admitted immutable
    // native interval. A newer edge, cancellation, context change or age does.
    bool current(const PosePacket &latest, const Receipt &receipt, uint64_t now) const noexcept {
        if (receipt.owner != this || receipt.hand >= 2) return false;
        const auto &slot = hands_[receipt.hand];
        return eligible(latest, receipt.hand) && context(slot.pulse, latest, receipt.hand) &&
            context(slot.pulse, receipt.hand) == receipt.context &&
            slot.generation == receipt.context.gestureGeneration && slot.pulse.gestureSequence[receipt.hand] == receipt.sequence &&
            slot.receivedMs == receipt.receivedMs && now >= slot.receivedMs &&
            now - slot.receivedMs <= MaxPoseAgeMs;
    }
};
} // namespace ss2vr::network
