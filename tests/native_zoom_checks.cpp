#include "common/native_zoom.hpp"
#include <cstdlib>
#include <iostream>
using namespace ss2vr;
static void check(bool ok,const char *message) {
    if(!ok) { std::cerr<<message<<'\n';std::exit(1); }
}
int main() {
    int weapon[NativeZoomClaims::Capacity+1]{},owner[2]{};
    NativeZoomClaims claims;
    auto left=claims.reserve(&weapon[0],&owner[0],1,100,0);
    auto right=claims.reserve(&weapon[1],&owner[0],2,100,1);
    check(left.serial && right.serial && left.serial!=right.serial && claims.token(left),"Hands have distinct source identities");
    claims.audioStarted(left.serial,left.revision);
    claims.change(left.serial,false,true,false);
    const auto external=claims.find(left.weapon);
    check(!claims.token(left) && !external.effect && external.leftAudio,
          "External takeover revokes old effect context, preserving only the audio obligation");
    claims.change(right.serial,true,false,false);
    const auto deleting=claims.find(right.weapon);
    check(!claims.token(right) && deleting.effect && deleting.deleting && deleting.blocked,
          "Deletion revokes old activation before preserving original native deactivation provenance");
    check(!claims.reserve(right.weapon,right.owner,2,100,1).serial,"Nested callbacks cannot resurrect deleting source");
    claims.sourceDeleted(right.serial);
    auto replacement=claims.reserve(right.weapon,right.owner,2,100,1);
    check(replacement.serial>right.serial && !claims.token(right),"Pointer/handle reuse cannot restore retired context");
    claims.change(replacement.serial,false,false,true);
    auto aborted=claims.find(right.weapon);
    check(aborted.effect && aborted.blocked && !claims.token(replacement),"Abort retains unresolved effect cleanup and revokes permission");
    claims.ownerDeleting(&owner[0]);
    check(claims.find(left.weapon).teardown && claims.find(left.weapon).leftAudio,
          "Owner teardown preserves cleanup metadata before original source deletion");
    check(!claims.reserve(left.weapon,&owner[1],1,101,0).serial,"Owner reassociation cannot inherit an old source claim");
    {
        NativeZoomClaims routing;
        auto originalRight=routing.reserve(&weapon[0],&owner[0],1,100,1);
        auto currentLeft=routing.reserve(&weapon[1],&owner[1],2,101,0);
        routing.change(originalRight.serial,true,false,false);
        const NativeZoomDeleteRouting reassociatedRight{&owner[1],101,1};
        check(nativeZoomPreserveDelete(reassociatedRight,currentLeft) &&
              routing.find(originalRight.weapon).owner==&owner[0] &&
              routing.find(originalRight.weapon).serial==originalRight.serial &&
              routing.find(originalRight.weapon).effect,
              "Current Delete owner B preserves B's left while right cleanup provenance remains owner A");
        check(nativeZoomPreserveDelete(reassociatedRight,currentLeft),
              "Delete routing requires no persistent claim on the deleting right source");
        routing.ownerDeleting(&owner[1]);
        check(!nativeZoomPreserveDelete(reassociatedRight,routing.find(currentLeft.weapon)),
              "Owner B teardown prevents cross-delete suppression despite current routing identity");
    }
    NativeZoomClaims bounded;
    for(unsigned i=0;i<NativeZoomClaims::Capacity;++i)
        check(bounded.reserve(&weapon[i],&owner[0],i+1,100,i%2).serial,"Bounded source capacity admits distinct lifetimes");
    check(!bounded.reserve(&weapon[NativeZoomClaims::Capacity],&owner[0],200,100,0).serial,
          "Capacity exhaustion declines activation without evicting cleanup");
    const auto retained=bounded.find(&weapon[0]);
    bounded.audioStarted(retained.serial,retained.revision);
    check(bounded.reserve(&weapon[0],&owner[0],1,100,0).serial==retained.serial && bounded.find(&weapon[0]).leftAudio,
          "Repeated zoom cycles reuse one source record and retain cleanup provenance");
    bounded.sourceDeleted(retained.serial);
    check(bounded.reserve(&weapon[NativeZoomClaims::Capacity],&owner[0],200,100,0).serial>retained.serial,
          "Only actual source deletion reclaims capacity");
    check(nativeZoomStateAllowed(1,false,0) && nativeZoomStateAllowed(7,false,0) &&
          !nativeZoomStateAllowed(4,false,0) && nativeZoomStateAllowed(4,true,0) &&
          !nativeZoomStateAllowed(8,false,0) && nativeZoomStateAllowed(8,true,0) &&
          !nativeZoomStateAllowed(2,true,0) && !nativeZoomStateAllowed(3,true,0) &&
          !nativeZoomStateAllowed(1,true,2),"Native cooldown retains zoom but holster/bringup/deletion cannot activate");
    {
        NativeZoomClaims audio;
        auto claim=audio.reserve(&weapon[0],&owner[0],1,100,0);
        audio.audioStarted(claim.serial,claim.revision);
        check(nativeZoomStepPredicate(3,0,claim,audio.find(claim.weapon),true,false)==0 &&
              nativeZoomStepPredicate(4,0,claim,audio.find(claim.weapon),true,false)==0 &&
              nativeZoomStepPredicate(5,0,claim,audio.find(claim.weapon),true,false)==1,
              "Nested PutDown denies new effects while preserving the still-borrowed left Stop");
        audio.change(claim.serial,false,true,false);
        const auto live=audio.find(claim.weapon);
        check(nativeZoomStepPredicate(3,0,claim,live,true,false)==0 &&
              nativeZoomStepPredicate(4,0,claim,live,true,false)==0 &&
              nativeZoomStepPredicate(5,0,claim,live,true,false)==1,
              "External takeover denies old effects but preserves same-source native Stop");
        check(nativeZoomStepPredicate(5,0,claim,live,true,true)==0,
              "Deleted borrowed source cannot receive an outer-frame Stop");
        audio.change(claim.serial,true,false,false);
        check(nativeZoomStepPredicate(5,0,claim,audio.find(claim.weapon),true,false)==0,
              "Shared deletion metadata independently rejects stale cleanup on another context");
        audio.sourceDeleted(claim.serial);
        auto replacement=audio.reserve(&weapon[0],&owner[0],1,100,0);
        check(nativeZoomStepPredicate(5,1,claim,replacement,true,false)==0,
              "Reused source cannot inherit the retired frame's cleanup opportunity");
        check(nativeZoomPhaseAllowed(false,true,true,false,NativeZoomQuery::PreserveCrossDelete) &&
              !nativeZoomPhaseAllowed(false,true,true,false,NativeZoomQuery::Effect) &&
              !nativeZoomPhaseAllowed(true,false,true,false,NativeZoomQuery::PreserveCrossDelete) &&
              !nativeZoomPhaseAllowed(true,true,false,true,NativeZoomQuery::Effect) &&
              nativeZoomPhaseAllowed(false,true,false,true,NativeZoomQuery::Effect),
              "Preparation grants only preservation; lexical A-B-A nesting cannot regain effects");
    }
    std::cout<<"Native zoom provenance checks passed; no native callback executed.\n";
}
