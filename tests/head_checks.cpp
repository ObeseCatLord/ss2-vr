#include "common/head_palette.hpp"
#include "common/palette_provenance.hpp"
#include "common/presentation_identity.hpp"
#include "common/frame_policy.hpp"
#include <memory>
#include <cassert>
#include <cstring>
#include <limits>
using namespace ss2vr;
static Vec3 point(const Matrix34 &m, Vec3 p) {
    return {m.m[0]*p.x+m.m[1]*p.y+m.m[2]*p.z+m.m[3],
            m.m[4]*p.x+m.m[5]*p.y+m.m[6]*p.z+m.m[7],
            m.m[8]*p.x+m.m[9]*p.y+m.m[10]*p.z+m.m[11]};
}
static void near(Vec3 a, Vec3 b) {
    assert(std::abs(a.x-b.x)<.0003f && std::abs(a.y-b.y)<.0003f && std::abs(a.z-b.z)<.0003f);
}
int main() {
    std::vector<PaletteBone> bones{{0,-1,0,false},{1,0,11,true},{1,1,12,true},
                                  {1,2,13,true},{2,2,14,true},{2,0,12,true}};
    std::vector<PaletteMap> maps{{0,-1},{0,1},{0,2},{1,2},{1,3},{2,4},{2,5}};
    std::vector<int32_t> drawOwners{1,1,2};
    std::vector<Matrix34> worlds{matrix({}),{{-1.2f,.1f,0,3, 0,1.7f,.2f,1, 0,0,.8f,-4}},matrix({})};
    std::vector<Matrix34> native(maps.size(),matrix({})), out;
    for(size_t i=0;i<native.size();++i) { native[i].m[3]=float(i)*.13f; native[i].m[7]=.4f; }
    Pose eye{yaw(.8f),{2,1.4f,-3}}, delta{normalize({.17f,.24f,-.13f,.9f}),{.2f,-.5f,.3f}};
    
    std::vector<PaletteModelRange> modelRanges{{-1,0},{0,1},{1,1}};
    std::vector<PaletteMeshRange> meshRanges{{1,0,2},{2,2,1}};
    std::vector<PaletteDrawRange> drawRanges{{0,0,3},{0,3,2},{1,5,2}};
    std::vector<int32_t> ownership;
    assert(paletteDrawOwners(modelRanges,meshRanges,drawRanges,maps,ownership));
    assert(ownership==drawOwners);
    drawRanges[2].first=4;
    assert(!paletteDrawOwners(modelRanges,meshRanges,drawRanges,maps,ownership) && ownership.empty());
    drawRanges[2].first=5; meshRanges[1].owner=1;
    assert(!paletteDrawOwners(modelRanges,meshRanges,drawRanges,maps,ownership));
    meshRanges[1].owner=2; maps[3].draw=0;
    assert(!paletteDrawOwners(modelRanges,meshRanges,drawRanges,maps,ownership)); maps[3].draw=1;
    std::vector<uintptr_t> instances{0,100,200};
    assert(paletteBodyOwner(instances,300)==-1); // unrelated static draw: no canonical requirement
    assert(paletteBodyOwner(instances,100)==1);
    instances.push_back(100); assert(paletteBodyOwner(instances,100)==-2);
    std::vector<PaletteBone> headless{{0,-1,0,false},{1,0,11,true}};
    std::vector<PaletteMap> headlessMaps{{0,1}};
    std::vector<Matrix34> headlessPalette{matrix({})};
    assert(retargetHeadPalette(headless,headlessMaps,drawOwners,worlds,1,12,eye,delta,headlessPalette,out)==
           HeadPaletteResult::Unchanged && out.empty());
    assert(disjointMatrixStorage(0x1000,10,0x2000,10));
    assert(!disjointMatrixStorage(0x1000,10,0x1100,10));
    assert(!disjointMatrixStorage(UINTPTR_MAX-3,10,0x2000,10));
    assert(!validPaletteSpan(INT32_MAX,INT32_MAX,7));
    network::PosePacket oldPose, newer;
    oldPose.clientNonce=newer.clientNonce=11; oldPose.serverNonce=newer.serverNonce=22;
    oldPose.trackingGeneration=newer.trackingGeneration=3;
    oldPose.validMask=newer.validMask=7; newer.sequence=99; newer.head.p.x=.5f;
    assert(samePresentationIdentity(5,8,oldPose,5,8,newer,true,true));
    newer.validMask=0; assert(!samePresentationIdentity(5,8,oldPose,5,8,newer,true,true));
    newer.validMask=7; assert(!samePresentationIdentity(5,8,oldPose,5,9,newer,true,true));
    newer.trackingGeneration=4; assert(!samePresentationIdentity(5,8,oldPose,5,8,newer,true,true));
    bool active=false, invalidated=false;
    int originals=0, adaptations=0, faults=0;
    auto original=[&] { ++originals; };
    auto adapt=[&] { ++adaptations; };
    auto fault=[&] { ++faults; };
    withFreshNativePalette(false,active,invalidated,original,adapt,fault);
    assert(originals==1 && !adaptations && !faults && !active);
    withFreshNativePalette(true,active,invalidated,original,adapt,fault);
    assert(originals==2 && adaptations==1 && !faults && !active);
    withFreshNativePalette(true,active,invalidated,[&] {
        ++originals;
        withFreshNativePalette(false,active,invalidated,original,adapt,fault);
    },adapt,fault);
    assert(originals==4 && adaptations==1 && faults==1 && !active);
    int writes=0;
    withFreshNativePalette(true,active,invalidated,original,[&] {
        withFreshNativePalette(false,active,invalidated,original,adapt,fault);
        if (!invalidated) ++writes; // Actual adapter's pre-write fence.
    },fault);
    assert(!writes && faults==2 && !active);
    bool escaped=false;
    int beforeAdapt=adaptations;
    try {
        withFreshNativePalette(true,active,invalidated,[] { throw 7; },adapt,fault);
    } catch (int value) { escaped=value==7; }
    assert(escaped && !active && adaptations==beforeAdapt);
    // Explicit native cleanup must also work when the GNU destructor never ran.
    for(bool previous : {false,true}) {
        active=true;invalidated=false;
        retireNativePaletteInvocation(previous,true,active,invalidated);
        assert(active==previous&&invalidated);
        retireNativePaletteInvocation(previous,true,active,invalidated);
        assert(active==previous&&invalidated); // Idempotent scalar retirement.
        active=true;invalidated=false;
        retireNativePaletteInvocation(previous,false,active,invalidated);
        assert(active==previous&&!invalidated);
    }
    active=false;invalidated=false;
    unsigned objectReads=0;
    auto unsupportedOwner=[&] { ++objectReads; return false; };
    auto slot=std::make_unique<Slot>(); Request request; request.sequence=1;
    slot->state=SlotState::Rendering; slot->request=request;
    bool pairAllowed=eligiblePresentationPair(true,true,true,false,unsupportedOwner);
    assert(!commitNativeFrame(*slot,request,pairAllowed));
    assert(slot->state==SlotState::Rendering && objectReads==1);
    pairAllowed=eligiblePresentationPair(true,true,false,false,unsupportedOwner);
    assert(commitNativeFrame(*slot,request,pairAllowed) && slot->state==SlotState::Ready && objectReads==1);
    // A lost-and-restored identity cannot silently re-label an older frozen pose.
    assert(validFrozenPresentationRevision(1,1));
    assert(!validFrozenPresentationRevision(1,2));
    assert(!validFrozenPresentationRevision(0,0));
    assert(validFrozenPresentationRevision(2,2));
    // Native refresh restores canonical bytes on every invocation, including
    // an animation-cache hit. Production phase ordering prevents accumulation.
    std::vector<Matrix34> scratch=native, canonical=native;
    for(int frame=0;frame<3;++frame) {
        withFreshNativePalette(true,active,invalidated,[&] { scratch=canonical; },[&] {
            assert(retargetHeadPalette(bones,maps,drawOwners,worlds,1,12,eye,delta,scratch,out)==HeadPaletteResult::Changed);
            scratch=out;
        },fault);
        Vec3 originalWorld=point(worlds[1],point(canonical[2],{0,0,0}));
        Vec3 expected=eye.p+rotate(eye.q,delta.p+rotate(delta.q,rotate(inverse(eye.q),originalWorld-eye.p)));
        near(point(worlds[1],point(scratch[2],{0,0,0})),expected);
        assert(!std::memcmp(canonical.data(),native.data(),native.size()*sizeof(Matrix34)));
    }
    auto run=[&](Pose d) { return retargetHeadPalette(bones,maps,drawOwners,worlds,1,12,eye,d,native,out); };
    assert(run(delta)==HeadPaletteResult::Changed);
    for(size_t i=0;i<native.size();++i) {
        bool selected=i==2||i==3||i==4;
        if(!selected) assert(!std::memcmp(&out[i],&native[i],sizeof(Matrix34)));
        else for(Vec3 p: {Vec3{0,0,0},Vec3{.2f,-.7f,.1f}}) {
            // Independent rigid world reference: move native point to eye-local,
            // apply tracked delta, and return to world. Affine native W stays intact.
            Vec3 world=point(worlds[1],point(native[i],p));
            Vec3 local=rotate(inverse(eye.q),world-eye.p);
            Vec3 expected=eye.p+rotate(eye.q,delta.p+rotate(delta.q,local));
            near(point(worlds[1],point(out[i],p)),expected);
        }
    }
    assert(std::memcmp(&out[2],&out[3],sizeof(Matrix34)) != 0); // repeated mapping keeps each native offset
    assert(run({})==HeadPaletteResult::Unchanged && out.empty());
    Pose negativeIdentity; negativeIdentity.q.w=-1;
    assert(run(negativeIdentity)==HeadPaletteResult::Unchanged && out.empty());
    auto saved=bones;
    bones[1].parent=3; assert(run(delta)==HeadPaletteResult::Invalid && out.empty());
    bones=saved; bones[3].name=12; assert(run(delta)==HeadPaletteResult::Invalid && out.empty());
    bones=saved; bones[2].named=false; assert(run(delta)==HeadPaletteResult::Invalid && out.empty());
    bones=saved; bones[3].parent=int32_t(bones.size()); assert(run(delta)==HeadPaletteResult::Invalid);
    bones=saved; maps[0].draw=3; assert(run(delta)==HeadPaletteResult::Invalid); maps[0].draw=0;
    maps[6].bone=-2; assert(run(delta)==HeadPaletteResult::Invalid); maps[6].bone=5;
    bones[2].name=99; assert(run(delta)==HeadPaletteResult::Unchanged && out.empty()); bones=saved;
    auto w=worlds[1]; worlds[1].m[0]=worlds[1].m[1]=worlds[1].m[2]=0;
    assert(run(delta)==HeadPaletteResult::Invalid && out.empty()); worlds[1]=w;
    native[2].m[0]=std::numeric_limits<float>::infinity();
    assert(run(delta)==HeadPaletteResult::Invalid && out.empty()); native[2].m[0]=1;
    delta.q.w=0; delta.q.x=0; delta.q.y=0; delta.q.z=0;
    assert(run(delta)==HeadPaletteResult::Invalid && out.empty());
}
