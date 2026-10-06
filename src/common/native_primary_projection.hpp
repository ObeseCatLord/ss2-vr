#pragma once
#include <cstdint>

namespace ss2vr {
// Snapshot.initialized is established only by successful enabled tracking.
// Channel availability, pawn enumeration and a local transport nonce do not
// establish XR ownership. Current tracking validity controls admission later;
// ordinary focus/head/hand loss does not turn an initialized owner into desktop.
constexpr bool nativePrimaryLocalRecognized(const void *subject, uint32_t handle,
                                            const void *snapshotPlayer, uint32_t snapshotHandle,
                                            bool initialized) noexcept {
    return initialized && subject && handle && subject == snapshotPlayer && handle == snapshotHandle;
}

enum class NativePrimaryOwnership : uint8_t { Unclaimed, Handheld, Carry };
// resolvedCarry is the result of the native handle resolver on puppet+564,
// not a ride flag or merely a nonzero handle. Carry consumes native primary for
// throw preparation/release and is outside the tracked-handheld projection.
constexpr NativePrimaryOwnership nativePrimaryOwnership(bool recognized, bool resolvedCarry) noexcept {
    return resolvedCarry ? NativePrimaryOwnership::Carry :
           recognized ? NativePrimaryOwnership::Handheld : NativePrimaryOwnership::Unclaimed;
}

// A view of native current bits, never an attack/edge/history state machine.
// Unclaimed consumers retain the original byte. A recognized but inadmissible
// consumer owns both primary lanes even when its projected value is zero.
struct NativePrimaryValue {
    uint8_t mask = 0, bits = 0;
    static constexpr NativePrimaryValue neutral() noexcept { return {0x03, 0}; }
    constexpr uint32_t merge(uint32_t original) const noexcept {
        return (original & ~uint32_t(mask)) | (uint32_t(bits) & mask);
    }
};

// Revoke only the existing prepared record. Its interval never regains high;
// stack-owned invocation copies keep their immutable value until return.
constexpr void nativePrimaryRevoke(NativePrimaryValue &prepared, bool &revoked) noexcept {
    revoked = true;
    prepared.bits = 0;
}

// Indices are results of the ORIGINAL GetWeaponFiringButton, not a VR mapping
// table. The caller has already verified actual hand/owner/noncarrying identity.
constexpr NativePrimaryValue nativePrimaryProjection(uint8_t hands, uint8_t fire,
                                                     int left, int right,
                                                     bool uncoupledDual) noexcept {
    auto value = NativePrimaryValue::neutral();
    if (!hands || (hands & ~3u) ||
        ((hands & 1) && (left < 0 || left > 1)) ||
        ((hands & 2) && (right < 0 || right > 1)) ||
        (hands == 3 && (!uncoupledDual || left == right)))
        return value;
    if ((hands & fire & 1) != 0) value.bits |= uint8_t(1u << left);
    if ((hands & fire & 2) != 0) value.bits |= uint8_t(1u << right);
    return value;
}

// Stack-owned invocation copies survive prepared-record revocation. Finding an
// ancestor before root/caller checks also handles A -> B -> A native nesting.
struct NativePrimaryInvocation {
    const void *subject = nullptr;
    NativePrimaryValue value{};
    const NativePrimaryInvocation *previous = nullptr;
    bool held = false;
    // Only an exact, independently admitted weapon-held invocation may supply
    // this overlay. Operator down/press/release/history reads ignore it, so
    // physical motion cannot enter native manual command history. Existing
    // callers leave it empty; it is never inherited merely by pawn identity.
    NativePrimaryValue gesture{};
};
constexpr const NativePrimaryInvocation *nativePrimaryInherited(
    const NativePrimaryInvocation *frame, const void *subject) noexcept {
    for (; frame; frame = frame->previous)
        if (frame->subject == subject) return frame;
    return nullptr;
}
constexpr uint32_t nativePrimaryRead(uint32_t original, const void *subject, unsigned kind,
                                    const NativePrimaryInvocation *frame) noexcept {
    if (!frame || frame->subject != subject || kind > 4 || frame->held != (kind == 4))
        return original;
    const uint32_t manual = frame->value.merge(original);
    return kind == 4 ? manual | uint32_t(frame->gesture.mask & frame->gesture.bits & 3u) : manual;
}
} // namespace ss2vr
