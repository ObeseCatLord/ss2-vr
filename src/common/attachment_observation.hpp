#pragma once
#include "math.hpp"
#include <cstdint>
#include <cstring>
namespace ss2vr {
// Value-only receipt for one EXISTING native attachment call. The IDENT is an
// opaque runtime token. Neither this record nor draw serials certify animation
// equivalence; that requires comparison with the producing draw's own factors.
struct AttachmentObservation {
    uintptr_t model=0;
    uint32_t ident=0,calls=0;
    int result=0;
    Matrix34 absolute{};
    bool copied=false,nested=false,aborted=false;
    void begin(uintptr_t argumentModel,uint32_t argumentIdent) noexcept {
        if(calls!=UINT32_MAX)++calls;
        model=argumentModel;ident=argumentIdent;
    }
    void complete(int returned,const Matrix34 &output) noexcept {
        result=returned;
        if(returned==1 && calls==1 && !nested && !aborted) {
            static_assert(sizeof(absolute)==48);
            std::memcpy(&absolute,&output,sizeof(absolute));copied=true;
        }
    }
    bool admitted(uintptr_t currentModel) const noexcept {
        if(!currentModel || model!=currentModel || calls!=1 || result!=1 ||
           !copied || nested || aborted)return false;
        for(float value:absolute.m)if(!std::isfinite(value))return false;
        return true;
    }
};
}
