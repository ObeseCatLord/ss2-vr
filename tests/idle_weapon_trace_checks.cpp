#include "common/idle_weapon_trace.hpp"
#include <cassert>
#include <limits>
#include <string_view>
#include <iostream>
using namespace ss2vr;
#ifdef NDEBUG
#error Idle diagnostic checks need active assertions
#endif
int main() {
    {IdlePaletteCopy p;
     p.metadata.modelAddress=100;p.metadata.drawAddress=200;p.metadata.modelRecord=1;
     p.metadata.drawRecord=3;p.metadata.surface=300;p.metadata.instance=400;
     p.metadata.rootConfig={500,600,1};p.metadata.renderConfig=p.metadata.rootConfig;
     p.first=4;p.count=3;p.mapCount=7;p.paletteCount=7;p.canonicalCount=12;p.modelCount=2;
     p.evaluated=700;p.matrices=800;p.mappingAddress=900;p.paletteAddress=1000;p.copied=true;
     p.world[0]=p.world[5]=p.world[10]=0x3f800000;
     for(unsigned i=0;i<3;++i) {
        auto &e=p.entries[i];e.draw=3;e.bone=int32_t(2+i);e.owner=1;e.definition=1100+i*120;e.name=1200+i;
        e.canonical[0]=e.canonical[5]=e.canonical[10]=0x3f800000;
        e.canonical[3]=std::bit_cast<uint32_t>(float(i+1)*7);
        e.palette=e.canonical;
     }
     assert(p.bounded() && p.paletteCopiesCanonical());
     auto different=p;different.entries[1].palette=p.entries[0].palette;
     assert(different.bounded() && !different.paletteCopiesCanonical()); // Independent reference catches a crossed palette.
     assert(different!=p);
     for(unsigned change=0;change<14;++change) {
        auto bad=p;
        if(change==0)bad.count=0;else if(change==1)bad.count=4;
        else if(change==2)bad.first=UINT32_MAX;else if(change==3)bad.paletteCount=6;
        else if(change==4)bad.mapCount=6;else if(change==5)bad.canonicalCount=4;
        else if(change==6)bad.entries[0].bone=-1;else if(change==7)bad.entries[2].draw=2;
        else if(change==8)bad.entries[2].owner=0;else if(change==9)bad.entries[1].definition=0;
        else if(change==10)bad.world[3]=0x7f800000;else if(change==11)bad.entries[0].canonical[0]=0x7fc00001;
        else if(change==12)bad.entries[2].palette[0]=0xff800000;else bad.copied=false;
        assert(!bad.bounded() && !bad.paletteCopiesCanonical());
     }
     for(unsigned count:{1u,2u,3u}) {auto part=p;part.count=count;
        assert(part.bounded() && part.paletteCopiesCanonical());}
     // A matching matrix is insufficient when the row's identity changes.
     different=p;different.metadata.instance=401;
     assert(different.bounded() && different.paletteCopiesCanonical() && different!=p);
     different=p;different.evaluated=701;assert(different!=p);
     different=p;different.entries[2].definition=1400;assert(different!=p);
     const ScopeIndexedDraw draw{4,0,0,3017,0,2673};
     IdleSubmissionTrace s;const auto slot=s.reserve(draw);
     s.before(slot,p.metadata,true);s.paletteBefore(slot,p,true);
     s.after(slot,p.metadata,true,0);s.paletteAfter(slot,p,true);s.finalize(slot,false,true,false);
     assert(!s.palettePublishable(slot)); // A DIP alone cannot complete the original weapon invocation.
     s.outerReturned=true;
     for(unsigned missing=0;missing<5;++missing) {
        s.completeOuter(missing!=0,missing!=1,missing!=2,missing!=3,missing!=4);
        assert(!s.palettePublishable(slot));
     }
     s.completeOuter(true,true,true,true,true);assert(s.palettePublishable(slot));
     s.paletteAfter(slot,different,true);assert(!s.palettePublishable(slot));
     // Disagreement between native rows is diagnostic data, not a filter that
     // seeks a matching sample or substitutes upload rows as the reference.
     s=IdleSubmissionTrace{};different=p;different.entries[1].palette=p.entries[0].palette;
     const auto mismatchSlot=s.reserve(draw);s.before(mismatchSlot,different.metadata,true);
     s.paletteBefore(mismatchSlot,different,true);s.after(mismatchSlot,different.metadata,true,0);
     s.paletteAfter(mismatchSlot,different,true);s.finalize(mismatchSlot,false,true,false);
     s.outerReturned=true;s.completeOuter(true,true,true,true,true);
     assert(s.palettePublishable(mismatchSlot) && !s.rows[mismatchSlot].palette.paletteCopiesCanonical());
     s.reenter(mismatchSlot);s.finalize(mismatchSlot,false,true,false);
     assert(!s.palettePublishable(mismatchSlot) && !s.rows[mismatchSlot].palette.copied);
     // The API copy has its own capacity and exact ordinal; rejected geometry
     // and failed getters must not select a later passing draw as a substitute.
     IdleWeaponTrace t;t.nativeId=2;t.draws=IdleWeaponTrace::MaxDraws;
     t.stage=IdleWeaponTrace::Stage::Rejected;t.rejection=IdleWeaponTrace::Rejection::RasterMapping;
     for(unsigned i=0;i<IdleWeaponTrace::MaxPaletteApiPayloads;++i) {
        const auto n=t.submissions.reserve(draw),api=t.reservePaletteApi(n);
        assert(api==i && t.paletteApiPayloads[api].submissionSlot==n &&
               t.paletteApiPayloads[api].ordinal==i+1 && t.submissions.rows[n].paletteApiSlot==api);
        assert(!t.paletteApiPublishable(n)); // Missing getter copies still consume a slot.
     }
     const auto overflow=t.submissions.reserve(draw);
     assert(t.reservePaletteApi(overflow)==IdleWeaponTrace::MaxPaletteApiPayloads && t.paletteApiOverflow);
     assert(t.paletteApiCount==IdleWeaponTrace::MaxPaletteApiPayloads &&
            t.rejection==IdleWeaponTrace::Rejection::RasterMapping && t.draws==IdleWeaponTrace::MaxDraws);
     t.submissions.before(0,p.metadata,true);t.submissions.paletteBefore(0,p,true);
     t.submissions.after(0,p.metadata,true,0);t.submissions.paletteAfter(0,p,true);
     t.submissions.finalize(0,false,true,false);t.submissions.outerReturned=true;
     t.submissions.completeOuter(true,true,true,true,true);
     auto &api=t.paletteApiPayloads[0];api.words=2;api.before.status=IdleWeaponTrace::StreamSnapshot::Copied;
     api.beforeCopied=api.afterCopied=api.matched=true;
     assert(t.paletteApiPublishable(0)); // Overall RasterMapping rejection is diagnostic, not a new admission rule.
     const auto good=api;
     for(unsigned failure=0;failure<9;++failure) {
        api=good;
        if(failure==0)api.submissionSlot=1;else if(failure==1)++api.ordinal;
        else if(failure==2)api.words=1;else if(failure==3)api.words=uint32_t(api.program.size()+1);
        else if(failure==4)api.beforeCopied=false;else if(failure==5)api.afterCopied=false;
        else if(failure==6)api.matched=false;else if(failure==7)api.before.status=IdleWeaponTrace::StreamSnapshot::Interrupted;
        else t.submissions.rows[0].paletteApiSlot=IdleWeaponTrace::MaxPaletteApiPayloads;
        assert(!t.paletteApiPublishable(0));
     }
     api=good;t.submissions.rows[0].paletteApiSlot=0;
     t.submissions.finalize(0,false,false,false);assert(!t.paletteApiPublishable(0)); // Release-time retirement.
     for(int id:{1,13}) {t=IdleWeaponTrace{};t.nativeId=id;
        const auto n=t.submissions.reserve(draw);
        assert(t.reservePaletteApi(n)==IdleWeaponTrace::MaxPaletteApiPayloads && !t.paletteApiCount);}
     t=IdleWeaponTrace{};t.nativeId=2;
     assert(t.reservePaletteApi(IdleSubmissionTrace::NoSlot)==IdleWeaponTrace::MaxPaletteApiPayloads && !t.paletteApiCount);
    }
    {IdleSubmissionMetadata a;a.modelAddress=100;a.drawAddress=200;a.modelRecord=1;a.drawRecord=3;
     a.surface=300;a.instance=400;a.bone=0;a.rootConfig={500,600,1};a.renderConfig=a.rootConfig;
     a.layout.vertices=132;a.layout.triangles=108;
     const ScopeIndexedDraw draw{4,0,0,132,4782,108};
     IdleWeaponTrace trace;trace.stage=IdleWeaponTrace::Stage::Rejected;
     trace.rejection=IdleWeaponTrace::Rejection::Draw;trace.draws=IdleWeaponTrace::MaxDraws;trace.rejectionChecks=32639;
     auto &s=trace.submissions;
     for(unsigned i=0;i<64;++i) {
        const auto slot=s.reserve(draw);assert(slot==i && s.rows[i].ordinal==i+1);
        s.before(slot,a,i%2==0);s.after(slot,a,true,0);s.finalize(slot,false,true,false);
        assert(s.rows[i].status==(i%2==0?IdleSubmissionTrace::Status::Qualified:IdleSubmissionTrace::Status::PreUnknown));
     }
     assert(s.reserve(draw)==IdleSubmissionTrace::NoSlot && s.attempts==65 && s.overflow);
     assert(trace.rejection==IdleWeaponTrace::Rejection::Draw && trace.draws==IdleWeaponTrace::MaxDraws && trace.rejectionChecks==32639);
     s.attempts=UINT32_MAX;assert(s.reserve(draw)==IdleSubmissionTrace::NoSlot && s.attempts==UINT32_MAX);
     for(unsigned changed=0;changed<4;++changed) {
        IdleSubmissionTrace t;const auto slot=t.reserve(draw);auto b=a;
        if(changed==0)++b.modelAddress;else if(changed==1)++b.drawAddress;
        else if(changed==2)++b.instance;else ++b.rootConfig.resource;
        t.before(slot,a,true);t.after(slot,b,true,0);t.finalize(slot,false,true,false);
        assert(t.rows[0].status==IdleSubmissionTrace::Status::Mismatch && !t.rows[0].metadata.instance);
     }
     for(unsigned phase=0;phase<3;++phase) {
        IdleSubmissionTrace t;const auto slot=t.reserve(draw);
        if(phase==0)t.reenter(slot); // AddRef before pre sample.
        t.before(slot,a,true);if(phase==1)t.reenter(slot); // Original forwarding.
        t.after(slot,a,true,0);if(phase==2)t.reenter(slot); // Release after matching post sample.
        t.finalize(slot,false,true,false);
        assert(t.rows[0].status==IdleSubmissionTrace::Status::Reentered && t.rows[0].returned && !t.rows[0].metadata.instance);
     }
     for(unsigned failure=0;failure<5;++failure) {
        IdleSubmissionTrace t;auto slot=t.reserve(draw);t.before(slot,a,true);
        if(failure!=4)t.after(slot,a,failure!=3,failure==2?-1:0);
        t.finalize(slot,failure==0,failure!=1,false);
        const IdleSubmissionTrace::Status expected[]{IdleSubmissionTrace::Status::Aborted,IdleSubmissionTrace::Status::Retired,
          IdleSubmissionTrace::Status::OriginalFailed,IdleSubmissionTrace::Status::PostUnknown,IdleSubmissionTrace::Status::NoForward};
        assert(t.rows[0].status==expected[failure] && !t.rows[0].metadata.instance);
     }
     IdleSubmissionTrace split;auto slot=split.reserve(draw);split.finalize(slot,false,true,true);
     assert(split.rows[slot].status==IdleSubmissionTrace::Status::Split);
     std::cout<<"Submission row bytes="<<sizeof(IdleSubmissionTrace::Row)<<" inventory bytes="<<sizeof(IdleSubmissionTrace)
              <<" whole idle trace bytes="<<sizeof(IdleWeaponTrace)<<'\n';}
    {IdleWeaponTrace::InputFailure d;d.step=20;d.valid=15;d.declarationCount=5;
     const std::array<ScopeDeclarationElement,5> rows{{
        {0,0,2,0,5,0},{2,0,1,0,5,2},{7,0,8,0,5,7},
        {8,0,8,0,5,8},{255,0,17,0,0,0}}};
     std::copy(rows.begin(),rows.end(),d.declaration.begin());
     const std::array<unsigned,3> expected{0,7,8};
     assert(IdleWeaponTrace::passiveStreamNumbers(d)==expected);
     bool weights=false;GeometryInputLayout layout{};
     assert(!declaredGeometryInputs(rows,weights));
     assert(idleGeometryInputs(rows,layout,weights) && layout==GeometryInputLayout::NoUV78 && weights);
     assert(!declaredGeometryInputs(rows,weights,GeometryInputLayout::NoUV78)); // Scope13 never opts in.
     for(unsigned i=0;i<rows.size();++i)for(unsigned field=0;field<6;++field) {
        auto bad=d;auto &e=bad.declaration[i];
        switch(field) {case 0:++e.stream;break;case 1:++e.offset;break;
          case 2:++e.type;break;case 3:++e.method;break;case 4:++e.usage;break;case 5:++e.usageIndex;break;}
        assert(!IdleWeaponTrace::observedStreamFamily(bad));
     }
     for(unsigned i=0;i<5;++i) {auto bad=d;
        if(i==0)bad.step=19;else if(i==1)bad.valid=11;
        else bad.declarationCount=i==2?4:i==3?6:66;
        assert(!IdleWeaponTrace::observedStreamFamily(bad));}}
    {IdleWeaponTrace::InputFailure d;d.step=20;d.valid=15;d.declarationCount=8;
     const std::array<ScopeDeclarationElement,8> rows{{
        {0,0,2,0,5,0},{2,0,1,0,5,2},{3,0,1,0,5,3},{4,0,1,0,5,4},
        {5,0,1,0,5,5},{7,0,8,0,5,7},{8,0,8,0,5,8},{255,0,17,0,0,0}}};
     std::copy(rows.begin(),rows.end(),d.declaration.begin());
     const std::array<unsigned,3> expected{0,7,8};
     assert(IdleWeaponTrace::passiveStreamNumbers(d)==expected);
     assert(!observed78Declaration(rows) && !noUV56Declaration(rows));
     for(unsigned i=0;i<rows.size();++i) {
        for(unsigned field=0;field<6;++field) {
           auto bad=d;auto &e=bad.declaration[i];
           switch(field) {case 0:++e.stream;break;case 1:++e.offset;break;
             case 2:++e.type;break;case 3:++e.method;break;case 4:++e.usage;break;case 5:++e.usageIndex;break;}
           assert(!IdleWeaponTrace::observedStreamFamily(bad));
        }
     }
     for(unsigned i=0;i<3;++i) {auto bad=d;
        if(i==0)bad.step=19;else if(i==1)bad.valid=11;else bad.declarationCount=7;
        assert(!IdleWeaponTrace::observedStreamFamily(bad));}}
    {IdleWeaponTrace::InputFailure d;d.step=20;d.valid=15;d.declarationCount=6;
     d.declaration[0]={0,0,2,0,5,0};d.declaration[1]={2,0,1,0,5,2};d.declaration[2]={3,0,1,0,5,3};
     d.declaration[3]={7,0,8,0,5,7};d.declaration[4]={8,0,8,0,5,8};d.declaration[5]={255,0,17,0,0,0};
     assert(IdleWeaponTrace::observedStreamFamily(d));
     for(unsigned i=0;i<6;++i){auto changed=d;changed.declaration[i].stream++;
        assert(!IdleWeaponTrace::observedStreamFamily(changed));}
     for(unsigned i=0;i<3;++i){auto changed=d;if(i==0)changed.step=19;else if(i==1)changed.valid=11;else changed.declarationCount=65;
        assert(!IdleWeaponTrace::observedStreamFamily(changed));}}
    {IdleWeaponTrace::InputFailure d;d.step=20;d.valid=15;d.declarationCount=5;
     d.declaration[0]={0,0,2,0,5,0};d.declaration[1]={1,0,2,0,5,1};d.declaration[2]={5,0,8,0,5,5};
     d.declaration[3]={6,0,8,0,5,6};d.declaration[4]={255,0,17,0,0,0};
     const std::array<unsigned,3> expected{0,5,6};
     assert(IdleWeaponTrace::observedStreamFamily(d) && IdleWeaponTrace::passiveStreamNumbers(d)==expected);
     bool weights=false;assert(!declaredGeometryInputs(std::span(d.declaration).first(5),weights));
     for(unsigned i=0;i<5;++i){auto changed=d;changed.declaration[i].usageIndex++;
        assert(!IdleWeaponTrace::observedStreamFamily(changed));}
     auto resampled=IdleWeaponTrace::StreamSnapshot{};resampled.declaration[0].stream=7;
     assert(IdleWeaponTrace::passiveStreamNumbers(d)==expected); // Resampling cannot select another family.
     auto mixed=d;mixed.declaration[2].stream=7;assert(!IdleWeaponTrace::observedStreamFamily(mixed));
     for(unsigned i=0;i<3;++i){auto bad=d;if(i==0)bad.step=19;else if(i==1)bad.valid=11;else bad.declarationCount=66;
        assert(!IdleWeaponTrace::observedStreamFamily(bad));}}
    {IdleWeaponTrace::StreamSnapshot a;a.status=IdleWeaponTrace::StreamSnapshot::Copied;a.caps=256;
     a.declarationCount=6;a.declarationObject=1;a.indexObject=2;a.shaderObject=3;
     a.streams={ScopeStreamInput{4,0,12,1},ScopeStreamInput{4,48384,4,1},ScopeStreamInput{4,49256,4,1}};
     assert(IdleWeaponTrace::sameStreamSnapshots(a,a));
     for(unsigned i=0;i<9;++i){auto b=a;
        switch(i){case 0:b.status=IdleWeaponTrace::StreamSnapshot::Interrupted;break;
            case 1:b.streams[1].offset++;break;case 2:b.streams[2].object++;break;
            case 3:b.declarationObject++;break;case 4:b.indexObject++;break;case 5:b.shaderObject++;break;
            case 6:b.constants[0][0]++;break;case 7:b.declaration[0].usage++;break;case 8:b.caps=0;break;}
        assert(!IdleWeaponTrace::sameStreamSnapshots(a,b));}}
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
    {IdleWeaponTrace t;assert(t.admit(id));assert(t.event(id,cfg,true,2));
     auto value=borrowed;t.rejectAnimationName(0,77,value);value={};
     assert(t.rejection==IdleWeaponTrace::Rejection::QueryAnimationName && t.animationNameFailure.copied);
     assert(t.animationNameFailure.index==0 && t.animationNameFailure.expected==77 && t.animationNameFailure.header==borrowed.header);
     assert(t.stage==IdleWeaponTrace::Stage::Rejected && t.animationsCopied==0);
     t.rejectAnimationName(1,88,value);t.reject(IdleWeaponTrace::Rejection::QueryAbort);
     assert(t.animationNameFailure.expected==77 && t.animationNameFailure.header==borrowed.header);}
    for(unsigned failure=0;failure<5;++failure) {
        IdleWeaponTrace t;assert(t.admit(id));assert(t.event(id,cfg,true,1));
        if(failure==0)t.reject(IdleWeaponTrace::Rejection::QueryMemory);
        if(failure==1)t.stage=IdleWeaponTrace::Stage::Palette;
        if(failure==2)t.animationsCopied=1;
        t.rejectAnimationName(failure==3?1:0,failure==4?55:77,borrowed);
        assert(!t.animationNameFailure.copied);
    }
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
    for(unsigned miss=1;miss<4;++miss) {
        IdleWeaponTrace t;copiedEvent(t);
        const uint32_t evaluated=100,linked=miss&1?101:100,owner=miss&2?201:200,instance=200;
        t.notePaletteOwnershipFailure(id,cfg,evaluated,linked,owner,instance,1);
        assert(t.paletteOwnershipFailure.copied);
        assert(!t.palette(id,cfg,evaluated==linked && owner==instance,1));
        assert(t.rejection==IdleWeaponTrace::Rejection::Palette && t.rejectionChecks==111);
        assert(t.paletteOwnershipFailure.evaluated==100 && t.paletteOwnershipFailure.linked==linked &&
               t.paletteOwnershipFailure.owner==owner && t.paletteOwnershipFailure.instance==200);
        t.notePaletteOwnershipFailure(id,cfg,300,300,400,400,1);t.reject();
        assert(t.paletteOwnershipFailure.evaluated==100 && !t.finish(true,true));
    }
    for(unsigned invalid=0;invalid<6;++invalid) {
        IdleWeaponTrace t;copiedEvent(t);auto changed=id;auto changedConfig=cfg;
        int count=1;uint32_t instance=200;
        switch(invalid) {
            case 0:changed.model++;break;
            case 1:changedConfig.resource++;break;
            case 2:count=0;break;
            case 3:count=65;break;
            case 4:instance=0;break;
            case 5:t.reject();break;
        }
        t.notePaletteOwnershipFailure(changed,changedConfig,100,101,201,instance,count);
        assert(!t.paletteOwnershipFailure.copied);
    }
    {IdleWeaponTrace t;copiedEvent(t);t.notePaletteOwnershipFailure(id,cfg,100,100,200,200,1);
     assert(!t.paletteOwnershipFailure.copied && t.palette(id,cfg,true,1));}
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
    {auto diagnostic=g;
     auto &f=diagnostic.raster.factors;
     f.model=f.local=f.view=identity;
     f.model.m[3]=262.25f;f.local.m[3]=-262.25f; // Large cancelling primitive translations.
     for(unsigned i=0;i<4;++i)f.projection.m[i*4+i]=1;
     f.paletteIndex=17;f.modelCopied=f.cameraCopied=true;
     assert(f.valid() && diagnostic.raster==g.raster); // Optional factors never change old equality.
     diagnostic.factorBookend(diagnostic.raster,false);
     assert(diagnostic.factorChecks==1 && !diagnostic.factorsAvailable());
     diagnostic.factorBookend(diagnostic.raster,true);
     assert(diagnostic.factorsAvailable());
     auto t=ready();assert(t.draw(diagnostic,true,true));
     f.local.m[3]=0; // Owned factors survive later native source reuse.
     assert(t.geometry[0].raster.factors.local.m[3]==-262.25f);
     for(unsigned variant=0;variant<9;++variant) {
        auto copy=t.geometry[0];copy.factorChecks=0;auto now=copy.raster;
        copy.factorBookend(now,false);
        switch(variant) {case 0:now.factors.paletteIndex++;break;
            case 1:now.factors.model.m[3]=100;now.factors.local.m[3]=-100;break; // Same combined result, different descendant operands.
            case 2:now.factors.cameraCopied=false;break;case 3:now.factors.modelCopied=false;break;
            case 4:now.factors.view.m[3]=.1f;break;case 5:now.factors.projection.m[3]=.1f;break;
            case 6:now.factors.local.m[0]=std::numeric_limits<float>::quiet_NaN();break;
            case 7:now.factors.model.m[1]=-0.f;break; // Preserve raw bits, not just numeric equality.
            case 8:now.drawRecord++;break;}
        copy.factorBookend(now,true);assert(!copy.factorsAvailable());
        auto ordinary=ready();assert(ordinary.draw(copy,true,true)); // Companion loss is not draw loss.
     }
     auto missing=g;missing.factorChecks=3;assert(!missing.factorsAvailable());
     auto ordinary=ready();assert(ordinary.draw(missing,true,true));
     auto stable=t.geometry[0];stable.factorChecks=0;auto before=stable.raster;
     before.factors.model.m[3]++;stable.factorBookend(before,false);
     stable.factorBookend(stable.raster,true);assert(stable.factorChecks==2 && !stable.factorsAvailable());
     stable.factorChecks=0;stable.factorBookend(stable.raster,true);assert(!stable.factorsAvailable());
     for(unsigned variant=0;variant<7;++variant) {
        auto original=t.geometry[0];original.factorChecks=0;auto changed=original.raster;
        switch(variant) {case 0:changed.binding.eye^=1;break;case 1:changed.binding.hand^=1;break;
            case 2:changed.modelRecord++;break;case 3:changed.instance++;break;case 4:changed.bone++;break;
            case 5:changed.rootConfig.file++;break;case 6:changed.renderConfig.configuration++;break;}
        original.factorBookend(changed,false);original.factorBookend(original.raster,true);
        assert(!original.factorsAvailable() && !(changed==original.raster));
     }
    }
    {IdleRasterFactors factors;
     const char *labels[]{"factorModel","factorLocal","factorView","factorProjection"};
     float *matrices[]{factors.model.m,factors.local.m,factors.view.m,factors.projection.m};
     std::array<std::array<uint32_t,16>,4> expected{};
     for(unsigned i=0;i<4;++i)for(unsigned j=0;j<(i==3?16u:12u);++j) {
        expected[i][j]=std::bit_cast<uint32_t>(float(i*100+j+1));
        matrices[i][j]=std::bit_cast<float>(expected[i][j]);
     }
     expected[0][1]=0x80000000;expected[1][2]=1;expected[2][3]=0x80000001;
     for(unsigned i=0;i<3;++i)matrices[i][i+1]=std::bit_cast<float>(expected[i][i+1]);
     unsigned count=0;
     factors.emitMatrices([&](const char *label,std::span<const float> values) {
        assert(count<4 && std::string_view(label)==labels[count] && values.size()==(count==3?16u:12u));
        for(unsigned j=0;j<values.size();++j)assert(std::bit_cast<uint32_t>(values[j])==expected[count][j]);
        ++count;
     });
     assert(count==4);
    }
    {static_assert(IdleWeaponTrace::MaxDraws==10);
     auto t=ready();auto copy=g;
     for(unsigned i=0;i<10;++i) {
        copy.raster.drawRecord=5;copy.raster.instance=100+i;
        copy.constants[0][0]=float(i);copy.hashes[0][0]=uint8_t(i);
        assert(t.draw(copy,true,true));
     }
     assert(t.finish(true,true) && t.draws==10);
     for(unsigned i=0;i<10;++i) {
        assert(t.geometry[i].raster.instance==100+i && t.geometry[i].raster.drawRecord==5);
        assert(t.geometry[i].constants[0][0]==float(i) && t.geometry[i].hashes[0][0]==i);
     }
    }
    {auto t=ready();t.callbacks=63;
        auto stored=g;stored.program[0]=0xfffe0101;stored.program[1]=0xffff;
        stored.constants[0][0]=12;stored.hashes[0][0]=73;
        stored.raster.factors.model=stored.raster.factors.local=stored.raster.factors.view=identity;
        stored.raster.factors.model.m[3]=42;stored.raster.factors.paletteIndex=17;
        stored.raster.factors.modelCopied=stored.raster.factors.cameraCopied=true;
        stored.factorChecks=3;
        assert(t.draw(stored,true,true));
        auto second=stored;second.raster.drawRecord=0;second.program[1]=123;
        second.constants[0][0]=34;second.raster.instance=9;second.raster.modelRecord=2;second.raster.bone=4;
        second.hashes[0][0]=99;
        for(unsigned i=0;i<12;++i){second.raster.factors.model.m[i]+=1;second.raster.factors.local.m[i]+=2;second.raster.factors.view.m[i]+=3;}
        for(auto &v:second.raster.factors.projection.m)v+=4;
        assert(t.draw(second,true,true) && t.draws==2 && t.stage==IdleWeaponTrace::Stage::Palette);
        assert(t.geometry[0].program[1]==0xffff && t.geometry[0].constants[0][0]==12 &&
               t.geometry[0].hashes[0][0]==73 && t.geometry[0].raster.factors.model.m[3]==42);
        assert(t.geometry[1].program[1]==123 && t.geometry[1].constants[0][0]==34 &&
               t.geometry[1].hashes[0][0]==99 && t.geometry[1].raster.instance==9);
        assert(t.geometry[0].raster.drawRecord==t.geometry[1].raster.drawRecord);
        assert(t.geometry[0].raster.factors.model.m[3]!=t.geometry[1].raster.factors.model.m[3]);
        for(bool completed:{false,true}) {
            auto failed=t;assert(!failed.finish(completed,!completed));
            assert(failed.stage==IdleWeaponTrace::Stage::Rejected && failed.draws==2 && !failed.retainedDrawCopiesAvailable());
            assert(failed.geometry[0].constants[0][0]==12 && failed.geometry[1].constants[0][0]==34);
        }
        assert(t.finish(true,true));
        auto invalid=ready();assert(invalid.draw(stored,true,true));
        second.constants[0][0]=std::numeric_limits<float>::quiet_NaN();
        assert(!invalid.draw(second,true,true) && invalid.rejection==IdleWeaponTrace::Rejection::DrawConstants && invalid.draws==1);
        assert(invalid.geometry[0].constants[0][0]==12);
        for(auto reason:{IdleWeaponTrace::Rejection::CollectHash,IdleWeaponTrace::Rejection::CollectRebind,IdleWeaponTrace::Rejection::GpuAbort}) {
            auto aborted=ready();assert(aborted.draw(stored,true,true));aborted.reject(reason);
            assert(!aborted.draw(stored,true,true) && !aborted.finish(true,true) && aborted.draws==1);
        }}
    {auto t=ready();t.callbacks=63;t.reject(IdleWeaponTrace::Rejection::DrawDuplicate);
     assert(!t.retainedDrawCopiesAvailable());}
    {auto t=ready();t.callbacks=63;auto copy=g;
     for(unsigned i=0;i<IdleWeaponTrace::MaxDraws;++i){copy.constants[0][0]=float(i);assert(t.draw(copy,true,true));}
     copy.raster.drawRecord=0;assert(!t.draw(copy,true,true));
     assert(t.draws==10);for(unsigned i=0;i<10;++i)assert(t.geometry[i].constants[0][0]==float(i));
     assert(t.rejection==IdleWeaponTrace::Rejection::Draw && t.rejectionChecks==32639);
     assert(t.retainedCapacityCopiesAvailable() && t.retainedDrawCopiesAvailable());
     for(unsigned i=0;i<IdleWeaponTrace::MaxDraws;++i)assert(t.geometry[i].raster.drawRecord==g.raster.drawRecord);
     for(unsigned bit=0;bit<15;++bit)if(bit!=7) {
        auto bad=t;bad.rejectionChecks&=~(1u<<bit);assert(!bad.retainedDrawCopiesAvailable());
     }
     for(unsigned variant=0;variant<9;++variant) {
        auto bad=t;
        switch(variant) {case 0:bad.draws=IdleWeaponTrace::MaxDraws-1;break;case 1:bad.draws=IdleWeaponTrace::MaxDraws+1;break;
            case 2:bad.precedingStage=IdleWeaponTrace::Stage::Event;break;case 3:bad.callbacks=47;break;
            case 4:bad.rejectionState^=1;break;case 5:bad.inputFailure.step=20;break;
            case 6:bad.streamProbe.selected=true;break;case 7:bad.config.file=0;break;
            case 8:bad.rejection=IdleWeaponTrace::Rejection::DrawConstants;break;}
        assert(!bad.retainedCapacityCopiesAvailable() && !bad.retainedDrawCopiesAvailable());
     }
     auto mixed=ready();mixed.callbacks=63;
     for(unsigned i=0;i<IdleWeaponTrace::MaxDraws;++i)assert(mixed.draw(copy,true,true));
     assert(!mixed.draw(copy,false,true) && !mixed.retainedDrawCopiesAvailable());
     assert(!t.finish(true,true) && t.retainedDrawCopiesAvailable()); // Still rejected, no outer claim.
    }
    for(bool original:{false,true}){auto t=ready();t.callbacks=63;assert(t.draw(g,true,true));
        assert(!t.draw(g,original,!original));assert(t.rejection==IdleWeaponTrace::Rejection::Draw);
        assert(t.draws==1 && !t.retainedDrawCopiesAvailable());}
    {auto t=ready();assert(t.draw(g,true,true));g.constants[0][0]=1;
     assert(t.geometry[0].constants[0][0]==0);assert(t.draw(g,true,true));assert(t.geometry[1].constants[0][0]==1);}
    for(unsigned failure=0;failure<3;++failure) {
        auto t=ready();auto bad=g;
        if(failure==0)bad.raster.binding.generation++;
        assert(!t.draw(bad,failure!=1,failure!=2));assert(t.stage==IdleWeaponTrace::Stage::Rejected);
    }
    {auto t=ready();for(unsigned i=0;i<IdleWeaponTrace::MaxDraws;++i){g.raster.drawRecord=i;assert(t.draw(g,true,true));}
     g.raster.drawRecord=IdleWeaponTrace::MaxDraws;assert(!t.draw(g,true,true));}

    {auto t=ready();assert(t.finish(true,true));assert(t.draws==0);} // Event/pose-only, not geometry evidence.
    {auto changed=g.raster;changed.clip.m[0]=1;assert(!(changed==g.raster));}
    {auto changed=g.raster;changed.affine.m[3]=.1f;assert(!(changed==g.raster));}
    {auto t=ready();assert(t.draw(g,true,true));
     t.inputFailure.step=20;t.inputFailure.valid=15;
     t.reject(IdleWeaponTrace::Rejection::CollectInputs);
     assert(t.retainedDrawCopiesAvailable() && t.draws==1 && t.geometry[0].words==g.words);
     t.reject(IdleWeaponTrace::Rejection::GpuAbort);t.reject(IdleWeaponTrace::Rejection::GpuRetired);
     assert(!t.finish(false,false));
     assert(t.retainedDrawCopiesAvailable() && t.rejection==IdleWeaponTrace::Rejection::CollectInputs &&
            t.stage==IdleWeaponTrace::Stage::Rejected && t.geometry[0].words==g.words);
     for(unsigned i=0;i<8;++i){auto bad=t;
        switch(i){case 0:bad.draws=0;break;case 1:bad.inputFailure.step=19;break;case 2:bad.inputFailure.valid=11;break;
        case 3:bad.referencesCopied=false;break;case 4:bad.poseCopied=false;break;case 5:bad.animationsCopied=0;break;
        case 6:bad.config.file=0;break;case 7:bad.precedingStage=IdleWeaponTrace::Stage::Event;break;}
        assert(!bad.retainedDrawCopiesAvailable());}}
    for(bool returned:{false,true}){auto t=ready();assert(!t.draw(g,returned,!returned));
        t.inputFailure.step=20;t.inputFailure.valid=15;t.reject(IdleWeaponTrace::Rejection::CollectInputs);
        assert(t.draws==0 && !t.retainedDrawCopiesAvailable());}
    {auto t=ready();g.raster.clipValid=false;assert(!t.draw(g,true,true));}

    {IdleWeaponTrace t;assert(t.admit(id));assert(t.placement(id,identity,identity,identity));
     auto changed=id;changed.input++;assert(!t.references(changed,identity,true,identity));}
    {IdleWeaponTrace t;assert(t.admit(id));assert(t.placement(id,identity,identity,identity));
     Matrix34 bad=identity;bad.m[0]=std::numeric_limits<float>::quiet_NaN();
     assert(t.references(id,bad,false,identity));assert(!t.rawGripValid && t.referencesCopied);}
    {IdleWeaponTrace t;assert(t.admit(id));assert(t.placement(id,identity,identity,identity));
     assert(t.references(id,identity,true,identity));assert(!t.references(id,identity,true,identity));}

}
