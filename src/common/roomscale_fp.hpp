#pragma once
#include <array>
#include <cstdint>
#include <type_traits>
#if !defined(__GNUC__) || (!defined(__i386__) && !defined(__x86_64__))
#error Roomscale floating-point containment requires the inspected GNU x86 target
#endif
namespace ss2vr::roomscale {
struct alignas(16) HardwareFpState { std::array<unsigned char,512> bytes; };
static_assert(sizeof(HardwareFpState)==512 && alignof(HardwareFpState)==16 &&
              std::is_trivially_destructible_v<HardwareFpState>);
#define SS2VR_FP_NOINLINE __attribute__((noinline,target("sse2"),no_sanitize_address,no_sanitize_undefined))
#define SS2VR_X87_CLOBBERS "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
#if defined(__i386__)
#define SS2VR_XMM_CLOBBERS "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
#else
#define SS2VR_XMM_CLOBBERS "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", \
    "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
#endif
// Separate integer-only call boundaries prevent compiler FP temporaries from
// spanning restoration of an earlier register image. Native SEH cleanup is the
// game adapter's responsibility; these helpers do not provide a scope alone.
SS2VR_FP_NOINLINE inline void saveHardwareFp(HardwareFpState &state) noexcept {
    __asm__ volatile("fxsave %0" : "=m"(state.bytes) : : "memory");
}
SS2VR_FP_NOINLINE inline void installRoomscaleFp() noexcept {
    const uint16_t control=0x027f; // nearest, binary64 precision, masked x87 exceptions
    const uint32_t mxcsr=0x1f80;   // nearest, gradual underflow, masked SSE exceptions
    __asm__ volatile("fninit\n\tfldcw %0\n\tldmxcsr %1" : : "m"(control),"m"(mxcsr)
                     : "memory", SS2VR_X87_CLOBBERS);
}
SS2VR_FP_NOINLINE inline void restoreHardwareFp(const HardwareFpState &state) noexcept {
    __asm__ volatile("fxrstor %0" : : "m"(state.bytes)
                     : "memory", SS2VR_X87_CLOBBERS, SS2VR_XMM_CLOBBERS);
}
inline uint16_t hardwareX87Control() noexcept {
    uint16_t result;__asm__ volatile("fnstcw %0":"=m"(result));return result;
}
inline uint32_t hardwareMxcsr() noexcept {
    uint32_t result;__asm__ volatile("stmxcsr %0":"=m"(result));return result;
}
#undef SS2VR_FP_NOINLINE
#undef SS2VR_X87_CLOBBERS
#undef SS2VR_XMM_CLOBBERS
} // namespace ss2vr::roomscale
