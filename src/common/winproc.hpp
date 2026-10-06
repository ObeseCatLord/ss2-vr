#pragma once
#include <cstring>
#include <windows.h>
namespace ss2vr {
template <class T> T loadProc(HMODULE module, const char *name) {
    FARPROC address = module ? GetProcAddress(module, name) : nullptr;
    T function = nullptr;
    static_assert(sizeof(function) == sizeof(address));
    std::memcpy(&function, &address, sizeof(function));
    return function;
}
} // namespace ss2vr
