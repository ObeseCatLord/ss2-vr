#pragma once
#include <array>
#include <cstdint>
#include <span>

namespace ss2vr {
// Admission budget, not a statement of the native format's maximum size.
constexpr size_t ScopeProgramMaxWords = 4096;
namespace scope_program {
struct Register {
    unsigned type=0, index=0, channels=0, modifier=0;
    bool relative=false;
};
inline unsigned type(uint32_t token) { return ((token>>28)&7) | ((token>>8)&0x18); }
inline bool destination(uint32_t token, Register &out) {
    // Register marker, split type, number and nonzero write mask only. No
    // destination addressing/modifiers/shifts or reserved parameter bits.
    constexpr uint32_t allowed=0x80000000u|0x70001800u|0x7ffu|0xf0000u;
    if (!(token&0x80000000u) || (token&~allowed) || !(token&0xf0000u)) return false;
    out={type(token),token&0x7ffu,(token>>16)&15,0,false};
    return true;
}
inline bool source(uint32_t token,const std::array<bool,16> &declared,Register &out) {
    constexpr uint32_t allowed=0x80000000u|0x70001800u|0x7ffu|0x2000u|0xff0000u|0xf000000u;
    if (!(token&0x80000000u) || (token&~allowed)) return false;
    out={type(token),token&0x7ffu,(token>>16)&255,(token>>24)&15,bool(token&0x2000u)};
    if (out.modifier>1 || (out.relative && out.type!=2)) return false;
    switch(out.type) {
    case 0: return out.index<12;
    case 1: return out.index<declared.size() && declared[out.index];
    case 2: return out.index<256; // Relative VS1.1 constants use implicit a0.x, no extra DWORD.
    default: return false;
    }
}
inline bool arithmeticDestination(const Register &reg,unsigned opcode) {
    switch(reg.type) {
    case 0: return reg.index<12;
    case 3: return reg.index==0 && opcode==1 && reg.channels==1; // MOV a0.x only.
    case 4: return reg.index<3 && (reg.index==0 || reg.channels==1 || reg.channels==15);
    case 5: return reg.index<2;
    case 6: return reg.index<8;
    default: return false; // No executable input/constant writes.
    }
}
inline unsigned sources(unsigned opcode) {
    switch(opcode) {
    case 1: case 6: case 7: case 16: return 1;
    case 2: case 3: case 5: case 8: case 9: case 10: case 11: case 12: case 13: case 17:
    case 20: case 21: case 22: case 23: case 24: return 2;
    case 4: return 3;
    default: return 0;
    }
}
} // namespace scope_program
// Bounded framing shared with the scope UV proof. The true mode additionally
// requires its exact UV interface; neither mode proves native position, skinning,
// alpha, material or image validity. Unknown encodings decline.
inline bool boundedVertexProgram(std::span<const uint32_t> words,bool requireScopeUv) {
    using namespace scope_program;
    if (words.size()<2 || words.size()>ScopeProgramMaxWords || words.front()!=0xfffe0101u) return false;
    std::array<bool,16> declared{};
    std::array<bool,256> definitions{};
    bool body=false, x=false, y=false;
    size_t cursor=1;
    while(cursor<words.size()) {
        const uint32_t instruction=words[cursor++];
        if (instruction==0xffffu) return cursor==words.size() && (requireScopeUv?(declared[3] && x && y):declared[0]);
        if ((instruction&0xffffu)==0xfffeu) {
            if (instruction&0x80000000u) return false;
            const size_t count=(instruction>>16)&0x7fff;
            if (count>words.size()-cursor) return false;
            cursor+=count; // Opaque payload: never interpret it as instructions.
            continue;
        }
        // VS1.1 has no length/predication/co-issue flags. Arity comes solely
        // from the opcode below, including its implicit relative addressing.
        if (instruction&0xffff0000u) return false;
        const unsigned opcode=instruction;
        if (opcode==31) {
            if (body || words.size()-cursor<2) return false;
            const uint32_t semantic=words[cursor++], input=words[cursor++];
            const unsigned n=(semantic>>16)&15;
            if (semantic!=(0x80000005u|(n<<16)) || input!=(0x900f0000u|n) || declared[n]) return false;
            declared[n]=true;
            continue;
        }
        if (opcode==81) {
            if (words.size()-cursor<5) return false;
            Register reg;
            if (!destination(words[cursor++],reg) || reg.type!=2 || reg.index>=definitions.size() ||
                reg.channels!=15 || (requireScopeUv && (reg.index==8 || reg.index==9)) || definitions[reg.index]) return false;
            definitions[reg.index]=true;
            cursor+=4; // Literal float bits are opaque, not register/instruction tokens.
            continue;
        }
        body=true;
        if (opcode==0) continue;
        const unsigned count=sources(opcode);
        if (!count || words.size()-cursor<count+1) return false;
        const uint32_t dest=words[cursor++];
        Register target;
        if (!destination(dest,target) || !arithmeticDestination(target,opcode)) return false;
        const auto operands=words.subspan(cursor,count);
        std::array<Register,3> inputs{};
        for(unsigned i=0;i<count;++i) if (!source(operands[i],declared,inputs[i])) return false;
        cursor+=count;
        if (opcode==6 || opcode==7) {
            const unsigned swizzle=inputs[0].channels;
            if (swizzle!=0 && swizzle!=0x55 && swizzle!=0xaa && swizzle!=0xff) return false;
        }
        if (opcode>=20 && opcode<=24) {
            const unsigned rows=opcode==20 || opcode==22 ? 4 : opcode==24 ? 2 : 3;
            const unsigned components=opcode==20 || opcode==22 ? 15 : opcode==24 ? 3 : 7;
            // Full default masks are retained for legacy assembler output;
            // the matrix opcode itself defines its row count. No UV destination
            // is authorized by this generic matrix admission.
            if ((target.channels!=components && target.channels!=15) || inputs[0].channels!=0xe4 ||
                inputs[1].type!=2 || inputs[1].channels!=0xe4 || inputs[1].modifier ||
                inputs[1].index+rows>256) return false;
        }
        if ((opcode==16 || opcode==17) &&
            (inputs[0].channels!=0xe4 || (count==2 && inputs[1].channels!=0xe4))) return false;
        if (requireScopeUv && target.type==6 && target.index==3) {
            if (opcode!=9 || operands[0]!=0x90e40003u) return false;
            if (dest==0xe0010003u && operands[1]==0xa0e40008u && !x) x=true;
            else if (dest==0xe0020003u && operands[1]==0xa0e40009u && !y) y=true;
            else return false; // Any other write/mask/component or repeated UV write declines.
        }
    }
    return false; // Missing terminal END.
}
// Keep the existing scope UV policy as the production reference.
inline bool scopeUvProgram(std::span<const uint32_t> words) {return boundedVertexProgram(words,true);}
// Framing only for offline position replay. Unsupported inputs propagate unknown
// in the existing evaluator; this does not admit a new native shader/image path.
inline bool vertexPositionProgram(std::span<const uint32_t> words) {return boundedVertexProgram(words,false);}
} // namespace ss2vr
