// Offline copied-input assessment only: no game, GPU, OpenXR or native hooks.
#include "common/scope_position_program.hpp"
#include "common/idle_geometry.hpp"
#include <fstream>
#include <iostream>
#include <vector>
#include <cstring>
#include <limits>
#include <stdexcept>
using namespace ss2vr;
static_assert(std::endian::native==std::endian::little);
struct Reader {
    std::span<const uint8_t> bytes;
    template<class T> T take() {
        static_assert(std::is_trivially_copyable_v<T>);
        if(bytes.size()<sizeof(T))throw std::runtime_error("truncated-input");
        T value;std::memcpy(&value,bytes.data(),sizeof(T));bytes=bytes.subspan(sizeof(T));return value;
    }
};
static int result(bool accepted,const char *reason,unsigned vertex,double maximum=0) {
    std::cout << "{\"schema\":1,\"position_replay_agrees_with_reference\":" << (accepted?"true":"false")
              << ",\"reason\":\"" << reason << "\",\"vertex\":" << vertex
              << ",\"maximum_absolute_clip_error\":" << maximum
              << ",\"gpu_execution\":false,\"positive_grasp_verified\":false,\"alignment_accepted\":false}\n";
    return 0;
}
int main(int argc,char **argv) {
    try {
        if(argc!=2)throw std::runtime_error("one-private-input-required");
        std::ifstream f(argv[1],std::ios::binary|std::ios::ate);
        const auto size=f.tellg();if(!f || size<0 || size>65536)throw std::runtime_error("input-budget");
        std::vector<uint8_t> bytes(static_cast<size_t>(size));f.seekg(0);
        if(!f.read(reinterpret_cast<char*>(bytes.data()),std::streamsize(bytes.size())))throw std::runtime_error("input-read");
        Reader input{bytes};
        if(input.take<std::array<char,8>>()!=std::array<char,8>{'S','S','2','V','I','R','P','1'})throw std::runtime_error("input-magic");
        const auto schema=input.take<uint32_t>(),words=input.take<uint32_t>(),count=input.take<uint32_t>(),
                   vertices=input.take<uint32_t>(),weights=input.take<uint32_t>();
        if(schema!=1 || words<2 || words>512 || !count || count>256 || !vertices || vertices>IdleGeometryVertices || weights>1)
            throw std::runtime_error("input-bounds");
        const auto clip=input.take<Matrix44>();
        for(float v:clip.m)if(!std::isfinite(v))throw std::runtime_error("nonfinite-reference");
        std::array<uint32_t,512> program{};for(unsigned i=0;i<words;++i)program[i]=input.take<uint32_t>();
        std::array<std::array<float,4>,256> constants{};
        for(unsigned i=0;i<count;++i) {constants[i]=input.take<std::array<float,4>>();
            for(float v:constants[i])if(!std::isfinite(v))throw std::runtime_error("nonfinite-constant");}
        if(!vertexPositionProgram(std::span(program).first(words)))return result(false,"unsupported-program",0);
        // Decode every vertex before evaluating. No ignored trailing or missing
        // bytes can turn a partial assessment into success.
        struct Vertex {Vec3 p;std::array<float,2> uv;};
        static_assert(sizeof(Vertex)==20);
        std::array<Vertex,IdleGeometryVertices> source{};
        for(unsigned i=0;i<vertices;++i) {
            source[i]=input.take<Vertex>();
            const auto influence=input.take<std::array<uint8_t,8>>();
            if(influence!=std::array<uint8_t,8>{255,0,0,0,0,0,0,0})return result(false,"unsupported-influence",i);
            const auto &v=source[i];
            if(!std::isfinite(v.p.x) || !std::isfinite(v.p.y) || !std::isfinite(v.p.z) ||
               !std::isfinite(v.uv[0]) || !std::isfinite(v.uv[1]))throw std::runtime_error("nonfinite-vertex");
        }
        if(!input.bytes.empty())throw std::runtime_error("trailing-input");
        double maximum=0;
        for(unsigned i=0;i<vertices;++i) {
            std::array<float,4> actual{};const auto &v=source[i];
            if(!scope_position::position(std::span(program).first(words),std::span(constants).first(count),v.p,v.uv,weights!=0,actual))
                return result(false,"unknown-position-dependency",i);
            for(unsigned row=0;row<4;++row) {
                const double expected=double(clip.m[row*4])*v.p.x+double(clip.m[row*4+1])*v.p.y+
                                      double(clip.m[row*4+2])*v.p.z+clip.m[row*4+3];
                const double error=std::abs(double(actual[row])-expected);
                if(!std::isfinite(expected) || !std::isfinite(actual[row]) ||
                   error>1e-5+1e-5*std::max(std::abs(double(actual[row])),std::abs(expected)))
                    return result(false,"projection-mismatch",i,error);
                maximum=std::max(maximum,error);
            }
        }
        return result(true,"all-copied-vertices-agree",vertices,maximum);
    } catch(const std::exception &e) {
        std::cerr << e.what() << "\n";return 2;
    }
}
