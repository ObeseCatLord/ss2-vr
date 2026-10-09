#pragma once
#include "scope_buffer_layout.hpp"
namespace ss2vr {
constexpr uint32_t IdleGeometryVertices=1490,IdleGeometryTriangles=1332;
// Explicit weighted ID1 diagnostic layout, not a claim that every native LOD uses it.
inline bool idleBufferRanges(const ScopeBufferInputs &in,
                             std::span<const ScopeDeclarationElement> declaration,ScopeCopyRanges &out,uint32_t *passedChecks=nullptr) {
    out={};const auto &s=in.surface;
    uint32_t passed=0;
    auto record=[&](unsigned bit,bool value) {if(value)passed|=1u<<bit;return value;};
    const bool basic[]={
        record(0,s.vertices>0 && uint32_t(s.vertices)<=IdleGeometryVertices),
        record(1,s.triangles>0 && uint32_t(s.triangles)<=IdleGeometryTriangles),
        record(2,!in.softwarePositions),record(3,in.draw.topology==4 && !in.draw.base && !in.draw.minimum),
        record(4,in.draw.vertices==uint32_t(s.vertices) && in.draw.primitives==uint32_t(s.triangles) && uint64_t(in.draw.start)*2==s.channels[1].offset),
        record(5,s.channels[0].format==0x85 && s.channels[1].format==0x87 && s.channels[2].format==0x80 && s.channels[3].format==0x80),
        record(6,s.channels[0].buffer!=255 && s.channels[1].buffer!=255 && s.channels[2].buffer==s.channels[0].buffer && s.channels[3].buffer==s.channels[0].buffer),
        record(7,in.positions.object && in.localIndices.object==in.positions.object && in.uv.object==in.positions.object && in.indexObject),
        record(8,!in.vertex.usage && in.vertex.pool==1 && in.vertex.format==100 && !in.vertex.fvf),
        record(9,!in.index.usage && in.index.pool==1 && in.index.format==101),
        record(10,in.positions.offset==s.channels[0].offset && in.positions.stride==12 && in.positions.frequency==1),
        record(11,in.localIndices.offset==s.channels[3].offset && in.localIndices.stride==4 && in.localIndices.frequency==1),
        record(12,in.uv.stride==8 && in.uv.frequency==1)};
    if(passedChecks)*passedChecks=passed;
    for(bool value:basic)if(!value)return false;
    bool weights=false;
    if(!record(13,declaredGeometryInputs(declaration,weights)))return false;
    if(passedChecks)*passedChecks=passed;
    if(!record(14,!weights || (in.weights.object==in.positions.object && in.weights.offset==s.channels[2].offset &&
                   in.weights.stride==4 && in.weights.frequency==1)))return false;
    if(passedChecks)*passedChecks=passed;
    ScopeCopyRanges next{{{{s.channels[0].offset,uint32_t(s.vertices)*12},{s.channels[1].offset,uint32_t(s.triangles)*6},
                           {s.channels[2].offset,uint32_t(s.vertices)*4},{s.channels[3].offset,uint32_t(s.vertices)*4},
                           {in.uv.offset,uint32_t(s.vertices)*8}}},weights};
    for(unsigned i=0;i<5;++i) {const bool valid=record(15+i,scopeByteRange(next.slices[i].offset,next.slices[i].size,
                                               i==1?in.index.size:in.vertex.size));
        if(passedChecks)*passedChecks=passed;
        if(!valid)return false;
    }
    out=next;return true;
}
}
