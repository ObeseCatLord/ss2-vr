#pragma once
#include <array>
#include <cstdint>

namespace ss2vr {
struct NativeRayCleanupNode {
    uint32_t link=0,vtable=0,callback=0,next=0,previous=0;
};
struct NativeRayCleanupList {
    uint32_t head=0,first=0,sentinelNext=0,last=0;
    std::array<NativeRayCleanupNode,2> nodes{};
};
// Evidence for the two inspected scalar-only ray cleanup callbacks. This is a
// copied shape check, not a lock, a lease, or permission to unlink native nodes.
// Each link is object+4, so the expected object vtable is kept separately.
inline bool knownNativeRayCleanup(const NativeRayCleanupList& list,
                                  const std::array<uint32_t,2>& expectedLinks,
                                  const std::array<uint32_t,2>& expectedVtables,
                                  const std::array<uint32_t,2>& expectedCallbacks) noexcept {
    if(!list.head||list.head>UINT32_MAX-8||list.sentinelNext||
       !expectedLinks[0]||!expectedLinks[1]||expectedLinks[0]==expectedLinks[1]||
       !expectedVtables[0]||!expectedVtables[1]||!expectedCallbacks[0]||!expectedCallbacks[1])return false;
    const uint32_t sentinel=list.head+4;
    for(unsigned i=0;i<2;++i) {
        if(expectedLinks[i]==list.head||expectedLinks[i]==sentinel||
           list.nodes[i].link!=expectedLinks[i]||list.nodes[i].vtable!=expectedVtables[i]||
           list.nodes[i].callback!=expectedCallbacks[i])return false;
    }
    uint32_t current=list.first,previous=list.head;
    unsigned used=0;
    while(current!=sentinel) {
        unsigned index=2;
        for(unsigned i=0;i<2;++i)if(current==expectedLinks[i])index=i;
        if(index==2||(used&(1u<<index)))return false;
        const auto& node=list.nodes[index];
        if(node.previous!=previous||!node.next)return false;
        used|=1u<<index;previous=current;current=node.next;
    }
    if(list.last!=previous)return false;
    // An unlisted node must be genuinely detached, not owned by another list.
    for(unsigned i=0;i<2;++i)if(!(used&(1u<<i)) &&
        (list.nodes[i].next||list.nodes[i].previous))return false;
    return true;
}
} // namespace ss2vr
