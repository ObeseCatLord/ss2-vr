#pragma once
#include "math.hpp"
#include "roomscale_primitive_query.hpp"
#include <array>
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
struct BodyHullGeometry {
    uint32_t address=0,category=0,flags=0;
    ss2vr::Pose pose{},relativePose{};
    Primitive primitive{};
};
struct BodyGeometry {
    // The installed standing/crouching rig has one hull; swimming has two.
    // More shapes are rejected as a whole, never truncated or silently ignored.
    static constexpr unsigned MaximumHulls=2;
    uint32_t playerHandle=0,player=0,mechanismHandle=0,mechanism=0;
    uint32_t rootHandle=0,root=0,modelHandle=0,model=0,parts=0,rootFlags=0;
    ss2vr::Pose modelPose{},rootPose{};
    std::array<BodyHullGeometry,MaximumHulls> hulls{};
    unsigned hullCount=0;
};
inline bool bodyUnitQuaternion(ss2vr::Quat rotation) {
    const double norm=double(rotation.x)*rotation.x+double(rotation.y)*rotation.y+
                      double(rotation.z)*rotation.z+double(rotation.w)*rotation.w;
    return std::isfinite(norm)&&std::abs(norm-1)<=16*std::numeric_limits<float>::epsilon();
}

// Root placement and each hull's relative placement are copied independently
// from model/absolute-hull poses; they must not be inferred from each other.
// Parent-attached roots are outside this free-body movement adapter.
// A narrow complete-shape reader for one native hybrid body with up to two sphere or
// capsule hulls, including the installed swimming rig. It does not certify the
// caller's worker/lifetime ownership. Capturing a rotated hull does not authorize
// using an upright-only cover: the eventual query must cover EVERY captured hull
// under its actual transform or reject the complete movement request.
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
    uint32_t count=0,root=0,modelHandle=0,vtable=0,owner=0,hullAddress=0,parent=0;
    if (!word(value.mechanism,8,count)||count!=1||!word(value.mechanism,4,value.parts)||
        !word(value.parts,8,root)||root!=value.root||!root||
        !word(value.mechanism,0x38,modelHandle)||modelHandle!=value.modelHandle||
        !word(value.root,0,vtable)||vtable!=layout.hybridBodyVtable||
        !word(value.root,0x48,owner)||owner!=value.player||
        !word(value.root,4,parent)||parent||
        !word(value.root,0x4c,value.rootFlags)||!(value.rootFlags&1)||
        !copy(value.root,0x2c,&value.rootPose,sizeof(value.rootPose))||
        !ss2vr::finite(value.rootPose)||!bodyUnitQuaternion(value.rootPose.q)||
        !word(value.root,8,hullAddress)||!hullAddress||
        !copy(value.model,0x2c,&value.modelPose,sizeof(value.modelPose))||
        !ss2vr::finite(value.modelPose)||!bodyUnitQuaternion(value.modelPose.q)) return false;
    while (hullAddress) {
        if (value.hullCount==BodyGeometry::MaximumHulls) return false;
        for (unsigned i=0;i<value.hullCount;++i)
            if (value.hulls[i].address==hullAddress) return false;
        auto& hull=value.hulls[value.hullCount];
        hull.address=hullAddress;
        uint32_t parent=0,children=0,sibling=0,mechanism=0;
        if (!word(hullAddress,0,vtable)||vtable!=layout.primitiveHullVtable||
            !word(hullAddress,4,parent)||parent!=value.root||
            !word(hullAddress,8,children)||children||!word(hullAddress,0xc,sibling)||
            !word(hullAddress,0x48,owner)||owner!=value.player||
            !word(hullAddress,0x70,mechanism)||mechanism!=value.mechanism||
            !word(hullAddress,0x54,hull.category)||
            !word(hullAddress,0x4c,hull.flags)||!(hull.flags&1)||
            !copy(hullAddress,0x78,&hull.primitive,sizeof(hull.primitive))||
            !copy(hullAddress,0x2c,&hull.pose,sizeof(hull.pose))||
            !copy(hullAddress,0x10,&hull.relativePose,sizeof(hull.relativePose))) return false;
        const auto& shape=hull.primitive;
        if ((shape.kind!=0&&shape.kind!=2)||!std::isfinite(shape.width)||shape.width<=0||
            (shape.kind==2&&(!std::isfinite(shape.height)||shape.height<shape.width))||
            !ss2vr::finite(hull.pose)||!bodyUnitQuaternion(hull.pose.q)||
            !ss2vr::finite(hull.relativePose)||!bodyUnitQuaternion(hull.relativePose.q)) return false;
        ++value.hullCount;
        hullAddress=sibling;
    }
    // Re-resolve every owner after copying. The enclosing native phase must
    // still prohibit deletion/worker mutation; readability is not a lease.
    if (resolve(playerHandle)!=value.player||resolve(value.mechanismHandle)!=value.mechanism||
        resolve(value.rootHandle)!=value.root||resolve(value.modelHandle)!=value.model) return false;
    uint32_t current=0;
    if (!word(value.player,0x114,current)||current!=value.mechanismHandle||
        !word(value.player,0x118,current)||current!=value.rootHandle||
        !word(value.player,0x120,current)||current!=value.modelHandle||
        !word(value.root,8,current)||current!=value.hulls[0].address) return false;
    for (unsigned i=0;i<value.hullCount;++i) {
        const uint32_t next=i+1<value.hullCount?value.hulls[i+1].address:0;
        if (!word(value.hulls[i].address,0xc,current)||current!=next) return false;
    }
    output=value;
    return true;
}
inline bool sameBodyGeometry(const BodyGeometry& a,const BodyGeometry& b) {
    if (a.hullCount>BodyGeometry::MaximumHulls || b.hullCount!=a.hullCount ||
        a.playerHandle!=b.playerHandle||a.player!=b.player||
        a.mechanismHandle!=b.mechanismHandle||a.mechanism!=b.mechanism||
        a.rootHandle!=b.rootHandle||a.root!=b.root||a.modelHandle!=b.modelHandle||a.model!=b.model||
        a.parts!=b.parts||a.rootFlags!=b.rootFlags||std::memcmp(&a.modelPose,&b.modelPose,sizeof(a.modelPose))||
        std::memcmp(&a.rootPose,&b.rootPose,sizeof(a.rootPose))) return false;
    for (unsigned i=0;i<a.hullCount;++i) {
        const auto& x=a.hulls[i];const auto& y=b.hulls[i];
        if (x.address!=y.address||x.category!=y.category||x.flags!=y.flags||
            std::memcmp(&x.primitive,&y.primitive,sizeof(x.primitive))||
            std::memcmp(&x.pose,&y.pose,sizeof(x.pose))||
            std::memcmp(&x.relativePose,&y.relativePose,sizeof(x.relativePose))) return false;
    }
    return true;
}
} // namespace ss2vr::roomscale
