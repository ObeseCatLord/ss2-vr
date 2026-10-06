#pragma once
#include <windows.h>
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
} // namespace ss2vr::game
