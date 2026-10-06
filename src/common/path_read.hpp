#pragma once
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace ss2vr {
// Win32 path getters report either capacity (GetModuleFileName) or required
// capacity (GetSystemDirectory) on truncation. Never consume truncated or
// unterminated output, including older Wine/Windows behavior. This helper is
// portable so failure/truncation paths can be exercised without running PE code.
template<class Getter>
bool readBoundedWidePath(Getter getter,std::wstring &out) noexcept {
    out.clear();
    try {
        constexpr uint32_t maximum=32768;
        uint32_t capacity=260;
        for (;;) {
            std::vector<wchar_t> buffer(capacity,L'\0');
            const uint32_t length=getter(buffer.data(),capacity);
            if (!length) return false;
            if (length<capacity) {
                if (buffer[length]!=L'\0' || std::find(buffer.begin(),buffer.begin()+length,L'\0')!=buffer.begin()+length)
                    return false;
                out.assign(buffer.data(),length);
                return true;
            }
            if (capacity==maximum || length>maximum) return false;
            capacity=std::min(maximum,std::max(capacity*2,length));
        }
    } catch (...) { out.clear(); return false; }
}
inline bool pathDirectory(const std::wstring &path,std::wstring &out) noexcept {
    try {
        const auto slash=path.find_last_of(L"\\/");
        if (slash==std::wstring::npos || !slash) { out.clear(); return false; }
        out=path.substr(0,slash+1);
        return true;
    } catch (...) { out.clear(); return false; }
}
} // namespace ss2vr
