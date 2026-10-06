#include "roomscale_placement.hpp"
#include "common/checked_placement.hpp"
#include "native_finally.hpp"
#include "native_memory.hpp"
#include <array>
#include <cstring>

namespace ss2vr::game {
namespace {
using MechanismSet=int(__thiscall *)(void *,const Pose &,uint32_t);
using PoseSet=void(__thiscall *)(void *,const Pose &);
MechanismSet originalMechanism=nullptr,publicMechanism=nullptr;
PoseSet originalPart=nullptr,originalAspect=nullptr;
uintptr_t engineBase=0,partReturn=0,aspectReturn=0;
bool queued=false,armed=false;
struct Invocation { void *subject; const Invocation *previous; uintptr_t caller; };
struct Request {
    void *mechanism,*part,*root;
    Pose target;
    RoomscalePlacementCheck check;
    void *context;
    CheckedPlacementState state{};
    const Invocation *owner=nullptr;
    bool pending=true;
    int nativeResult=0;
};
thread_local Request *request=nullptr;
thread_local const Invocation *mechanismInvocation=nullptr,*partInvocation=nullptr;

bool singlePart(const Request &r) noexcept {
    const auto m=reinterpret_cast<uintptr_t>(r.mechanism);
    const auto p=reinterpret_cast<uintptr_t>(r.part);
    return readableMemory(r.mechanism,0x3c) && readableMemory(r.part,0x38) &&
        readableMemory(r.root,0x48) &&
        *reinterpret_cast<volatile const uint32_t*>(m+8)==1 &&
        *reinterpret_cast<volatile const uintptr_t*>(m+4)==p &&
        *reinterpret_cast<volatile const uintptr_t*>(p+8)==reinterpret_cast<uintptr_t>(r.root);
}
int __fastcall mechanismSet(void *self,void *,const Pose &target,uint32_t flags) {
    auto *r=request;
    if (!r) return originalMechanism(self,target,flags);
    const bool owner=r->pending;
    if (owner) {
        r->pending=false;
        if (mechanismInvocation || self!=r->mechanism || flags!=1 || !singlePart(*r)) {
            r->state.failed=true; return 0;
        }
    }
    Invocation frame{self,mechanismInvocation,0};
    int result=0;
    withNativeFinally([&] {
        mechanismInvocation=&frame;
        if (owner) r->owner=&frame;
        result=originalMechanism(self,target,flags);
    },[&](bool aborted) noexcept {
        if (owner) { if (aborted) r->state.failed=true; r->owner=nullptr; }
        mechanismInvocation=frame.previous;
    });
    return result;
}
void __fastcall partSet(void *self,void *,const Pose &target) {
    if (!request) { originalPart(self,target); return; }
    const auto caller=reinterpret_cast<uintptr_t>(__builtin_return_address(0));
    Invocation frame{self,partInvocation,caller};
    withNativeFinally([&] {
        partInvocation=&frame;
        originalPart(self,target);
    },[&](bool aborted) noexcept {
        if (aborted && request && mechanismInvocation==request->owner) request->state.failed=true;
        partInvocation=frame.previous;
    });
}
void __fastcall aspectSet(void *self,void *,const Pose &target) {
    auto *r=request;
    const auto caller=reinterpret_cast<uintptr_t>(__builtin_return_address(0));
    const bool owned=r && r->owner && mechanismInvocation==r->owner && partInvocation &&
        partInvocation->subject==r->part && partInvocation->caller==partReturn && caller==aspectReturn &&
        !r->state.commitStarted;
    if (!owned) { originalAspect(self,target); return; }
    if (!r->state.claimRoot(self==r->root && finite(target))) return;
    // Native part composition is complete. No root write has occurred yet.
    // The check may issue only the owner's admitted fresh native queries.
    withNativeFinally([&] {
        if (!r->check(r->context,target)) { r->state.failed=true; return; }
        if (!finishRoomscaleResourceScopeForCommit(r->state.failed) || !r->state.beginCommit(true)) {
            r->state.failed=true; return;
        }
        // From the first write onward, all original commit callbacks stay
        // native. A fault is an uncertain mutation, never a retryable rejection.
        originalAspect(self,target);
        r->state.commitReturned=true;
    },[&](bool aborted) noexcept { if (aborted) r->state.failed=true; });
}
} // namespace

extern "C" int __cdecl ss2vrPlacementDecision(void *part,unsigned after) noexcept {
    auto *r=request;
    if (!r || !r->owner || mechanismInvocation!=r->owner || r->state.commitStarted) return 0;
    const bool allowed=after?r->state.afterPart():r->state.beforePart(part==r->part);
    return allowed?0:1;
}
#define STR_(x) #x
#define STR(x) STR_(x)
#define PLACEMENT_GATE(name,after) \
 extern "C" { void *name##_original=nullptr; void *name##_cancel=nullptr; } \
 extern "C" __attribute__((naked,used)) void name() { __asm__( \
  "pushfl\n\tpushal\n\tcld\n\tmovl %esp,%ebp\n\tandl $-16,%esp\n\tsubl $512,%esp\n\t" \
  "fxsave (%esp)\n\tfninit\n\tfldcw (%esp)\n\tsubl $8,%esp\n\tpushl $" STR(after) "\n\tpushl 4(%ebp)\n\t" \
  "call _ss2vrPlacementDecision\n\taddl $16,%esp\n\ttestl %eax,%eax\n\tjnz 1f\n\t" \
  "fxrstor (%esp)\n\tmovl %ebp,%esp\n\tpopal\n\tpopfl\n\tjmp *_" STR(name) "_original\n\t" \
  "1: fxrstor (%esp)\n\tmovl %ebp,%esp\n\tpopal\n\tpopfl\n\tjmp *_" STR(name) "_cancel\n\t"); }
PLACEMENT_GATE(roomscaleBeforePart,0)
PLACEMENT_GATE(roomscaleAfterPart,1)
extern "C" { void *roomscalePlacementExit=nullptr; }
extern "C" __attribute__((naked,used)) void roomscaleCancelPlacement() { __asm__(
    "leal -0x370(%ebp),%esp\n\tjmp *_roomscalePlacementExit\n\t"); }

namespace {
struct Binding { uint32_t rva; void *target; };
std::array<Binding,5> bindings{};
bool jumpTargets(uintptr_t address,uintptr_t target) noexcept {
    if (!readableMemory(reinterpret_cast<void*>(address),5)) return false;
    const auto *p=reinterpret_cast<const uint8_t*>(address);
    int32_t delta=0;std::memcpy(&delta,p+1,4);
    return p[0]==0xe9 && uint32_t(address+5+uint32_t(delta))==uint32_t(target);
}
}
bool queueRoomscalePlacementHooks(HMODULE engine,RoomscaleQueueHook exports,RoomscaleInternalHook internals) {
    if (!engine || !exports || !internals || queued || originalMechanism || originalPart || originalAspect) return false;
    auto *base=reinterpret_cast<uint8_t*>(engine);
    struct Prefix { uint32_t rva; std::array<uint8_t,9> bytes; unsigned size; };
    const std::array prefixes{
        Prefix{0x134010,{0x55,0x8b,0xec,0x81,0xec,0x64,0x03,0x00,0x00},9},
        Prefix{0x131870,{0x55,0x8b,0xec,0x83,0xec,0x4c},6},
        Prefix{0x575d0,{0x55,0x8b,0xec,0x81,0xec,0x84,0x00,0x00,0x00},9},
        Prefix{0x134d7b,{0xe8,0xf0,0xca,0xff,0xff},5},
        Prefix{0x134d80,{0x8b,0x46,0x10,0x85,0xc0},5},
    };
    for (const auto &p:prefixes) if (std::memcmp(base+p.rva,p.bytes.data(),p.size)) return false;
    constexpr std::array<uint8_t,11> failure{0x5f,0x5e,0x33,0xc0,0x5b,0x8b,0xe5,0x5d,0xc2,0x08,0x00};
    if (std::memcmp(base+0x135c62,failure.data(),failure.size())) return false;
    constexpr auto mechanismName="?SetAbsPlacement@CMechanism@SeriousEngine@@QAEHABVQuatVect@2@K@Z";
    const auto entry=GetProcAddress(engine,mechanismName);
    static_assert(sizeof(entry)==sizeof(publicMechanism));
    std::memcpy(&publicMechanism,&entry,sizeof(entry));
    if (reinterpret_cast<uintptr_t>(publicMechanism)!=reinterpret_cast<uintptr_t>(base+0x134010)) return false;
    if (!exports(engine,mechanismName,reinterpret_cast<void*>(&mechanismSet),reinterpret_cast<void**>(&originalMechanism)) ||
        !exports(engine,"?SetAbsPlacement@CMechanismPart@SeriousEngine@@QAEXABVQuatVect@2@@Z",
                 reinterpret_cast<void*>(&partSet),reinterpret_cast<void**>(&originalPart)) ||
        !exports(engine,"?SetAbsPlacement@CAspect@SeriousEngine@@QAEXABVQuatVect@2@@Z",
                 reinterpret_cast<void*>(&aspectSet),reinterpret_cast<void**>(&originalAspect))) return false;
    roomscalePlacementExit=base+0x135c62;
    roomscaleBeforePart_cancel=roomscaleAfterPart_cancel=reinterpret_cast<void*>(&roomscaleCancelPlacement);
    if (!internals(engine,0x134d7b,reinterpret_cast<void*>(&roomscaleBeforePart),&roomscaleBeforePart_original) ||
        !internals(engine,0x134d80,reinterpret_cast<void*>(&roomscaleAfterPart),&roomscaleAfterPart_original)) return false;
    // The pre-call gate relocates CALL into MinHook's trampoline. Its return
    // address is not Engine134D80; bind the actual relocated call provenance.
    const auto call=reinterpret_cast<uintptr_t>(roomscaleBeforePart_original);
    if (!readableMemory(reinterpret_cast<void*>(call),5)) return false;
    int32_t delta=0;std::memcpy(&delta,reinterpret_cast<void*>(call+1),4);
    if (*reinterpret_cast<const uint8_t*>(call)!=0xe8 ||
        uint32_t(call+5+uint32_t(delta))!=uint32_t(reinterpret_cast<uintptr_t>(base+0x131870))) return false;
    engineBase=reinterpret_cast<uintptr_t>(base);partReturn=call+5;aspectReturn=engineBase+0x131abe;
    bindings={Binding{0x134010,reinterpret_cast<void*>(&mechanismSet)},
              Binding{0x131870,reinterpret_cast<void*>(&partSet)},
              Binding{0x575d0,reinterpret_cast<void*>(&aspectSet)},
              Binding{0x134d7b,reinterpret_cast<void*>(&roomscaleBeforePart)},
              Binding{0x134d80,reinterpret_cast<void*>(&roomscaleAfterPart)}};
    queued=true;return true;
}
bool armRoomscalePlacementAfterEnable() noexcept {
    armed=false;
    if (!queued || !engineBase) return false;
    for (const auto &b:bindings)
        if (!jumpTargets(engineBase+b.rva,reinterpret_cast<uintptr_t>(b.target))) return false;
    armed=true;return true;
}
void resetRoomscalePlacementHooksAfterRemoval() noexcept {
    armed=queued=false;engineBase=partReturn=aspectReturn=0;
    publicMechanism=originalMechanism=nullptr;originalPart=originalAspect=nullptr;
    roomscaleBeforePart_original=roomscaleAfterPart_original=nullptr;
    roomscaleBeforePart_cancel=roomscaleAfterPart_cancel=roomscalePlacementExit=nullptr;
    bindings={};
}
RoomscalePlacementOutcome runCheckedRoomscalePlacement(void *mechanism,void *part,void *root,
    const Pose &target,DWORD thread,RoomscalePlacementCheck check,void *context) noexcept {
    if (!armed || !queued || !publicMechanism || !mechanism || !part || !root || !check ||
        !thread || GetCurrentThreadId()!=thread || request || mechanismInvocation || partInvocation || !finite(target)) return {};
    if (!armRoomscalePlacementAfterEnable()) return {}; // Detect removed/replaced guards before any call.
    Request state{mechanism,part,root,target,check,context};
    if (!singlePart(state)) return {};
    struct Call {
        Request &state;
        static void __cdecl run(void *opaque) noexcept {
            auto &s=static_cast<Call*>(opaque)->state;
            s.nativeResult=publicMechanism(s.mechanism,s.target,1);
        }
    } call{state};
    withNativeFinally([&] {
        request=&state;
        if (!runRoomscaleResourceScope(state.state.failed,thread,Call::run,&call)) state.state.failed=true;
    },[&](bool aborted) noexcept {
        if (aborted) state.state.failed=true;
        request=nullptr;
    });
    return {state.state.completed(state.nativeResult),state.state.commitStarted,state.nativeResult};
}
} // namespace ss2vr::game
