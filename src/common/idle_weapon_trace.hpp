#pragma once
#include "model_tree.hpp"
#include "idle_geometry.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <initializer_list>
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
// Independent primitive operands, copied only at the already validated raster
// borrow. These diagnostics never participate in ordinary raster admission.
struct IdleRasterFactors {
    Matrix34 model{},local{},view{};
    Matrix44 projection{};
    uint32_t paletteIndex=0;
    bool modelCopied=false,cameraCopied=false;
    bool valid() const noexcept {
        if(!modelCopied || !cameraCopied || paletteIndex>=32768 ||
           !finiteMatrix(model) || !finiteMatrix(local) || !finiteMatrix(view))return false;
        for(float v:projection.m)if(!std::isfinite(v))return false;
        return true;
    }
    bool same(const IdleRasterFactors &other) const noexcept {
        if(!valid() || !other.valid() || paletteIndex!=other.paletteIndex)return false;
        const auto bits=[](const auto &a,const auto &b) {
            for(unsigned i=0;i<std::size(a.m);++i)
                if(std::bit_cast<uint32_t>(a.m[i])!=std::bit_cast<uint32_t>(b.m[i]))return false;
            return true;
        };
        return bits(model,other.model) && bits(local,other.local) &&
               bits(view,other.view) && bits(projection,other.projection);
    }
    template<class Emit> void emitMatrices(Emit &&emit) const {
        emit("factorModel",std::span(model.m));emit("factorLocal",std::span(local.m));
        emit("factorView",std::span(view.m));emit("factorProjection",std::span(projection.m));
    }
};
struct IdleRasterCopy {
    IdleDrawIdentity binding{};
    IdleConfigIdentity rootConfig{},renderConfig{};
    uint32_t modelRecord=0,drawRecord=0,surface=0,instance=0,surfaceName=0,boneName=0;
    int32_t bone=-1;
    ScopeSurfaceLayout layout{};
    Matrix34 affine{};
    Matrix44 clip{};
    bool clipValid=false;
    IdleRasterFactors factors{}; // Deliberately excluded from operator== below.
    bool operator==(const IdleRasterCopy &other) const noexcept {
        return binding==other.binding && rootConfig==other.rootConfig && renderConfig==other.renderConfig &&
            modelRecord==other.modelRecord && drawRecord==other.drawRecord && surface==other.surface &&
            instance==other.instance && surfaceName==other.surfaceName && boneName==other.boneName && bone==other.bone &&
            layout==other.layout && clipValid==other.clipValid &&
            std::equal(std::begin(clip.m),std::end(clip.m),std::begin(other.clip.m)) && std::equal(std::begin(affine.m),std::end(affine.m),std::begin(other.affine.m));
    }
};
struct IdleGeometryCopy {
    static constexpr unsigned MaxProgramWords=512,MaxConstants=256;
    IdleRasterCopy raster{};
    ScopeBufferInputs inputs{};
    std::array<std::array<uint8_t,32>,5> hashes{};
    std::array<uint32_t,MaxProgramWords> program{};
    std::array<std::array<float,4>,MaxConstants> constants{};
    std::array<ScopeDeclarationElement,65> declaration{};
    unsigned declarationCount=0;
    unsigned words=0,constantCount=0;
    uint32_t factorChecks=0; // 1: pre-original bookend, 2: post-original bookend.
    void factorBookend(const IdleRasterCopy &now,bool postOriginal) noexcept {
        const uint32_t bit=postOriginal?2u:1u;
        if(raster==now && raster.factors.same(now.factors))factorChecks|=bit;
        else factorChecks&=~bit;
    }
    bool factorsAvailable() const noexcept {return factorChecks==3 && raster.factors.valid();}
};
struct IdleWeaponTrace {
    static constexpr unsigned MaxContributors=16,MaxMatrices=64,MaxDraws=8;
    static constexpr unsigned CopyLayout=1; // Per-trace qualified-copy ordinals, not native record identity.
    enum class Stage : uint8_t { Empty,Event,Palette,Rejected,Complete };
    // Invocation-local scalars only; first rejection survives later cleanup.
    enum class Rejection : uint8_t { None,Unspecified,Admit,Placement,References,Event,Palette,Animation,Pose,PoseMatrix,Draw,DrawDuplicate,DrawConstants,Finish,
        QueryBorrow,QueryMemory,QueryConfig,QueryEntries,QueryAnimationMemory,QueryAnimationName,QueryCurrent,QueryAbort,
        PaletteRecords,PaletteRootConfig,PaletteEvaluated,PaletteMatrices,PaletteCurrent,PaletteAbort,
        RasterPrerequisites,RasterMapping,RasterClip,CollectDevice,CollectInputs,CollectRanges,CollectProgram,CollectSlice,CollectHash,
        CollectRebind,CollectChanged,CollectRaster,CollectLifetime,GpuAdmission,GpuAbort,GpuRetired,GpuReentry,NestedGun,GunAbort };
    // QuerySeen includes unrelated child queries; QueryRootSeen requires the
    // existing typed queue borrow and matching selected model instance.
    enum Callback : uint32_t { QuerySeen=1,PaletteSeen=2,RasterSeen=4,GeometrySeen=8,FinishSeen=16,QueryRootSeen=32 };
    Stage stage=Stage::Empty,precedingStage=Stage::Empty;
    Rejection rejection=Rejection::None;
    uint32_t callbacks=0,rejectionChecks=0,rejectionState=0;
    struct InputFailure {
        uint32_t step=0,index=0,valid=0,caps=0,declarationCount=0,rangeChecks=0;
        int32_t hresult=0;
        GeometryInputLayout layout=GeometryInputLayout::Legacy56;
        ScopeBufferInputs inputs{};
        std::array<ScopeDeclarationElement,65> declaration{};
    } inputFailure{};
    // Already copied at the original typed query boundary. A name rejection
    // is diagnostic data, not a completed/winning animation or cache receipt.
    struct AnimationNameFailure {
        uint32_t index=0,expected=0;
        std::array<uint32_t,4> header{};
        bool copied=false;
    } animationNameFailure{};
    struct StreamSnapshot {
        enum Status : uint32_t { Missing, Copied, Failed, Interrupted };
        uint32_t status=Missing,step=0,index=0,caps=0,declarationCount=0;
        int32_t hresult=0;
        uint32_t declarationObject=0,indexObject=0,shaderObject=0;
        std::array<ScopeStreamInput,3> streams{}; // Selected actual numbers, roles unresolved.
        std::array<ScopeDeclarationElement,65> declaration{};
        std::array<std::array<uint32_t,4>,256> constants{};
    };
    struct StreamProbe {
        enum Flag : uint32_t { BeforeCopied=1,BeforeCurrent=2,ForwardCalled=4,ForwardReturned=8,ForwardSucceeded=16,
            AfterCopied=32,SameInputs=64,AfterCurrent=128,CleanupCurrent=256 };
        bool selected=false;
        uint32_t attempts=0,flags=0,words=0,invalidations=0;
        int32_t forwardResult=0;
        std::array<StreamSnapshot,2> snapshots{};
        std::array<uint32_t,IdleGeometryCopy::MaxProgramWords> program{};
    } streamProbe{};
    // Select passive diagnostics from the immutable original rejection, never
    // either resampled declaration. Reuse the exact ID1 declaration predicates.
    static std::array<unsigned,3> passiveStreamNumbers(const InputFailure &d) noexcept {
        if(d.step!=20 || d.valid!=15 || d.declarationCount>65)return {};
        const auto rows=std::span(d.declaration).first(d.declarationCount);
        if(observed78Declaration(rows))return {0,7,8};
        // Observed ID1 material pass: additional FLOAT2 streams 4/5 are not
        // copied or interpreted. This selects only the existing passive 0/7/8
        // observation after rejection; production geometry grammar is unchanged.
        static constexpr std::array<ScopeDeclarationElement,8> multiUv{{
            {0,0,2,0,5,0},{2,0,1,0,5,2},{3,0,1,0,5,3},
            {4,0,1,0,5,4},{5,0,1,0,5,5},{7,0,8,0,5,7},
            {8,0,8,0,5,8},{255,0,17,0,0,0}}};
        if(rows.size()==multiUv.size() && std::equal(rows.begin(),rows.end(),multiUv.begin()))return {0,7,8};
        if(noUV56Declaration(rows))return {0,5,6};
        return {};
    }
    static bool observedStreamFamily(const InputFailure &d) noexcept {
        return passiveStreamNumbers(d)[1]!=0;
    }
    static bool sameStreamSnapshots(const StreamSnapshot &a,const StreamSnapshot &b) noexcept {
        return a.status==StreamSnapshot::Copied && b.status==StreamSnapshot::Copied &&
            a.caps && a.caps<=256 && a.caps==b.caps && a.declarationCount &&
            a.declarationCount<=65 && a.declarationCount==b.declarationCount &&
            a.declarationObject && a.declarationObject==b.declarationObject &&
            a.indexObject && a.indexObject==b.indexObject && a.shaderObject && a.shaderObject==b.shaderObject &&
            a.streams==b.streams && a.declaration==b.declaration && a.constants==b.constants;
    }
    static constexpr uint32_t checks(std::initializer_list<bool> values) noexcept {
        uint32_t result=0,bit=1;for(bool value:values){if(value)result|=bit;bit<<=1;}return result;
    }
    IdleDrawIdentity binding{};
    IdleConfigIdentity config{};
    std::array<IdleAnimationValue,MaxContributors> animations{};
    std::array<Matrix34,MaxMatrices> matrices{};
    std::array<IdleGeometryCopy,MaxDraws> geometry{};
    unsigned draws=0;
    Matrix34 world{};
    Matrix34 nativePlacement{},trackedPlacement{},controller{},rawGrip{},rawAim{};
    bool referencesCopied=false,rawGripValid=false;
    Vec3 stretch{};
    bool admitted=false,placementObserved=false,poseCopied=false;
    unsigned contributors=0,animationsCopied=0,matrixCount=0;

