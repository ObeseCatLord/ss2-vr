// Compile/disassemble ONLY. Never execute or link this fixture into the mod.
#include "game/roomscale_query.hpp"
#include <type_traits>
using namespace ss2vr;
using namespace ss2vr::game;
static_assert(sizeof(void*)==4&&sizeof(float)==4);
static_assert(std::is_convertible_v<decltype(&ss2vrRoomscaleTriangleQuery),RoomscaleTriangleKernel>);

extern "C" __attribute__((noinline)) float __cdecl roomscaleAbiCall(
    RoomscaleTriangleKernel kernel,const roomscale::Ray& ray,const roomscale::Vector& a,
    const roomscale::Vector& b,const roomscale::Vector& c,const roomscale::Vector& normal,float radius) {
    // Volatile forces an x87 result spill: a wrong integer/SSE result ABI would
    // be visible in the object. The six native argument slots must be 24 bytes.
    volatile float result=kernel(ray,a,b,c,normal,radius);
    return result;
}
extern "C" __attribute__((noinline)) float __cdecl roomscaleAbiEntry(
    const roomscale::Ray& ray,const roomscale::Vector& a,const roomscale::Vector& b,
    const roomscale::Vector& c,const roomscale::Vector& normal,float radius) {
    volatile float result=ss2vrRoomscaleTriangleQuery(ray,a,b,c,normal,radius);
    return result;
}
