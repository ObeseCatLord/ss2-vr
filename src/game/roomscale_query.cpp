#include "roomscale_query.hpp"
#include "native_finally.hpp"

namespace ss2vr::game {
static RoomscaleTriangleKernel originalTriangle=nullptr;
static thread_local roomscale::QueryScope* activeScope=nullptr;

bool queueRoomscaleTriangleHook(HMODULE core,RoomscaleQueueHook registrar) {
    if (!core||!registrar||originalTriangle) return false;
    return registrar(core,RoomscaleTriangleExport,reinterpret_cast<void*>(&ss2vrRoomscaleTriangleQuery),
                     reinterpret_cast<void**>(&originalTriangle)) && originalTriangle;
}
void resetRoomscaleTriangleHookAfterRemoval() noexcept { originalTriangle=nullptr; }

bool runRoomscaleModelQueryScope(roomscale::QueryScope& scope,DWORD recognizedSimulationThread,
                                RoomscaleQueryBody body,void* context) noexcept {
    if (activeScope) { activeScope->failed=true; scope.failed=true; return false; }
    if (!originalTriangle||!body||!roomscale::validScope(scope)||!recognizedSimulationThread||
        GetCurrentThreadId()!=recognizedSimulationThread||!roomscale::arithmeticSupported()) {
        scope.failed=true;
        return false;
    }
    // The scope and cleanup captures are above the native-finally frame. TLS is
    // changed only inside it. Native faults propagate; finally rejects the
    // scope/restores TLS on unwind. No borrowed native pointer or native cleanup.
    auto* previous=activeScope;
    const bool completed=withNativeFinally([&] {
        activeScope=&scope;
        body(context);
    },[&](bool aborted) noexcept {
        if (aborted) scope.failed=true;
        activeScope=previous;
    });
    return completed&&!scope.failed;
}

extern "C" __attribute__((force_align_arg_pointer,noinline))
float __cdecl ss2vrRoomscaleTriangleQuery(const roomscale::Ray& ray,const roomscale::Vector& a,
    const roomscale::Vector& b,const roomscale::Vector& c,const roomscale::Vector& normal,float radius) noexcept {
    // Initialization supplies the trampoline before queued enable. Removal must
    // quiesce all callers. No concurrent rebind or lazy native symbol lookup.
    if (!activeScope) return originalTriangle(ray,a,b,c,normal,radius);
    try {
        return roomscale::dispatch(activeScope,originalTriangle,ray,a,b,c,normal,radius);
    } catch (...) {
        activeScope->failed=true;
        return 0; // Contain GNU errors; native SEH remains the outer finally's job.
    }
}
} // namespace ss2vr::game
