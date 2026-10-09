#include "common/idle_weapon_trace.hpp"
#include <cassert>
#include <limits>
using namespace ss2vr;
#ifdef NDEBUG
#error Native projection diagnostics need active assertions
#endif
static IdleProjectionSnapshot sample(uint32_t flags=0) {
    IdleProjectionSnapshot s{};s.control=0x007f;s.flags=flags;s.modelRecord=100;s.drawRecord=200;
    for(unsigned i=0;i<12;++i) {s.model[i]=0x3f800000+i;s.view[i]=0x3f000000+i;}
    for(unsigned i=0;i<16;++i) {s.projection[i]=0x3f400000+i;s.cachedVP[i]=0x3f200000+i;s.cachedMVP[i]=0x3f600000+i;}
    return s;
}
int main() {
    {IdleProjectionProbe p;assert(p.enterHelper());assert(!p.enterHelper());
     assert(p.helperActive && p.blocked);p.leaveHelper(false);assert(!p.helperActive && !p.begin());}
    {IdleProjectionProbe p;assert(p.enterHelper());p.leaveHelper(true);assert(p.blocked);}
    {IdleProjectionProbe p;assert(!p.enterFog() && p.blocked);}
    {IdleProjectionProbe p;*p.begin()=sample();assert(p.enterFog());assert(!p.enterFog());
     assert(p.fogActive && p.blocked);p.leaveFog(false);assert(!p.fogActive && !p.pending);}
    {IdleProjectionProbe p;assert(p.enterHelper());p.retire();assert(p.blocked && !p.helperActive && !p.fogActive);}
    {IdleProjectionProbe q;assert(q.enterHelper());q.leaveHelper(false);*q.begin()=sample();
     assert(q.enterFog());q.end(sample(6),true);q.leaveFog(false);
     assert(q.count==1 && !q.helperActive && !q.fogActive && q.pairs[0].qualified());}
    {IdleProjectionProbe q;assert(q.enterHelper());q.leaveHelper(false);*q.begin()=sample();
     assert(q.enterFog());q.leaveFog(true);assert(q.blocked && !q.count && !q.pending);}
    IdleProjectionProbe p;auto before=sample();auto after=sample(6);
    *p.begin()=before;p.end(after,true);
    assert(p.count==1 && !p.pending && !p.blocked && p.pairs[0].qualified());
    auto reused=sample(6);*p.begin()=reused;p.end(reused,true);
    assert(p.count==2 && p.pairs[1].before.flags==6); // Reuse, not an earlier producer certificate.
    for(unsigned fail=0;fail<3;++fail) {
        IdleProjectionProbe q;auto b=sample(6),a=sample(6);*q.begin()=b;
        if(fail==0)a.cachedVP[15]++;
        if(fail==1)a.cachedMVP[3]++;
        if(fail==2)a.flags|=8;
        q.end(a,true);assert(q.blocked && !q.count);
    }
    {IdleProjectionProbe q;auto b=sample();b.cachedVP[0]=0x7fc00000;b.cachedMVP[0]=0x80000000;
     *q.begin()=b;q.end(sample(6),true);assert(q.count==1 && q.pairs[0].qualified());}
    for(unsigned fail=0;fail<7;++fail) {
        IdleProjectionProbe q;*q.begin()=before;auto bad=after;
        if(fail==0)bad.control^=0x100;
        if(fail==1)bad.modelRecord++;
        if(fail==2)bad.drawRecord++;
        if(fail==3)bad.model[11]++;
        if(fail==4)bad.view[7]++;
        if(fail==5)bad.projection[15]++;
        if(fail==6)bad.flags=2;
        q.end(bad,true);assert(q.blocked && q.count==0 && !q.pending && !q.begin());
    }
    {IdleProjectionProbe q;*q.begin()=before;q.end(after,false);assert(!q.count && q.blocked);}
    {IdleProjectionProbe q;*q.begin()=before;assert(!q.begin() && q.blocked);q.end(after,true);assert(!q.count);}
    {IdleProjectionProbe q;q.end(after,true);assert(q.blocked && !q.count);}
    {IdleProjectionProbe q;*q.begin()=before;q.retire();assert(q.blocked && !q.pending);}
    while(p.count<IdleProjectionProbe::MaxPairs) {*p.begin()=before;p.end(after,true);}
    auto first=p.pairs[0];assert(!p.begin() && p.blocked && p.count==8 && p.pairs[0]==first);
    IdleGeometryCopy g;g.constantCount=5;
    g.raster.projectionSequence=1;g.raster.projectionModelAddress=100;g.raster.projectionDrawAddress=200;
    g.raster.factors.modelCopied=g.raster.factors.cameraCopied=true;
    auto &f=g.raster.factors;f.local=matrix(Pose{});g.raster.clipValid=true;
    for(unsigned i=0;i<12;++i) {f.model.m[i]=std::bit_cast<float>(after.model[i]);f.view.m[i]=std::bit_cast<float>(after.view[i]);}
    for(unsigned i=0;i<16;++i)f.projection.m[i]=std::bit_cast<float>(after.projection[i]);
    for(unsigned i=0;i<16;++i)g.constants[1+i/4][i%4]=std::bit_cast<float>(after.cachedMVP[i]);
    g.factorBookend(g.raster,false);g.factorBookend(g.raster,true);
    assert(g.projectionAvailable(first));
    for(unsigned fail=0;fail<5;++fail) {
        auto bad=g.raster;
        if(fail==0)bad.factors.model.m[11]=std::bit_cast<float>(after.model[11]+1);
        if(fail==1)bad.factors.view.m[7]=std::bit_cast<float>(after.view[7]+1);
        if(fail==2)bad.factors.projection.m[15]=std::bit_cast<float>(after.projection[15]+1);
        if(fail==3)bad.projectionModelAddress++;
        if(fail==4)bad.projectionDrawAddress++;
        assert(!bad.matchesProjection(first));
    }
    auto changed=g.raster;changed.projectionSequence=2;
    assert(changed==g.raster); // Optional evidence cannot alter ordinary raster admission.
    g.factorBookend(changed,true);assert(!g.projectionAvailable(first));
    g.factorBookend(g.raster,true);g.constants[4][3]=std::bit_cast<float>(after.cachedMVP[15]+1);
    assert(!g.projectionAvailable(first)); // Each draw's actual upload remains independent.
}
