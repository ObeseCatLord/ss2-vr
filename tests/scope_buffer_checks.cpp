#ifdef NDEBUG
#error Scope buffer verification requires assertions enabled
#endif
#include "common/scope_buffer_layout.hpp"
#include "common/scope_lock.hpp"
#include <cassert>
using namespace ss2vr;
int main() {
    ScopeBufferInputs stock;
    stock.surface = {884,928, {{{0,0x85,0},{0,0x87,0},{151520,0x80,0},{155056,0x80,0}}}};
    stock.draw = {4,0,0,884,0,928};
    stock.positions = {1,0,12,1};
    stock.localIndices = {1,155056,4,1};
    stock.weights = {1,151520,4,1};
    stock.uv = {1,181824,8,1};
    stock.vertex = {212128,0,1,100,0};
    stock.index = {19038,0,1,101,0};
    stock.indexObject = 2;
    const ScopeDeclarationElement end{0xff,0,17,0,0,0};
    std::array<ScopeDeclarationElement,5> declaration{{{0,0,2,0,5,0},{5,0,8,0,5,5},
                                                     {3,0,1,0,5,3},{6,0,8,0,5,6},end}};
    ScopeCopyRanges ranges;
    assert(scopeBufferRanges(stock,declaration,ranges) && ranges.weightsActive);
    assert((ranges.slices == std::array<ScopeByteRange,5>{{{0,10608},{0,5568},{151520,3536},{155056,3536},{181824,7072}}}));
    // Single-weight programs omit stream 6; source weights still require a
    // fingerprint, and a stale stream-6 binding must not become active input.
    auto single = declaration;
    single[3] = end;
    auto singleInput = stock;
    singleInput.weights = {3,17,999,0};
    assert(scopeBufferRanges(singleInput,std::span(single).first(4),ranges) && !ranges.weightsActive);
    assert(ranges.slices[2].size == 3536);
    auto unused = declaration;
    unused[3].type = 17;
    assert(scopeBufferRanges(singleInput,unused,ranges) && !ranges.weightsActive);
    std::array<ScopeDeclarationElement,6> duplicateUnused{declaration[0],declaration[1],declaration[2],unused[3],declaration[3],end};
    assert(!scopeBufferRanges(stock,duplicateUnused,ranges));
    for (uint8_t semantic : {uint8_t(0),uint8_t(3),uint8_t(5),uint8_t(6)}) {
        std::array<ScopeDeclarationElement,6> collision{declaration[0],declaration[1],declaration[2],
            declaration[3],ScopeDeclarationElement{7,0,2,0,5,semantic},end};
        assert(!scopeBufferRanges(stock,collision,ranges));
    }
    auto reject = [&](const ScopeBufferInputs &bad) {
        ranges.weightsActive = true;
        ranges.slices[0] = {1,2};
        assert(!scopeBufferRanges(bad,declaration,ranges));
        assert(!ranges.weightsActive && ranges.slices[0].size == 0);
    };
    auto bad = stock; bad.vertex.usage = 8; reject(bad); // WRITEONLY
    bad = stock; bad.vertex.usage = 0x200; reject(bad); // DYNAMIC
    bad = stock; bad.index.usage = 0x218; reject(bad); // Ring fallback
    bad = stock; bad.vertex.pool = 0; reject(bad);
    bad = stock; bad.vertex.format = 101; reject(bad);
    bad = stock; bad.index.format = 102; reject(bad);
    bad = stock; bad.vertex.size = UINT32_MAX; reject(bad);
    bad = stock; bad.index.size = 5567; reject(bad);
    bad = stock; bad.positions.frequency = 0x40000001; reject(bad); // Instancing
    bad = stock; bad.weights.object = 3; reject(bad);
    bad = stock; bad.localIndices.object = 3; reject(bad);
    bad = stock; bad.positions.stride = 16; reject(bad);
    bad = stock; bad.weights.offset = 151524; reject(bad);
    bad = stock; bad.uv.object = 3; reject(bad);
    bad = stock; bad.uv.offset = 181828; reject(bad);
    bad = stock; bad.uv.stride = 12; reject(bad);
    bad = stock; bad.uv.frequency = 0x40000001; reject(bad);
    bad = stock; bad.vertex.size = 188895; reject(bad);
    bad = stock; bad.softwarePositions = true; reject(bad);
    bad = stock; bad.surface.channels[0].format = 3; reject(bad);
    bad = stock; bad.surface.channels[3].buffer = 1; reject(bad);
    bad = stock; bad.draw.start = 64; reject(bad);
    bad = stock; bad.draw.base = -1; reject(bad);
    bad = stock; bad.draw.vertices = 885; reject(bad);
    bad = stock; bad.draw.primitives = 22; reject(bad);
    auto badDecl = declaration; badDecl[1].usageIndex = 6;
    assert(!scopeBufferRanges(stock,badDecl,ranges));
    badDecl = declaration; badDecl[1].type = 5;
    assert(!scopeBufferRanges(stock,badDecl,ranges));
    badDecl = declaration; badDecl[2] = badDecl[1];
    assert(!scopeBufferRanges(stock,badDecl,ranges));
    assert(!scopeBufferRanges(stock,std::span(declaration).first(3),ranges));
    badDecl = declaration; badDecl[2] = end;
    assert(!scopeBufferRanges(stock,badDecl,ranges));
    badDecl = declaration; badDecl[2].type = 2;
    assert(!scopeBufferRanges(stock,badDecl,ranges));
    badDecl = declaration; badDecl[2].offset = 4;
    assert(!scopeBufferRanges(stock,badDecl,ranges));
    badDecl = declaration; badDecl[2].method = 1;
    assert(!scopeBufferRanges(stock,badDecl,ranges));
    badDecl = declaration; badDecl[2].usageIndex = 0;
    assert(!scopeBufferRanges(stock,badDecl,ranges));
    badDecl = declaration; badDecl[2].type = 17;
    assert(!scopeBufferRanges(stock,badDecl,ranges));
    std::array<ScopeDeclarationElement,6> duplicateUv{declaration[0],declaration[1],declaration[2],
                                                    declaration[2],declaration[3],end};
    assert(!scopeBufferRanges(stock,duplicateUv,ranges));
    duplicateUv[3].stream = 4; // Conflicting TEXCOORD3 on a different stream.
    assert(!scopeBufferRanges(stock,duplicateUv,ranges));
    assert(scopeByteRange(181824,7072,212128));
    assert(!scopeByteRange(181824,7072,188895));
    assert(scopeByteRange(155056,3536,212128));
    assert(!scopeByteRange(UINT32_MAX-2,8,UINT32_MAX));
    assert(!scopeByteRange(0,0,212128));
    ScopeLockLedger lock;
    assert(lock.beginLock() && lock.outstanding() && lock.uncertain());
    assert(!lock.beginLock() && !lock.beginUnlock());
    lock.finishLock(false); // Failed Lock owes no Unlock.
    assert(!lock.outstanding());
    assert(lock.beginLock()); lock.finishLock(true);
    assert(lock.outstanding() && !lock.uncertain()); // Includes success+null.
    assert(lock.beginUnlock() && lock.uncertain());
    assert(!lock.beginUnlock()); // Native unwind/failed Unlock never retried.
    lock.finishUnlock(false);
    assert(lock.uncertain() && !lock.beginLock() && !lock.beginUnlock());
    ScopeLockLedger completed;
    assert(completed.beginLock()); completed.finishLock(true);
    assert(completed.beginUnlock()); completed.finishUnlock(true);
    assert(!completed.outstanding() && completed.beginLock());
    for (unsigned failure=0;failure<5;++failure) {
        ScopeLockLedger sequential;
        for (unsigned slice=0;slice<5;++slice) {
            assert(sequential.beginLock());
            sequential.finishLock(slice!=failure);
            if (slice==failure) { assert(!sequential.outstanding()); break; }
            assert(sequential.beginUnlock()); sequential.finishUnlock(true);
            assert(!sequential.outstanding());
        }
    }
    ScopeLockLedger fifthUnwind;
    for (unsigned slice=0;slice<4;++slice) {
        assert(fifthUnwind.beginLock()); fifthUnwind.finishLock(true);
        assert(fifthUnwind.beginUnlock()); fifthUnwind.finishUnlock(true);
    }
    assert(fifthUnwind.beginLock() && fifthUnwind.uncertain()); // Unwound fifth Lock cannot retry.
    assert(!fifthUnwind.beginLock() && !fifthUnwind.beginUnlock());
    unsigned acquisitions=0, forwarded=0;
    const auto attempt=[&] { ++acquisitions; return 17; };
    const auto native=[&] { ++forwarded; return 23; };
    const std::array<bool,6> allHooks{true,true,true,true,true,true};
    for (size_t missing=0;missing<allHooks.size();++missing) {
        auto installed=allHooks; installed[missing]=false;
        const bool complete=std::all_of(installed.begin(),installed.end(),[](bool hooked) { return hooked; });
        assert(withScopeDrawCoverage(complete,attempt,native)==23);
    }
    assert(acquisitions==0 && forwarded==allHooks.size());
    assert(withScopeDrawCoverage(true,attempt,native)==17 && acquisitions==1 && forwarded==allHooks.size());
}
