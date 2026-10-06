#include "common/roomscale_body_geometry.hpp"
#include <array>
#include <cassert>
#include <cstring>
#include <limits>
using namespace ss2vr;
struct Memory {
    std::array<uint8_t,0x8000> bytes{};
    std::array<uint32_t,5> handle{0,0x1000,0x2000,0x3000,0x4000};
    template<class T> void write(uint32_t address,const T& value) {
        assert(address+sizeof(value)<=bytes.size());std::memcpy(bytes.data()+address,&value,sizeof(value));
    }
    void word(uint32_t address,uint32_t value) { write(address,value); }
    Memory() {
        word(0x1114,2);word(0x1118,3);word(0x1120,4);
        word(0x2008,1);word(0x2004,0x5000);word(0x2038,4);word(0x5008,0x3000);
        word(0x3000,0x7000);word(0x3048,0x1000);word(0x3008,0x6000);
        word(0x6000,0x7100);word(0x6004,0x3000);word(0x6048,0x1000);word(0x6070,0x2000);
        word(0x6054,0x12345678);
        write(0x602c,Pose{Quat{0,0,0,1},{1,2,3}});
        write(0x402c,Pose{Quat{0,0,0,1},{1,1,3}});
        write(0x6078,roomscale::Primitive{2,.6f,1.8f,0});
    }
    bool capture(roomscale::BodyGeometry& result, size_t failedRead=0, size_t* readCount=nullptr,
                 bool replaceAfterCopy=false) {
        size_t reads=0,resolves=0;
        auto read=[&](uint32_t at,void* out,size_t size) {
            ++reads; if(readCount)*readCount=reads;
            if(reads==failedRead)return false;
            if(at>bytes.size()||size>bytes.size()-at)return false;
            std::memcpy(out,bytes.data()+at,size);return true;
        };
        auto resolve=[&](uint32_t value) {
            if(replaceAfterCopy && ++resolves==5)return uint32_t{0x7000};
            return value<handle.size()?handle[value]:0;
        };
        return roomscale::readBodyGeometry(read,resolve,{0x7000,0x7100},1,result);
    }
};
int main() {
    Memory valid;roomscale::BodyGeometry a,b;
    assert(valid.capture(a)&&a.root!=a.model&&a.hullCount==1&&a.hulls[0].category==0x12345678);
    assert(valid.capture(b)&&roomscale::sameBodyGeometry(a,b));
    for(auto [address,value]:std::array<std::pair<uint32_t,uint32_t>,13>{{
        {0x1010,2},{0x1564,7},{0x1544,7},{0x1114,0},{0x2008,2},{0x5008,0x4000},
        {0x2038,3},{0x3000,0x7100},{0x3048,0x4000},{0x6004,0},
        {0x6008,0x6500},{0x600c,0x6500},{0x6070,0x3000}}}) {
        Memory bad;bad.word(address,value);b=a;
        assert(!bad.capture(b)&&b.player==0);
    }
    Memory tilted;tilted.write(0x602c,Pose{Quat{.1f,0,0,.9949874f},{1,2,3}});
    assert(tilted.capture(b)); // Swimming includes an independently rotated hull.
    Memory recycled;recycled.handle[3]=0x5000;assert(!recycled.capture(b));
    size_t reads=0;assert(valid.capture(b,0,&reads));
    for(size_t fail=1;fail<=reads;++fail) {
        b=a;assert(!valid.capture(b,fail)&&b.player==0);
    }
    b=a;assert(!valid.capture(b,0,nullptr,true)&&b.player==0);
    for(auto shape:std::array<roomscale::Primitive,6>{{
        {1,.6f,1.8f,0},{2,0,1.8f,0},{2,-1,1.8f,0},{2,.6f,.5f,0},
        {2,std::numeric_limits<float>::infinity(),1.8f,0},
        {2,.6f,std::numeric_limits<float>::quiet_NaN(),0}}}) {
        Memory bad;bad.write(0x6078,shape);assert(!bad.capture(b));
    }
    Memory sphere;sphere.write(0x6078,roomscale::Primitive{0,.6f,0,0});
    assert(sphere.capture(b));
    Memory badModel;badModel.write(0x402c,Pose{Quat{0,0,0,0},{1,2,3}});
    assert(!badModel.capture(b));
    Memory overflow;overflow.word(0x3008,UINT32_MAX-4);assert(!overflow.capture(b));
    // A pose change invalidates a comparison even if every handle is unchanged.
    Memory moved;moved.write(0x402c,Pose{Quat{0,0,0,1},{2,1,3}});
    assert(moved.capture(b)&&!roomscale::sameBodyGeometry(a,b));
    valid.word(0x6054,0xabcdef);assert(valid.capture(b)&&!roomscale::sameBodyGeometry(a,b));
    Memory swimming;
    std::memcpy(swimming.bytes.data()+0x6800,swimming.bytes.data()+0x6000,0x90);
    swimming.word(0x600c,0x6800);
    swimming.write(0x682c,Pose{Quat{.70710677f,0,0,.70710677f},{1,2,3}});
    assert(swimming.capture(b)&&b.hullCount==2&&b.hulls[1].address==0x6800);
    auto both=b;
    size_t compoundReads=0;assert(swimming.capture(b,0,&compoundReads));
    for(size_t fail=1;fail<=compoundReads;++fail) {
        b=both;assert(!swimming.capture(b,fail)&&b.player==0&&b.hullCount==0);
    }

    swimming.word(0x6854,0x89abcdef);
    assert(swimming.capture(b)&&!roomscale::sameBodyGeometry(both,b));
    swimming.word(0x680c,0x6000);assert(!swimming.capture(b)&&b.player==0); // Cycle.
    swimming.word(0x680c,0x7000);assert(!swimming.capture(b)&&b.player==0); // Never truncate a third shape.
    Memory selfCycle;selfCycle.word(0x600c,0x6000);assert(!selfCycle.capture(b));

}
