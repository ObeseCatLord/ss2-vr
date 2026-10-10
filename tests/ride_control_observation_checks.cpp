#include "common/ride_control_observation.hpp"
#include "common/ride_render_observation.hpp"
#include "common/ride_position.hpp"
#include <cassert>
#include <initializer_list>
#include <vector>
#ifdef NDEBUG
#error Observation checks require assertions
#endif
using namespace ss2vr;
static RideControlObservation fixture() {return {123,456,7};}
static void handleGeometry() {
    for(auto profile:{RideHandleProfile::Fighter,RideHandleProfile::Saucer}) {
        const bool fighter=profile==RideHandleProfile::Fighter;
        const unsigned vertices=fighter?2741:2464,triangles=fighter?2806:2626;
        std::array<std::vector<uint8_t>,5> bytes{{std::vector<uint8_t>(vertices*12),
            std::vector<uint8_t>(triangles*6),std::vector<uint8_t>(vertices*4),
            std::vector<uint8_t>(vertices*4),std::vector<uint8_t>(vertices*8)}};
        const ScopeSliceBytes slices{bytes[0],bytes[1],bytes[2],bytes[3],bytes[4]};
        const unsigned count=fighter?25:35;
        for(unsigned hand=0;hand<2;++hand) {
            const unsigned first=fighter?172+25*hand:1690+35*hand;
            const unsigned triangle=fighter?1788+30*hand:2430+30*hand;
            for(unsigned v=0;v<count;++v) {
                const float t=float(v)*.02f;
                const Vec3 p{t,t*t,-2-t*t*t}; // Synthetic, no game coordinates.
                std::memcpy(bytes[0].data()+(first+v)*12,&p,12);
                bytes[2][(first+v)*4]=255;bytes[3][(first+v)*4]=10;
            }
            for(unsigned i=0;i<90;++i) {
                const uint16_t index=uint16_t(first+i%count);
                std::memcpy(bytes[1].data()+(triangle*3+i)*2,&index,2);
            }
        }
        RideHandleGeometry mesh;
        assert(copyRideHandles(slices,profile,15,mesh) && mesh.copied && mesh.handles[0].vertices==count);
        assert(mesh.handles[1].indices[0]==0 && mesh.handles[1].indices[1]==1);
        auto bad=slices;bad.indices=bad.indices.first(bad.indices.size()-1);
        assert(!copyRideHandles(bad,profile,15,mesh) && !mesh.copied);
        assert(!copyRideHandles(slices,profile,10,mesh)); // Local10 outside current palette.
        const auto first=fighter?172:1690;
        bytes[2][first*4]=254;assert(!copyRideHandles(slices,profile,15,mesh));bytes[2][first*4]=255;
        bytes[3][first*4+1]=1;assert(!copyRideHandles(slices,profile,15,mesh));bytes[3][first*4+1]=0;
        const float nan=std::numeric_limits<float>::quiet_NaN();
        std::memcpy(bytes[4].data()+first*8,&nan,4);assert(!copyRideHandles(slices,profile,15,mesh));
    }
    std::array<std::array<uint8_t,32>,5> hashes{};
    assert(rideHandleProfile(hashes)==RideHandleProfile::Unknown);
}
static void handlePosition() {
    RideHandleGeometry geometry;geometry.profile=RideHandleProfile::Saucer;geometry.copied=true;
    for(auto &handle:geometry.handles) {
        handle.vertices=3;
        handle.positions[0]={0,0,-2};handle.positions[1]={.1f,0,-2};handle.positions[2]={0,.1f,-2};
        for(unsigned v=0;v<3;++v) {handle.localIndices[v]={10,0,0,0};handle.weights[v]={255,0,0,0};}
    }
    const Matrix34 identity{{1,0,0,0,0,1,0,0,0,0,1,0}};
    const auto projectionMatrix=projection({-.7f,.6f,.65f,-.7f});
    std::array<std::array<float,4>,256> constants{};
    for(unsigned r=0;r<4;++r)for(unsigned c=0;c<4;++c)constants[r][c]=projectionMatrix.m[4*r+c];
    const std::array<uint32_t,9> program{{0xfffe0101,31,0x80000005,0x900f0000,
        20,0xc00f0000,0x90e40000,0xa0e40000,0xffff}};
    std::array<std::array<Vec3,35>,2> world;
    assert(rideHandlesPosition(geometry,15,10,program,constants,GeometryInputLayout::Legacy56,
        identity,identity,identity,projectionMatrix,world));
    assert(world[0][1].x==.1f);
    assert(!rideHandlesPosition(geometry,15,9,program,constants,GeometryInputLayout::Legacy56,
        identity,identity,identity,projectionMatrix,world));
    constants[0][3]=.1f;
    assert(!rideHandlesPosition(geometry,15,10,program,constants,GeometryInputLayout::Legacy56,
        identity,identity,identity,projectionMatrix,world));
    assert(world[0][1].x==0);
    Matrix44 collapsed{};std::array<double,16> inv;
    assert(!rideCameraInverse(collapsed,inv));
    auto conditioned=projectionMatrix;conditioned.m[0]=1e-12f;
    assert(!rideCameraInverse(conditioned,inv));
    const Matrix34 model{{0,0,1,2,0,1,0,-1,-1,0,0,-4}},
        main{{1.5f,.2f,0,.1f,0,.8f,0,.2f,0,0,1.1f,-.3f}},
        view{{1,0,0,-.2f,0,.995f,-.0998f,.1f,0,.0998f,.995f,-.4f}};
    Matrix44 expectedClip;
    assert(scopeCapClip(projectionMatrix,view,affineMultiply(model,main),expectedClip));
    for(unsigned r=0;r<4;++r)for(unsigned c=0;c<4;++c)constants[r][c]=expectedClip.m[4*r+c];
    assert(rideHandlesPosition(geometry,15,10,program,constants,GeometryInputLayout::Legacy56,
        model,main,view,projectionMatrix,world)); // Noncommuting native affine/view order.
    auto compressed=projectionMatrix;compressed.m[0]=.001f;
    assert(rideCameraInverse(compressed,inv));
    for(unsigned r=0;r<4;++r)for(unsigned c=0;c<4;++c)constants[r][c]=compressed.m[4*r+c];
    assert(rideHandlesPosition(geometry,15,10,program,constants,GeometryInputLayout::Legacy56,
        identity,identity,identity,compressed,world));
    constants[0][3]=3e-6f;std::array<float,4> acceptedClip;
    assert(scope_position::position(program,constants,{0,0,-2},{0,0},true,acceptedClip));
    const double expectedX=-2*double(compressed.m[2]);
    assert(std::abs(acceptedClip[0]-expectedX)<1e-5); // Clip tolerance alone would pass.
    assert(!rideHandlesPosition(geometry,15,10,program,constants,GeometryInputLayout::Legacy56,
        identity,identity,identity,compressed,world)); // Backprojection differs by3mm.
    const std::vector<uint32_t> indexed{0xfffe0101,31,0x80000005,0x900f0000,31,0x80050005,0x900f0005,
        5,0x80010000,0x90000005,0xa0000004,1,0xb0010000,0x80000000,
        20,0xc00f0000,0x90e40000,0xa0e42000,0xffff};
    constants={};constants[4][0]=255*4.f;
    for(unsigned r=0;r<4;++r)for(unsigned c=0;c<4;++c)constants[40+r][c]=projectionMatrix.m[4*r+c];
    scope_position::VehicleRigidPaletteInput input{{10,0,0,0},{255,0,0,0},15};std::array<float,4> out;
    assert(scope_position::position(indexed,constants,{.1f,0,-2},{0,0},true,out,GeometryInputLayout::Legacy56,nullptr,&input));
    input.indices[0]=1;constants[4][0]=255*40.25f; // Known non-tie rounds to40.
    assert(scope_position::position(indexed,constants,{.1f,0,-2},{0,0},true,out,GeometryInputLayout::Legacy56,nullptr,&input));
    auto halfway=indexed;halfway[13]=0xa0000004;constants[4][0]=40.5f;
    assert(!scope_position::position(halfway,constants,{.1f,0,-2},{0,0},true,out,GeometryInputLayout::Legacy56,nullptr,&input));
    scope_position::RigidPaletteInput weapon{{10,0,0,0},{255,0,0,0},15};assert(!weapon.valid());
}
static void mainMappings() {
    std::array<PaletteMap,15> maps;
    for(unsigned i=0;i<maps.size();++i)maps[i]={2,int32_t(100+i)};
    uint32_t slot=0;
    const auto owned=[](int32_t bone) {return bone>=100 && bone<115;};
    assert(selectRideMainMapping(maps,15,2,0,15,103,owned,slot) && slot==3); // Not authored slot10.
    auto duplicate=maps;duplicate[10].bone=103;
    assert(!selectRideMainMapping(duplicate,15,2,0,15,103,owned,slot) && slot==UINT32_MAX);
    auto bad=maps;bad[14].bone=200;
    assert(!selectRideMainMapping(bad,15,2,0,15,103,owned,slot));
    bad=maps;bad[5].draw=3;assert(!selectRideMainMapping(bad,15,2,0,15,103,owned,slot));
    assert(!selectRideMainMapping(maps,14,2,0,15,103,owned,slot));
    assert(!selectRideMainMapping(maps,15,2,1,15,103,owned,slot));
    assert(!selectRideMainMapping(maps,15,2,-1,15,103,owned,slot));
    assert(!selectRideMainMapping(maps,15,2,0,0,103,owned,slot));
    assert(!selectRideMainMapping(maps,15,2,0,33,103,owned,slot));
    assert(!selectRideMainMapping(maps,15,2,0,15,3,owned,slot)); // Wrong relative/global index.
    RideRenderFrameCopy frame;frame.identity={1,2,3,4,0x2a8558,5,6,7};
    frame.configuration=8;frame.file=9;frame.resource=10;frame.modelRecord=1;
    frame.lod=12;frame.mainBone=103;frame.boneDefinition=14;frame.world[0]=0x3f800000;
    frame.main[0]=0x40000000; // Canonical differs; actual palette is the observation.
    RideMainDrawCopy draw;draw.identity=frame.identity;draw.instance=7;
    draw.configuration=8;draw.file=9;draw.resource=10;draw.modelRecord=1;draw.lod=12;
    draw.bone=103;draw.definition=14;draw.world=frame.world;draw.actualPalette[0]=0x40400000;
    draw.paletteCount=15;draw.localMainSlot=3;
    assert(rideMainDrawLinked(draw,frame));
    for(unsigned failure=0;failure<7;++failure) {
        auto crossed=draw;
        if(failure==0)++crossed.instance;
        if(failure==1)++crossed.modelRecord;
        if(failure==2)++crossed.lod;
        if(failure==3)++crossed.configuration;
        if(failure==4)++crossed.bone;
        if(failure==5)++crossed.definition;
        if(failure==6)crossed.localMainSlot=15;
        assert(!rideMainDrawLinked(crossed,frame));
    }
}
static void vehicleRanges() {
    const std::array<ScopeDeclarationElement,5> declaration{{{0,0,2,0,5,0},
        {5,0,8,0,5,5},{6,0,8,0,5,6},{3,0,1,0,5,3},{255,0,17,0,0,0}}};
    const std::array<ScopeSurfaceLayout,2> layouts{{
        {2741,2806,{{{3600,0x85,0},{2520,0x87,0},{124040,0x80,0},{135004,0x80,0}}}},
        {2464,2626,{{{3024,0x85,0},{1188,0x87,0},{166048,0x80,0},{175904,0x80,0}}}}}};
    for(const auto &surface:layouts) {
        const uint32_t uv=surface.channels[3].offset+uint32_t(surface.vertices)*4;
        ScopeBufferInputs in{surface,{4,0,0,uint32_t(surface.vertices),surface.channels[1].offset/2,uint32_t(surface.triangles)},
            {1,surface.channels[0].offset,12,1},{1,surface.channels[3].offset,4,1},
            {1,surface.channels[2].offset,4,1},{1,uv,8,1},
            {uv+uint32_t(surface.vertices)*8,0,1,100,0},{surface.channels[1].offset+uint32_t(surface.triangles)*6,0,1,101,0},2,false};
        ScopeCopyRanges ranges;
        assert(rideBufferRanges(in,declaration,ranges) && ranges.weightsActive);
        assert(ranges.slices[1].size==uint32_t(surface.triangles)*6);
        if(surface.triangles==2806)for(int id:{1,2,13})assert(!idleBufferRanges(in,declaration,ranges,nullptr,id));
        for(unsigned fault=0;fault<10;++fault) {
            auto bad=in;
            if(fault==0)++bad.surface.vertices;
            if(fault==1)++bad.draw.start;
            if(fault==2)bad.draw.base=1;
            if(fault==3)bad.positions.frequency=2;
            if(fault==4)bad.weights.object=3;
            if(fault==5)bad.index.size--;
            if(fault==6)bad.vertex.size--;
            if(fault==7)bad.softwarePositions=true;
            if(fault==8)bad.vertex.pool=0;
            if(fault==9)bad.localIndices.stride=8;
            assert(!rideBufferRanges(bad,declaration,ranges) && !ranges.slices[0].size);
        }
        auto unknown=declaration;unknown[2].type=17;
        assert(!rideBufferRanges(in,unknown,ranges)); // Main frame remains independent at caller.
    }
    uint32_t total=0;
    for(unsigned bank=0;bank<8;++bank) {
        uint32_t eye=0;
        for(unsigned attempt=0;attempt<8;++attempt)assert(chargeRideGpuAttempt(eye,total,false));
        assert(!chargeRideGpuAttempt(eye,total,false) && eye==8);
    }
    uint32_t next=0;assert(!chargeRideGpuAttempt(next,total,false) && total==64 && next==0);
    total=0;assert(!chargeRideGpuAttempt(next,total,true) && !total && !next);
}
int main() {
    mainMappings();vehicleRanges();handleGeometry();handlePosition();
    for(auto table:{0x2a8558u,0x2b8420u}) {
        auto row=fixture();assert(row.enter(123,456,7));
        row.copy({table,2,4,41,0,0}); // Null association tokens remain unassociated.
        assert(!row.publishable());row.finishCallback(false);row.finishOuter(false);
        assert(row.publishable() && row.values.movementAbilities==41 && row.values.executionAbilities==4);
    }
    for(unsigned failure=0;failure<7;++failure) {
        auto row=fixture();
        if(failure<3) {
            assert(!row.enter(failure==0?124:123,failure==1?457:456,failure==2?8:7));
        } else {
            assert(row.enter(123,456,7));row.copy({0x2a8558,2,4,41,1,2});
            if(failure==3)assert(!row.enter(123,456,7)); // Nested callback cannot lend its result.
            if(failure==4)row.copy({0x2a7ea0,2,4,41,1,2});
            row.finishCallback(failure==5);row.finishOuter(failure==6);
        }
        assert(!row.publishable());
    }
    auto duplicate=fixture();assert(duplicate.enter(123,456,7));duplicate.copy({0x2a8558,2,4,41,0,0});
    duplicate.finishCallback(false);assert(!duplicate.enter(123,456,7));assert(!duplicate.publishable());
    auto missing=fixture();missing.finishOuter(false);assert(!missing.publishable());
}
