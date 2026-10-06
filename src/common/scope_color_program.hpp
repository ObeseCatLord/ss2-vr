#pragma once
#include "scope_program.hpp"
#include <bit>
#include <cmath>

namespace ss2vr {
namespace scope_color_program {
struct Register {
    unsigned type=0,index=0,channels=0,modifier=0;
};
inline bool destination(uint32_t word,Register &out) {
    constexpr uint32_t allowed=0x80000000u|0x70001800u|0x7ffu|0xf0000u|0xf00000u;
    if (!(word&0x80000000u) || (word&~allowed)) return false;
    out={scope_program::type(word),word&0x7ffu,(word>>16)&15,(word>>20)&15};
    return out.modifier<=3; // NONE, SAT, PP, SAT|PP; never centroid/shift/addressing.
}
inline bool source(uint32_t word,Register &out) {
    constexpr uint32_t allowed=0x80000000u|0x70001800u|0x7ffu|0xff0000u|0xf000000u;
    if (!(word&0x80000000u) || (word&~allowed)) return false;
    out={scope_program::type(word),word&0x7ffu,(word>>16)&255,(word>>24)&15};
    return out.modifier<=1 && (out.channels==0xe4 || out.channels==0 ||
        out.channels==0x55 || out.channels==0xaa || out.channels==0xff);
}
inline unsigned consumed(const Register &reg,unsigned lanes) {
    unsigned mask=0;
    for(unsigned lane=0;lane<4;++lane) if(lanes&(1u<<lane)) mask|=1u<<((reg.channels>>(2*lane))&3);
    return mask;
}
inline bool same(const Register &a,const Register &b) { return a.type==b.type && a.index==b.index; }
} // namespace scope_color_program

// Deliberately narrow PS2.0 property predicate, not full shader validation or
// stock-source identity. Proves one final unconditional defined RGBA output,
// no fragment kill, explicit/implicit depth output or MRT. Unknowns decline.
inline bool scopeColorProgram(std::span<const uint32_t> words) {
    using namespace scope_color_program;
    if(words.size()<2 || words.size()>ScopeProgramMaxWords || words.front()!=0xffff0200u) return false;
    std::array<unsigned,12> initialized{};
    std::array<unsigned,8> texture{};
    std::array<bool,16> sampler{};
    std::array<bool,32> definitions{};
    bool body=false,output=false;
    size_t cursor=1;
    auto defined=[&](const Register &reg,unsigned lanes) {
        const unsigned needed=consumed(reg,lanes);
        switch(reg.type) {
        case 0: return reg.index<initialized.size() && (initialized[reg.index]&needed)==needed;
        case 2: return reg.index<32; // Declared/device float constants are external inputs.
        case 3: return reg.index<texture.size() && (texture[reg.index]&needed)==needed;
        default: return false;
        }
    };
    while(cursor<words.size()) {
        const uint32_t instruction=words[cursor++];
        if(instruction==0xffffu) return cursor==words.size() && output;
        if((instruction&0xffffu)==0xfffeu) {
            if(instruction&0x80000000u) return false;
            const size_t count=(instruction>>16)&0x7fff;
            if(count>words.size()-cursor) return false;
            cursor+=count; // Opaque COMMENT; END/kills in its payload have no effects.
            continue;
        }
        if(output) return false; // Output MOV is the final executable instruction.
        const unsigned opcode=instruction&0xffffu;
        unsigned length=0,sources=0;
        switch(opcode) {
        case 1: case 36: length=2; sources=1; break;
        case 5: case 8: case 32: case 66: length=3; sources=2; break;
        case 4: length=4; sources=3; break;
        case 31: length=2; break;
        case 81: length=5; break;
        default: return false; // Includes kill, legacy depth, flow, PHASE and unknown encodings.
        }
        if(instruction!=(opcode|(length<<24)) || words.size()-cursor<length) return false;
        if(opcode==31) {
            if(body) return false;
            const uint32_t semantic=words[cursor++];
            Register reg;
            if(!destination(words[cursor++],reg)) return false;
            if(reg.type==3) {
                if(semantic!=0x80000000u || reg.index>=texture.size() || !reg.channels ||
                    (reg.modifier!=0 && reg.modifier!=2) || texture[reg.index]) return false;
                texture[reg.index]=reg.channels;
            } else if(reg.type==10) {
                if(semantic!=0x90000000u || reg.index>=sampler.size() || reg.modifier ||
                    (reg.channels!=0 && reg.channels!=15) || sampler[reg.index]) return false;
                sampler[reg.index]=true;
            } else return false;
            continue;
        }
        if(opcode==81) {
            if(body) return false;
            Register reg;
            if(!destination(words[cursor++],reg) || reg.type!=2 || reg.index>=definitions.size() ||
                reg.channels!=15 || reg.modifier || definitions[reg.index]) return false;
            definitions[reg.index]=true;
            for(unsigned i=0;i<4;++i) if(!std::isfinite(std::bit_cast<float>(words[cursor++]))) return false;
            continue;
        }
        body=true;
        Register target;
        if(!destination(words[cursor++],target) || !target.channels) return false;
        std::array<Register,3> input{};
        for(unsigned i=0;i<sources;++i) if(!source(words[cursor++],input[i])) return false;
        if(target.type==8) {
            if(opcode!=1 || target.index || target.channels!=15 || (target.modifier!=0 && target.modifier!=2) ||
                input[0].type!=0 || input[0].channels!=0xe4 || input[0].modifier || !defined(input[0],15)) return false;
            output=true;
            continue;
        }
        if(target.type!=0 || target.index>=initialized.size()) return false;
        if(opcode==66) {
            if(target.channels!=15 || (target.modifier!=0 && target.modifier!=2) ||
                input[0].type!=3 || input[0].channels!=0xe4 || input[0].modifier || !defined(input[0],3) ||
                input[1].type!=10 || input[1].index>=sampler.size() || !sampler[input[1].index] ||
                input[1].channels!=0xe4 || input[1].modifier) return false;
        } else {
            const unsigned lanes=opcode==8 || opcode==36 ? 7 : opcode==32 ? 1 : target.channels;
            if(opcode==36 && (target.channels!=7 || input[0].channels!=0xe4 || same(target,input[0]))) return false;
            if(opcode==32 && (input[0].channels==0xe4 || input[1].channels==0xe4 || same(target,input[1]))) return false;
            for(unsigned i=0;i<sources;++i) if(!defined(input[i],lanes)) return false;
        }
        // All source reads precede writes, including self-modifying arithmetic.
        initialized[target.index]|=target.channels;
    }
    return false;
}
} // namespace ss2vr