    void noteRejection(Rejection reason,uint32_t passedChecks=0) noexcept {
        if(rejection==Rejection::None) {
            rejection=reason;precedingStage=stage;rejectionChecks=passedChecks;
            rejectionState=checks({admitted,placementObserved,referencesCopied,poseCopied,rawGripValid,animationsCopied==contributors});
        }
    }
    void reject(Rejection reason=Rejection::Unspecified,uint32_t passedChecks=0) noexcept {
        noteRejection(reason,passedChecks);stage=Stage::Rejected;
    }
    void rejectAnimationName(unsigned index,uint32_t expected,const IdleAnimationValue &value) noexcept {
        if(rejection==Rejection::None && stage==Stage::Event && index==animationsCopied &&
           index<contributors && value.header[0]!=expected)
            animationNameFailure={index,expected,value.header,true};
        reject(Rejection::QueryAnimationName);
    }
    bool admit(const IdleDrawIdentity &identity) noexcept {
        if(admitted || stage!=Stage::Empty || !identity.request || !identity.input || !identity.owner ||
           !identity.weapon || !identity.model || !identity.generation || identity.hand>=2 || identity.eye>=2) {
            reject(Rejection::Admit);return false;
        }
        binding=identity;admitted=true;return true;
    }
    bool placement(const IdleDrawIdentity &identity,const Matrix34 &native,const Matrix34 &tracked,
                   const Matrix34 &physicalController) noexcept {
        if(!admitted || stage!=Stage::Empty || binding!=identity || placementObserved ||
           !finiteMatrix(native) || !finiteMatrix(tracked) || !finiteMatrix(physicalController)) {
            reject(Rejection::Placement);return false;
        }
        nativePlacement=native;trackedPlacement=tracked;controller=physicalController;
        placementObserved=true;return true;
    }
    bool references(const IdleDrawIdentity &identity,const Matrix34 &grip,bool gripAvailable,const Matrix34 &aim) noexcept {
        if(!admitted || binding!=identity || stage!=Stage::Empty || !placementObserved || referencesCopied ||
           !finiteMatrix(aim) || (gripAvailable && !finiteMatrix(grip))) {reject(Rejection::References);return false;}
        rawAim=aim;rawGripValid=gripAvailable;if(gripAvailable)rawGrip=grip;
        referencesCopied=true;return true;
    }
    bool event(const IdleDrawIdentity &identity,const IdleConfigIdentity &resource,
               bool callerQueueCurrent,int count) noexcept {
        if(!admitted || binding!=identity || stage!=Stage::Empty || !callerQueueCurrent || count<=0 || unsigned(count)>MaxContributors) {
            reject(Rejection::Event,checks({admitted,binding==identity,stage==Stage::Empty,callerQueueCurrent,count>0,count>0 && unsigned(count)<=MaxContributors}));return false;
        }
        config=resource;contributors=unsigned(count);stage=Stage::Event;return true;
    }
    bool palette(const IdleDrawIdentity &identity,const IdleConfigIdentity &resource,
                 bool evaluatedCurrent,int count) noexcept {
        if(stage!=Stage::Event || animationsCopied!=contributors || binding!=identity || config!=resource || !evaluatedCurrent ||
           count<=0 || unsigned(count)>MaxMatrices) {reject(Rejection::Palette,checks({stage==Stage::Event,animationsCopied==contributors,binding==identity,config==resource,evaluatedCurrent,count>0,count>0 && unsigned(count)<=MaxMatrices}));return false;}
        matrixCount=unsigned(count);stage=Stage::Palette;return true;
    }
    bool animation(unsigned index,const IdleAnimationValue &value) noexcept {
        if(stage!=Stage::Event || index!=animationsCopied || index>=contributors) {reject(Rejection::Animation);return false;}
        animations[index]=value;++animationsCopied;return true;
    }
    bool pose(const Matrix34 &transform,Vec3 scale,std::span<const Matrix34> source) noexcept {
        if(stage!=Stage::Palette || source.size()!=matrixCount || !finiteMatrix(transform) ||
           !std::isfinite(scale.x) || !std::isfinite(scale.y) || !std::isfinite(scale.z)) {reject(Rejection::Pose,checks({stage==Stage::Palette,source.size()==matrixCount,finiteMatrix(transform),std::isfinite(scale.x),std::isfinite(scale.y),std::isfinite(scale.z)}));return false;}
        for(const auto &m:source)if(!finiteMatrix(m)) {reject(Rejection::PoseMatrix);return false;}
        world=transform;stretch=scale;std::copy(source.begin(),source.end(),matrices.begin());poseCopied=true;return true;
    }
    bool draw(const IdleGeometryCopy &copy,bool originalCompleted,bool current) noexcept {
        if(stage!=Stage::Palette || !poseCopied || copy.raster.binding!=binding || copy.raster.rootConfig!=config ||
           !copy.raster.clipValid || !originalCompleted || !current || draws>=MaxDraws || copy.words<2 || copy.words>IdleGeometryCopy::MaxProgramWords ||
           !copy.declarationCount || copy.declarationCount>65 || !copy.constantCount || copy.constantCount>IdleGeometryCopy::MaxConstants || !finiteMatrix(copy.raster.affine)) {
            reject(Rejection::Draw,checks({stage==Stage::Palette,poseCopied,copy.raster.binding==binding,copy.raster.rootConfig==config,copy.raster.clipValid,originalCompleted,current,draws<MaxDraws,copy.words>=2,copy.words<=IdleGeometryCopy::MaxProgramWords,copy.declarationCount>0,copy.declarationCount<=65,copy.constantCount>0,copy.constantCount<=IdleGeometryCopy::MaxConstants,finiteMatrix(copy.raster.affine)}));return false;
        }
        for(unsigned i=0;i<copy.constantCount;++i)for(float value:copy.constants[i])
            if(!std::isfinite(value)) {reject(Rejection::DrawConstants);return false;}
        geometry[draws++]=copy;return true;
    }
    // Owned historical copies only: draw() stored these after original return
    // and its current check, before cleanup. Neither cleanup nor outer finish is
    // certified. Keep the later rejection and whole-trace admission unchanged.
    bool retainedDrawCopiesAvailable() const noexcept {
        return stage==Stage::Rejected && rejection==Rejection::CollectInputs &&
            precedingStage==Stage::Palette && inputFailure.step==20 && inputFailure.valid==15 &&
            admitted && placementObserved && referencesCopied && poseCopied &&
            contributors && contributors<=MaxContributors && animationsCopied==contributors &&
            matrixCount && matrixCount<=MaxMatrices && draws && draws<=MaxDraws &&
            binding.request && binding.input && binding.owner && binding.weapon && binding.model &&
            binding.generation && binding.hand<2 && binding.eye<2 && config.configuration && config.file;
    }
    bool finish(bool nativeCompleted,bool generationCurrent) noexcept {
        callbacks|=FinishSeen;
        if(stage!=Stage::Palette || !poseCopied || !placementObserved || !referencesCopied || !nativeCompleted || !generationCurrent) {reject(Rejection::Finish,checks({stage==Stage::Palette,poseCopied,placementObserved,referencesCopied,nativeCompleted,generationCurrent}));return false;}
        stage=Stage::Complete;return true;
    }
};
} // namespace ss2vr
