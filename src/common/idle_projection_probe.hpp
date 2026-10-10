#pragma once
#include <array>
#include <cstdint>
#include <span>

namespace ss2vr {
// Invocation-local chronology only; never an owner, producer selector or
// reference. Capacity loss remains explicit instead of choosing passing rows.
struct IdleProjectionOpportunities {
    struct Row {
        uint32_t ordinal=0,kind=0,source=0,passStage=0,traceStage=0;
        uint32_t poseCopied=0,exactDraw=0,oldGate=0;
    };
    static constexpr unsigned MaxRows=64;
    std::array<Row,MaxRows> rows{};
    uint32_t count=0,ordinal=0,rootQuery=0,palette=0,submission=0;
    bool overflow=false;
    uint32_t next() noexcept {
        if(ordinal==UINT32_MAX) {overflow=true;return 0;}
        return ++ordinal;
    }
    void mark(uint32_t &first) noexcept {
        const auto value=next();if(!first)first=value;
    }
    void observe(uint32_t kind,uint32_t source,uint32_t pass,uint32_t stage,
                 bool pose,bool exact,bool gate) noexcept {
        const auto value=next();
        if(!value || count>=MaxRows || kind>1 || source>2 || stage>4) {overflow=true;return;}
        rows[count++]={value,kind,source,pass,stage,unsigned(pose),unsigned(exact),unsigned(gate)};
    }
};
// Diagnostic values only. Pointer-shaped words are comparison keys, never
// retained borrows or independent projection inputs. No native state is written.
struct IdleProjectionSnapshot {
    uint32_t control, flags, modelRecord, drawRecord;
    uint32_t model[12],view[12],projection[16],cachedVP[16],cachedMVP[16];
    bool operator==(const IdleProjectionSnapshot &) const = default;
};
struct IdleProjectionPair {
    IdleProjectionSnapshot before{}, after{};
    uint32_t sequence=0,source=1;
    bool complete=false;
    bool sameOperands() const noexcept {
        if(before.modelRecord!=after.modelRecord || before.drawRecord!=after.drawRecord)return false;
        for(unsigned i=0;i<12;++i)
            if(before.model[i]!=after.model[i] || before.view[i]!=after.view[i])return false;
        for(unsigned i=0;i<16;++i)if(before.projection[i]!=after.projection[i])return false;
        return true;
    }
    bool qualified() const noexcept {
        // Reserved precision encodings are reported raw, not interpreted. The
        // matching whole-function bookends establish actual production/reuse.
        if(!(complete && sequence && (source==1 || source==2) && before.modelRecord && before.drawRecord &&
            before.control<=0xffff && before.control==after.control &&
            sameOperands() && after.flags==(before.flags|6)))return false;
        for(unsigned i=0;i<16;++i) {
            if((before.flags&2) && before.cachedVP[i]!=after.cachedVP[i])return false;
            if((before.flags&4) && before.cachedMVP[i]!=after.cachedMVP[i])return false;
        }
        return true;
    }
    bool operator==(const IdleProjectionPair &) const = default;
};
struct IdleProjectionProbe {
    static constexpr unsigned MaxPairs=8;
    std::array<IdleProjectionPair,MaxPairs> pairs{};
    unsigned count=0;
    uint32_t invalidations=0;
    bool pending=false, blocked=false, helperActive=false, fogActive=false;
    bool enterHelper() noexcept {
        if(blocked || pending || helperActive || fogActive) {invalidate();return false;}
        helperActive=true;return true;
    }
    void leaveHelper(bool aborted) noexcept {helperActive=false;if(aborted)invalidate();}
    bool enterFog(uint32_t source=1) noexcept {
        if(blocked || !pending || helperActive || fogActive || count>=MaxPairs ||
           (source!=1 && source!=2) || pairs[count].source!=source) {invalidate();return false;}
        fogActive=true;return true;
    }
    void leaveFog(bool aborted) noexcept {fogActive=false;if(aborted)invalidate();}
    // Reserve before measuring: nothing after the pre-production snapshot
    // needs allocation, callbacks, interpretation or an owner lookup.
    IdleProjectionSnapshot *begin(uint32_t source=1) noexcept {
        if(pending || blocked || helperActive || fogActive || count>=MaxPairs || (source!=1 && source!=2)) {
            invalidate();return nullptr;
        }
        pending=true;pairs[count].complete=false;pairs[count].sequence=count+1;pairs[count].source=source;
        return &pairs[count].before;
    }
    void end(const IdleProjectionSnapshot &sample,bool originalCompleted) noexcept {
        if(!pending || blocked || count>=MaxPairs || !originalCompleted) {
            invalidate();return;
        }
        auto &p=pairs[count];p.after=sample;p.complete=true;pending=false;
        if(!p.qualified()) {invalidate();return;}
        ++count;
    }
    // Exact latest observed producer, never a search by reused address or by
    // arithmetic success. Call at each native/API submission bookend.
    uint32_t currentSequence(const IdleProjectionSnapshot &now,uint32_t model,
                             uint32_t draw,std::span<const uint32_t,12> world) const noexcept {
        if(blocked || pending || helperActive || fogActive || !count || count>MaxPairs)return 0;
        const auto &p=pairs[count-1];
        if(p.source!=1 || !p.qualified() || p.after!=now ||
           now.modelRecord!=model || now.drawRecord!=draw)return 0;
        for(unsigned i=0;i<12;++i)if(now.model[i]!=world[i])return 0;
        return p.sequence;
    }
    void invalidate() noexcept {
        pending=false;blocked=true;
        if(invalidations!=0xffffffffu)++invalidations;
    }
    void retire() noexcept {
        if(pending || helperActive || fogActive)invalidate();
        helperActive=fogActive=false;
    }
};
} // namespace ss2vr
