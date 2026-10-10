#pragma once
#include "idle_probe_selection.hpp"
#include "idle_projection_probe.hpp"
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
    uint32_t projectionModelAddress=0,projectionDrawAddress=0;
    uint32_t projectionSequence=0; // Existing trace's value receipt, excluded too.
    bool matchesProjection(const IdleProjectionPair &p) const noexcept {
        if(!p.qualified() || !clipValid || !factors.valid() ||
           p.after.modelRecord!=projectionModelAddress || p.after.drawRecord!=projectionDrawAddress)return false;
        for(unsigned i=0;i<12;++i)
            if(p.after.model[i]!=std::bit_cast<uint32_t>(factors.model.m[i]) ||
               p.after.view[i]!=std::bit_cast<uint32_t>(factors.view.m[i]))return false;
        for(unsigned i=0;i<16;++i)
            if(p.after.projection[i]!=std::bit_cast<uint32_t>(factors.projection.m[i]))return false;
        return true;
    }
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
    uint32_t projectionChecks=0;
    void factorBookend(const IdleRasterCopy &now,bool postOriginal) noexcept {
        const uint32_t bit=postOriginal?2u:1u;
        if(raster==now && raster.factors.same(now.factors))factorChecks|=bit;
        else factorChecks&=~bit;
        if(raster==now && raster.projectionSequence && raster.projectionSequence==now.projectionSequence &&
           raster.projectionModelAddress==now.projectionModelAddress &&
           raster.projectionDrawAddress==now.projectionDrawAddress)projectionChecks|=bit;
        else projectionChecks&=~bit;
    }
    bool factorsAvailable() const noexcept {return factorChecks==3 && raster.factors.valid();}
    bool projectionAvailable(const IdleProjectionPair &pair) const noexcept {
        if(projectionChecks!=3 || !factorsAvailable() || !raster.matchesProjection(pair) ||
           pair.sequence!=raster.projectionSequence || constantCount<5)return false;
        for(unsigned i=0;i<16;++i)
            if(std::bit_cast<uint32_t>(constants[1+i/4][i%4])!=pair.after.cachedMVP[i])return false;
        return true;
    }
};
// Independent bounded API-submission diagnostics. These values are not vertex
// content, GPU visibility, an authored asset join, or a grasp certificate.
struct IdleSubmissionMetadata {
    IdleConfigIdentity rootConfig{},renderConfig{};
    uint32_t modelAddress=0,drawAddress=0,modelRecord=0,drawRecord=0;
    uint32_t surface=0,instance=0,surfaceName=0,boneName=0;
    int32_t bone=-1;
    ScopeSurfaceLayout layout{};
    bool operator==(const IdleSubmissionMetadata &) const = default;
    static IdleSubmissionMetadata copy(const IdleRasterCopy &r) noexcept {
        return {r.rootConfig,r.renderConfig,r.projectionModelAddress,r.projectionDrawAddress,
                r.modelRecord,r.drawRecord,r.surface,r.instance,r.surfaceName,r.boneName,r.bone,r.layout};
    }
};
// Optional copied draw-local palette evidence. This does not supply a single
// affine, a shader register base, a qualified image, or a grasp reference.
// The native/API submission owner must establish those separate relationships.
struct IdlePaletteCopy {
    static constexpr uint32_t MaxPalette=3,MaxMappings=32768,MaxBones=8192,MaxModels=2048;
    struct Entry {
        int32_t draw=-1,bone=-1,owner=-1;
        uint32_t definition=0,name=0;
        std::array<uint32_t,12> canonical{},palette{};
        bool operator==(const Entry &) const = default;
    };
    IdleSubmissionMetadata metadata;
    uint32_t first=0,count=0,mapCount=0,paletteCount=0,canonicalCount=0,modelCount=0;
    uint32_t evaluated=0,matrices=0,mappingAddress=0,paletteAddress=0;
    std::array<uint32_t,12> world{};
    std::array<Entry,MaxPalette> entries{};
    bool copied=false;
    bool operator==(const IdlePaletteCopy &) const = default;
    static bool finiteWords(const std::array<uint32_t,12> &m) noexcept {
        for(auto bits:m)if((bits&0x7f800000u)==0x7f800000u)return false;
        return true; // Raw IEEE float finiteness only, not an invertible frame.
    }
    bool bounded() const noexcept {
        if(!copied || !count || count>MaxPalette || mapCount>MaxMappings || paletteCount>MaxMappings ||
           first>mapCount || count>mapCount-first || first>paletteCount || count>paletteCount-first ||
           !canonicalCount || canonicalCount>MaxBones || !modelCount || modelCount>MaxModels ||
           !metadata.modelRecord || metadata.modelRecord>=modelCount || !metadata.modelAddress ||
           !metadata.drawAddress || !metadata.surface || !metadata.instance ||
           !metadata.rootConfig.configuration || !metadata.renderConfig.configuration ||
           !evaluated || !matrices || !mappingAddress || !paletteAddress || !finiteWords(world))return false;
        for(unsigned i=0;i<count;++i) {
            const auto &e=entries[i];
            if(e.draw<0 || uint32_t(e.draw)!=metadata.drawRecord || e.bone<0 ||
               uint32_t(e.bone)>=canonicalCount || e.owner<0 || uint32_t(e.owner)!=metadata.modelRecord ||
               !e.definition || !finiteWords(e.canonical) || !finiteWords(e.palette))return false;
        }
        return true;
    }
    bool paletteCopiesCanonical() const noexcept {
        if(!bounded())return false;
        for(unsigned i=0;i<count;++i)if(entries[i].palette!=entries[i].canonical)return false;
        return true; // Native row-copy agreement; no shader address/clip proof.
    }
};
constexpr unsigned IdlePaletteApiCapacity=10;
struct IdleSubmissionTrace {
    static constexpr unsigned MaxAttempts=64,NoSlot=MaxAttempts;
    enum class Status : uint32_t { Pending,PreUnknown,PostUnknown,Mismatch,OriginalFailed,Reentered,Retired,Aborted,Qualified,NoForward,Split };
    struct Row {
        uint32_t ordinal=0;
        ScopeIndexedDraw draw{};
        IdleSubmissionMetadata metadata{};
        IdlePaletteCopy palette{};
        Status status=Status::Pending;
        int32_t hresult=0;
        bool preCopied=false,postCopied=false,returned=false,matched=false;
        bool paletteBeforeCopied=false,paletteAfterCopied=false,paletteMatched=false;
        uint32_t paletteApiSlot=IdlePaletteApiCapacity; // Independent capacity, never geometry.draws.
    };
    std::array<Row,MaxAttempts> rows{};
    uint32_t attempts=0,count=0;
    bool overflow=false,outerReturned=false,outerCompleted=false;
    unsigned reserve(const ScopeIndexedDraw &draw) noexcept {
        if(attempts!=UINT32_MAX)++attempts;
        if(count==MaxAttempts){overflow=true;return NoSlot;}
        const auto slot=count++;rows[slot].ordinal=attempts;rows[slot].draw=draw;return slot;
    }
    void reenter(unsigned slot) noexcept {if(slot<count)rows[slot].status=Status::Reentered;}
    bool pending(unsigned slot) const noexcept {return slot<count && rows[slot].status==Status::Pending;}
    void before(unsigned slot,const IdleSubmissionMetadata &value,bool known) noexcept {
        if(!pending(slot))return;
        rows[slot].preCopied=known;if(known)rows[slot].metadata=value;
    }
    void after(unsigned slot,const IdleSubmissionMetadata &value,bool known,int32_t result) noexcept {
        if(slot>=count)return;
        auto &r=rows[slot];r.returned=true;r.hresult=result;
        if(r.status==Status::Reentered)return;
        r.postCopied=known;r.matched=r.preCopied && known && r.metadata==value;
    }
    void paletteBefore(unsigned slot,const IdlePaletteCopy &value,bool known) noexcept {
        if(!pending(slot))return;
        auto &r=rows[slot];r.paletteBeforeCopied=known && value.bounded();
        if(r.paletteBeforeCopied)r.palette=value;
    }
    void paletteAfter(unsigned slot,const IdlePaletteCopy &value,bool known) noexcept {
        if(slot>=count || rows[slot].status==Status::Reentered)return;
        auto &r=rows[slot];r.paletteAfterCopied=known && value.bounded();
        r.paletteMatched=r.paletteBeforeCopied && r.paletteAfterCopied && r.palette==value;
    }
    void completeOuter(bool originalReturned,bool passComplete,bool generationCurrent,
                       bool noWeaponFault,bool normalCleanup) noexcept {
        outerCompleted=originalReturned && passComplete && generationCurrent && noWeaponFault && normalCleanup;
    }
    bool palettePublishable(unsigned slot) const noexcept {
        if(slot>=count || !outerReturned || !outerCompleted)return false;
        const auto &r=rows[slot];
        return r.status==Status::Qualified && r.paletteBeforeCopied && r.paletteAfterCopied &&
               r.paletteMatched && r.palette.bounded() && r.palette.metadata==r.metadata;
    }
    void finalize(unsigned slot,bool aborted,bool current,bool split) noexcept {
        if(slot>=count)return;
        auto &r=rows[slot];
        if(aborted)r.status=Status::Aborted;
        else if(!current)r.status=Status::Retired;
        else if(r.status!=Status::Reentered) {
            r.status=split?Status::Split:!r.returned?Status::NoForward:r.hresult<0?Status::OriginalFailed:
                !r.preCopied?Status::PreUnknown:!r.postCopied?Status::PostUnknown:
                !r.matched?Status::Mismatch:Status::Qualified;
        }
        if(r.status!=Status::Qualified) {r.metadata={};r.palette={};}
    }
};
static_assert(sizeof(IdleSubmissionTrace)<=48*1024); // Bounded in-row copies; no second palette allocator.
struct IdleWeaponTrace {
    static constexpr unsigned MaxContributors=16,MaxMatrices=64,MaxDraws=10;
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
    IdleSubmissionTrace submissions{};
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
    struct PaletteOwnershipFailure {
        uint32_t evaluated=0,linked=0,owner=0,instance=0;
        int32_t count=0;
        bool copied=false;
    } paletteOwnershipFailure{};
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
    struct PaletteApiPayload {
        uint32_t submissionSlot=IdleSubmissionTrace::NoSlot,ordinal=0,words=0;
        bool beforeCopied=false,afterCopied=false,matched=false;
        StreamSnapshot before;
        std::array<uint32_t,IdleGeometryCopy::MaxProgramWords> program{};
        ScopeBufferInputs content{};
        std::array<std::array<uint8_t,32>,5> contentHashes{};
        bool contentCopied=false,contentMatched=false;
    };
    static constexpr unsigned MaxPaletteApiPayloads=IdlePaletteApiCapacity;
    static_assert(MaxPaletteApiPayloads==MaxDraws); // Reuse the capacity, not the geometry counter.
    std::array<PaletteApiPayload,MaxPaletteApiPayloads> paletteApiPayloads{};
    uint32_t paletteApiCount=0;
    bool paletteApiOverflow=false;
    unsigned reservePaletteApi(unsigned submissionSlot) noexcept {
        if(nativeId!=2 || !submissions.pending(submissionSlot))return MaxPaletteApiPayloads;
        if(paletteApiCount==MaxPaletteApiPayloads) {paletteApiOverflow=true;return MaxPaletteApiPayloads;}
        const unsigned slot=paletteApiCount++;
        auto &p=paletteApiPayloads[slot];p.submissionSlot=submissionSlot;
        p.ordinal=submissions.rows[submissionSlot].ordinal;
        submissions.rows[submissionSlot].paletteApiSlot=slot;
        return slot; // Failed samples consume capacity too; never search for a passing replacement.
    }
    bool paletteApiPublishable(unsigned submissionSlot) const noexcept {
        if(!submissions.palettePublishable(submissionSlot))return false;
        const auto &r=submissions.rows[submissionSlot];
        if(r.paletteApiSlot>=paletteApiCount || r.paletteApiSlot>=MaxPaletteApiPayloads)return false;
        const auto &p=paletteApiPayloads[r.paletteApiSlot];
        return p.submissionSlot==submissionSlot && p.ordinal==r.ordinal && p.words>=2 &&
               p.words<=p.program.size() && p.beforeCopied && p.afterCopied && p.matched &&
               p.before.status==StreamSnapshot::Copied;
    }
    bool paletteContentPublishable(unsigned submissionSlot) const noexcept {
        if(!paletteApiPublishable(submissionSlot))return false;
        const auto &r=submissions.rows[submissionSlot];const auto &p=paletteApiPayloads[r.paletteApiSlot];
        if(!p.contentCopied || !p.contentMatched || p.content.surface!=r.palette.metadata.layout || p.content.draw!=r.draw ||
           p.before.indexObject!=p.content.indexObject ||
           p.before.streams!=std::array<ScopeStreamInput,3>{p.content.positions,p.content.localIndices,p.content.weights})return false;
        ScopeCopyRanges ranges;
        return idleBufferRanges(p.content,std::span(p.before.declaration).first(p.before.declarationCount),ranges,nullptr,2);
    }
    IdleProjectionProbe projectionProbe{};
    // Select passive diagnostics from the immutable original rejection, never
    // either resampled declaration. Reuse the exact ID1 declaration predicates.
    static std::array<unsigned,3> passiveStreamNumbers(const InputFailure &d) noexcept {
        if(d.step!=20 || d.valid!=15 || d.declarationCount>65)return {};
        const auto rows=std::span(d.declaration).first(d.declarationCount);
        if(noUV78Declaration(rows))return {0,7,8};
        if(observed78Declaration(rows))return {0,7,8};
        // Select from the original failure even when the later copy adapter
        // supports this family. Passive receipts never establish admission.
        if(observedMultiUV78Declaration(rows))return {0,7,8};
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
    int nativeId=1; // Legacy ID1 traces; explicit on every new production admission.
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
    bool admit(const IdleDrawIdentity &identity,int selectedNativeId=1) noexcept {
        if(!idleProbeWeaponSupported(selectedNativeId) || admitted || stage!=Stage::Empty || !identity.request || !identity.input || !identity.owner ||
           !identity.weapon || !identity.model || !identity.generation || identity.hand>=2 || identity.eye>=2) {
            reject(Rejection::Admit);return false;
        }
        binding=identity;nativeId=selectedNativeId;admitted=true;return true;
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
    // Values already read by the existing palette borrow. Record only the
    // first ownership-only failure, after own-event/name/config admission. This
    // receipt neither changes rejection nor makes a cache usable.
    void notePaletteOwnershipFailure(const IdleDrawIdentity &identity,const IdleConfigIdentity &resource,
                                     uint32_t evaluated,uint32_t linked,uint32_t owner,
                                     uint32_t instance,int32_t count) noexcept {
        if(paletteOwnershipFailure.copied || rejection!=Rejection::None || stage!=Stage::Event ||
           binding!=identity || config!=resource || animationsCopied!=contributors ||
           !instance || count<=0 || unsigned(count)>MaxMatrices ||
           (evaluated==linked && owner==instance))return;
        paletteOwnershipFailure={evaluated,linked,owner,instance,count,true};
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
    bool retainedCopyOwnerQualified() const noexcept {
        return stage==Stage::Rejected && precedingStage==Stage::Palette &&
            admitted && placementObserved && referencesCopied && poseCopied &&
            contributors && contributors<=MaxContributors && animationsCopied==contributors &&
            matrixCount && matrixCount<=MaxMatrices && draws && draws<=MaxDraws &&
            binding.request && binding.input && binding.owner && binding.weapon && binding.model &&
            binding.generation && binding.hand<2 && binding.eye<2 && config.configuration && config.file;
    }
    bool retainedCapacityCopiesAvailable() const noexcept {
        // Exactly the original capacity check failed after a normal/current
        // eleventh draw. No eleventh copy exists; expose only ten owned values.
        constexpr uint32_t capacityOnly=((1u<<15)-1) & ~(1u<<7);
        return retainedCopyOwnerQualified() && rejection==Rejection::Draw &&
            draws==MaxDraws && rejectionChecks==capacityOnly && callbacks==63 &&
            rejectionState==(47u|unsigned(rawGripValid)*16u) &&
            !inputFailure.step && !streamProbe.selected;
    }
    bool retainedDrawCopiesAvailable() const noexcept {
        return (retainedCopyOwnerQualified() && rejection==Rejection::CollectInputs &&
                inputFailure.step==20 && inputFailure.valid==15) || retainedCapacityCopiesAvailable();
    }
    bool finish(bool nativeCompleted,bool generationCurrent) noexcept {
        callbacks|=FinishSeen;
        if(stage!=Stage::Palette || !poseCopied || !placementObserved || !referencesCopied || !nativeCompleted || !generationCurrent) {reject(Rejection::Finish,checks({stage==Stage::Palette,poseCopied,placementObserved,referencesCopied,nativeCompleted,generationCurrent}));return false;}
        stage=Stage::Complete;return true;
    }
};
} // namespace ss2vr
