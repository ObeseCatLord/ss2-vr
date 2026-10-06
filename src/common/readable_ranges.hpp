#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <limits>
namespace ss2vr {
struct ReadableRange {uintptr_t begin=0,end=0;bool readable=false;};
// Cache page-readability evidence ONLY inside a separately owned lifetime
// extent. This does not pin allocations or prevent deletion/protection changes.
// Seal after capture; clear before crossing an unconstrained native callback.
template<size_t Capacity=24> class ReadableRanges {
    mutable std::array<ReadableRange,Capacity> regions{};
    mutable size_t count=0;
    bool sealed=false;
public:
    void clear() noexcept {count=0;sealed=false;}
    void seal() noexcept {sealed=true;}
    template<class Query> bool contains(uintptr_t begin,size_t bytes,Query&& query) const {
        if(!begin||bytes>std::numeric_limits<uintptr_t>::max()-begin)return false;
        const auto end=begin+bytes;
        auto cursor=begin;
        while(cursor<end) {
            uintptr_t covered=cursor;
            for(size_t i=0;i<count;++i)
                if(regions[i].begin<=cursor&&cursor<regions[i].end)covered=std::max(covered,regions[i].end);
            if(covered==cursor) {
                if(sealed||count==Capacity)return false;
                ReadableRange region;
                if(!query(cursor,region)||!region.readable||region.begin>cursor||region.end<=cursor)return false;
                regions[count++]=region;covered=region.end;
            }
            cursor=std::min(covered,end);
        }
        return true;
    }
};
} // namespace ss2vr
