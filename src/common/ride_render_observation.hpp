#pragma once
#include <array>
#include <cstdint>

namespace ss2vr {
// Copied identities only. These numbers never authorize a later dereference or
// steering input. The native sampler re-establishes every edge in world render.
struct RideRenderIdentity {
    uint32_t player=0,brain=0,ride=0,seat=0,classRva=0,renderableHandle=0;
    uint32_t renderable=0,instance=0;
    bool operator==(const RideRenderIdentity &) const = default;
    bool valid() const noexcept {
        return player && brain && ride && renderableHandle && renderable && instance &&
               (classRva==0x2a8558 || classRva==0x2b8420);
    }
};
struct RideRenderFrameCopy {
    RideRenderIdentity identity;
    uint32_t configuration=0,file=0,resource=0,modelRecord=0;
    uint32_t evaluated=0,matrices=0,boneDefinition=0,mainBone=0;
    uint32_t skeleton=0,lod=0,definitions=0,definitionCount=0,boneFirst=0,boneCount=0;
    uint32_t canonicalCount=0,cacheRows=0,cacheRowCount=0;
    std::array<uint32_t,12> world{},main{};
    bool operator==(const RideRenderFrameCopy &) const = default;
};
} // namespace ss2vr
