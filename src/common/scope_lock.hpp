#pragma once
namespace ss2vr {
// One explicit foreign acquisition obligation; not a resource recovery policy.
enum class ScopeLockPhase { Clear, Acquiring, Acquired, UnlockAttempted, Uncertain };
struct ScopeLockLedger {
    ScopeLockPhase phase = ScopeLockPhase::Clear;
    bool beginLock() {
        if (phase != ScopeLockPhase::Clear) return false;
        phase = ScopeLockPhase::Acquiring; return true;
    }
    void finishLock(bool success) { phase = success ? ScopeLockPhase::Acquired : ScopeLockPhase::Clear; }
    bool beginUnlock() {
        if (phase != ScopeLockPhase::Acquired) return false;
        phase = ScopeLockPhase::UnlockAttempted; return true;
    }
    void finishUnlock(bool success) { phase = success ? ScopeLockPhase::Clear : ScopeLockPhase::Uncertain; }
    bool outstanding() const { return phase != ScopeLockPhase::Clear; }
    bool uncertain() const { return phase == ScopeLockPhase::Acquiring ||
        phase == ScopeLockPhase::UnlockAttempted || phase == ScopeLockPhase::Uncertain; }
};
// Mandatory draw-hook coverage is checked before invoking any acquisition.
// Callers use the same disposition again after copying, before forwarding.
template<class Probe,class Forward>
auto withScopeDrawCoverage(bool complete,Probe probe,Forward forward) {
    return complete ? probe() : forward();
}
} // namespace ss2vr
