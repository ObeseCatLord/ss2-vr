#pragma once
#include "math.hpp"
#include <array>
#include <cstdint>
#include <limits>

namespace ss2vr::roomscale {
// Provisional origin-coordinate history, not a movement request or input ACK.
// Native prediction/replication ownership is not implemented by this header.
// Keep double cumulative coordinates so a long session does not repeatedly add
// small accepted movements to a large float accumulator.
struct StageOffset {
    double x=0, y=0, z=0;
};
inline bool finiteOffset(StageOffset p) {
    return std::isfinite(p.x)&&std::isfinite(p.y)&&std::isfinite(p.z);
}
// Each retained hand pose needs its OWN capture basis/revision. A historical
// tap must never borrow the newer head pose's turn or consumed-origin revision.
struct OriginFrame {
    Quat stageToBody{};
    uint32_t revision=0;
};
inline bool validOriginFrame(const OriginFrame& frame) {
    const auto q=frame.stageToBody;
    if(!finite(Pose{q,{}}))return false;
    const double norm=double(q.x)*q.x+double(q.y)*q.y+double(q.z)*q.z+double(q.w)*q.w;
    return std::abs(norm-1.0)<=32*std::numeric_limits<float>::epsilon();
}
template<size_t Capacity=128> class OriginLedger {
    static_assert(Capacity>=2);
    struct Entry {uint32_t revision=0;StageOffset offset{};};
    std::array<Entry,Capacity> entries_{};
    size_t begin_=0,count_=0;
    uint32_t epoch_=0;
public:
    void reset(uint32_t epoch=0) noexcept {
        begin_=0;count_=epoch?1:0;epoch_=epoch;
        entries_[0]={};
    }
    uint32_t epoch() const noexcept {return epoch_;}
    size_t size() const noexcept {return count_;}
    uint32_t revision() const noexcept {
        return count_?entries_[(begin_+count_-1)%Capacity].revision:0;
    }
    bool lookup(uint32_t epoch,uint32_t revision,StageOffset& result) const noexcept {
        if(!epoch||epoch!=epoch_)return false;
        for(size_t i=0;i<count_;++i) {
            const auto& entry=entries_[(begin_+i)%Capacity];
            if(entry.revision==revision) {result=entry.offset;return true;}
        }
        return false;
    }
    // The caller must prove no pending/active/retained pose needs an older
    // revision. Never evict implicitly to make a new physical movement fit.
    bool retireBefore(uint32_t epoch,uint32_t revision) noexcept {
        StageOffset ignored;
        if(!lookup(epoch,revision,ignored))return false;
        while(count_>1&&entries_[begin_].revision<revision) {
            begin_=(begin_+1)%Capacity;--count_;
        }
        return true;
    }
    bool canAppend(uint32_t epoch,uint32_t expectedRevision) const noexcept {
        return epoch&&epoch==epoch_&&count_&&count_<Capacity&&
               expectedRevision==revision()&&expectedRevision!=UINT32_MAX;
    }
    // Preflight capacity with canAppend BEFORE native movement. It is not an
    // ownership lease; the native owner must still exclude superseding changes.
    // expectedRevision closes nested/stale completion. This records ACTUAL
    // accepted native motion only, after the owner established its completion.
    // Bounds are per movement and per epoch; failure leaves the ledger intact.
    bool append(uint32_t epoch,uint32_t expectedRevision,StageOffset actualDelta,
                double maximumStep,double maximumCumulative) noexcept {
        if(!canAppend(epoch,expectedRevision)||
           !finiteOffset(actualDelta)||!std::isfinite(maximumStep)||maximumStep<=0||
           !std::isfinite(maximumCumulative)||maximumCumulative<=0)return false;
        const double distance=std::hypot(actualDelta.x,actualDelta.y,actualDelta.z);
        if(!std::isfinite(distance)||distance>maximumStep)return false;
        const auto previous=entries_[(begin_+count_-1)%Capacity].offset;
        const StageOffset next{previous.x+actualDelta.x,previous.y+actualDelta.y,previous.z+actualDelta.z};
        if(!finiteOffset(next)||std::abs(next.x)>maximumCumulative||
           std::abs(next.y)>maximumCumulative||std::abs(next.z)>maximumCumulative)return false;
        entries_[(begin_+count_)%Capacity]={expectedRevision+1,next};++count_;
        return true;
    }
};

// Coordinates only: callers preserve every intent, raw release witness, weapon
// identity, consumption token and generation. Unknown/retired history fails;
// do not normalize a basis, invent a neutral sample or rebase a newer owner.
template<size_t Capacity>
inline bool rebaseBodyPose(const OriginLedger<Capacity>& ledger,uint32_t epoch,
                          const OriginFrame& frame,const Pose& captured,Pose& result) {
    StageOffset from,to;
    if(!finite(captured)||!validOriginFrame(frame)||
       !ledger.lookup(epoch,frame.revision,from)||!ledger.lookup(epoch,ledger.revision(),to))return false;
    const double x=to.x-from.x,y=to.y-from.y,z=to.z-from.z;
    const auto q=frame.stageToBody;
    // Quaternion expansion in double avoids converting the cumulative delta to
    // float before rotating it. The captured native orientation is unchanged.
    const double tx=2*(double(q.y)*z-double(q.z)*y);
    const double ty=2*(double(q.z)*x-double(q.x)*z);
    const double tz=2*(double(q.x)*y-double(q.y)*x);
    const double px=double(captured.p.x)-(x+double(q.w)*tx+double(q.y)*tz-double(q.z)*ty);
    const double py=double(captured.p.y)-(y+double(q.w)*ty+double(q.z)*tx-double(q.x)*tz);
    const double pz=double(captured.p.z)-(z+double(q.w)*tz+double(q.x)*ty-double(q.y)*tx);
    constexpr double limit=std::numeric_limits<float>::max();
    if(!std::isfinite(px)||!std::isfinite(py)||!std::isfinite(pz)||
       std::abs(px)>limit||std::abs(py)>limit||std::abs(pz)>limit)return false;
    Pose candidate=captured;candidate.p={float(px),float(py),float(pz)};
    if(!finite(candidate))return false;
    result=candidate;return true;
}
} // namespace ss2vr::roomscale
