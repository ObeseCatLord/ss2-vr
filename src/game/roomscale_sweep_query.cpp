#include "roomscale_sweep_query.hpp"
#include "roomscale_query.hpp"
#include "roomscale_resource_gate.hpp"
#include "common/winproc.hpp"

namespace ss2vr::game {
namespace {
struct NativeQueries {
    void(__cdecl *init)()=nullptr;
    void(__cdecl *setRay)(const roomscale::Ray&)=nullptr;
    void(__cdecl *maximum)(float)=nullptr;
    void(__cdecl *minimum)(float)=nullptr;
    void(__cdecl *radius)(float)=nullptr;
    void(__cdecl *category)(uint32_t)=nullptr;
    void(__cdecl *thickCategory)(uint32_t)=nullptr;
    void(__cdecl *avatar)(void*)=nullptr;
    void(__cdecl *mechanism)(void*)=nullptr;
    void(__cdecl *fluids)(int)=nullptr;
    int(__cdecl *check)()=nullptr;
    int(__cdecl *hit)()=nullptr;
    bool ready() const noexcept {
        return init&&setRay&&maximum&&minimum&&radius&&category&&thickCategory&&avatar&&mechanism&&fluids&&check&&hit;
    }
} native;
struct Query {
    const roomscale::BodyGeometry &body;
    const roomscale::BodySweepCover &cover;
    RoomscaleQueryOwnerCurrent current;
    void *context;
    roomscale::QueryScope &scope;
    unsigned hull,index;
    bool clear=false;
    static void __cdecl run(void *opaque) noexcept {
        auto& q=*static_cast<Query*>(opaque);
        if(!roomscaleResourceScopeUsable()||!q.current(q.context)) {q.scope.failed=true;return;}
        const auto& sphere=q.cover.body.hulls[q.hull].sphere[q.index];
        native.init();
        // init has only the two validated native scalar cleanup callbacks and
        // native unlink. Recheck ownership before touching the fresh ray state.
        if(!roomscaleResourceScopeUsable()||!q.current(q.context)) {q.scope.failed=true;return;}
        const roomscale::Ray ray{sphere.centre,q.cover.direction};
        native.setRay(ray);native.maximum(q.cover.maximumParameter);native.minimum(0);
        native.radius(sphere.radius);
        native.category(q.body.hulls[q.hull].category);
        native.thickCategory(q.body.hulls[q.hull].category);
        native.avatar(reinterpret_cast<void*>(q.body.player));
        native.mechanism(reinterpret_cast<void*>(q.body.mechanism));
        native.fluids(0);
        native.check();
        const bool clear=native.hit()==0;
        // Failure does not permit unknown cleanup callbacks. The native normal
        // traversal already retired its visited hulls; don't fabricate repair
        // if the owner or inspected list became invalid during that traversal.
        if(!q.current(q.context)) {q.scope.failed=true;return;}
        native.init();
        q.clear=clear&&!q.scope.failed&&roomscaleResourceScopeUsable()&&q.current(q.context);
    }
};
}
bool configureRoomscaleSweepQueries(HMODULE engine) noexcept {
    if(!engine||native.ready())return false;
    NativeQueries next;
#define BIND(member, name) next.member=loadProc<decltype(next.member)>(engine,name)
    BIND(init,"?rayInit@SeriousEngine@@YAXXZ");
    BIND(setRay,"?raySetRay@SeriousEngine@@YAXABVRay3f@1@@Z");
    BIND(maximum,"?raySetMaxDistance@SeriousEngine@@YAXM@Z");
    BIND(minimum,"?raySetMinDistance@SeriousEngine@@YAXM@Z");
    BIND(radius,"?raySetRayRadius@SeriousEngine@@YAXM@Z");
    BIND(category,"?cldSetRayCategory@SeriousEngine@@YAXVIDENT@1@@Z");
    BIND(thickCategory,"?cldSetThickRayCategory@SeriousEngine@@YAXVIDENT@1@@Z");
    BIND(avatar,"?cldSetAvatar@SeriousEngine@@YAXPAVCEntity@1@@Z");
    BIND(mechanism,"?cldSetAvatarMechanism@SeriousEngine@@YAXPAVCMechanism@1@@Z");
    BIND(fluids,"?cldSetRayTestsFluids@SeriousEngine@@YAXH@Z");
    BIND(check,"?cldCheckRay@SeriousEngine@@YAHXZ");
    BIND(hit,"?rayIsHit@SeriousEngine@@YAHXZ");
#undef BIND
    if(!next.ready())return false;
    native=next;return true;
}
void resetRoomscaleSweepQueriesAfterQuiescence() noexcept {native={};}
bool runRoomscaleSweepQueries(const roomscale::BodyGeometry& body,const roomscale::BodySweepCover& cover,
    float contactDepthBudget,DWORD thread,RoomscaleQueryOwnerCurrent current,void *context,bool& failed,bool rejectInitialContact) noexcept {
    if(!roomscale::arithmeticSupported()) {failed=true;return false;}
    const double directionNorm=double(cover.direction.x)*cover.direction.x+
        double(cover.direction.y)*cover.direction.y+double(cover.direction.z)*cover.direction.z;
    if(failed||!native.ready()||!roomscaleCollisionKernelsUsable()||!current||!thread||thread!=GetCurrentThreadId()||
       !roomscaleResourceScopeUsable()||
       !cover.valid||!cover.body.valid||!body.player||!body.mechanism||
       !roomscale::detail::finite(roomscale::detail::convert(cover.direction))||
       !std::isfinite(directionNorm)||
       std::abs(directionNorm-1)>32*std::numeric_limits<float>::epsilon()||
       !body.hullCount||body.hullCount>roomscale::BodyGeometry::MaximumHulls||
       cover.body.hullCount!=body.hullCount||!std::isfinite(cover.maximumParameter)||cover.maximumParameter<=0||
       !std::isfinite(contactDepthBudget)||contactDepthBudget<0) {failed=true;return false;}
    unsigned count=0;
    for(unsigned hull=0;hull<body.hullCount;++hull) {
        const auto& spheres=cover.body.hulls[hull];
        if(!spheres.valid||!spheres.count||spheres.count>spheres.sphere.size()) {failed=true;return false;}
        for(unsigned index=0;index<spheres.count;++index) {
            const auto& sphere=spheres.sphere[index];
            if(!roomscale::detail::finite(roomscale::detail::convert(sphere.centre))||
               !std::isfinite(sphere.radius)||sphere.radius<=contactDepthBudget) {failed=true;return false;}
            roomscale::QueryScope scope{cover.maximumParameter,contactDepthBudget,false,true,rejectInitialContact};
            Query query{body,cover,current,context,scope,hull,index};
            if(!runRoomscaleModelQueryScope(scope,thread,Query::run,&query)||!query.clear) {
                failed=true;return false;
            }
            ++count;
        }
    }
    if(count!=cover.body.queryCount||!current(context)||!roomscaleResourceScopeUsable()) {failed=true;return false;}
    return true;
}
} // namespace ss2vr::game
