#include "scope_capture_command.hpp"
#include "native_finally.hpp"
#include "native_memory.hpp"
#include "common/scope_capture_command.hpp"
#include <cstddef>
#include <cstring>
#include <type_traits>

namespace ss2vr::game {
namespace {
struct Command {
    uintptr_t vtable;
    void *parent;
    uint32_t rank;
    float tie;
    ScopeCaptureCallback callback;
    void *context;
};
static_assert(sizeof(void *) == 4 && sizeof(Command) == 0x18 && std::is_standard_layout_v<Command>);
static_assert(offsetof(Command,parent) == 4 && offsetof(Command,rank) == 8 && offsetof(Command,tie) == 0xc &&
              offsetof(Command,callback) == 0x10 && offsetof(Command,context) == 0x14);
void __cdecl abandonedCapture(void *) noexcept {} // Safe failure command, never an image implementation.
template<class T> T read(const void *object, size_t offset = 0) noexcept {
    T result;
    std::memcpy(&result, static_cast<const uint8_t *>(object) + offset, sizeof(result));
    return result;
}
struct CommandArray { void *owner = nullptr; void **data = nullptr; int32_t count = 0; };
bool commandArray(void *root, CommandArray &out) noexcept {
    out = {};
    if (!readableMemory(root,0x14)) return false;
    out.owner = read<void *>(root,0x10);
    if (!readableMemory(out.owner,0x10)) return false;
    const auto capacity = read<int32_t>(out.owner);
    out.data = read<void **>(out.owner,4);
    out.count = read<int32_t>(out.owner,8);
    return out.count >= 0 && out.count <= ScopeCaptureMaxCommands && capacity >= out.count &&
        (!out.count || readableMemory(out.data,size_t(out.count)*sizeof(void *)));
}
} // namespace
bool ScopeCaptureCommands::configure(HMODULE engine, HMODULE sam) noexcept {
    if (!engine || !sam) return false;
    const auto base = reinterpret_cast<uintptr_t>(engine);
    const auto caller = reinterpret_cast<uintptr_t>(sam) + 0xfda0b;
    if (engine_) return engine_ == base && injectionReturn_ == caller;
    const auto allocate = GetProcAddress(engine,"??2CRenCmd@SeriousEngine@@SAPAXI@Z");
    const auto construct = GetProcAddress(engine,"??0CRenCmd@SeriousEngine@@QAE@XZ");
    const auto table = reinterpret_cast<void *>(base+0x218f50);
    if (reinterpret_cast<uintptr_t>(allocate) != base+0x1556f0 ||
        reinterpret_cast<uintptr_t>(construct) != base+0x155740 || !readableMemory(table,8) ||
        read<uintptr_t>(table) != base+0x14bf60 || read<uintptr_t>(table,4) != base+0x14ade0 ||
        !readableMemory(reinterpret_cast<void *>(base+0x2ef17c),4)) return false;
    allocate_ = reinterpret_cast<Allocate>(allocate);
    construct_ = reinterpret_cast<Construct>(construct);
    injectionReturn_ = caller; engine_ = base;
    return true;
}
bool ScopeCaptureCommands::queue(void *root, uintptr_t injectionCaller, ScopeCaptureCallback callback,
                                 ScopeCaptureCurrent current, void *context) const noexcept {
    if (!engine_ || !allocate_ || !construct_ || !root || !callback || !current || !context ||
        injectionCaller != injectionReturn_ || !current(context)) return false;
    bool queued = false;
    const auto collection = reinterpret_cast<void *>(engine_+0x2ef17c);
    withNativeFinally([&] {
        CommandArray before;
        if (read<void *>(collection) != root || !commandArray(root,before) ||
            before.count >= ScopeCaptureMaxCommands || !current(context)) return;
        void *storage = allocate_(sizeof(Command));
        if (!storage || !readableMemory(storage,sizeof(Command),true) ||
            read<void *>(collection) != root || !current(context)) return;
        construct_(storage); // Native constructor alone appends to its current collection.
        // Complete a harmless command immediately after normal constructor
        // return, BEFORE any further callback/getter can reject the source.
        // Native destruction owns it even if later membership checks fail.
        Command command{engine_+0x218f50,read<void *>(storage,4),ScopeCaptureRank,0,
                        abandonedCapture,nullptr};
        std::memcpy(storage,&command,sizeof(command));
        CommandArray after;
        if (!current(context) || !commandArray(root,after) || !after.count) return;
        const ScopeCommandAppend append{
            reinterpret_cast<uintptr_t>(root),reinterpret_cast<uintptr_t>(read<void *>(collection)),
            reinterpret_cast<uintptr_t>(command.parent),reinterpret_cast<uintptr_t>(before.owner),
            reinterpret_cast<uintptr_t>(after.owner),reinterpret_cast<uintptr_t>(storage),
            reinterpret_cast<uintptr_t>(read<void *>(after.data,size_t(after.count-1)*4)),before.count,after.count};
        if (!scopeCommandAppendConfirmed(append) || !current(context)) return;
        // Arm only after exact native ownership is confirmed. No native call
        // occurs between these writes and returning the result to the owner.
        std::memcpy(static_cast<uint8_t *>(storage)+0x14,&context,4);
        std::memcpy(static_cast<uint8_t *>(storage)+0x10,&callback,4);
        queued = true;
    },[&](bool aborted) noexcept {
        if (aborted) queued = false;
        // Allocation/partial construction is native pile ownership. Do not
        // catch a fatal native allocator path and attempt synthetic recovery.
    });
    return queued;
}
} // namespace ss2vr::game
