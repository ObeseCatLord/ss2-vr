#pragma once
#include "path_read.hpp"
#include <windows.h>
namespace ss2vr {
inline bool modulePath(HMODULE module,std::wstring &out) noexcept {
    return readBoundedWidePath([&](wchar_t *buffer,uint32_t capacity) {
        return GetModuleFileNameW(module,buffer,capacity);
    },out);
}
inline bool moduleDirectory(HMODULE module,std::wstring &out) noexcept {
    out.clear();
    std::wstring path;
    return modulePath(module,path) && pathDirectory(path,out);
}
inline bool systemDirectory(std::wstring &out) noexcept {
    return readBoundedWidePath([](wchar_t *buffer,uint32_t capacity) {
        return GetSystemDirectoryW(buffer,capacity);
    },out);
}
} // namespace ss2vr
