#pragma once
#include <type_traits>
#if !defined(__MINGW32__) || !defined(__GNUC__) || defined(__clang__) || !defined(__i386__)
#error This containment wrapper requires the supported GNU MinGW x86 exception ABI
#endif
namespace ss2vr::game {
using NativeBody = void(__cdecl *)(void *);
using NativeCleanup = void(__cdecl *)(void *, int);
extern "C" void __cdecl ss2vrNativeFinally(NativeBody, NativeCleanup, void *);

// Context is ABOVE the native finally frame, but native unwinding may skip this
// caller's destructors too. Captures must be non-owning; explicitly owned state
// must cover every inner resource, lock and TLS context before native entry.
// Cleanup must be idempotent, bounded, allocation-free and must not call native
// game callbacks on abort. Do not refer to unwound inner stack storage.
// Direct GNU exceptions stay inside run; nothing is rethrown across a native
// boundary. Every reentered mod callback must separately contain its GNU errors
// before traversing intervening native frames. Native SEH is not caught/converted.
// A finally runs only when unwinding crosses it, not on every first-chance event.
template<class Body, class Cleanup>
bool withNativeFinally(Body &&body, Cleanup &&cleanup) noexcept {
    static_assert(std::is_nothrow_invocable_v<Cleanup &, bool>);
    static_assert(std::is_trivially_destructible_v<std::remove_reference_t<Body>>);
    static_assert(std::is_trivially_destructible_v<std::remove_reference_t<Cleanup>>);
    struct Context {
        Body &body;
        Cleanup &cleanup;
        bool failed = false;
        // MSVC C and its unwind funclet promise only four-byte incoming stack
        // alignment. Realign these two GNU callbacks locally, including SSE.
        static __attribute__((force_align_arg_pointer)) void __cdecl run(void *opaque) noexcept {
            auto &self = *static_cast<Context *>(opaque);
            try { self.body(); }
            catch (...) { self.failed = true; }
        }
        static __attribute__((force_align_arg_pointer)) void __cdecl finish(void *opaque, int abnormal) noexcept {
            auto &self = *static_cast<Context *>(opaque);
            self.cleanup(abnormal != 0 || self.failed);
        }
    } context{body, cleanup};
    ss2vrNativeFinally(Context::run, Context::finish, &context);
    return !context.failed;
}
} // namespace ss2vr::game
