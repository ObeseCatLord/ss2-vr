#ifdef NDEBUG
#error Scope geometry verification requires assertions enabled
#endif
#include "common/scope_geometry.hpp"
#include "common/scope_uv.hpp"
#include "common/scope_buffer_layout.hpp"
#include <cassert>
#include <limits>
#include <fstream>
#include <iostream>
using namespace ss2vr;
static void verifyMapping(const ScopeCapGeometry &cap,const Matrix34 &affine) {
    ScopeUvTransform native;
    native.rows[0]={2,.4f,7,.3f}; native.rows[1]={-.5f,1.5f,-9,-.2f};
    ScopeImageCoordinates map;
    ScopeOpticalFrame frame;
    assert(scopeImageCoordinates(cap,affine,native,map) && scopeOpticalFrame(cap,affine,frame));
    Vec3 mean{}; for (auto p:cap.positions) mean=mean+p;
    mean=mean*(1.f/float(ScopeCapVertices));
    auto check=[&](float u,float v,Vec3 p) {
        const Vec3 delta=p-mean;
        const Vec3 moved{affine.m[0]*delta.x+affine.m[1]*delta.y+affine.m[2]*delta.z,
                         affine.m[4]*delta.x+affine.m[5]*delta.y+affine.m[6]*delta.z,
                         affine.m[8]*delta.x+affine.m[9]*delta.y+affine.m[10]*delta.z};
        for(unsigned r=0;r<3;++r) {
            const float expected=dot(moved,{frame.camera.m[r],frame.camera.m[4+r],frame.camera.m[8+r]});
            const float actual=map.rows[r][0]*u+map.rows[r][1]*v+map.rows[r][2];
            assert(std::abs(actual-expected)<1e-5f);
        }
    };
    std::array<std::array<float,2>,ScopeCapVertices> post{};
    for(size_t i=0;i<ScopeCapVertices;++i) {
        const auto uv=cap.uv[i];
        post[i]={2*uv[0]+.4f*uv[1]+.3f,-.5f*uv[0]+1.5f*uv[1]-.2f};
        check(post[i][0],post[i][1],cap.positions[i]);
    }
    // Every actual triangle, including asymmetric perspective interpolation.
    const double clipW[]{.15,.4,.8};
    const double barycentric[][3]{{.2,.3,.5},{.8,.1,.1},{.1,.8,.1},{.1,.1,.8}};
    for(size_t t=0;t<ScopeCapIndices;t+=3) {
        for(const auto &bary:barycentric) {
            double denominator=0; for(unsigned i=0;i<3;++i) denominator+=bary[i]/clipW[i];
            float u=0,v=0; Vec3 p{};
            for(unsigned i=0;i<3;++i) {
                const size_t vertex=cap.indices[t+i];
                const float weight=float((bary[i]/clipW[i])/denominator);
                u+=post[vertex][0]*weight; v+=post[vertex][1]*weight;
                p=p+cap.positions[vertex]*weight;
            }
            check(u,v,p);
        }
    }
    auto distant=affine; distant.m[3]+=1048576; distant.m[7]-=1048576; distant.m[11]+=1048576;
    ScopeImageCoordinates far;
    assert(scopeImageCoordinates(cap,distant,native,far) && far.rows==map.rows);
}
int main(int argc, char **argv) {
    std::array<uint8_t, ScopeVertices*12> positions{};
    std::array<uint8_t, ScopeTriangles*3*2> indices{};
    std::array<uint8_t, ScopeVertices*4> weights{}, localIndices{};
    std::array<uint8_t, ScopeVertices*8> uv{};
    for (size_t i=0; i<ScopeCapVertices; ++i) {
        const float angle = float(i)*2*Pi/float(ScopeCapVertices);
        const Vec3 p{.02f*std::cos(angle),.3f+.02f*std::sin(angle),.05f};
        std::memcpy(positions.data()+i*12,&p,12);
        const float coordinates[]{.9f+5*p.x,.78f-10*(p.y-.3f)};
        std::memcpy(uv.data()+i*8,coordinates,8);
    }
    for (size_t i=0; i<ScopeCapEnd-ScopeCapFirst; ++i) {
        const uint16_t triangle[]{0,uint16_t(i+1),uint16_t(i+2)};
        std::memcpy(indices.data()+(ScopeCapFirst+i)*6,triangle,6);
    }
    ScopeSliceBytes slices{positions,indices,weights,localIndices,uv};
    ScopeCapGeometry cap, world;
    assert(copyScopeCap(slices,cap) && cap.valid);
    assert(cap.indices.front()==0 && cap.indices.back()==23);
    ScopeCapGeometry invocation;
    bool rejected=false;
    assert(acceptScopeGeometry(invocation,rejected,cap));
    applyScopeRasterStatus(ScopeRasterStatus::Unrelated,invocation,rejected);
    assert(invocation.valid && !rejected); // Other sniper surfaces preserve evidence.
    applyScopeRasterStatus(ScopeRasterStatus::Rejected,invocation,rejected);
    assert(!invocation.valid && rejected);
    applyScopeRasterStatus(ScopeRasterStatus::Observed,invocation,rejected);
    assert(!acceptScopeGeometry(invocation,rejected,cap) && !invocation.valid); // No same-invocation revival.
    ScopeCapGeometry unsupportedFirst; bool firstRejected=false;
    applyScopeRasterStatus(ScopeRasterStatus::Rejected,unsupportedFirst,firstRejected);
    assert(!acceptScopeGeometry(unsupportedFirst,firstRejected,cap) && firstRejected);
    const Matrix34 mirrored{{-2,.25f,0,4,0,.5f,0,-3,0,0,1.5f,2}};
    verifyMapping(cap,mirrored);
    assert(scopeCapWorld(cap,mirrored,world));
    assert(world.indices==cap.indices && world.valid);
    for (size_t i=0;i<ScopeCapVertices;++i) {
        assert(world.positions[i].x == -2*cap.positions[i].x+.25f*cap.positions[i].y+4);
        assert(world.positions[i].y == .5f*cap.positions[i].y-3);
        assert(world.positions[i].z == 1.5f*cap.positions[i].z+2);
    }
    ScopeCapGeometry inPlace = cap;
    assert(scopeCapWorld(inPlace,mirrored,inPlace));
    for(size_t i=0;i<ScopeCapVertices;++i) {
        assert(inPlace.positions[i].x==world.positions[i].x);
        assert(inPlace.positions[i].y==world.positions[i].y);
        assert(inPlace.positions[i].z==world.positions[i].z);
    }
    assert(!scopeCapWorld(cap,{},world) && !world.valid);
    ScopeSliceBytes shortSlices = slices;
    shortSlices.indices = shortSlices.indices.first(shortSlices.indices.size()-1);
    assert(!copyScopeCap(shortSlices,cap) && !cap.valid);
    shortSlices=slices; shortSlices.uv={};
    assert(!copyScopeCap(shortSlices,cap) && !cap.valid);
    shortSlices=slices; shortSlices.uv=shortSlices.uv.first(shortSlices.uv.size()-1);
    assert(!copyScopeCap(shortSlices,cap) && !cap.valid);
    const uint16_t bad = ScopeVertices;
    std::memcpy(indices.data()+ScopeCapFirst*6,&bad,2);
    assert(!copyScopeCap(slices,cap) && !cap.valid);
    const uint16_t zero = 0;
    std::memcpy(indices.data()+ScopeCapFirst*6,&zero,2);
    const float nan = std::numeric_limits<float>::quiet_NaN();
    std::memcpy(positions.data(),&nan,4);
    assert(!copyScopeCap(slices,cap) && !cap.valid);
    if (argc == 2) {
        // This optional input is supplied only by the fingerprint-bound Python
        // verifier in private temporary storage. No game asset is a fixture.
        std::ifstream input(argv[1],std::ios::binary);
        assert(input);
        const std::array<std::span<uint8_t>,5> destinations{positions,indices,weights,localIndices,uv};
        for (auto destination : destinations) {
            input.read(reinterpret_cast<char *>(destination.data()),std::streamsize(destination.size()));
            assert(input.gcount()==std::streamsize(destination.size()));
        }
        char extra = 0; assert(!input.read(&extra,1));
        assert(copyScopeCap(slices,cap) && cap.valid);
        // Independently trace each source index, including repeated vertices.
        for(size_t i=0;i<ScopeCapIndices;++i) {
            uint16_t source=0;
            std::memcpy(&source,indices.data()+(ScopeCapFirst*3+i)*2,2);
            const size_t mapped=cap.indices[i];
            assert(mapped<ScopeCapVertices);
            assert(!std::memcmp(&cap.positions[mapped],positions.data()+source*12,12));
            assert(!std::memcmp(cap.uv[mapped].data(),uv.data()+source*8,8));
        }
        verifyMapping(cap,mirrored);
        ScopeBufferInputs live;
        live.surface={884,928, {{{0,0x85,0},{0,0x87,0},{151520,0x80,0},{155056,0x80,0}}}};
        live.draw={4,0,0,884,0,928};
        live.positions={1,0,12,1}; live.localIndices={1,155056,4,1};
        live.uv={1,181824,8,1}; live.weights={1,151520,4,1};
        live.vertex={212128,0,1,100,0}; live.index={19038,0,1,101,0}; live.indexObject=2;
        const ScopeDeclarationElement declaration[]{{0,0,2,0,5,0},{5,0,8,0,5,5},
            {3,0,1,0,5,3},{6,0,8,0,5,6},{0xff,0,17,0,0,0}};
        ScopeCopyRanges ranges;
        assert(scopeBufferRanges(live,declaration,ranges));
        for(size_t i=0;i<destinations.size();++i) assert(ranges.slices[i].size==destinations[i].size());
        ScopeImageCoordinates coordinates;
        assert(scopeImageCoordinates(cap,mirrored,{},coordinates));
        ScopeUvTransform largeOffset; largeOffset.rows[0][3]=4096;
        assert(!scopeImageCoordinates(cap,mirrored,largeOffset,coordinates) && !coordinates.valid);
        Vec3 center{};
        for (const auto &p : cap.positions) center = center+p;
        center = center*(1.f/float(ScopeCapVertices));
        assert(std::abs(center.x)<1e-6f && std::abs(center.y-.303f)<1e-6f &&
               std::abs(center.z-.0486f)<1e-6f);
        const auto normal = cross(cap.positions[cap.indices[1]]-cap.positions[cap.indices[0]],
                                  cap.positions[cap.indices[2]]-cap.positions[cap.indices[0]]);
        assert(scopeCapWorld(cap,mirrored,world));
        const auto changed = cross(world.positions[world.indices[1]]-world.positions[world.indices[0]],
                                   world.positions[world.indices[2]]-world.positions[world.indices[0]]);
        assert(normal.z*changed.z<0); // True reflected winding, no guessed rigid frame.
        assert(normal.z>0); // Audited local rear normal, not a hand convention.
        ScopeOpticalFrame optic;
        assert(scopeOpticalFrame(cap,mirrored,optic) && optic.valid);
        Vec3 x{optic.camera.m[0],optic.camera.m[4],optic.camera.m[8]};
        Vec3 y{optic.camera.m[1],optic.camera.m[5],optic.camera.m[9]};
        Vec3 z{optic.camera.m[2],optic.camera.m[6],optic.camera.m[10]};
        assert(std::abs(dot(x,x)-1)<1e-5f && std::abs(dot(y,y)-1)<1e-5f &&
               std::abs(dot(z,z)-1)<1e-5f && std::abs(dot(x,y))<1e-5f &&
               std::abs(dot(x,z))<1e-5f && std::abs(dot(y,z))<1e-5f);
        assert(std::abs(dot(cross(x,y),z)-1)<1e-5f && dot(changed,z)<0);
        assert(std::abs(optic.camera.m[3]-4.07575f)<1e-5f &&
               std::abs(optic.camera.m[7]+2.8485f)<1e-5f &&
               std::abs(optic.camera.m[11]-2.0729f)<1e-5f);
        std::cout << "{\"owned_cap_vertices\":24,\"owned_cap_indices\":66,\"full_affine_reflection_preserved\":true,\"owned_optical_frame\":true,\"owned_uv_pairs\":true,\"owned_uv_mapping\":true}\n";
    } else assert(argc == 1);
}
