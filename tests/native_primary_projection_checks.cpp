#include "common/native_primary_projection.hpp"
using namespace ss2vr;

// Compile-time checks run in a compile-only job too. They verify the scalar
// contract, not native callback closure, hook installation or game behavior.
constexpr bool preservation() {
    for (uint32_t raw = 0; raw < 256; ++raw)
        for (uint8_t bits = 0; bits < 4; ++bits) {
            const NativePrimaryValue projection{3, bits};
            const uint32_t original = 0xa1b2c300u | raw;
            const auto result = projection.merge(original);
            if ((result & 3) != bits || (result & ~3u) != (original & ~3u)) return false;
            if (NativePrimaryValue{}.merge(original) != original) return false;
            if (NativePrimaryValue::neutral().merge(original) != (original & ~3u)) return false;
        }
    return true;
}
static_assert(preservation(), "Only the two managed primary bits may change, including CL upper bits");

constexpr bool localOwnershipRecognition() {
    int player = 0, replacement = 0;
    constexpr uint32_t handle = 7;
    // These are the Snapshot fields consumed by the production helper. A
    // populated player/handle alone occurs in all three startup regressions:
    // no HMD, renderer-not-ready, and a local nonce before enabled tracking.
    bool initialized = false;
    const auto recognized = [&] {
        return nativePrimaryLocalRecognized(&player, handle, &player, handle, initialized);
    };
    if (recognized()) return false;
    NativePrimaryInvocation startup{&player, recognized() ? NativePrimaryValue::neutral() : NativePrimaryValue{},
                                    nullptr, false};
    NativePrimaryInvocation reenteredHeld{&player, nativePrimaryInherited(&startup, &player)->value,
                                          &startup, true};
    for (uint32_t byte = 0; byte < 256; ++byte) {
        const auto original = 0xabcd1200u | byte;
        for (unsigned kind = 0; kind < 4; ++kind)
            if (nativePrimaryRead(original, &player, kind, &startup) != original) return false;
        if (nativePrimaryRead(original, &player, 4, &reenteredHeld) != original) return false;
    }
    initialized = true; // Existing update publishes its first enabled tracking.
    if (!recognized()) return false;
    // Ordinary focus/head/hand loss leaves initialized set. Recognition must
    // survive failed current admission, rather than expose raw desktop high.
    NativePrimaryInvocation trackingLost{&player, recognized() ? NativePrimaryValue::neutral() : NativePrimaryValue{},
                                         nullptr, true};
    if (nativePrimaryRead(0xabcd1203u, &player, 4, &trackingLost) != 0xabcd1200u) return false;
    if (nativePrimaryLocalRecognized(&replacement, handle, &player, handle, initialized) ||
        nativePrimaryLocalRecognized(&player, handle + 1, &player, handle, initialized) ||
        nativePrimaryLocalRecognized(&player, 0, &player, 0, initialized)) return false;
    initialized = false; // Existing identity/session/reference/rider reset.
    return !recognized();
}
static_assert(localOwnershipRecognition(),
              "No-HMD/renderer-unready/nonce-only startup preserves native primary; established tracking loss owns neutral");

static_assert(nativePrimaryProjection(2, 2, -1, 0, false).bits == 1, "Single right uses actual index0");
static_assert(nativePrimaryProjection(1, 1, 1, -1, false).bits == 2, "Single left can use native index1");
static_assert(nativePrimaryProjection(1, 1, 0, -1, false).bits == 1, "Single left can also use index0");
static_assert(nativePrimaryProjection(3, 1, 1, 0, true).bits == 2, "Uncoupled left native mapping");
static_assert(nativePrimaryProjection(3, 2, 1, 0, true).bits == 1, "Uncoupled right native mapping");
static_assert(nativePrimaryProjection(3, 1, 0, 1, true).bits == 1, "Native flipped mapping is supplied, not invented");
static_assert(nativePrimaryProjection(3, 3, 0, 1, true).bits == 3, "Both native lanes remain independent");
static_assert(nativePrimaryProjection(3, 3, 0, 0, true).bits == 0, "Aliased native mapping is neutral");
static_assert(nativePrimaryProjection(3, 3, 0, 1, false).bits == 0, "Unsupported coupled dual is neutral");
static_assert(nativePrimaryProjection(2, 3, -1, 2, true).bits == 0, "Out-of-range native index is neutral");
static_assert(nativePrimaryProjection(0, 3, -1, -1, false).mask == 3, "Absent weapons retain recognized neutral");

