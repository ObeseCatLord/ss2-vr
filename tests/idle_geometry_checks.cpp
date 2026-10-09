#include "common/idle_geometry.hpp"
#include <cassert>
using namespace ss2vr;
#ifdef NDEBUG
#error Geometry checks require assertions
#endif
int main() {
    const ScopeBufferInputs hand{
        {317,338,{{{0,0x85,0},{0,0x87,0},{12680,0x80,0},{13948,0x80,0}}}},
        {4,0,0,317,0,338}, {1,0,12,1},{1,13948,4,1},{1,12680,4,1},{1,15216,8,1},
        {17752,0,1,100,0},{2028,0,1,101,0},2,false};
    std::array<ScopeDeclarationElement,5> decl{{{0,0,2,0,5,0},{5,0,8,0,5,5},
        {6,0,8,0,5,6},{3,0,1,0,5,3},{255,0,17,0,0,0}}};
    ScopeCopyRanges ranges;
    uint32_t diagnostic=0;
    assert(idleBufferRanges(hand,decl,ranges,&diagnostic) && diagnostic==((1u<<20)-1));
    {auto unsupported=hand;unsupported.vertex.pool=0;
     assert(!idleBufferRanges(unsupported,decl,ranges,&diagnostic));
     assert((diagnostic&0x1fff)==(0x1fff&~(1u<<8)) && !(diagnostic&(1u<<13)));
     assert(ranges.slices[0].size==0);}
    {auto truncated=hand;truncated.uv.offset=truncated.vertex.size;
     assert(!idleBufferRanges(truncated,decl,ranges,&diagnostic));
     assert(diagnostic==((1u<<19)-1) && ranges.slices[0].size==0);}
    assert(idleBufferRanges(hand,decl,ranges));assert(ranges.slices[0].size==3804);
    assert(ranges.slices[1].size==2028 && ranges.slices[4].offset==15216);
    for(unsigned variant=0;variant<18;++variant) {
        auto bad=hand;
        switch(variant) {
        case 0:bad.vertex.usage=8;break; // WRITEONLY never locked READONLY.
        case 1:bad.vertex.pool=0;break;
        case 2:bad.index.usage=8;break;
        case 3:bad.positions.frequency=2;break;
        case 4:bad.localIndices.object=3;break;
        case 5:bad.uv.offset=17751;break;
        case 6:bad.surface.channels[2].format=0;break; // Unweighted LOD is unsupported.
        case 7:bad.surface.channels[3].buffer=1;break;
        case 8:bad.draw.base=1;break;
        case 9:bad.draw.minimum=1;break;
        case 10:bad.draw.start=1;break;
        case 11:bad.draw.primitives--;break;
        case 12:bad.surface.vertices=1491;bad.draw.vertices=1491;break;
        case 13:bad.surface.triangles=1333;bad.draw.primitives=1333;break;
        case 15:bad.surface.vertices=-1;break;
        case 16:bad.surface.triangles=-1;break;
        case 17:bad.surface.channels[0].buffer=255;bad.surface.channels[2].buffer=255;bad.surface.channels[3].buffer=255;break;
        case 14:bad.surface.channels[0].offset=0xfffffff0;bad.positions.offset=0xfffffff0;break;
        }
        assert(!idleBufferRanges(bad,decl,ranges));assert(ranges.slices[0].size==0);
    }
    auto badDecl=decl;badDecl[1].usageIndex=0;assert(!idleBufferRanges(hand,badDecl,ranges));
    badDecl=decl;badDecl[4].type=0;assert(!idleBufferRanges(hand,badDecl,ranges));
    auto optional=hand;optional.weights={};decl[2].type=17;
    assert(idleBufferRanges(optional,decl,ranges));assert(!ranges.weightsActive);
    // A relocated surface/LOD is admitted by declared ranges, never ID13 offsets.
    auto moved=hand;for(auto &c:moved.surface.channels)c.buffer=1;
    moved.surface.channels[0].offset=12;moved.positions.offset=12;
    moved.surface.channels[1].offset=6;moved.draw.start=3;
    moved.vertex.size+=12;moved.index.size+=6;
    assert(idleBufferRanges(moved,decl,ranges));assert(ranges.slices[0].offset==12 && ranges.slices[1].offset==6);
}
