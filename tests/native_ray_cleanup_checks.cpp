#include "common/native_ray_cleanup.hpp"
#include <cassert>
using namespace ss2vr;
int main() {
    constexpr std::array<uint32_t,2> links{0x2004,0x3004},tables{0x4000,0x5000},callbacks{0x6000,0x7000};
    NativeRayCleanupList empty{0x1000,0x1004,0,0x1000,{{{links[0],tables[0],callbacks[0],0,0},{links[1],tables[1],callbacks[1],0,0}}}};
    auto good=[&](const auto& l){return knownNativeRayCleanup(l,links,tables,callbacks);};
    assert(good(empty));
    auto one=empty;one.first=links[0];one.last=links[0];one.nodes[0].next=0x1004;one.nodes[0].previous=0x1000;
    assert(good(one));
    auto two=one;two.nodes[0].next=links[1];two.nodes[1].next=0x1004;two.nodes[1].previous=links[0];two.last=links[1];
    assert(good(two));
    auto reversed=two;reversed.first=links[1];reversed.last=links[0];
    reversed.nodes[1].previous=0x1000;reversed.nodes[1].next=links[0];
    reversed.nodes[0].previous=links[1];reversed.nodes[0].next=0x1004;assert(good(reversed));
    auto bad=two;bad.nodes[1].next=links[0];assert(!good(bad));
    bad=two;bad.nodes[0].previous=links[1];assert(!good(bad));
    bad=two;bad.nodes[1].vtable=0x6000;assert(!good(bad));
    bad=two;bad.nodes[0].callback=0x8000;assert(!good(bad));
    bad=two;bad.first=0x7004;assert(!good(bad));
    bad=two;bad.nodes[0].next=0;assert(!good(bad));
    bad=two;bad.last=links[0];assert(!good(bad));
    bad=two;bad.sentinelNext=links[0];assert(!good(bad));
    bad=empty;bad.nodes[0].previous=0x8004;assert(!good(bad));
    bad=empty;bad.head=UINT32_MAX-3;assert(!good(bad));
    assert(!knownNativeRayCleanup(empty,{links[0],links[0]},tables,callbacks));
    assert(!knownNativeRayCleanup(empty,links,{0,tables[1]},callbacks));
}