constexpr bool invocationCopies() {
    int pawnA = 0, pawnB = 0;
    auto prepared = nativePrimaryProjection(3, 2, 1, 0, true);
    NativePrimaryInvocation outer{&pawnA, prepared, nullptr, false};
    prepared.bits = 0; // Deletion/expiry revokes future admission, not this call.
    NativePrimaryInvocation other{&pawnB, NativePrimaryValue::neutral(), &outer, false};
    const auto *ancestor = nativePrimaryInherited(&other, &pawnA);
    if (ancestor != &outer || ancestor->value.bits != 1) return false;
    NativePrimaryInvocation nested{&pawnA, ancestor->value, &other, false};
    NativePrimaryInvocation held{&pawnA, nested.value, &nested, true};
    for (unsigned kind = 0; kind < 4; ++kind)
        if (nativePrimaryRead(0x123400acu, &pawnA, kind, &nested) != 0x123400adu) return false;
    if (nativePrimaryRead(0xac, &pawnA, 4, &held) != 0xad) return false;
    if (nativePrimaryRead(0xac, &pawnB, 4, &held) != 0xac) return false;
    if (nativePrimaryRead(0xac, &pawnA, 3, &held) != 0xac) return false;
    if (nativePrimaryRead(0xac, &pawnA, 4, &outer) != 0xac) return false;
    if (nativePrimaryRead(0xac, &pawnA, 5, &held) != 0xac) return false;
    if (nativePrimaryRead(0xac, &pawnA, 0, nullptr) != 0xac) return false;
    NativePrimaryInvocation later{&pawnA, prepared, nullptr, true};
    if (nativePrimaryRead(0xaf, &pawnA, 4, &later) != 0xac) return false;
    // Recognition before preparation/getters supplies neutral to native reentry.
    NativePrimaryInvocation preparing{&pawnA, NativePrimaryValue::neutral(), nullptr, false};
    return nativePrimaryInherited(&preparing, &pawnA)->value.merge(3) == 0 && outer.value.bits == 1;
}
static_assert(invocationCopies(), "Immutable A->B->A inheritance survives record revocation and inner commits");

constexpr bool admissionRecognition() {
    int desktop = 0, vr = 0;
    // The wrappers publish mask0 before recognition, including while resolving
    // an unknown subject's handle. A desktop admission cannot lend mask03.
    NativePrimaryInvocation unclaimed{&desktop, {}, nullptr, false};
    NativePrimaryInvocation other{&vr, NativePrimaryValue::neutral(), &unclaimed, false};
    const auto *ancestor = nativePrimaryInherited(&other, &desktop);
    if (!ancestor || ancestor->value.mask) return false;
    NativePrimaryInvocation nested{&desktop, ancestor->value, &other, false};
    NativePrimaryInvocation nestedHeld{&desktop, ancestor->value, &nested, true};
    for (uint32_t byte = 0; byte < 256; ++byte) {
        const uint32_t original = 0xfedcba00u | byte;
        for (unsigned kind = 0; kind < 4; ++kind)
            if (nativePrimaryRead(original, &desktop, kind, &nested) != original) return false;
        if (nativePrimaryRead(original, &desktop, 4, &nestedHeld) != original) return false;
    }
    // Only established VR recognition reserves neutral before validation's
    // native getters. An admitted ancestor still lends its exact immutable V.
    NativePrimaryInvocation recognized{&vr, {}, nullptr, false};
    recognized.value = NativePrimaryValue::neutral();
    NativePrimaryInvocation duringGetters{&vr, nativePrimaryInherited(&recognized, &vr)->value,
                                          &recognized, true};
    if (nativePrimaryRead(0xfedcba03u, &vr, 4, &duringGetters) != 0xfedcba00u) return false;
    recognized.value = {3, 2}; // Admission finishes before original invocation.
    NativePrimaryInvocation activeNested{&vr, nativePrimaryInherited(&recognized, &vr)->value,
                                         &recognized, false};
    return nativePrimaryRead(0xfedcba01u, &vr, 3, &activeNested) == 0xfedcba02u &&
           unclaimed.value.mask == 0;
}
static_assert(admissionRecognition(), "Admission reentry preserves unclaimed original and recognized exact inheritance");

