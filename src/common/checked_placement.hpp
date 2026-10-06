#pragma once
namespace ss2vr {
// Progress of one additional checked-placement request, not native body state.
// A started commit is never classified as a safe-to-retry rejection.
struct CheckedPlacementState {
    bool failed=false, rootSeen=false, commitStarted=false, commitReturned=false;
    bool beforePart(bool identity) noexcept {
        if (commitStarted) return true; // Native post-commit callbacks retain native policy.
        if (failed || !identity) { failed=true; return false; }
        return true;
    }
    bool claimRoot(bool identity) noexcept {
        if (failed || commitStarted || rootSeen || !identity) { failed=true; return false; }
        rootSeen=true;
        return true;
    }
    bool beginCommit(bool checked) noexcept {
        if (failed || !rootSeen || commitStarted || !checked) { failed=true; return false; }
        commitStarted=true;
        return true;
    }
    bool afterPart() noexcept {
        if (commitStarted) return true;
        failed=true; // The expected root setter was skipped or declined.
        return false;
    }
    bool completed(int nativeResult) const noexcept {
        return !failed && rootSeen && commitStarted && commitReturned && nativeResult==1;
    }
};
} // namespace ss2vr
