#pragma once
#include <atomic>
#include <cstdint>
#include <limits>

namespace ss2vr {
// Internal body/origin publication identity, independent of input generations
// and network intent epochs. Odd revisions hide an incomplete rig transition.
// This does not lock native state. The owner holds the existing snapshot lock
// while changing snapshot fields and completing/recovering a transition.
class RigRevision {
    std::atomic<uint64_t> revision{0};
public:
    explicit constexpr RigRevision(uint64_t initial=0) noexcept:revision(initial) {}
    uint64_t current() const noexcept {return revision.load(std::memory_order_acquire);}
    bool usable(uint64_t captured) const noexcept {return !(captured&1u)&&captured==current();}
    uint64_t begin(uint64_t expected) noexcept {
        if((expected&1u)||expected>=std::numeric_limits<uint64_t>::max()-1)return 0;
        return revision.compare_exchange_strong(expected,expected+1,std::memory_order_acq_rel)?expected+1:0;
    }
    bool finish(uint64_t ticket) noexcept {
        if(!(ticket&1u)||ticket==std::numeric_limits<uint64_t>::max())return false;
        return revision.compare_exchange_strong(ticket,ticket+1,std::memory_order_acq_rel);
    }
    // Use only with a genuinely new tracking origin/owner, never just to clear
    // an uncertain movement failure. The caller also owns ordinary epoch resets.
    uint64_t recoverForNewOrigin() noexcept {
        auto value=current();
        while(value&1u) {
            if(value==std::numeric_limits<uint64_t>::max())return value;
            if(revision.compare_exchange_weak(value,value+1,std::memory_order_acq_rel))return value+1;
        }
        return value;
    }
};
} // namespace ss2vr
