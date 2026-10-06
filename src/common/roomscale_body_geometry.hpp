#pragma once
#include "math.hpp"
#include "roomscale_primitive_query.hpp"
#include <cstdint>
#include <cstring>
#include <limits>
#include <type_traits>

namespace ss2vr::roomscale {
struct BodyLayout {
    uint32_t hybridBodyVtable=0,primitiveHullVtable=0;
};
static_assert(sizeof(ss2vr::Pose)==28 && sizeof(Primitive)==16);
static_assert(std::is_trivially_copyable_v<ss2vr::Pose>);
// Addresses are comparison tokens within ONE already-owned native extent.
// They are not retained references and must never be serialized or used after
// a native callback without a fresh read and full identity comparison.
struct BodyGeometry {
    uint32_t playerHandle=0,player=0,mechanismHandle=0,mechanism=0;
    uint32_t rootHandle=0,root=0,modelHandle=0,model=0,parts=0,hull=0,category=0;
    ss2vr::Pose modelPose{},hullPose{};
    Primitive primitive{};
};

// A narrow complete-shape reader for one native hybrid body with one sphere or
// capsule hull. It does not certify the caller's worker/lifetime ownership.
// Read must copy exactly the requested bytes or fail; resolve returns a current
// generation-checked native address. Neither operation may run gameplay code.
template<class Read,class Resolve>
inline bool readBodyGeometry(Read&& read,Resolve&& resolve,const BodyLayout& layout,
                             uint32_t playerHandle,BodyGeometry& output) {
    output={};
    BodyGeometry value;
    value.playerHandle=playerHandle;
    if (!playerHandle||!layout.hybridBodyVtable||!layout.primitiveHullVtable) return false;
    value.player=resolve(playerHandle);
    auto copy=[&](uint32_t address,uint32_t offset,void* target,size_t size) {
        return address && offset<=UINT32_MAX-address && size<=UINT32_MAX-(address+offset) &&
               read(address+offset,target,size);
    };
    auto word=[&](uint32_t address,uint32_t offset,uint32_t& result) {
        return copy(address,offset,&result,sizeof(result));
    };
    uint32_t flags=0,carry=0,ride=0;
    if (!word(value.player,0x10,flags)||(flags&2)||
        !word(value.player,0x564,carry)||carry||!word(value.player,0x544,ride)||ride||
        !word(value.player,0x114,value.mechanismHandle)||!value.mechanismHandle||
        !word(value.player,0x118,value.rootHandle)||!value.rootHandle||
        !word(value.player,0x120,value.modelHandle)||!value.modelHandle) return false;
    value.mechanism=resolve(value.mechanismHandle);
    value.root=resolve(value.rootHandle);
    value.model=resolve(value.modelHandle);
    uint32_t count=0,root=0,modelHandle=0,vtable=0,owner=0;
    if (!word(value.mechanism,8,count)||count!=1||!word(value.mechanism,4,value.parts)||
        !word(value.parts,8,root)||root!=value.root||!root||
        !word(value.mechanism,0x38,modelHandle)||modelHandle!=value.modelHandle||
        !word(value.root,0,vtable)||vtable!=layout.hybridBodyVtable||
        !word(value.root,0x48,owner)||owner!=value.player||
        !word(value.root,8,value.hull)||!value.hull) return false;
    uint32_t parent=0,children=0,sibling=0,mechanism=0;
    if (!word(value.hull,0,vtable)||vtable!=layout.primitiveHullVtable||
        !word(value.hull,4,parent)||parent!=value.root||
        !word(value.hull,8,children)||children||!word(value.hull,0xc,sibling)||sibling||
        !word(value.hull,0x48,owner)||owner!=value.player||
        !word(value.hull,0x70,mechanism)||mechanism!=value.mechanism||
        !word(value.hull,0x54,value.category)||
        !copy(value.hull,0x78,&value.primitive,sizeof(value.primitive))||
        !copy(value.hull,0x2c,&value.hullPose,sizeof(value.hullPose))||
        !copy(value.model,0x2c,&value.modelPose,sizeof(value.modelPose))) return false;
    const auto& shape=value.primitive;
    if ((shape.kind!=0&&shape.kind!=2)||!std::isfinite(shape.width)||shape.width<=0||
        (shape.kind==2&&(!std::isfinite(shape.height)||shape.height<shape.width))||
        !ss2vr::finite(value.hullPose)||!ss2vr::finite(value.modelPose)) return false;
    // This first placement adapter is for an upright hybrid avatar, not ragdoll
    // or vehicle/body rotation. No quaternion normalization changes native data.
    const auto q=value.hullPose.q;
    auto unit=[](ss2vr::Quat rotation) {
        const double norm=double(rotation.x)*rotation.x+double(rotation.y)*rotation.y+
                          double(rotation.z)*rotation.z+double(rotation.w)*rotation.w;
        return std::abs(norm-1)<=16*std::numeric_limits<float>::epsilon();
    };
    if (q.x!=0||q.z!=0||!unit(q)||!unit(value.modelPose.q)) return false;
    // Re-resolve every owner after copying. The enclosing native phase must
    // still prohibit deletion/worker mutation; readability is not a lease.
    if (resolve(playerHandle)!=value.player||resolve(value.mechanismHandle)!=value.mechanism||
        resolve(value.rootHandle)!=value.root||resolve(value.modelHandle)!=value.model) return false;
    uint32_t current=0;
    if (!word(value.player,0x114,current)||current!=value.mechanismHandle||
        !word(value.player,0x118,current)||current!=value.rootHandle||
        !word(value.player,0x120,current)||current!=value.modelHandle||
        !word(value.root,8,current)||current!=value.hull) return false;
    output=value;
    return true;
}
inline bool sameBodyGeometry(const BodyGeometry& a,const BodyGeometry& b) {
    return a.playerHandle==b.playerHandle&&a.player==b.player&&
        a.mechanismHandle==b.mechanismHandle&&a.mechanism==b.mechanism&&
        a.rootHandle==b.rootHandle&&a.root==b.root&&a.modelHandle==b.modelHandle&&a.model==b.model&&
        a.parts==b.parts&&a.hull==b.hull&&a.category==b.category&&
        std::memcmp(&a.primitive,&b.primitive,sizeof(a.primitive))==0&&
        std::memcmp(&a.modelPose,&b.modelPose,sizeof(a.modelPose))==0&&
        std::memcmp(&a.hullPose,&b.hullPose,sizeof(a.hullPose))==0;
}
} // namespace ss2vr::roomscale
