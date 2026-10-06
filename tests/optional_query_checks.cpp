#ifdef NDEBUG
#error Optional query checks require assertions
#endif
#include "common/optional_query.hpp"
#include <cassert>
#include <initializer_list>
using namespace ss2vr;
int main() {
    using D=OptionalQueryDecision;
    for (unsigned kind=0;kind<4;++kind)
        for (bool pending:{false,true})
            assert(optionalQueryDecision(nullptr,pending,kind)==D::original);
    bool failed=false;
    assert(optionalQueryDecision(&failed,false,0)==D::clearBranch && !failed);
    assert(optionalQueryDecision(&failed,false,1)==D::original && !failed);
    assert(optionalQueryDecision(&failed,true,1)==D::original && !failed);
    assert(optionalQueryDecision(&failed,true,0)==D::cancel && failed);
    // Resource recovery, later material/recursive returns, and earlier hits
    // cannot make a partially cancelled query usable again.
    for (unsigned kind=0;kind<4;++kind)
        for (bool pending:{false,true})
            assert(optionalQueryDecision(&failed,pending,kind)==D::cancel && failed);
    bool other=false;
    assert(optionalQueryDecision(&other,false,0)==D::clearBranch && !other);
    assert(optionalQueryDecision(&other,false,2)==D::cancel && other);
    for(bool supported:{false,true})
        assert(optionalQueryTargetDecision(nullptr,supported)==D::original);
    bool query=false;
    assert(optionalQueryTargetDecision(&query,true)==D::original && !query);
    assert(optionalQueryTargetDecision(&query,false)==D::cancel && query);
    assert(optionalQueryTargetDecision(&query,true)==D::cancel && query);

    assert(optionalQueryBufferReadable(0x1000,120,0,0,120,0));
    assert(optionalQueryBufferReadable(0x1000,120,0,0x40,12,108));
    assert(optionalQueryBufferReadable(0x1000,120,0,0,0,0));
    assert(!optionalQueryBufferReadable(0x1000,120,0,0,0,1));
    assert(!optionalQueryBufferReadable(0x1000,120,1,0,12,0));
    assert(!optionalQueryBufferReadable(0x1000,120,-1,0,12,0));
    assert(!optionalQueryBufferReadable(0x1000,120,0,0x1f,12,0));
    assert(!optionalQueryBufferReadable(0x1000,120,0,0xdf,12,0));
    assert(!optionalQueryBufferReadable(0x1000,120,0,0,12,109));
    assert(!optionalQueryBufferReadable(0x1000,120,0,0,12,-1));
    assert(!optionalQueryBufferReadable(0,120,0,0,12,0));
    assert(!optionalQueryBufferReadable(0xfffffff0u,120,0,0,12,0));
    assert(optionalQueryBufferReadable(0xfffffff0u,16,0,0,16,0));
    assert(!optionalQueryBufferReadable(0x1000,-1,0,0,12,0));
}
