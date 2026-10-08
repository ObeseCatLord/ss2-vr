#pragma once
#include <cstdint>
#include <limits>

namespace ss2vr {

// Metadata for a completed native input consumption, NOT a combat state
// machine. The native binding owner must provide stable storage and a unique,
// nonzero lifetime epoch. Never embed this in a copied sample row.
// No native hook uses this helper yet; callback/lifetime admission is separate.
constexpr uint64_t nextConsumptionRevision(uint64_t revision) noexcept {
    return revision == std::numeric_limits<uint64_t>::max() ? 0 : revision + 1;
}

class NativeConsumptionRecord {
public:
    enum class Edge : uint8_t { Unknown, None, Press, Release };
    class Receipt {
        friend class NativeConsumptionRecord;
        const NativeConsumptionRecord *owner_ = nullptr;
        uint64_t epoch_ = 0, revision_ = 0;
        Receipt(const NativeConsumptionRecord *owner, uint64_t epoch, uint64_t revision) noexcept
            : owner_(owner), epoch_(epoch), revision_(revision) {}
    public:
        Receipt() = default;
    };

    explicit NativeConsumptionRecord(uint64_t uniqueBindingEpoch) noexcept
        : epoch_(uniqueBindingEpoch), live_(uniqueBindingEpoch != 0) {}
    NativeConsumptionRecord(const NativeConsumptionRecord &) = delete;
    NativeConsumptionRecord &operator=(const NativeConsumptionRecord &) = delete;
    NativeConsumptionRecord(NativeConsumptionRecord &&) = delete;
    NativeConsumptionRecord &operator=(NativeConsumptionRecord &&) = delete;

    // Unknown history is never interpreted as a consumed low. A caller may
    // only reconcile once a real, proven native completion establishes it.
    Edge edge(bool desiredHigh) const noexcept {
        if (!live_ || !known_) return Edge::Unknown;
        if (high_ == desiredHigh) return Edge::None;
        return desiredHigh ? Edge::Press : Edge::Release;
    }
    Receipt prepare() const noexcept {
        return live_ ? Receipt(this, epoch_, revision_) : Receipt{};
    }
    // Invoke ONLY after the exact native completion boundary is proved and
    // reached. An exception/aborted call does not call this method at all;
    // ambiguous partial native effects require the owner to retire the record.
    // A later nested completion wins even if it records the same logical level.
    bool complete(const Receipt &receipt, bool consumedHigh) noexcept {
        if (!current(receipt)) return false;
        revision_ = nextConsumptionRevision(revision_);
        if (!revision_) { retire(); return false; }
        high_ = consumedHigh;
        known_ = true;
        return true;
    }
    // Retirement is terminal. Reusing the storage requires a new lifetime
    // epoch; neither tracking recovery nor a stale sample may resurrect it.
    void retire() noexcept { live_ = false; known_ = false; }
    // Ambiguous callback cleanup may retire only the observation it entered
    // with. A completed inner callback supersedes an older outer unwind.
    // Actual lifetime teardown still uses unconditional owner retirement above.
    bool retire(const Receipt &receipt) noexcept {
        if (!current(receipt)) return false;
        retire();
        return true;
    }

private:
    bool current(const Receipt &receipt) const noexcept {
        return live_ && receipt.owner_ == this && receipt.epoch_ == epoch_ &&
               receipt.revision_ == revision_;
    }
    const uint64_t epoch_;
    uint64_t revision_ = 1;
    bool live_ = false, known_ = false, high_ = false;
};
} // namespace ss2vr
