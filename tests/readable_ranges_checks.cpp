#include "common/readable_ranges.hpp"
#include <cassert>
using namespace ss2vr;
int main() {
    unsigned calls=0;
    auto query=[&](uintptr_t at,ReadableRange& out) {
        ++calls;
        out={at&~uintptr_t(0xfff),(at&~uintptr_t(0xfff))+0x1000,at<0x4000};return true;
    };
    ReadableRanges<> ranges;
    assert(ranges.contains(0x1100,4,query)&&calls==1);
    for(unsigned i=0;i<1000;++i)assert(ranges.contains(0x1100+i,4,query));
    assert(calls==1);
    assert(ranges.contains(0x1ff0,32,query)&&calls==2);
    ranges.seal();
    assert(ranges.contains(0x1ff0,32,query));
    assert(!ranges.contains(0x3000,1,query)&&calls==2);
    ranges.clear();
    assert(ranges.contains(0x3000,1,query)&&calls==3);
    assert(!ranges.contains(0x4000,1,query)&&calls==4);
    assert(!ranges.contains(0,1,query));
    assert(!ranges.contains(UINTPTR_MAX-2,4,query));
    ReadableRanges<1> bounded;
    assert(bounded.contains(0x1000,1,query));
    const auto before=calls;
    assert(!bounded.contains(0x2000,1,query)&&calls==before);
    ReadableRanges<> invalid;
    assert(!invalid.contains(0x1000,4,[](uintptr_t at,ReadableRange& r){r={at+1,at+4,true};return true;}));
    assert(!invalid.contains(0x1000,4,[](uintptr_t at,ReadableRange& r){r={at,at,true};return true;}));
    assert(!invalid.contains(0x1000,4,[](uintptr_t,ReadableRange&){return false;}));
    invalid.seal();assert(!invalid.contains(0x1000,4,query));
}
