#ifdef NDEBUG
#error Path checks require assertions
#endif
#include "common/path_read.hpp"
#include <cassert>
#include <stdexcept>
using namespace ss2vr;
int main() {
    for (size_t length : {1u,259u,260u,261u,1024u,32767u}) {
        const std::wstring expected(length,L'x');
        for (bool requiredSize : {false,true}) {
            unsigned calls=0;
            std::wstring out=L"stale";
            assert(readBoundedWidePath([&](wchar_t *p,uint32_t capacity) {
                ++calls;
                if (expected.size()>=capacity) {
                    std::fill(p,p+capacity,L'x'); // Deliberately no terminator.
                    return requiredSize?uint32_t(expected.size()+1):capacity;
                }
                std::copy(expected.begin(),expected.end(),p);p[expected.size()]=0;
                return uint32_t(expected.size());
            },out));
            assert(out==expected && calls<=8);
        }
    }
    std::wstring out=L"stale";
    assert(!readBoundedWidePath([](wchar_t *,uint32_t){ return 0u; },out) && out.empty());
    assert(!readBoundedWidePath([](wchar_t *,uint32_t){ return UINT32_MAX; },out) && out.empty());
    assert(!readBoundedWidePath([](wchar_t *,uint32_t capacity){ return capacity; },out) && out.empty());
    assert(!readBoundedWidePath([](wchar_t *p,uint32_t){p[0]=L'x';p[1]=L'x';return 1u;},out));
    assert(!readBoundedWidePath([](wchar_t *p,uint32_t){p[0]=0;p[1]=L'x';return 2u;},out));
    assert(!readBoundedWidePath([](wchar_t *,uint32_t)->uint32_t{throw std::runtime_error("getter");},out));
    assert(pathDirectory(L"Z:\\home\\Library With Spaces\\SS2\\Bin\\Sam2.exe",out));
    assert(out==L"Z:\\home\\Library With Spaces\\SS2\\Bin\\");
    assert(pathDirectory(L"C:\\Sam2.exe",out) && out==L"C:\\");
    assert(!pathDirectory(L"Sam2.exe",out) && out.empty());
}
