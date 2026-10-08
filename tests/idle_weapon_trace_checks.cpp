#include "common/idle_weapon_trace.hpp"
#include <cassert>
#include <limits>
using namespace ss2vr;
#ifdef NDEBUG
#error Idle diagnostic checks need active assertions
#endif
int main() {
    {IdleWeaponTrace t;t.admitted=true;t.placementObserved=true;t.stage=IdleWeaponTrace::Stage::Event;
     t.noteRejection(IdleWeaponTrace::Rejection::RasterPrerequisites,1);
     assert(t.stage==IdleWeaponTrace::Stage::Event); // Recording adds no retirement policy.
     t.callbacks|=IdleWeaponTrace::RasterSeen;
     t.reject(IdleWeaponTrace::Rejection::GpuAdmission,3);t.reject();
     assert(t.rejection==IdleWeaponTrace::Rejection::RasterPrerequisites && t.precedingStage==IdleWeaponTrace::Stage::Event);
     assert(t.rejectionChecks==1 && (t.rejectionState&3)==3 && t.callbacks==4 && t.stage==IdleWeaponTrace::Stage::Rejected);}
    const IdleDrawIdentity id{100,90,1,2,3,4,0,1};
    const IdleConfigIdentity cfg{20,30,5};
    const auto identity=matrix(Pose{});
    IdleAnimationValue borrowed{{1,2,3,4,5,6,7,0x1000},{55,0,10,0x3f800000}};
    std::array<Matrix34,1> nativePose{identity};
    IdleWeaponTrace trace;
    assert(trace.admit(id));assert(trace.placement(id,identity,identity,identity));assert(trace.references(id,identity,true,identity));
    assert(trace.event(id,cfg,true,1));
    assert(trace.animation(0,borrowed));
    borrowed={}; // Hostile original End destroys/reuses its entry and value storage.
    assert(trace.animations[0].header[0]==55 && trace.animations[0].contribution[7]==0x1000);
    assert(trace.palette(id,cfg,true,1));
    assert(trace.pose(identity,{-1,1,1},nativePose)); // Native handed reflection is data.
    nativePose[0].m[0]=std::numeric_limits<float>::quiet_NaN(); // Original later retires evaluation storage.
    assert(trace.finish(true,true));
    assert(trace.matrices[0].m[0]==1 && trace.stretch.x==-1);
    const auto copiedEvent=[&](IdleWeaponTrace &t) {
        assert(t.admit(id));assert(t.placement(id,identity,identity,identity));assert(t.references(id,identity,true,identity));
        assert(t.event(id,cfg,true,1));
        assert(t.animation(0,{}));
    };
    unsigned queueReads=0;
    const auto probe=[&](uintptr_t caller,uintptr_t active,uintptr_t queue) {
        if(!nativeIdleQueryBorrow(caller,0xddded,active,queue))return;
        ++queueReads; // Only entered after production typed-borrow gate.
    };
    probe(0xddde8,5,5);probe(0xddded,5,6);probe(0xddded,0,0);
    assert(queueReads==0);probe(0xddded,5,5);assert(queueReads==1);
    {auto changed=id;changed.model++;IdleWeaponTrace t;assert(t.admit(id));
     assert(t.placement(id,identity,identity,identity));assert(!t.event(changed,cfg,true,1));}
    {auto changed=id;changed.weapon++;IdleWeaponTrace t;assert(t.admit(id));
     assert(!t.placement(changed,identity,identity,identity));}
    for(int count:{-1,0,17}) {IdleWeaponTrace t;assert(t.admit(id));assert(!t.event(id,cfg,true,count));assert(!t.finish(true,true));}
    {IdleWeaponTrace t;assert(t.admit(id));assert(t.event(id,cfg,true,16));}
    {IdleWeaponTrace t;assert(t.admit(id));assert(!t.event(id,cfg,false,1));}
    {IdleWeaponTrace t;assert(!t.palette(id,cfg,true,1));} // Missing event/cache evidence.
    {IdleWeaponTrace t;assert(t.admit(id));assert(t.event(id,cfg,true,1));assert(!t.event(id,cfg,true,1));}
    {IdleWeaponTrace t;assert(t.admit(id));assert(t.event(id,cfg,true,1));t.reject();assert(!t.palette(id,cfg,true,1));}
    for(int count:{-1,0,65}) {IdleWeaponTrace t;copiedEvent(t);assert(!t.palette(id,cfg,true,count));}
    {IdleWeaponTrace t;copiedEvent(t);assert(t.palette(id,cfg,true,64));}
    {IdleWeaponTrace t;copiedEvent(t);assert(!t.palette(id,cfg,false,1));}
    {auto changed=id;changed.model++;IdleWeaponTrace t;copiedEvent(t);assert(!t.palette(changed,cfg,true,1));}
    {auto changed=cfg;changed.resource++;IdleWeaponTrace t;copiedEvent(t);assert(!t.palette(id,changed,true,1));}
    {IdleWeaponTrace t;assert(t.admit(id));assert(t.event(id,cfg,true,1));assert(!t.animation(1,{}));}
    {IdleWeaponTrace t;copiedEvent(t);assert(t.palette(id,cfg,true,1));assert(!t.palette(id,cfg,true,1));}
    {IdleWeaponTrace t;copiedEvent(t);assert(t.palette(id,cfg,true,1));
     assert(!t.pose(identity,{1,1,1},{}));assert(!t.finish(true,true));} // Partial copy never publishes.
    {IdleWeaponTrace t;copiedEvent(t);assert(t.palette(id,cfg,true,1));
     assert(!t.pose(identity,{1,1,1},nativePose));assert(!t.finish(true,true));}
    {IdleWeaponTrace t;assert(t.admit(id));assert(t.event(id,cfg,true,1));assert(!t.palette(id,cfg,true,1));} // No animation copy.
    {IdleWeaponTrace t;copiedEvent(t);assert(t.palette(id,cfg,true,1));
     t.placementObserved=true;assert(!t.finish(true,true));} // No pose copy.
    for(unsigned failure=0;failure<3;++failure) {
        IdleWeaponTrace t;copiedEvent(t);assert(t.palette(id,cfg,true,1));
        assert(t.pose(identity,{1,1,1},{&identity,1}));
        if(failure==0)t.placementObserved=false;
        assert(!t.finish(failure!=1,failure!=2));
    }
    const auto ready=[&] {
        IdleWeaponTrace t;assert(t.admit(id));assert(t.placement(id,identity,identity,identity));assert(t.references(id,identity,true,identity));
        assert(t.event(id,cfg,true,1));assert(t.animation(0,{}));assert(t.palette(id,cfg,true,1));
        assert(t.pose(identity,{1,1,1},{&identity,1}));return t;
    };
    IdleGeometryCopy g;g.raster.binding=id;g.raster.rootConfig=cfg;g.raster.affine=identity;g.raster.clipValid=true;
    g.words=2;g.constantCount=1;g.declarationCount=1;
    {auto t=ready();assert(t.draw(g,true,true));g.constants[0][0]=1;
     assert(t.geometry[0].constants[0][0]==0);assert(!t.draw(g,true,true));}
    for(unsigned failure=0;failure<3;++failure) {
        auto t=ready();auto bad=g;
        if(failure==0)bad.raster.binding.generation++;
        assert(!t.draw(bad,failure!=1,failure!=2));assert(t.stage==IdleWeaponTrace::Stage::Rejected);
    }
    {auto t=ready();for(unsigned i=0;i<8;++i){g.raster.drawRecord=i;assert(t.draw(g,true,true));}
     g.raster.drawRecord=8;assert(!t.draw(g,true,true));}

    {auto t=ready();assert(t.finish(true,true));assert(t.draws==0);} // Event/pose-only, not geometry evidence.
    {auto changed=g.raster;changed.clip.m[0]=1;assert(!(changed==g.raster));}
    {auto changed=g.raster;changed.affine.m[3]=.1f;assert(!(changed==g.raster));}
    {auto t=ready();g.raster.clipValid=false;assert(!t.draw(g,true,true));}

    {IdleWeaponTrace t;assert(t.admit(id));assert(t.placement(id,identity,identity,identity));
     auto changed=id;changed.input++;assert(!t.references(changed,identity,true,identity));}
    {IdleWeaponTrace t;assert(t.admit(id));assert(t.placement(id,identity,identity,identity));
     Matrix34 bad=identity;bad.m[0]=std::numeric_limits<float>::quiet_NaN();
     assert(t.references(id,bad,false,identity));assert(!t.rawGripValid && t.referencesCopied);}
    {IdleWeaponTrace t;assert(t.admit(id));assert(t.placement(id,identity,identity,identity));
     assert(t.references(id,identity,true,identity));assert(!t.references(id,identity,true,identity));}

}
