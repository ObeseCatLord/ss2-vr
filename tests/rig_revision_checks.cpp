#include "common/rig_revision.hpp"
#include <cassert>
using namespace ss2vr;
int main() {
    RigRevision revision;
    assert(revision.usable(0));
    const auto first=revision.begin(0);assert(first==1);
    assert(!revision.usable(0)&&!revision.usable(1));
    assert(!revision.begin(0)&&!revision.begin(1));
    assert(!revision.finish(0)&&!revision.finish(3));
    assert(revision.finish(first)&&revision.usable(2));
    assert(!revision.usable(0)&&!revision.finish(first));
    assert(!revision.begin(0));
    const auto second=revision.begin(2);assert(second==3);
    // Leaving a transition unfinished keeps every presentation copy ineligible.
    assert(!revision.usable(2)&&!revision.usable(3));
    assert(revision.recoverForNewOrigin()==4&&revision.usable(4));
    assert(!revision.finish(second));
    assert(revision.recoverForNewOrigin()==4);
    RigRevision exhausted(UINT64_MAX-1);
    assert(!exhausted.begin(UINT64_MAX-1)&&exhausted.usable(UINT64_MAX-1));
    RigRevision unavailable(UINT64_MAX);
    assert(unavailable.recoverForNewOrigin()==UINT64_MAX&&!unavailable.usable(UINT64_MAX));
}
