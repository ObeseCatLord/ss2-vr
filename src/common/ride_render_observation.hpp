#pragma once
#include <array>
#include <cstdint>
#include "head_palette.hpp"
#include "scope_buffer_layout.hpp"
#include "idle_geometry.hpp"
#include "scope_program.hpp"

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
// Optional declared attachment relationship, not an evaluated child world or
// a certificate of installed geometry, input-time freshness, grasp or steering.
struct RideAttachmentCopy {
    bool mapped=false;
    uint32_t parameter=0,parameterFlags=0,seatData=0,attachment=0;
    uint32_t childState=0,childArray=0,childCount=0,descriptor=0,parentName=0,childFlags=0;
    uint32_t childRecordPresent=0,childRecord=0;
    std::array<uint32_t,7> pose{};
    std::array<uint32_t,3> scale{};
    bool operator==(const RideAttachmentCopy &) const = default;
};
struct RideRenderFrameCopy {
    RideRenderIdentity identity;
    uint32_t configuration=0,file=0,resource=0,modelRecord=0;
    uint32_t evaluated=0,matrices=0,boneDefinition=0,mainBone=0;
    uint32_t seatDefinition=0,seatBone=0;
    uint32_t skeleton=0,lod=0,definitions=0,definitionCount=0,boneFirst=0,boneCount=0;
    uint32_t canonicalCount=0,cacheRows=0,cacheRowCount=0;
    std::array<uint32_t,12> world{},main{},seat{};
    RideAttachmentCopy attachment;
    bool operator==(const RideRenderFrameCopy &) const = default;
};
// One actual native Main mapping, not a whole-draw affine or verified vertex
// subset. All identities are value-only event keys, never pointer leases.
struct RideMainDrawCopy {
    uint32_t bank=0;
    int32_t eye=-2;
    RideRenderIdentity identity;
    uint32_t configuration=0,file=0,resource=0,modelRecord=0,lod=0;
    uint32_t drawRecord=0,surface=0,surfaceName=0,instance=0,bone=0,definition=0;
    uint32_t paletteFirst=0,paletteCount=0,localMainSlot=0;
    ScopeSurfaceLayout layout;
    ScopeIndexedDraw api;
    std::array<uint32_t,12> world{},actualPalette{};
    bool operator==(const RideMainDrawCopy &) const = default;
};
constexpr uint32_t RideMainDrawCapacity=8, RideMainMappingBudget=32;
constexpr uint32_t RideGpuSessionAttempts=64, RideGeometryTriangles=2806;
// These exact metadata profiles only select bounded observation. GPU digests,
// program consumption and physical grasp are separate evidence.
inline bool rideSurfaceSupported(const ScopeSurfaceLayout &s) {
    constexpr ScopeSurfaceLayout fighter{2741,2806,{{{3600,0x85,0},{2520,0x87,0},
        {124040,0x80,0},{135004,0x80,0}}}};
    constexpr ScopeSurfaceLayout saucer{2464,2626,{{{3024,0x85,0},{1188,0x87,0},
        {166048,0x80,0},{175904,0x80,0}}}};
    return s==fighter || s==saucer;
}
inline bool rideBufferRanges(const ScopeBufferInputs &in,
        std::span<const ScopeDeclarationElement> declaration,ScopeCopyRanges &out) {
    out={};
    if(!rideSurfaceSupported(in.surface))return false;
    if(!boundedGeometryBufferRanges(in,declaration,out,{2741,RideGeometryTriangles}) || !out.weightsActive) {
        out={};return false;
    }
    return true;
}
struct RideDrawGpuCopy {
    ScopeBufferInputs inputs;
    std::array<ScopeDeclarationElement,65> declaration{};
    std::array<uint32_t,ScopeProgramMaxWords> program{};
    std::array<uint32_t,1024> constants{};
    std::array<std::array<uint8_t,32>,5> hashes{};
    uint32_t declarationElements=0,programWords=0,constantRows=0,inputLayout=0;
    uint32_t declarationObject=0,shaderObject=0;
    bool copied=false;
};
// No reservation lifecycle: each charge occurs before foreign GPU work, even
// when the attempt fails or the frame never publishes.
inline bool chargeRideGpuAttempt(uint32_t &eyeAttempts,uint32_t &sessionAttempts,bool publicationExhausted) {
    if(publicationExhausted || eyeAttempts>=RideMainDrawCapacity || sessionAttempts>=RideGpuSessionAttempts)return false;
    ++eyeAttempts;++sessionAttempts;return true;
}
// Select the actual map position, not a serialized palette ordinal/name or a
// model-relative bone index. The budget is diagnostic capacity, not a native limit.
template<class BoneOwned> bool selectRideMainMapping(std::span<const PaletteMap> maps,
        size_t paletteSize,int32_t draw,int32_t first,int32_t count,int32_t main,
        BoneOwned owned,uint32_t &slot) {
    slot=UINT32_MAX;
    if(draw<0 || main<0 || first<0 || count<1 || uint32_t(count)>RideMainMappingBudget ||
       size_t(first)>maps.size() || size_t(count)>maps.size()-size_t(first) ||
       size_t(first)>paletteSize || size_t(count)>paletteSize-size_t(first))return false;
    uint32_t found=0,candidate=0;
    for(int32_t i=0;i<count;++i) {
        const auto &m=maps[size_t(first)+size_t(i)];
        if(m.draw!=draw || m.bone<0 || !owned(m.bone))return false;
        if(m.bone==main) {++found;candidate=uint32_t(i);}
    }
    if(found!=1)return false;
    slot=candidate;return true;
}
inline bool rideMainDrawLinked(const RideMainDrawCopy &draw,const RideRenderFrameCopy &frame) {
    return frame.identity.valid() && frame.modelRecord && frame.lod && frame.mainBone && frame.boneDefinition &&
        draw.identity==frame.identity && draw.instance==frame.identity.instance &&
        draw.configuration==frame.configuration && draw.file==frame.file && draw.resource==frame.resource &&
        draw.modelRecord==frame.modelRecord && draw.lod==frame.lod &&
        draw.bone==frame.mainBone && draw.definition==frame.boneDefinition && draw.world==frame.world &&
        draw.paletteCount && draw.paletteCount<=RideMainMappingBudget && draw.localMainSlot<draw.paletteCount;
}
} // namespace ss2vr
