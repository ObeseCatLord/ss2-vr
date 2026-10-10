#include "common/ride_control_observation.hpp"
#include "common/ride_render_observation.hpp"
#include "common/ride_position.hpp"
#include "common/ride_grasp.hpp"
#include "common/presentation_identity.hpp"
#include <cassert>
#include <initializer_list>
#include <vector>
#ifdef NDEBUG
#error Observation checks require assertions
#endif
using namespace ss2vr;
static RideControlObservation fixture() {return {123,456,7};}
static void retainedRideBank() {
    // The actual eye/Ready ordering: draw ends before copied-bank publication.
    std::atomic<uint32_t> owner{11};std::atomic<bool> invalid{false};
    uint32_t local=11;bool drawing=true;
    const auto read=[&](RideReadPhase phase) {
        return frozenPresentationOwnerMatches(owner.load(),local,11) && !invalid.load() &&
            rideReadPhaseCurrent(drawing,phase);
    };
    assert(read(RideReadPhase::Drawing));drawing=false;
    assert(!read(RideReadPhase::Drawing) && read(RideReadPhase::Retained));
    // Invalid/reset bank and foreign/zero/replaced owners still cannot publish.
    invalid=true;assert(!read(RideReadPhase::Retained));invalid=false;
    owner=12;assert(!read(RideReadPhase::Retained));owner=11;
    assert(!rideReadPhaseCurrent(true,static_cast<RideReadPhase>(2)));
    retireFrozenPresentationOwner(owner,invalid,local,drawing,11);
    assert(!read(RideReadPhase::Drawing) && !read(RideReadPhase::Retained));
    // Mono completion may happen while its native draw-use flag is still set.
    assert(rideReadPhaseCurrent(true,RideReadPhase::Retained));
}
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

