#pragma once
#include "model_tree.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <span>

namespace ss2vr {
constexpr bool nativeIdleQueryBorrow(uintptr_t caller,uintptr_t expectedReturn,uintptr_t active,uintptr_t queue) noexcept {
    return queue && caller==expectedReturn && active==queue;
}
// Copied diagnostic values only. Native pointers never escape as usable borrows.
struct IdleDrawIdentity {
    uint64_t request=0,input=0;
    uint32_t owner=0,weapon=0,model=0,generation=0;
    unsigned hand=2,eye=2;
    bool operator==(const IdleDrawIdentity&) const = default;
};
struct IdleConfigIdentity {
    uint32_t configuration=0,file=0;
    int32_t resource=-1;
    bool operator==(const IdleConfigIdentity&) const = default;
};
struct IdleAnimationValue {
    std::array<uint32_t,8> contribution{};
    std::array<uint32_t,4> header{}; // IDENT, first/last frame, raw speed bits; not CResource.
};
struct IdleWeaponTrace {
    static constexpr unsigned MaxContributors=16,MaxMatrices=64;
    enum class Stage : uint8_t { Empty,Event,Palette,Rejected,Complete };
    Stage stage=Stage::Empty;
    IdleDrawIdentity binding{};
    IdleConfigIdentity config{};
    std::array<IdleAnimationValue,MaxContributors> animations{};
    std::array<Matrix34,MaxMatrices> matrices{};
    Matrix34 world{};
    Matrix34 nativePlacement{},trackedPlacement{},controller{};
    Vec3 stretch{};
    bool admitted=false,placementObserved=false,poseCopied=false;
    unsigned contributors=0,animationsCopied=0,matrixCount=0;

    void reject() noexcept {stage=Stage::Rejected;} // Scalar abnormal retirement.
    bool admit(const IdleDrawIdentity &identity) noexcept {
        if(admitted || stage!=Stage::Empty || !identity.request || !identity.input || !identity.owner ||
           !identity.weapon || !identity.model || !identity.generation || identity.hand>=2 || identity.eye>=2) {
            reject();return false;
        }
        binding=identity;admitted=true;return true;
    }
    bool placement(const IdleDrawIdentity &identity,const Matrix34 &native,const Matrix34 &tracked,
                   const Matrix34 &physicalController) noexcept {
        if(!admitted || stage!=Stage::Empty || binding!=identity || placementObserved ||
           !finiteMatrix(native) || !finiteMatrix(tracked) || !finiteMatrix(physicalController)) {
            reject();return false;
        }
        nativePlacement=native;trackedPlacement=tracked;controller=physicalController;
        placementObserved=true;return true;
    }
    bool event(const IdleDrawIdentity &identity,const IdleConfigIdentity &resource,
               bool callerQueueCurrent,int count) noexcept {
        if(!admitted || binding!=identity || stage!=Stage::Empty || !callerQueueCurrent || count<=0 || unsigned(count)>MaxContributors) {
            reject();return false;
        }
        config=resource;contributors=unsigned(count);stage=Stage::Event;return true;
    }
    bool palette(const IdleDrawIdentity &identity,const IdleConfigIdentity &resource,
                 bool evaluatedCurrent,int count) noexcept {
        if(stage!=Stage::Event || animationsCopied!=contributors || binding!=identity || config!=resource || !evaluatedCurrent ||
           count<=0 || unsigned(count)>MaxMatrices) {reject();return false;}
        matrixCount=unsigned(count);stage=Stage::Palette;return true;
    }
    bool animation(unsigned index,const IdleAnimationValue &value) noexcept {
        if(stage!=Stage::Event || index!=animationsCopied || index>=contributors) {reject();return false;}
        animations[index]=value;++animationsCopied;return true;
    }
    bool pose(const Matrix34 &transform,Vec3 scale,std::span<const Matrix34> source) noexcept {
        if(stage!=Stage::Palette || source.size()!=matrixCount || !finiteMatrix(transform) ||
           !std::isfinite(scale.x) || !std::isfinite(scale.y) || !std::isfinite(scale.z)) {reject();return false;}
        for(const auto &m:source)if(!finiteMatrix(m)) {reject();return false;}
        world=transform;stretch=scale;std::copy(source.begin(),source.end(),matrices.begin());poseCopied=true;return true;
    }
    bool finish(bool nativeCompleted,bool generationCurrent) noexcept {
        if(stage!=Stage::Palette || !poseCopied || !placementObserved || !nativeCompleted || !generationCurrent) {reject();return false;}
        stage=Stage::Complete;return true;
    }
};
} // namespace ss2vr
