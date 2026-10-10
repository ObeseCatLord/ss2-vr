#pragma once
#include "controls.hpp"

namespace ss2vr {
// Reuses the existing availability stream: an omitted/failed world frame cannot
// disappear when latest-value publication skips that interval.
struct WorldSubmissionStream {
    ActionStream continuity;
    WorldSubmission value;
    void invalidate() noexcept {
        continuity.invalidate();value={};value.continuity=continuity.generation;
    }
    void sample(uint32_t producer,const Request &request,bool submitted) noexcept {
        const bool valid=producer && request.sequence && request.session && request.reference &&
            request.trackingGeneration && !(request.trackingGeneration&0x80000000u) &&
            request.input.tickMs && !request.reserved && request.input.session==request.session &&
            request.input.reference==request.reference;
        const bool active=continuity.sample(submitted && valid);
        value={};
        value.continuity=continuity.generation;
        if(active)value={request.sequence,request.input.tickMs,producer,request.session,request.reference,
            request.trackingGeneration,continuity.generation,1};
    }
};
inline bool submittedWorldMatches(const WorldSubmission &value,uint32_t producer,const Request &request,
                                 uint64_t now,uint64_t maximumAge=100) noexcept {
    return value.active==1 && value.continuity && producer && value.producer==producer &&
        value.requestSequence==request.sequence && value.sourceTickMs==request.input.tickMs &&
        value.session==request.session && value.reference==request.reference &&
        value.trackingGeneration==request.trackingGeneration && value.sourceTickMs &&
        now>=value.sourceTickMs && now-value.sourceTickMs<=maximumAge;
}
} // namespace ss2vr
