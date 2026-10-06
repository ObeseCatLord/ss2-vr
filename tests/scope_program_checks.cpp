#ifdef NDEBUG
#error Scope program checks require assertions
#endif
#include "common/scope_program.hpp"
#include <cassert>
#include <fstream>
#include <iostream>
#include <vector>
using namespace ss2vr;
// Mod-owned tiny program, independently decoded with native Linux vkd3d.
static std::vector<uint32_t> direct() {
    return {0xfffe0101,31,0x80000005,0x900f0000,31,0x80030005,0x900f0003,
            1,0xc00f0000,0x90e40000,9,0xe0010003,0x90e40003,0xa0e40008,
            9,0xe0020003,0x90e40003,0xa0e40009,0xffff};
}
int main(int argc,char **argv) {
    const auto valid=direct();
    assert(scopeUvProgram(valid));
    for(size_t length=0;length<valid.size();++length) assert(!scopeUvProgram(std::span(valid).first(length)));
    auto bad=valid; bad[0]=0xfffe0200; assert(!scopeUvProgram(bad));
    bad=valid; bad[10]|=0x03000000; assert(!scopeUvProgram(bad)); // VS2 length bits.
    bad=valid; bad[10]|=0x40000000; assert(!scopeUvProgram(bad)); // Co-issue.
    bad=valid; bad[10]|=0x10000000; assert(!scopeUvProgram(bad)); // Predication.
    bad=valid; bad.push_back(0); assert(!scopeUvProgram(bad));
    bad=valid; bad[5]=0x80040005; assert(!scopeUvProgram(bad)); // Remapped semantic.
    bad=valid; bad[6]=0x900f0004; assert(!scopeUvProgram(bad));
    bad=valid; bad.insert(bad.begin()+7,{31,0x80030005,0x900f0003}); assert(!scopeUvProgram(bad));
    bad=valid; bad[3]=0x90030000; assert(!scopeUvProgram(bad)); // Declaration mask.
    bad=valid; bad.erase(bad.begin()+4,bad.begin()+7); assert(!scopeUvProgram(bad));
    bad=valid; bad[12]|=0x01000000; assert(!scopeUvProgram(bad)); // Negated input.
    bad=valid; bad[12]=0x90000003; assert(!scopeUvProgram(bad)); // Swizzle.
    bad=valid; bad[13]|=0x2000; assert(!scopeUvProgram(bad)); // Relative UV constant.
    bad=valid; bad[11]=0xe0030003; assert(!scopeUvProgram(bad)); // Broader write mask.
    bad=valid; bad[11]|=0x00100000; assert(!scopeUvProgram(bad)); // Saturation.
    bad=valid; bad[11]|=0x01000000; assert(!scopeUvProgram(bad)); // Destination shift.
    bad=valid; bad[11]|=0x4000; assert(!scopeUvProgram(bad)); // Reserved bit.
    bad=valid; bad[12]&=0x7fffffff; assert(!scopeUvProgram(bad)); // Missing parameter marker.
    bad=valid; bad[12]|=0x1000; assert(!scopeUvProgram(bad)); // Extended register bank.
    bad=valid; bad[9]=0x90e40001; assert(!scopeUvProgram(bad)); // Undeclared input.
    bad=valid; bad[8]=0x900f0000; assert(!scopeUvProgram(bad)); // Executable input write.
    bad=valid; bad[8]=0xa00f0008; assert(!scopeUvProgram(bad)); // Executable constant write.
    bad=valid; bad[8]=0xc00f0003; assert(!scopeUvProgram(bad)); // Invalid raster register.
    bad=valid; bad[8]=0x800f000c; assert(!scopeUvProgram(bad)); // Temporary out of range.
    bad=valid; bad[10]=28; assert(!scopeUvProgram(bad)); // Flow control.
    bad=valid; bad.insert(bad.end()-1,{1,0xe0040003,0x90e40000}); assert(!scopeUvProgram(bad));
    bad=valid; bad.insert(bad.end()-1,{9,0xe0010003,0x90e40003,0xa0e40008}); assert(!scopeUvProgram(bad));
    bad=valid; bad.insert(bad.begin()+7,{81,0xa00f0008,0,0,0,0}); assert(!scopeUvProgram(bad));
    bad=valid; bad.insert(bad.begin()+7,{81,0xa00f0009,0,0,0,0}); assert(!scopeUvProgram(bad));
    // Complete opaque COMMENT/DEF payloads may resemble executable tokens.
    auto comment=valid;
    comment.insert(comment.begin()+7,{0x0004fffe,9,0xe0010003,0x90e40003,0xa0e40008});
    assert(scopeUvProgram(comment));
    bad={0xfffe0101,31,0x80030005,0x900f0003,0x0008fffe,9,0xe0010003,0x90e40003,0xa0e40008,
         9,0xe0020003,0x90e40003,0xa0e40009,0xffff};
    assert(!scopeUvProgram(bad)); // Neither pair exists in executable body.
    bad={0xfffe0101,0x7ffffffE,0xffff}; assert(!scopeUvProgram(bad)); // Truncated comment.
    bad=comment; bad[7]|=0x80000000; assert(!scopeUvProgram(bad));
    auto definition=valid;
    definition.insert(definition.begin()+7,{81,0xa00f0014,9,0xe0010003,0x90e40003,0xa0e40008});
    assert(scopeUvProgram(definition));
    bad=definition; bad.insert(bad.begin()+7,{81,0xa00f0014,0,0,0,0}); assert(!scopeUvProgram(bad));
    bad={0xfffe0101,31,0x80030005,0x900f0003,81,0xa00f0014,9,0xe0010003,0x90e40003,0xa0e40008,
         81,0xa00f0015,9,0xe0020003,0x90e40003,0xa0e40009,0xffff};
    assert(!scopeUvProgram(bad)); // DEF literals do not prove executable writes.
    // Native skinning's relative constant source has no extra address DWORD.
    auto skin=valid;
    skin.insert(skin.begin()+10,{1,0xb0010000,0x90e40000,21,0x80070000,0x90e40000,0xa0e42015});
    assert(scopeUvProgram(skin));
    auto defaultMatrixMask=skin; defaultMatrixMask[14]=0x800f0000;
    assert(scopeUvProgram(defaultMatrixMask)); // Legacy assembler's default full mask.
    bad=defaultMatrixMask; bad[14]=0xe00f0003;
    assert(!scopeUvProgram(bad)); // Generic matrix admission cannot write scope UV.
    auto scalarRaster=valid; scalarRaster.insert(scalarRaster.begin()+10,{1,0xc00f0001,0x90ff0000});
    assert(scopeUvProgram(scalarRaster)); // Fog output's legacy default full mask.
    bad=scalarRaster; bad[11]=0xe00f0003; assert(!scopeUvProgram(bad));
    bad=skin; bad.insert(bad.begin()+17,0xb0000000); assert(!scopeUvProgram(bad));
    bad=skin; bad[11]=0xb0020000; assert(!scopeUvProgram(bad)); // Address y write.
    bad=valid; bad.insert(bad.begin()+10,{6,0x800f0000,0x90e40000}); assert(!scopeUvProgram(bad));
    auto scalar=valid; scalar.insert(scalar.begin()+10,{6,0x800f0000,0x90ff0000}); assert(scopeUvProgram(scalar));
    auto huge=valid; huge.insert(huge.end()-1,ScopeProgramMaxWords,0); assert(!scopeUvProgram(huge));
    if(argc==3 && std::string(argv[1])=="--write-fixture") {
        std::ofstream file(argv[2],std::ios::binary); assert(file);
        file.write(reinterpret_cast<const char *>(valid.data()),std::streamsize(valid.size()*4));
        assert(file);
        std::cout<<"{\"mod_owned_direct_uv_fixture_words\":"<<valid.size()<<"}\n";
    } else if(argc==3) {
        std::ifstream file(argv[1],std::ios::binary|std::ios::ate); assert(file);
        const auto size=file.tellg(); assert(size>=8 && size<=std::streamoff(ScopeProgramMaxWords*4) && size%4==0);
        std::vector<uint32_t> words(size_t(size)/4); file.seekg(0);
        file.read(reinterpret_cast<char *>(words.data()),size); assert(file.gcount()==size);
        const bool expected=std::string(argv[2])=="accept"; assert(expected || std::string(argv[2])=="reject");
        assert(scopeUvProgram(words)==expected);
        std::cout<<"{\"offline_uv_program_admission\":"<<(expected?"true":"false")<<"}\n";
    } else assert(argc==1);
}
