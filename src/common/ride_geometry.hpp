#pragma once
#include "scope_geometry.hpp"
#include <cstdint>

namespace ss2vr {
enum class RideHandleProfile : uint32_t { Unknown,Fighter,Saucer };
// Resource fingerprints/ranges only; no proprietary coordinates are embedded.
inline RideHandleProfile rideHandleProfile(const std::array<std::array<uint8_t,32>,5> &digests) {
    constexpr const char *profiles[2][5]{
        {"9f33c7008308ed7d1687c8c9c0b80d4db600d9ceb4a33fbf5dfbbdd5b0e40a96",
         "3e9a89d387ca1c9dcdb579a8f5d40759409265073a137d6130e2a1c5fb721097",
         "3d44b5776ad377a00fa69c3e42ff430e2b1ea61e11f988ad712ce89ddd064635",
         "e846221764b3ab82b37af479040fba6b2c8044514929a551ca3b5d6834364b78",
         "b89ca733917eeca6ec07e0aea1f1b1c55d62849e4fb6aaa4067601a9ec1c317d"},
        {"54c8cef9a868fd2196d81c3d6adf907721ec719aa175e2f232b0ee20b82ff17c",
         "bf2e3e9bee6be7c7fa56b9c4465dabf7dda746e833349fa4af39585dd9a6c3ba",
         "e53a3b32b223127ad1e83f157b5cfc8f4c0bdcd48cd395d9cf720902d7e88885",
         "046c64d0f6c4e1c0afa4d86a34475043474297289babdfb950a35f84d1b56b96",
         "0f62607311ba155663cd6c71d0757cdf671eaa83faa3502ea66799e278a71fa8"}};
    constexpr char hex[]="0123456789abcdef";
    for(unsigned family=0;family<2;++family) {
        bool equal=true;
        for(unsigned channel=0;channel<5;++channel)for(unsigned byte=0;byte<32;++byte) {
            equal=equal && profiles[family][channel][2*byte]==hex[digests[channel][byte]>>4] &&
                profiles[family][channel][2*byte+1]==hex[digests[channel][byte]&15];
        }
        if(equal)return family?RideHandleProfile::Saucer:RideHandleProfile::Fighter;
    }
    return RideHandleProfile::Unknown;
}
struct RideHandleMesh {
    static constexpr unsigned MaxVertices=35,Triangles=30;
    std::array<Vec3,MaxVertices> positions{};
    std::array<std::array<float,2>,MaxVertices> uv{};
    std::array<std::array<uint8_t,4>,MaxVertices> localIndices{},weights{};
    std::array<uint16_t,Triangles*3> indices{};
    uint32_t vertices=0;
};
struct RideHandleGeometry {
    std::array<RideHandleMesh,2> handles{};
    RideHandleProfile profile=RideHandleProfile::Unknown;
    bool copied=false;
};
// Preconditions: all five OWNED slice digests have selected this exact profile.
// A subset may contain duplicate seam coordinates; retain its original vertices
// and winding. Neither a one-hot byte tuple nor this copy certifies a Main influence.
inline bool copyRideHandles(const ScopeSliceBytes &slices,RideHandleProfile profile,
                            uint32_t paletteCount,RideHandleGeometry &out) {
    out={};
    const bool fighter=profile==RideHandleProfile::Fighter;
    if((!fighter && profile!=RideHandleProfile::Saucer) || !paletteCount || paletteCount>32)return false;
    const unsigned vertices=fighter?2741:2464,triangles=fighter?2806:2626;
    if(slices.positions.size()!=vertices*12 || slices.indices.size()!=triangles*6 ||
       slices.weights.size()!=vertices*4 || slices.localIndices.size()!=vertices*4 || slices.uv.size()!=vertices*8)return false;
    RideHandleGeometry next;next.profile=profile;
    for(unsigned hand=0;hand<2;++hand) {
        auto &mesh=next.handles[hand];mesh.vertices=fighter?25:35;
        const unsigned firstVertex=fighter?172+25*hand:1690+35*hand;
        const unsigned firstTriangle=fighter?1788+30*hand:2430+30*hand;
        std::array<bool,RideHandleMesh::MaxVertices> used{};
        for(unsigned vertex=0;vertex<mesh.vertices;++vertex) {
            const size_t original=firstVertex+vertex;
            std::memcpy(&mesh.positions[vertex],slices.positions.data()+original*12,12);
            std::memcpy(mesh.uv[vertex].data(),slices.uv.data()+original*8,8);
            std::memcpy(mesh.weights[vertex].data(),slices.weights.data()+original*4,4);
            std::memcpy(mesh.localIndices[vertex].data(),slices.localIndices.data()+original*4,4);
            const auto p=mesh.positions[vertex];
            if(!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z) ||
               !std::isfinite(mesh.uv[vertex][0]) || !std::isfinite(mesh.uv[vertex][1]) ||
               mesh.weights[vertex]!=std::array<uint8_t,4>{255,0,0,0} ||
               mesh.localIndices[vertex][0]>=paletteCount || mesh.localIndices[vertex][1] ||
               mesh.localIndices[vertex][2] || mesh.localIndices[vertex][3])return false;
        }
        for(unsigned i=0;i<mesh.indices.size();++i) {
            uint16_t source=0;std::memcpy(&source,slices.indices.data()+(size_t(firstTriangle)*3+i)*2,2);
            if(source<firstVertex || source>=firstVertex+mesh.vertices)return false;
            const auto local=uint16_t(source-firstVertex);mesh.indices[i]=local;used[local]=true;
        }
        for(unsigned vertex=0;vertex<mesh.vertices;++vertex)if(!used[vertex])return false;
        for(unsigned i=0;i<mesh.indices.size();i+=3) {
            const auto a=mesh.positions[mesh.indices[i]],b=mesh.positions[mesh.indices[i+1]],c=mesh.positions[mesh.indices[i+2]];
            const auto normal=cross(b-a,c-a);const auto area=dot(normal,normal);
            if(!std::isfinite(area) || area<1e-18f)return false;
        }
    }
    next.copied=true;out=next;return true;
}
} // namespace ss2vr
