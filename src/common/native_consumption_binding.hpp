#pragma once
#include "native_consumption_receipt.hpp"
#include <optional>

namespace ss2vr {
// Every admitted high must carry its own quiet witness after the native stop
// fence. Later arming cannot lend permission to an older delayed source pulse.
constexpr bool nativeGestureQuietWitness(uint64_t after, uint64_t sequence, uint64_t tick,
                                        uint64_t quietSequence, uint64_t quietTick) noexcept {
    return quietSequence > after && quietSequence <= sequence && quietTick && quietTick <= tick;
}
// Unknown is not Low. A separately admitted fresh gesture may request a real
// native press, whose normal completion establishes High. Unknown/manual-only
// awaits the real operator callback; denied and in-flight requests do nothing.
constexpr NativeConsumptionRecord::Edge nativeGestureReconcileEdge(
    NativeConsumptionRecord::Edge edge, bool admitted, bool physical, bool inFlight) noexcept {
    using Edge = NativeConsumptionRecord::Edge;
    if (!admitted || inFlight) return Edge::None;
    if (edge == Edge::Unknown) return physical ? Edge::Press : Edge::None;
    return edge;
}
constexpr bool nativeGestureFenceCurrent(bool armed, bool reset, bool sameGeneration,
    uint64_t after, uint64_t sequence, uint64_t tick, uint64_t quietSequence, uint64_t quietTick) noexcept {
    return armed && !reset && sameGeneration &&
        nativeGestureQuietWitness(after, sequence, tick, quietSequence, quietTick);
}
// One subrecord attached to an existing native-hand owner, never another entity
// lookup table. Input/pose snapshots may be copied without copying this owner.
// Used by the default-off unique-saw native adapter; ownership and exact native
// completion admission remain separate from this metadata helper.
struct NativeConsumptionBindingKey {
    uint32_t player=0,weapon=0;
    unsigned hand=2;
    bool operator==(const NativeConsumptionBindingKey&) const = default;
    bool valid() const noexcept {return player&&weapon&&hand<2;}
};
constexpr bool nativeConsumptionDispatchMatches(
    NativeConsumptionBindingKey active, uint64_t activeEpoch, bool activeAuthority, unsigned activeSlot,
    NativeConsumptionBindingKey current, uint64_t currentEpoch, bool currentAuthority, unsigned currentSlot) noexcept {
    return active == current && activeEpoch == currentEpoch &&
        activeAuthority == currentAuthority && activeSlot == currentSlot;
}
class NativeConsumptionBinding {
public:
    using Edge=NativeConsumptionRecord::Edge;
    using Receipt=NativeConsumptionRecord::Receipt;
    NativeConsumptionBinding()=default;
    NativeConsumptionBinding(const NativeConsumptionBinding&)=delete;
    NativeConsumptionBinding& operator=(const NativeConsumptionBinding&)=delete;
    NativeConsumptionBinding(NativeConsumptionBinding&&)=delete;
    NativeConsumptionBinding& operator=(NativeConsumptionBinding&&)=delete;

    // The existing owner supplies a new monotonically increasing lifetime epoch.
    // Replacement is explicit: never discard an accounted high in a generic
    // sample save, tracking refresh, same-key activation or capability update.
    bool activate(NativeConsumptionBindingKey key,uint64_t epoch) noexcept {
        if(active_||!key.valid()||!epoch||epoch<=lastEpoch_)return false;
        record_.emplace(epoch);
        key_=key;lastEpoch_=epoch;active_=true;
        return true;
    }
    bool matches(NativeConsumptionBindingKey key) const noexcept {
        return active_&&key==key_;
    }
    Edge edge(NativeConsumptionBindingKey key,bool desiredHigh) const noexcept {
        return matches(key)?record_->edge(desiredHigh):Edge::Unknown;
    }
    Receipt prepare(NativeConsumptionBindingKey key) const noexcept {
        return matches(key)?record_->prepare():Receipt{};
    }
    // Reacquire the existing owner and its current subrecord before calling.
    // A callback must not retain a pointer into an optional that may be replaced.
    bool complete(NativeConsumptionBindingKey key,const Receipt& receipt,bool high) noexcept {
        return matches(key)&&record_->complete(receipt,high);
    }
    void retire() noexcept {
        if(record_)record_->retire();
        active_=false;
    }
    // Reacquire by identity before an aborted native operation. Do not retire
    // a newer completed nested observation or a replacement at the same slot.
    bool retire(NativeConsumptionBindingKey key,const Receipt& receipt) noexcept {
        if(!matches(key)||!record_->retire(receipt))return false;
        active_=false;
        return true;
    }
private:
    std::optional<NativeConsumptionRecord> record_;
    NativeConsumptionBindingKey key_{};
    uint64_t lastEpoch_=0;
    bool active_=false;
};
} // namespace ss2vr
