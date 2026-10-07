#pragma once
#include "sphere_query_subject.hpp"
namespace ss2vr::roomscale {
// Read-only query-filter binding. No assumptions about foot capsule shape,
// active child-propagation flags, or a zero mechanism parent belong here.
// The caller owns the native phase and independently validates ride/seat/brain.
struct HeadQueryBinding {
    SphereQuerySubject subject;
    uint32_t playerHandle=0,player=0,mechanismHandle=0;
    uint32_t poseSourceHandle=0,poseSource=0,parentMechanism=0,viewResource=0,playerModel=0;
    Pose bodyPose{};
};
template<class Read,class Resolve>
inline bool readHeadQueryBinding(Read read,Resolve resolve,uint32_t playerHandle,
                                 uint32_t playerVtable,uint32_t category,HeadQueryBinding& out) {
    if(!playerHandle||!playerVtable)return false;
    HeadQueryBinding value;value.playerHandle=playerHandle;value.player=resolve(playerHandle);
    const auto copy=[&](uint32_t address,uint32_t offset,void* to,size_t size) {
        return address&&offset<=UINT32_MAX-address&&size<=UINT32_MAX-(address+offset)&&
            read(address+offset,to,size);
    };
    const auto word=[&](uint32_t address,uint32_t offset,uint32_t& to) {
        return copy(address,offset,&to,sizeof(to));
    };
    uint32_t table=0,flags=0;
    if(!word(value.player,0,table)||table!=playerVtable||!word(value.player,0x10,flags)||(flags&2)||
       !word(value.player,0x47c,value.viewResource)||!value.viewResource||
       !word(value.player,0x120,value.playerModel)||
       !word(value.player,0x114,value.mechanismHandle)||!value.mechanismHandle)return false;
    value.subject.avatar=value.player;value.subject.mechanism=resolve(value.mechanismHandle);
    if(!word(value.subject.mechanism,0x34,value.parentMechanism)||
       !word(value.subject.mechanism,0x38,value.poseSourceHandle)||!value.poseSourceHandle)return false;
    value.poseSource=resolve(value.poseSourceHandle);
    if(!copy(value.poseSource,0x2c,&value.bodyPose,sizeof(value.bodyPose))||
       !finite(value.bodyPose)||!bodyUnitQuaternion(value.bodyPose.q)||
       resolve(playerHandle)!=value.player||resolve(value.mechanismHandle)!=value.subject.mechanism||
       resolve(value.poseSourceHandle)!=value.poseSource)return false;
    value.subject.categories[0]=category;value.subject.categoryCount=1;
    if(!value.subject.valid())return false;
    out=value;return true;
}
inline bool sameHeadQueryBinding(const HeadQueryBinding& a,const HeadQueryBinding& b) noexcept {
    return a.subject==b.subject&&a.playerHandle==b.playerHandle&&a.player==b.player&&
        a.mechanismHandle==b.mechanismHandle&&a.poseSourceHandle==b.poseSourceHandle&&
        a.poseSource==b.poseSource&&a.parentMechanism==b.parentMechanism&&
        a.viewResource==b.viewResource&&a.playerModel==b.playerModel&&
        std::memcmp(&a.bodyPose,&b.bodyPose,sizeof(Pose))==0;
}
} // namespace ss2vr::roomscale