static RideGripFrame gripFixture() {
    RideGripFrame f{};f.valid=true;
    f.request.sequence=7;f.request.session=3;f.request.reference=4;f.request.trackingGeneration=5;
    f.request.input.session=3;f.request.input.reference=4;f.request.input.tickMs=1000;
    f.rig.rider={10,11,12,3,true};f.rig.producer=42;f.rig.generation=5;
    f.rig.revision=6;f.rig.graphics=7;f.profile=RideHandleProfile::Fighter;
    f.render={1,2,3,4,5,6};f.configuration=10;f.file=11;f.resource=12;f.parameter=13;
    for(unsigned h=0;h<2;++h) {
        auto &mesh=f.surfaces[h];mesh.vertices=3;
        const float x=h?.25f:-.25f;
        mesh.positions[0]={x-.04f,-.04f,-.4f};mesh.positions[1]={x+.04f,-.04f,-.4f};
        mesh.positions[2]={x,.04f,-.4f};
        for(unsigned i=0;i<90;++i)mesh.indices[i]=i%3;
    }
    return f;
}
static void submissionFeedback() {
    auto f=gripFixture();WorldSubmissionStream host;RideGripFrames frames;RideGripFrame out;
    frames.add(f);assert(!frames.submitted(host.value,42,1000,out)); // Ready is not submission.
    host.sample(42,f.request,true);const auto epoch=host.value.continuity;
    assert(frames.submitted(host.value,42,1000,out) && out.valid);
    assert(submittedWorldMatches(host.value,42,f.request,1100));
    assert(!submittedWorldMatches(host.value,42,f.request,1101));
    assert(!submittedWorldMatches(host.value,43,f.request,1000));
    assert(!submittedWorldMatches(host.value,42,f.request,999));
    for(unsigned fault=0;fault<5;++fault) {
        auto receipt=host.value;
        if(fault==0)++receipt.session;if(fault==1)++receipt.reference;
        if(fault==2)++receipt.trackingGeneration;if(fault==3)++receipt.requestSequence;
        if(fault==4)++receipt.sourceTickMs;
        assert(!submittedWorldMatches(receipt,42,f.request,1001));
    }
    host.sample(42,f.request,true);assert(host.value.sourceTickMs==1000);
    assert(!frames.submitted(host.value,42,1101,out)); // Cached resubmit cannot renew original age.
    host.sample(42,f.request,false);host.sample(42,f.request,true);
    assert(host.value.continuity!=epoch); // Loss survives skipped latest-value publication.
    assert(frames.submitted(host.value,42,1001,out));
    host.sample(42,f.request,false);assert(!frames.submitted(host.value,42,1001,out));
    for(unsigned fault=0;fault<7;++fault) {
        auto bad=f.request;
        if(fault==0)bad.sequence=0;if(fault==1)bad.session=0;if(fault==2)bad.reference=0;
        if(fault==3)bad.trackingGeneration|=0x80000000u;if(fault==4)bad.reserved=1;
        if(fault==5)bad.input.session++;if(fault==6)bad.input.reference++;
        host.sample(42,bad,true);assert(!host.value.active);
    }
    host.sample(0,f.request,true);assert(!host.value.active);
    RideGripFrames missing;
    for(unsigned i=0;i<3;++i) {auto later=f;later.request.sequence+=i;missing.add(later);}
    host.sample(42,f.request,true);assert(!missing.submitted(host.value,42,1000,out)); // Evicted candidate.
    assert(rideGripFrameCurrent(f,f.rig,f.request.input,1000));
    auto moved=f.rig;moved.anchor.p={100,10,-100};moved.anchor.q=yaw(1.f);
    assert(rideGripFrameCurrent(f,moved,f.request.input,1000)); // World anchor cancels in both operands.
    moved.revision++;assert(!rideGripFrameCurrent(f,moved,f.request.input,1000));
    assert(!rideGripFrameCurrent(f,f.rig,f.request.input,1101));
}
static void headingWrapReference() {
    // Native export/solver static evidence is documented in NATIVE_HOVER_HEADING.
    // This arithmetic fixture does not execute either native function.
    const float pi=3.1415927410125732f,turn=6.2831854820251465f;
    const auto converter=[](float delta,float &out) {out=std::atan2(std::sin(delta),std::cos(delta));return true;};
    const auto error=[&](float desired,float current) {
        const float r=std::fmod(desired-current+pi,turn);return (r<0?r+turn:r)-pi;
    };
    const auto current=[] {return true;};
    for(float base:{0.f,.5f,-.5f,pi-.01f,-pi+.01f})
        for(float delta:{0.f,.2f,-.2f,pi-.001f,pi+.001f,-pi-.001f,turn*4+.2f,-turn*4-.2f}) {
            float heading=base;assert(commitRideHeading(heading,base,delta,converter,current));
            float converted=0;converter(delta,converted);
            const float disparity=error(error(heading,base)-error(base+delta,base),0);
            assert(std::abs(disparity)<1e-5f);
            if(delta==0)assert(heading==base);
            if(std::abs(delta)<1)assert(delta>=0?heading>=base:heading<=base);
        }
    // Left-only initial fallback and a transfer near the converter branch cut.
    float heading=pi-.01f;assert(commitRideHeading(heading,heading,.02f,converter,current));
    const float returned=heading;
    assert(commitRideHeading(heading,returned,-.03f,converter,current));
    assert(std::abs(error(heading,pi-.02f))<1e-5f);
}
static void headingReentry() {
    float heading=10;bool eligible=true;unsigned conversions=0,originals=0;
    const auto admitted=[&] {return eligible;};
    const auto convert=[&](float delta,float &out) {++conversions;out=delta;return true;};
    eligible=false;assert(!commitRideHeading(heading,10,.2f,convert,admitted) && conversions==0 && heading==10);
    eligible=true;
    const auto reentrant=[&](float delta,float &out) {++conversions;out=delta;eligible=false;return true;};
    assert(!commitRideHeading(heading,10,.2f,reentrant,admitted) && conversions==1 && heading==10);
    eligible=true;const float fallback=heading;
    const bool intent=commitRideHeading(heading,10,.2f,convert,admitted);assert(intent && heading>10);
    ++originals;eligible=false; // A normal original return discovers nested rejection.
    finishRideHeading(heading,fallback,intent,admitted());assert(heading==fallback && originals==1);
    finishRideHeading(heading,fallback,false,false);assert(heading==fallback);
}
static void gripContinuity() {
    auto f=gripFixture();Input input=f.request.input;
    input.headValid=1;input.gripValid[0]=input.gripValid[1]=1;
    input.grip[0].p={-.25f,0,-.4f};input.grip[1].p={.25f,0,-.4f};
    input.wheelAdmissionMask=3;input.wheelInputEpoch[0]=input.wheelInputEpoch[1]=1;
    uint32_t poses[2]={1,1};float delta=0,base=0;RideGrasp grasp;
    auto step=[&](bool fresh=true,bool eligible=true,uint32_t epoch=1) {
        if(fresh) {++input.sequence;++input.tickMs;}
        return grasp.sample(f,input,poses,epoch,eligible,10.f,delta,base);
    };
    assert(!step());assert(!step());assert(!step()); // Key, epochs, then positive new Low.
    input.buttons[0]=Wheel;assert(step() && grasp.mask==1 && delta==0 && base==10);
    input.grip[0].q=yaw(.2f);assert(step() && std::abs(delta-.2f)<1e-5f);
    assert(step(false) && std::abs(delta-.2f)<1e-5f); // Cached held input never integrates twice.
    grasp.accept(12.f);input.buttons[1]=Wheel;
    assert(step() && grasp.mask==3 && delta==0 && base==12.f);
    input.grip[1].p={.24f,0,-.5f};assert(step() && std::abs(delta)>0.01f);
    grasp.accept(13.f);input.buttons[0]=0;
    assert(step() && grasp.mask==2 && delta==0 && base==13.f); // Smooth two -> one transfer.
    auto animated=f;animated.surfaces[1].positions[0].y+=.01f;f=animated;
    input.grip[1].q=yaw(.1f);assert(step() && std::abs(delta-.1f)<1e-5f); // Same identity animation keeps baseline.
    ActionStream poseStream;assert(poseStream.sample(true));poseStream.sample(false);poseStream.sample(true);
    poses[1]=poseStream.generation;assert(!step() && !grasp.mask);
    assert(!step() && !grasp.hands[1].armed); // Recovered held cannot stand in for release.
    input.grip[1].p={.25f,0,-.4f};input.buttons[1]=0;assert(!step() && grasp.hands[1].armed);
    input.buttons[1]=Wheel;assert(step());
    assert(!step(true,false));input.buttons[1]=0;
    assert(!step(false) && !grasp.hands[1].armed); // Cached neutral after interruption cannot rearm.
    assert(!step());assert(!step());input.buttons[1]=Wheel;assert(step());
    // A world omission/new epoch cancels acquisition even when consumer missed the omission itself.
    assert(!step(true,true,2));assert(!step(true,true,2));
    input.buttons[1]=0;assert(!step(true,true,2));input.buttons[1]=Wheel;assert(step(true,true,2));
    f.rig.rider.seat++;assert(!step(true,true,2)); // Mount identity replacement cannot inherit grasp.
    // Outside press consumes arming: moving an already squeezed controller onto handle cannot auto-grab.
    RideGrasp outside;input.buttons[0]=input.buttons[1]=0;poses[0]=poses[1]=1;
    auto outsideStep=[&] {++input.sequence;++input.tickMs;return outside.sample(f,input,poses,1,true,10,delta,base);};
    assert(!outsideStep());assert(!outsideStep());assert(!outsideStep());
    input.grip[0].p={0,0,-.8f};input.buttons[0]=Wheel;assert(!outsideStep());
    input.grip[0].p={-.25f,0,-.4f};assert(!outsideStep());
    input.buttons[0]=0;assert(!outsideStep());input.buttons[0]=Wheel;assert(outsideStep());
    input.grip[0].q={0,0,0,0};assert(!outsideStep()); // Invalid pose cancels.
    assert(rideContact(f,{-.25f,0,-.4f})==0);
    assert(rideContact(f,{10,10,10})==-1);
    auto invalid=f;invalid.surfaces[0].indices[0]=99;assert(rideContact(invalid,{-.25f,0,-.4f})==-1);
    assert(std::abs(ridePointTriangle({0,0,.2f},{-1,-1,0},{1,-1,0},{0,1,0})-.04)<1e-6);
    // Producer eligibility equals consumer eligibility, including losses skipped by latest IPC.
    for(unsigned failure=0;failure<4;++failure) {
        auto good=input;good.headValid=1;good.gripValid[0]=1;good.head={};good.grip[0]={};good.grip[0].p={-.25f,0,-.4f};
        ActionStream stream;assert(stream.sample(rideGripPoseEligible(good,0)));
        auto lost=good;
        if(failure==0)lost.head.q={0,0,0,0};
        if(failure==1)lost.grip[0].q={0,0,0,0};
        if(failure==2)lost.grip[0].p={100,0,0};
        if(failure==3)lost.headValid=0;
        assert(!stream.sample(rideGripPoseEligible(lost,0)));
        assert(stream.sample(rideGripPoseEligible(good,0)) && stream.generation!=1);
        RideGrasp recovered;auto neutral=good;neutral.buttons[0]=neutral.buttons[1]=0;
        uint32_t epoch[2]={1,1};
        auto run=[&] {++neutral.sequence;++neutral.tickMs;return recovered.sample(f,neutral,epoch,1,true,10,delta,base);};
        assert(!run());assert(!run());assert(!run());neutral.buttons[0]=Wheel;assert(run());
        epoch[0]=stream.generation;assert(!run());assert(!run() && !recovered.hands[0].armed);
        neutral.buttons[0]=0;assert(!run());neutral.buttons[0]=Wheel;assert(run());
    }
    // Compact abnormal cleanup invalidates use immediately and establishes a new boundary later.
    outside.interrupt();outside.interrupt();input.grip[0].q={};input.buttons[0]=0;
    assert(!outsideStep()); // A newly keyed cached Low is not a release witness.
    assert(!outside.sample(f,input,poses,1,true,10,delta,base) && !outside.hands[0].armed);
    input.buttons[0]=Wheel;assert(!outsideStep()); // Recovered-held still cannot acquire.
    input.buttons[0]=0;assert(!outsideStep());input.buttons[0]=Wheel;assert(outsideStep());
    // A lost left pose preserves the valid right clutch and uses accepted native heading.
    RideGrasp transfer;f=gripFixture();input.buttons[0]=input.buttons[1]=0;
    input.grip[0]={};input.grip[0].p={-.25f,0,-.4f};input.grip[1]={};input.grip[1].p={.25f,0,-.4f};
    poses[0]=poses[1]=1;
    auto transferStep=[&] {++input.sequence;++input.tickMs;return transfer.sample(f,input,poses,1,true,10,delta,base);};
    assert(!transferStep());assert(!transferStep());assert(!transferStep());
    input.buttons[0]=input.buttons[1]=Wheel;assert(transferStep() && transfer.mask==3);
    transfer.accept(15);++poses[0];assert(transferStep() && transfer.mask==2 && delta==0 && base==15);
    assert(!transfer.hands[0].armed && transfer.hands[1].held);
    input.grip[1].q=yaw(.3f);assert(transferStep() && std::abs(delta-.3f)<1e-5f && base==15);
    // A vertical one-hand axis has no heading reference; cancel rather than inventing yaw.
    input.grip[1].q={std::sqrt(.5f),0,0,std::sqrt(.5f)};
    assert(!transferStep() && !transfer.keyed && !transfer.mask);
    // Anchor-relative contact is invariant under native vehicle translation and rotation.
    const Pose origin{yaw(.3f),{.1f,0,.2f}},anchor{yaw(-.7f),{10,2,-5}};
    Pose head{},hand{};hand.p={.2f,0,-.4f};
    const auto local=bodyHandTracking(origin,.4f,head,hand);
    const auto world=worldHandTracking(anchor,origin,.4f,head,hand);
    const auto recovered=rotate(inverse(anchor.q),world.p-anchor.p);
    assert(dot(recovered-local.p,recovered-local.p)<1e-10f);

}

int main() {
    submissionFeedback();headingWrapReference();headingReentry();gripContinuity();
    mainMappings();vehicleRanges();handleGeometry();handlePosition();retainedRideBank();
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
