#include "roomscale_math_frame.hpp"
#include "native_finally.hpp"
#include "common/roomscale_fp.hpp"
#include "common/roomscale_triangle_query.hpp"

namespace ss2vr::game {
namespace {
thread_local bool *mathFailure=nullptr;
bool supportedMathMode() noexcept {
    // The game target uses x87, so __SSE__ need not be defined even though
    // native collision routines may use SSE. Inspect both control words.
    return roomscale::arithmeticSupported() &&
           roomscale::hardwareX87Control()==0x027f &&
           (roomscale::hardwareMxcsr() & ~uint32_t{0x3f})==0x1f80;
}
}
__attribute__((force_align_arg_pointer,noinline))
bool runRoomscaleMathFrame(bool &failed,DWORD thread,RoomscaleMathBody body,void *context) noexcept {
    if (mathFailure) { *mathFailure=true; failed=true; return false; }
    if (failed || !thread || thread!=GetCurrentThreadId() || !body ||
        !IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE)) {
        failed=true; return false;
    }
    roomscale::HardwareFpState original;
    bool saved=false,result=false;
    const bool completed=withNativeFinally([&] {
        mathFailure=&failed;
        roomscale::saveHardwareFp(original);
        saved=true;
        roomscale::installRoomscaleFp();
        if (!supportedMathMode()) { failed=true; return; }
        result=body(context);
        if (!supportedMathMode()) failed=true;
    },[&](bool aborted) noexcept {
        if (aborted) failed=true;
        mathFailure=nullptr;
        if (saved) roomscale::restoreHardwareFp(original);
    });
    return completed && result && !failed;
}
} // namespace ss2vr::game
