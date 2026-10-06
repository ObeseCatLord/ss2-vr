#include "common/checked_placement.hpp"
#include <cassert>
using ss2vr::CheckedPlacementState;
int main() {
    CheckedPlacementState s;
    assert(!s.completed(1) && !s.commitStarted);
    assert(s.beforePart(true) && s.claimRoot(true));
    assert(s.beginCommit(true) && s.commitStarted && !s.completed(1));
    s.commitReturned=true;
    assert(s.afterPart() && s.completed(1) && !s.completed(0));
    // Post-commit native callbacks cannot be turned into a synthetic rollback.
    assert(s.beforePart(false) && s.afterPart());
    s.failed=true;
    assert(!s.completed(1) && s.commitStarted);
    for (unsigned stop=0;stop<5;++stop) {
        CheckedPlacementState rejected;
        if(stop==0) rejected.failed=true;
        if(stop==1) assert(!rejected.beforePart(false));
        if(stop==2) assert(!rejected.claimRoot(false));
        if(stop==3) { assert(rejected.claimRoot(true)); assert(!rejected.beginCommit(false)); }
        if(stop==4) { assert(rejected.claimRoot(true)); assert(!rejected.claimRoot(true)); }
        assert(!rejected.afterPart() && rejected.failed && !rejected.commitStarted);
        assert(!rejected.beginCommit(true) && !rejected.completed(1));
    }
    CheckedPlacementState skipped;
    assert(skipped.beforePart(true));
    assert(!skipped.afterPart() && skipped.failed && !skipped.commitStarted);
    CheckedPlacementState missing;
    assert(!missing.beginCommit(true) && !missing.commitStarted);
    CheckedPlacementState independent;
    assert(independent.beforePart(true) && independent.claimRoot(true) && independent.beginCommit(true));
    // Once the first native write was entered, an abort is not retryable even
    // if the outer checked setter never returns or reports failure.
    independent.failed=true;
    assert(independent.commitStarted && !independent.completed(0));
}
