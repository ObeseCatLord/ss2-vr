#pragma once
#include <windows.h>
#include "common/readable_ranges.hpp"
#include <cstdint>

namespace ss2vr::game {
inline bool readableMemory(const void *pointer, size_t bytes, bool writable = false) {
    uintptr_t begin = reinterpret_cast<uintptr_t>(pointer);
    if (!begin || begin > UINTPTR_MAX - bytes)
        return false;
    const uintptr_t end = begin + bytes;
    while (begin < end) {
        MEMORY_BASIC_INFORMATION info{};
        if (!VirtualQuery(reinterpret_cast<void *>(begin), &info, sizeof(info)) || info.State != MEM_COMMIT ||
            (info.Protect & (PAGE_GUARD | PAGE_NOACCESS)))
            return false;
        const DWORD protection = info.Protect & 0xff;
        if (protection != PAGE_READONLY && protection != PAGE_READWRITE && protection != PAGE_WRITECOPY &&
            protection != PAGE_EXECUTE_READ && protection != PAGE_EXECUTE_READWRITE &&
            protection != PAGE_EXECUTE_WRITECOPY)
            return false;
        if (writable && protection != PAGE_READWRITE && protection != PAGE_WRITECOPY &&
            protection != PAGE_EXECUTE_READWRITE && protection != PAGE_EXECUTE_WRITECOPY)
            return false;
        const uintptr_t region = reinterpret_cast<uintptr_t>(info.BaseAddress);
        if (region > UINTPTR_MAX - info.RegionSize || region + info.RegionSize <= begin)
            return false;
        begin = region + info.RegionSize;
    }
    return true;
}
class NativeReadLease {
    ReadableRanges<> ranges;
public:
    void clear() noexcept {ranges.clear();}
    void seal() noexcept {ranges.seal();}
    bool contains(const void* pointer,size_t bytes) const noexcept {
        return ranges.contains(reinterpret_cast<uintptr_t>(pointer),bytes,[](uintptr_t at,ReadableRange& out) {
            MEMORY_BASIC_INFORMATION info{};
            if(!VirtualQuery(reinterpret_cast<void*>(at),&info,sizeof(info))||info.State!=MEM_COMMIT||
               (info.Protect&(PAGE_GUARD|PAGE_NOACCESS)))return false;
            const DWORD p=info.Protect&0xff;
            const bool readable=p==PAGE_READONLY||p==PAGE_READWRITE||p==PAGE_WRITECOPY||
                p==PAGE_EXECUTE_READ||p==PAGE_EXECUTE_READWRITE||p==PAGE_EXECUTE_WRITECOPY;
            const auto base=reinterpret_cast<uintptr_t>(info.BaseAddress);
            if(base>UINTPTR_MAX-info.RegionSize)return false;
            out={base,base+info.RegionSize,readable};return true;
        });
    }
};
} // namespace ss2vr::game
