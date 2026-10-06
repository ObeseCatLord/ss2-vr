#ifdef NDEBUG
#error Scope color program checks require assertions
#endif
#include "common/scope_color_program.hpp"
#include <cassert>
#include <fstream>
#include <iostream>
#include <vector>
using namespace ss2vr;
// Mod-owned direct PS2.0 fixture; independently disassembled on Linux.
static std::vector<uint32_t> direct() {
    return {0xffff0200,0x0200001f,0x80000000,0xb0030003,
            0x0200001f,0x90000000,0xa0000803,
            0x03000042,0x800f0000,0xb0e40003,0xa0e40803,
            0x02000001,0x800f0800,0x80e40000,0xffff};
}
int main(int argc,char **argv) {
    const auto good=direct(); assert(scopeColorProgram(good));
    for(size_t n=0;n<good.size();++n) assert(!scopeColorProgram(std::span(good).first(n)));
    auto bad=good; bad.push_back(0); assert(!scopeColorProgram(bad));
    for(uint32_t version:{0xffff0101u,0xffff0201u,0xffff0300u,0xfffe0200u}) {
        bad=good; bad[0]=version; assert(!scopeColorProgram(bad));
    }
    for(uint32_t flag:{0x00010000u,0x10000000u,0x40000000u,0x80000000u}) {
        bad=good; bad[7]|=flag; assert(!scopeColorProgram(bad));
    }
    bad=good; bad[7]=0x02000042; assert(!scopeColorProgram(bad)); // Wrong TEXLD operand count.
    bad=good; bad[1]=0x0100001f; assert(!scopeColorProgram(bad)); // DCL needs semantic+destination.
    bad=good; bad[9]|=0x2000; assert(!scopeColorProgram(bad)); // No PS2 relative-address token.
    bad=good; bad[9]|=0x4000; assert(!scopeColorProgram(bad));
    bad=good; bad[9]&=0x7fffffff; assert(!scopeColorProgram(bad));
    bad=good; bad[9]=0x90e40000; assert(!scopeColorProgram(bad)); // Input colors outside this subset.
    bad=good; bad[10]=0xa0e40804; assert(!scopeColorProgram(bad)); // Undeclared sampler.
    bad=good; bad[10]|=0x01000000; assert(!scopeColorProgram(bad)); // Sampler negation.
    bad=good; bad[8]=0x80070000; assert(!scopeColorProgram(bad)); // TEXLD full mask required.
    bad=good; bad[8]|=0x00100000; assert(!scopeColorProgram(bad)); // TEXLD cannot saturate here.
    bad=good; bad[8]=0x800f000c; assert(!scopeColorProgram(bad)); // Temp out of range.
    bad=good; bad[3]=0xb0010003; assert(!scopeColorProgram(bad)); // 2D lookup needs xy, not x only.
    bad=good; bad[5]=0x98000000; assert(!scopeColorProgram(bad)); // Cube outside first subset.
    bad=good; bad.insert(bad.begin()+7,{0x0200001f,0x90000000,0xa00f0803}); assert(!scopeColorProgram(bad));
    bad=good; bad.insert(bad.begin()+11,{0x0200001f,0x80000000,0xb0030004}); assert(!scopeColorProgram(bad));
    auto pp=good; pp[3]|=0x00200000; pp[8]|=0x00200000; pp[12]|=0x00200000;
    assert(scopeColorProgram(pp));
    auto samplerMask=good; samplerMask[6]|=0x000f0000; assert(scopeColorProgram(samplerMask));
    for(uint32_t output:{0x800f0801u,0x900f0800u,0x80080800u,0x80070800u}) {
        bad=good; bad[12]=output; assert(!scopeColorProgram(bad)); // MRT/depth/partial outputs.
    }
    bad=good; bad[12]|=0x00100000; assert(!scopeColorProgram(bad)); // Output SAT unsupported by PS2.
    bad=good; bad[12]|=0x00400000; assert(!scopeColorProgram(bad)); // Centroid destination.
    bad=good; bad[12]|=0x01000000; assert(!scopeColorProgram(bad)); // Shift.
    bad=good; bad[13]=0x80e40001; assert(!scopeColorProgram(bad)); // Undefined temp output.
    bad=good; bad[13]=0x80ff0000; assert(!scopeColorProgram(bad)); // Output identity required.
    bad=good; bad.insert(bad.end()-1,{0x02000001,0x800f0001,0x80e40000}); assert(!scopeColorProgram(bad));
    bad=good; bad.insert(bad.end()-1,{0x02000001,0x800f0800,0x80e40000}); assert(!scopeColorProgram(bad));
    for(uint32_t opcode:{65u,84u,87u,40u,0xfffdu}) {
        bad=good; bad.insert(bad.begin()+11,{opcode|0x01000000u,0x800f0000}); assert(!scopeColorProgram(bad));
    }
    auto comment=good;
    comment.insert(comment.begin()+7,{0x0003fffe,0xffff,0x01000041,0x800f0000}); assert(scopeColorProgram(comment));
    bad={0xffff0200,0x0003fffe,0x02000001,0x800f0800,0x80e40000,0xffff};
    assert(!scopeColorProgram(bad)); // COMMENT cannot define output.
    bad=comment; bad[7]=0x8003fffe; assert(!scopeColorProgram(bad));
    bad={0xffff0200,0x7ffffffE,0xffff}; assert(!scopeColorProgram(bad));
    auto def=good; def.insert(def.begin()+1,{0x05000051,0xa00f001f,0xffff,0x01000041,0x800f0000,0x800f0800});
    assert(scopeColorProgram(def)); // Literal float bits resembling instructions remain opaque.
    bad=def; bad[6]=0x7f800000; assert(!scopeColorProgram(bad)); // Infinite DEF literal.
    bad=def; bad.insert(bad.begin()+1,{0x05000051,0xa00f001f,0,0,0,0}); assert(!scopeColorProgram(bad));
    bad=good; bad.insert(bad.begin()+11,{0x05000051,0xa00f0000,0,0,0,0}); assert(!scopeColorProgram(bad));
    // Only xy is initialized. A DP3 writing alpha still consumes source xyz.
    bad=good; bad.insert(bad.begin()+11,{0x02000001,0x80030001,0xb0e40003,
        0x03000008,0x80080000,0x80e40001,0xa0e40000}); assert(!scopeColorProgram(bad));
    auto norm=good;
    norm.insert(norm.begin()+7,{0x0200001f,0x80000000,0xb0070000});
    norm.insert(norm.begin()+14,{0x02000024,0x80070001,0xb0e40000}); assert(scopeColorProgram(norm));
    bad=norm; bad[16]=0xb0ff0000; assert(!scopeColorProgram(bad));
    auto power=good; power.insert(power.begin()+11,{0x03000020,0x80010001,0x80000000,0xa0ff0000});
    assert(scopeColorProgram(power));
    auto baseAlias=power; baseAlias[12]=0x80010000; assert(scopeColorProgram(baseAlias));
    bad=power; bad[12]=0x80010000; bad[14]=0x80000000;
    assert(!scopeColorProgram(bad)); // POW exponent (SDK src1) alias.
    bad=power; bad[13]=0x80e40000; assert(!scopeColorProgram(bad));
    bad=good; bad.insert(bad.begin()+11,{0x02000024,0x80070000,0x80e40000});
    assert(!scopeColorProgram(bad)); // NRM cannot alias its source, even when defined.
    auto huge=good; huge.insert(huge.begin()+1,ScopeProgramMaxWords,0); assert(!scopeColorProgram(huge));
    if(argc==3 && std::string(argv[1])=="--write-fixture") {
        std::ofstream file(argv[2],std::ios::binary); assert(file);
        file.write(reinterpret_cast<const char *>(good.data()),std::streamsize(good.size()*4)); assert(file);
        std::cout<<"{\"mod_owned_color_fixture_words\":"<<good.size()<<"}\n";
    } else assert(argc==1);
}
