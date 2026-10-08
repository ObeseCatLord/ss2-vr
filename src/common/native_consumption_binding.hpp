#pragma once
#include "native_consumption_receipt.hpp"
#include <optional>

namespace ss2vr {
// One subrecord attached to an existing native-hand owner, never another entity
// lookup table. Input/pose snapshots may be copied without copying this owner.
// Not yet used by native hooks; ownership and completion admission remain separate.
struct NativeConsumptionBindingKey {
    uint32_t player=0,weapon=0;
    unsigned hand=2;
    bool operator==(const NativeConsumptionBindingKey&) const = default;
    bool valid() const noexcept {return player&&weapon&&hand<2;}
};
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