constexpr bool carryOwnership() {
    int pawn = 0;
    if (nativePrimaryOwnership(true, true) != NativePrimaryOwnership::Carry ||
        nativePrimaryOwnership(false, true) != NativePrimaryOwnership::Carry ||
        nativePrimaryOwnership(true, false) != NativePrimaryOwnership::Handheld ||
        nativePrimaryOwnership(false, false) != NativePrimaryOwnership::Unclaimed) return false;
    const auto preparedHandheld = nativePrimaryProjection(2, 2, -1, 0, false);
    // Carry was entered AFTER a handheld high was prepared. The production
    // ownership decision supersedes that record for this independent invocation.
    const auto ownership = nativePrimaryOwnership(true, true);
    NativePrimaryInvocation outer{&pawn, ownership == NativePrimaryOwnership::Handheld ?
                                  preparedHandheld : NativePrimaryValue{}, nullptr, false};
    NativePrimaryInvocation nested{&pawn, nativePrimaryInherited(&outer, &pawn)->value, &outer, false};
    NativePrimaryInvocation held{&pawn, nativePrimaryInherited(&nested, &pawn)->value, &nested, true};
    // Native press, held and release cases. Current/history inputs must remain
    // exact: neutralizing current1/history1 would fabricate a native throw.
    constexpr uint32_t current[] = {0xad, 0xad, 0xac};
    constexpr uint32_t prior[] = {0xac, 0xad, 0xad};
    for (unsigned phase = 0; phase < 3; ++phase) {
        for (unsigned kind = 0; kind < 4; ++kind)
            if (nativePrimaryRead(current[phase], &pawn, kind, &nested) != current[phase]) return false;
        const auto actual = nativePrimaryRead(current[phase], &pawn, 4, &held);
        if (actual != current[phase]) return false;
        if (bool((actual & 1) && !(prior[phase] & 1)) != (phase == 0)) return false;
        if (bool(!(actual & 1) && (prior[phase] & 1)) != (phase == 2)) return false;
    }
    // An already active admitted ancestor remains first; a later carry change
    // cannot rewrite its immutable value or its original native history commit.
    NativePrimaryInvocation activeHandheld{&pawn, preparedHandheld, nullptr, false};
    NativePrimaryInvocation inherited{&pawn, nativePrimaryInherited(&activeHandheld, &pawn)->value,
                                      &activeHandheld, false};
    return inherited.value.mask == 3 && inherited.value.bits == preparedHandheld.bits &&
           outer.value.mask == 0;
}
static_assert(carryOwnership(), "Resolved native carry preserves press/hold/release and nested native current/history");

constexpr bool carryRevokesPreparedHigh() {
    int pawn = 0;
    for (uint8_t high = 1; high <= 3; ++high) {
        auto prepared = nativePrimaryProjection(3, high, 0, 1, true);
        bool revoked = false;
        const NativePrimaryInvocation admitted{&pawn, prepared, nullptr, false};
        if (prepared.bits != high || revoked) return false;

        // Independent admission observes live carry after preparation. Use the
        // same ownership and record-revocation helpers as primaryAdmission.
        if (nativePrimaryOwnership(true, true) != NativePrimaryOwnership::Carry) return false;
        nativePrimaryRevoke(prepared, revoked);
        if (!revoked || prepared.bits || prepared.mask != 3) return false;
        NativePrimaryInvocation carrying{&pawn, {}, nullptr, false};
        NativePrimaryInvocation nestedCarry{&pawn, nativePrimaryInherited(&carrying, &pawn)->value,
                                            &carrying, true};
        if (nativePrimaryRead(0xabcd1203u, &pawn, 4, &nestedCarry) != 0xabcd1203u) return false;

        // Carry ends in the SAME interval. Ownership is handheld again, but
        // neither ownership selection nor validation replenishes this record.
        if (nativePrimaryOwnership(true, false) != NativePrimaryOwnership::Handheld) return false;
        if (!revoked || prepared.bits) return false;
        NativePrimaryInvocation laterHandheld{&pawn, prepared, nullptr, false};
        NativePrimaryInvocation laterHeld{&pawn, nativePrimaryInherited(&laterHandheld, &pawn)->value,
                                          &laterHandheld, true};
        for (unsigned kind = 0; kind < 4; ++kind)
            if (nativePrimaryRead(0xabcd1203u, &pawn, kind, &laterHandheld) != 0xabcd1200u) return false;
        if (nativePrimaryRead(0xabcd1203u, &pawn, 4, &laterHeld) != 0xabcd1200u) return false;

        // Revocation is idempotent and cannot change an already admitted copy.
        nativePrimaryRevoke(prepared, revoked);
        const auto *ancestor = nativePrimaryInherited(&admitted, &pawn);
        if (!revoked || prepared.bits || ancestor->value.bits != high || ancestor->value.mask != 3) return false;
    }
    return true;
}
static_assert(carryRevokesPreparedHigh(),
              "Prepared high -> live carry -> handheld stays revoked/low in the same interval; active copies survive");

