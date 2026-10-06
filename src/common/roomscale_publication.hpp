#pragma once
#include <cstdint>
namespace ss2vr::roomscale {
enum class PublicationAction:uint8_t {ignore,retireWithoutOrigin,publishOrigin,quarantine};
// Cleanup metadata only. An uncertain write is never reclassified as a clean
// refusal. A superseding/uninitialized owner must not receive the old origin.
constexpr PublicationAction publicationAction(bool currentTicket,bool mayHaveMoved,
    bool resetOwner,bool settled,bool sameOrigin) noexcept {
    if(!currentTicket)return PublicationAction::ignore;
    if(!mayHaveMoved||resetOwner)return PublicationAction::retireWithoutOrigin;
    if(settled&&sameOrigin)return PublicationAction::publishOrigin;
    return PublicationAction::quarantine;
}
} // namespace ss2vr::roomscale
