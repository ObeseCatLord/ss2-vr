#pragma once
#include "roomscale_body_geometry.hpp"
namespace ss2vr::roomscale {
// Actual native query filters only. This is not a collision-shape description,
// and these values never establish ownership of the native pointers themselves.
struct SphereQuerySubject {
    uint32_t avatar=0,mechanism=0;
    std::array<uint32_t,BodyGeometry::MaximumHulls> categories{};
    uint32_t categoryCount=0;
    bool operator==(const SphereQuerySubject&) const = default;
    bool valid() const noexcept {
        return avatar&&mechanism&&categoryCount&&categoryCount<=categories.size();
    }
};
static_assert(sizeof(SphereQuerySubject)==20&&offsetof(SphereQuerySubject,categories)==8);
inline SphereQuerySubject sphereSubjectForBody(const BodyGeometry& body) noexcept {
    SphereQuerySubject subject{body.player,body.mechanism,{},body.hullCount};
    if(!subject.valid())return {};
    for(unsigned i=0;i<body.hullCount;++i)subject.categories[i]=body.hulls[i].category;
    return subject;
}
} // namespace ss2vr::roomscale
