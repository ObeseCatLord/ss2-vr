// Compile-only native-boundary fixture. Never executed as a Windows binary.
#include "game/native_finally.hpp"
struct FinallyWitness { int body = 0, aborted = 0, returned = 0; };
extern "C" void nativeFinallyCaller(FinallyWitness *value) {
    value->returned = ss2vr::game::withNativeFinally([&] { ++value->body; },
        [&](bool aborted) noexcept { value->aborted = aborted ? 1 : 0; });
}
extern "C" void nativeFinallyThrower(FinallyWitness *value) {
    value->returned = ss2vr::game::withNativeFinally([&] { throw 7; },
        [&](bool aborted) noexcept { value->aborted = aborted ? 1 : 0; });
}
// An unresolved sink forces the actual callback ABI to provide aligned storage;
// this fixture is only compiled, so no replacement executable or runtime mock.
extern "C" void consumeAligned(const float *) noexcept;
extern "C" void nativeFinallyAligned(FinallyWitness *value) {
    value->returned = ss2vr::game::withNativeFinally([&] {
        alignas(16) float work[4]{1,2,3,4}; consumeAligned(work);
    }, [&](bool aborted) noexcept {
        alignas(16) float work[4]{5,6,7,8}; consumeAligned(work);
        value->aborted = aborted ? 1 : 0;
    });
}
