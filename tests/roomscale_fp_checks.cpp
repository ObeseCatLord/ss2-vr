#include "common/roomscale_fp.hpp"
#include "common/roomscale_triangle_query.hpp"
#include <cassert>
#include <cfenv>
#include <stdexcept>
using namespace ss2vr::roomscale;
int main() {
    HardwareFpState original;
    saveHardwareFp(original);
    const auto initialControl=hardwareX87Control();
    const auto initialMxcsr=hardwareMxcsr();
    // Simulate D3D-style binary32 precision, non-nearest rounding and FTZ/DAZ.
    const uint16_t hostile=0x047f;
    const uint32_t hostileMx=0xbfc0;
    __asm__ volatile("fninit; fldcw %0; ldmxcsr %1"::"m"(hostile),"m"(hostileMx):"memory");
    assert(!arithmeticSupported());
    HardwareFpState caller;
    const double sentinel=1.25;
    __asm__ volatile("fldl %0"::"m"(sentinel):"st");
    saveHardwareFp(caller);
    installRoomscaleFp();
    assert(arithmeticSupported());
    assert(hardwareX87Control()==0x027f && hardwareMxcsr()==0x1f80);
    // Exercise the production interval arithmetic in the installed mode.
    const auto enclosure=detail::plus(detail::exact(1.),detail::exact(1e-12));
    assert(enclosure.lo<=1.L+1e-12L && enclosure.hi>=1.L+1e-12L);
    restoreHardwareFp(caller);
    double returned=0;
    __asm__ volatile("fstpl %0":"=m"(returned)::"st");
    assert(returned==sentinel && hardwareX87Control()==hostile && hardwareMxcsr()==hostileMx);
    // Ordinary C++ exceptional control flow exercises the SAME restore helper.
    // It does not execute or emulate the game's native SEH boundary.
    saveHardwareFp(caller);
    bool caught=false;
    try {
        struct Restore { const HardwareFpState &s; ~Restore() { restoreHardwareFp(s); } } guard{caller};
        installRoomscaleFp();
        assert(arithmeticSupported());
        throw std::runtime_error("fixture");
    } catch (const std::runtime_error &) { caught=true; }
    assert(caught && hardwareX87Control()==hostile && hardwareMxcsr()==hostileMx);
    restoreHardwareFp(original);
    assert(hardwareX87Control()==initialControl && hardwareMxcsr()==initialMxcsr);
}
