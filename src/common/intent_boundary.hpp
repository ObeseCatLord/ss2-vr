#pragma once
#include <cstdint>

namespace ss2vr {
// One hand's causal boundary inside an existing capability. The native adapter
// owns ACK validation and cancellation; this helper only excludes samples that
// could have existed before the acknowledged native weapon boundary.
struct IntentInputBoundary {
    uint32_t epoch = 0;
    uint64_t inputSequence = 0, receiptMs = 0;
    bool install(uint32_t nextEpoch, uint64_t lastInputSequence, uint64_t now) {
        if (!nextEpoch || nextEpoch <= epoch)
            return false;
        epoch = nextEpoch;
        inputSequence = lastInputSequence;
        receiptMs = now;
        return true;
    }
    uint32_t echo(uint64_t sequence, uint64_t tickMs, uint64_t now) const {
        return epoch && sequence > inputSequence && tickMs > receiptMs && tickMs <= now ? epoch : 0;
    }
};
} // namespace ss2vr
