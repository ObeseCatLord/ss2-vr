// Compile-only ABI fixture. Never execute Windows/native code.
#include "game/roomscale_query.hpp"
using namespace ss2vr;
using namespace ss2vr::game;
extern "C" __attribute__((noinline)) roomscale::PrimitiveInterval* __cdecl primitiveAbiCall(
    RoomscalePrimitiveKernel kernel,roomscale::PrimitiveInterval* output,
    const roomscale::Ray& ray,const roomscale::Primitive& shape,float radius) {
    return kernel(output,ray,shape,radius);
}
extern "C" __attribute__((noinline)) roomscale::PrimitiveInterval* __cdecl primitiveAbiEntry(
    roomscale::PrimitiveInterval* output,const roomscale::Ray& ray,
    const roomscale::Primitive& shape,float radius) {
    return ss2vrRoomscalePrimitiveQuery(output,ray,shape,radius);
}