constexpr bool nativeReadAgreement() {
    // Raw accepted byte deliberately stays low: retained high then low must
    // reach native edge/current/history/held inputs without replaying an action.
    constexpr uint32_t raw = 0xacu; // unmanaged bits2/3 remain high
    constexpr NativePrimaryValue high{3, 1}, low = NativePrimaryValue::neutral();
    const auto highCurrent = high.merge(raw);
    const uint8_t nativeHistory = uint8_t(highCurrent); // original 8E75A store
    const auto lowCurrent = low.merge(raw);
    if (!(highCurrent & 1) || (lowCurrent & 1) || !(nativeHistory & 1)) return false;
    if ((highCurrent & 0xfc) != raw || (lowCurrent & 0xfc) != raw) return false;
    // Native held blocking and native release predicate consume the same byte.
    for (unsigned block = 0; block < 4; ++block) {
        const bool heldHigh = !(block & 1) && (highCurrent & 1);
        const bool released = (nativeHistory & 1) && !(lowCurrent & 1);
        if (heldHigh != !(block & 1) || !released) return false;
        // Blocked release is left for native code to clear; adapter adds no callback.
        if ((lowCurrent & 1) != 0) return false;
    }
    return true;
}
static_assert(nativeReadAgreement(), "Retained high/low join the actual current/history scalar contract");
int main() { return 0; }

constexpr bool gestureCannotBecomeManualHistory() {
    int pawn = 0, other = 0;
    for (uint32_t raw = 0; raw < 256; ++raw) {
        for (uint8_t manual = 0; manual < 4; ++manual) {
            for (uint8_t gesture = 0; gesture < 4; ++gesture) {
                const uint32_t original = 0xabcd1200u | raw;
                NativePrimaryInvocation operatorFrame{&pawn, {3, manual}, nullptr, false, {3, gesture}};
                for (unsigned kind = 0; kind < 4; ++kind)
                    if (nativePrimaryRead(original, &pawn, kind, &operatorFrame) !=
                        ((original & ~3u) | manual)) return false;
                NativePrimaryInvocation held{&pawn, {3, manual}, &operatorFrame, true, {3, gesture}};
                if (nativePrimaryRead(original, &pawn, 4, &held) !=
                    ((original & ~3u) | manual | gesture)) return false;
                if (nativePrimaryRead(original, &other, 4, &held) != original) return false;
                // An unclaimed manual view is additive for an independently
                // admitted observer-held call; it never clears stock input.
                held.value = {};
                if (nativePrimaryRead(original, &pawn, 4, &held) != (original | gesture)) return false;
                // A nested observer manual read shadows the active held frame
                // without projecting or inheriting G. Restoring it leaves the
                // ancestor's independently admitted held read unchanged.
                NativePrimaryInvocation rawManual{&pawn, {}, &held, true, {}};
                if (nativePrimaryRead(original, &pawn, 4, &rawManual) != original) return false;
                if (nativePrimaryRead(original, &pawn, 4, rawManual.previous) != (original | gesture)) return false;
                // Constructing another held view from the same-pawn ancestor
                // copies manual state only, never another weapon's gesture.
                NativePrimaryInvocation another{&pawn, operatorFrame.value, &held, true};
                if (nativePrimaryRead(original, &pawn, 4, &another) !=
                    ((original & ~3u) | manual)) return false;
            }
        }
    }
    NativePrimaryInvocation invalidMask{&pawn, {}, nullptr, true, {255, 255}};
    return nativePrimaryRead(0x123456a0, &pawn, 4, &invalidMask) == 0x123456a3;
}
static_assert(gestureCannotBecomeManualHistory(),
              "Gesture overlay is held-only, additive, bounded to primary lanes and never inherited by pawn alone");
