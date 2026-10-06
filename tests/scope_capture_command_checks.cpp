#include "common/scope_capture_command.hpp"
#include <cassert>
using namespace ss2vr;
int main() {
    ScopeCaptureOnce once;
    assert(!once.complete(true,true));
    assert(once.enter() && !once.complete(false,true) && !once.complete(true,false));
    assert(once.complete(true,true));
    assert(!once.enter() && once.rejected);
    assert(!once.complete(true,true)); // Outer successful copy cannot revive reentry.
    assert(!once.enter() && !once.complete(true,true));
    once = {};
    assert(once.enter() && once.complete(true,true)); // Separate retired native invocation.

    const ScopeCommandAppend good{1,1,1,2,2,3,3,4,5};
    assert(scopeCommandAppendConfirmed(good));
    for (unsigned field=0;field<9;++field) {
        auto bad=good;
        switch(field) {
        case 0: bad.root=0; break;
        case 1: bad.currentRoot=4; break;
        case 2: bad.parent=4; break;
        case 3: bad.beforeArray=0; break;
        case 4: bad.afterArray=4; break;
        case 5: bad.command=0; break;
        case 6: bad.last=4; break;
        case 7: bad.beforeCount=-1; break;
        case 8: bad.afterCount=6; break;
        }
        assert(!scopeCommandAppendConfirmed(bad));
    }
    auto empty=good; empty.beforeCount=0; empty.afterCount=1;
    assert(scopeCommandAppendConfirmed(empty));
    auto full=good; full.beforeCount=ScopeCaptureMaxCommands-1; full.afterCount=ScopeCaptureMaxCommands;
    assert(scopeCommandAppendConfirmed(full));
    ++full.beforeCount; ++full.afterCount; assert(!scopeCommandAppendConfirmed(full));
    // Count equality cannot establish membership after root/array replacement.
    auto replaced=good; replaced.currentRoot=replaced.parent=4;
    assert(!scopeCommandAppendConfirmed(replaced));
    // Both native float tie and key are deterministic, with no eye/hand sorting
    // inference. The chosen rank is strictly between visibility and bloom.
    static_assert((ScopeCaptureRank & 0xffffff) == ScopeCaptureRank);
    static_assert(ScopeCaptureRank > 0x90000 && ScopeCaptureRank > 0xa0000 &&
                  ScopeCaptureRank < 0xb0000 && ScopeCaptureRank < 0xc0000 && ScopeCaptureRank < 0xd0000);
}
