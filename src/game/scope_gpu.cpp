#include "scope_gpu.hpp"
#include "scope_observer.hpp"
#include "idle_observer.hpp"
#include "game.hpp"
#include "native_finally.hpp"
#include "common/scope_buffer_layout.hpp"
#include "common/scope_lock.hpp"
#include "common/scope_program.hpp"
#include "common/scope_color_program.hpp"
#include "common/scope_draw_order.hpp"
#include "common/scope_shader_constants.hpp"
#include "common/scope_position_program.hpp"
#include <bcrypt.h>
#include <atomic>
#include <algorithm>

namespace ss2vr::game {
static_assert(D3DDECLTYPE_FLOAT2 == 1 && D3DDECLTYPE_FLOAT3 == 2 && D3DDECLTYPE_UBYTE4N == 8 && D3DDECLTYPE_UNUSED == 17);
static_assert(D3DDECLUSAGE_TEXCOORD == 5 && D3DFMT_VERTEXDATA == 100 && D3DFMT_INDEX16 == 101);
static_assert(D3DPOOL_MANAGED == 1 && D3DPT_TRIANGLELIST == 4 && MAXD3DDECLLENGTH == 64);
// Fatal native-buffer ownership uncertainty lasts for this process. Managed
// resources survive Reset; neither generation changes nor deviceLost retire it.
static std::atomic<bool> fatalDraw{false};
static constexpr std::array colorStates{
    D3DRS_ZENABLE,D3DRS_ZWRITEENABLE,D3DRS_ZFUNC,D3DRS_ALPHABLENDENABLE,
    D3DRS_ALPHATESTENABLE,D3DRS_STENCILENABLE,D3DRS_FOGENABLE,D3DRS_COLORWRITEENABLE,
    D3DRS_FILLMODE,D3DRS_CULLMODE,D3DRS_SCISSORTESTENABLE,D3DRS_CLIPPLANEENABLE,
    D3DRS_DEPTHBIAS,D3DRS_SLOPESCALEDEPTHBIAS,D3DRS_CLIPPING,D3DRS_SRGBWRITEENABLE};
struct BoundColor {
    IDirect3DPixelShader9 *shader = nullptr;
    IDirect3DSurface9 *target = nullptr, *depth = nullptr, *extra = nullptr;
    IUnknown *identity[5]{}; // Actual PS/RT/depth; retained expected eye RT/depth.
    D3DVIEWPORT9 viewport{};
    RECT scissor{};
    std::array<DWORD,colorStates.size()> states{};
    std::array<std::array<float,4>,6> clipPlanes{};
    UINT width=0,height=0;
    bool valid=false;
};
struct BoundInputs {
    IDirect3DVertexDeclaration9 *declaration = nullptr;
    IDirect3DVertexBuffer9 *vertex[4]{};
    IDirect3DIndexBuffer9 *index = nullptr;
    IDirect3DVertexShader9 *shader = nullptr;
    std::array<std::array<float,4>,2> uvRows{};
    IUnknown *identity[7]{}; // Position/localindices/UV/weights/index/declaration/shader.
    ScopeBufferInputs values;
    std::array<ScopeDeclarationElement,MAXD3DDECLLENGTH+1> elements{};
    UINT count = 0;
    BoundColor color;
    std::array<std::array<float,4>,256> constants{};
    UINT constantCount = 0;
};
struct ProbeOwner {
    bool busy = false;
    uint64_t generation = 0;
    ScopeRasterObservation raster;
    IdleGeometryCopy idle;
    IdleWeaponTrace *idleTrace=nullptr;
    BoundInputs bindings[2];
    ScopeLockLedger lock;
    IDirect3DVertexBuffer9 *lockedVertex = nullptr;
    IDirect3DIndexBuffer9 *lockedIndex = nullptr;
    void *mapped = nullptr;
    std::array<uint8_t,IdleGeometryVertices*12> positions{};
    std::array<uint8_t,IdleGeometryTriangles*6> indices{};
    std::array<uint8_t,IdleGeometryVertices*4> weights{}, localIndices{};
    std::array<uint8_t,IdleGeometryVertices*8> uv{};
    BCRYPT_ALG_HANDLE algorithm = nullptr;
    BCRYPT_HASH_HANDLE hash = nullptr;
    std::array<uint8_t,1024> hashObject{};
    ScopeCapGeometry cap;
    std::array<uint32_t,ScopeProgramMaxWords> program{};
    std::array<uint32_t,ScopeProgramMaxWords> colorProgram{};
    std::array<std::array<float,4>,2> uvRows{};
    IDirect3DDevice9 *device = nullptr; // Invocation reference survives early frame retirement.
    IDirect3DTexture9 *image = nullptr;
    IDirect3DBaseTexture9 *originalTexture = nullptr, *restoredTexture = nullptr;
    IDirect3DPixelShader9 *restoredShader = nullptr;
    IDirect3DPixelShader9 *imageShader = nullptr;
    ScopeSourceView sourceView;
    ScopeShaderConstants imageConstants;
    size_t programWords = 0;
    std::array<std::array<float,4>,7> pixelConstants{};
    std::array<DWORD,10> sampler{};
    bool transaction = false, split = false, imageChanged = false, imageRestoreFailed = false;
};
static thread_local ProbeOwner probe;
static bool restoreImageState(IDirect3DDevice9 *) noexcept;
template<class T> static void release(T *&p) noexcept {
    T *owned = p; p = nullptr;
    if (owned) owned->Release();
}
static void releaseBindings(BoundInputs &b) noexcept {
    for (auto &id:b.color.identity) release(id);
    release(b.color.shader); release(b.color.target); release(b.color.depth); release(b.color.extra);
    b.color={};
    for (auto &identity : b.identity) release(identity);
    release(b.declaration);
    for (auto &v : b.vertex) release(v);
    release(b.index);
    release(b.shader);
    b.uvRows={};
    b.values={}; b.count=0; b.elements={}; // No stale object comparison keys survive this owner.
}
static void fatal() noexcept {
    fatalDraw.store(true,std::memory_order_release);
    scopeGpuFault();
    retireScopeGeometry(true);
}
bool scopeGpuForwardingAllowed() noexcept {
    if (probe.lock.outstanding()) fatal(); // Includes reentry while Lock/Unlock is indeterminate.
    return !fatalDraw.load(std::memory_order_acquire);
}
static bool unlock() noexcept {
    if (!probe.lock.beginUnlock()) return false;
    // The attempted flag is installed BEFORE the foreign call; no blind retry
    // after failure or native unwinding. Success+null still reaches this path.
    const HRESULT result = probe.lockedVertex ? probe.lockedVertex->Unlock() : probe.lockedIndex->Unlock();
    probe.lock.finishUnlock(SUCCEEDED(result));
    probe.mapped = nullptr;
    if (FAILED(result)) { fatal(); return false; }
    probe.lockedVertex = nullptr; probe.lockedIndex = nullptr;
    return true;
}
static void cleanup(bool aborted) noexcept {
    const bool retired = probe.generation != graphicsResourceGeneration();
    if (aborted && probe.idleTrace) probe.idleTrace->reject(IdleWeaponTrace::Rejection::GpuAbort);
    if (aborted) { retireScopeGeometry(); if (!retired) scopeGpuFault(); }
    if (probe.imageChanged) restoreImageState(probe.device);
    if (probe.lock.phase == ScopeLockPhase::Acquired) unlock();
    if (probe.lock.outstanding()) {
        fatal();
        // Transfer exactly one existing reference into the uncertain owner.
        // No process/reset path releases or retries this acquisition.
        if (probe.lockedVertex == probe.bindings[0].vertex[0]) probe.bindings[0].vertex[0] = nullptr;
        if (probe.lockedIndex == probe.bindings[0].index) probe.bindings[0].index = nullptr;
    }
    for (auto &b : probe.bindings) releaseBindings(b);
    if (probe.hash) { const auto h=probe.hash; probe.hash=nullptr; BCryptDestroyHash(h); }
    if (probe.algorithm) { const auto a=probe.algorithm; probe.algorithm=nullptr; BCryptCloseAlgorithmProvider(a,0); }
    release(probe.image); release(probe.originalTexture); release(probe.imageShader);
    release(probe.restoredTexture); release(probe.restoredShader);
    const bool current = !probe.transaction || scopeGpuTransactionCurrent(probe.device);
    auto *device = probe.device;
    release(probe.device);
    if (probe.transaction) {
        // Current performs identity comparisons only, never dereferences device.
        const bool afterRelease=scopeGpuTransactionCurrent(device);
        if(probe.idleTrace && (retired || !current || !afterRelease))probe.idleTrace->reject(IdleWeaponTrace::Rejection::GpuRetired,IdleWeaponTrace::checks({!retired,current,afterRelease}));
        if (!retired && probe.split && (!current || !afterRelease)) scopeGpuFault();
        scopeGpuTransactionEnd();
    }
    probe.sourceView = {}; probe.imageConstants = {};
    probe.transaction = probe.split = probe.imageChanged = probe.imageRestoreFailed = false;
    probe.cap = {}; probe.uvRows={}; probe.program={}; probe.colorProgram={};
    probe.idleTrace=nullptr;
    probe.busy = false;
}
static bool identity(IUnknown *object,IUnknown *&out,HRESULT *observed=nullptr) {
    if(!object){if(observed)*observed=E_POINTER;return false;}
    const HRESULT result=object->QueryInterface(IID_IUnknown,reinterpret_cast<void **>(&out));
    if(observed)*observed=result;
    return SUCCEEDED(result) && out;
}
static bool boundInputs(IDirect3DDevice9 *d,BoundInputs &b,const ScopeIndexedDraw &draw,bool idle=false,
                        IdleWeaponTrace::InputFailure *diagnostic=nullptr) {
    b.values = {}; b.count = MAXD3DDECLLENGTH+1;
    HRESULT result=S_OK;
    auto fail=[&](uint32_t step,uint32_t index=0) {
        if(diagnostic){diagnostic->step=step;diagnostic->index=index;diagnostic->hresult=int32_t(result);}
        return false;
    };
    D3DCAPS9 caps{};
    result=d->GetDeviceCaps(&caps);if(FAILED(result))return fail(1);
    if(diagnostic){diagnostic->valid|=1;diagnostic->caps=caps.MaxVertexShaderConst;}
    if(!caps.MaxVertexShaderConst || caps.MaxVertexShaderConst>256)return fail(2);
    b.constantCount=caps.MaxVertexShaderConst;
    result=d->GetVertexShaderConstantF(0,b.constants[0].data(),b.constantCount);if(FAILED(result))return fail(3);
    D3DVERTEXELEMENT9 elements[MAXD3DDECLLENGTH+1]{};
    result=d->GetVertexDeclaration(&b.declaration);if(FAILED(result))return fail(4);
    if(!b.declaration)return fail(5);
    result=b.declaration->GetDeclaration(elements,&b.count);if(FAILED(result))return fail(6);
    if(diagnostic){diagnostic->valid|=8;diagnostic->declarationCount=b.count;}
    if(!b.count || b.count>std::size(elements))return fail(7);
    bool weights=false;
    for(UINT i=0;i<b.count;++i) {
        const auto e=elements[i];b.elements[i]={e.Stream,e.Offset,e.Type,e.Method,e.Usage,e.UsageIndex};
        if(e.Stream==6 && e.Type!=D3DDECLTYPE_UNUSED)weights=true;
    }
    if(diagnostic){diagnostic->valid|=2;diagnostic->declaration=b.elements;}
    b.values.surface=idle?probe.idle.raster.layout:probe.raster.pose.layout;b.values.draw=draw;
    ScopeStreamInput *streams[]{&b.values.positions,&b.values.localIndices,&b.values.uv,&b.values.weights};
    const UINT streamNumbers[]{0,5,3,6};
    for(unsigned i=0;i<(weights?4u:3u);++i) {
        auto &stream=*streams[i];
        result=d->GetStreamSource(streamNumbers[i],&b.vertex[i],&stream.offset,&stream.stride);if(FAILED(result))return fail(8,streamNumbers[i]);
        if(!b.vertex[i])return fail(9,streamNumbers[i]);
        result=d->GetStreamSourceFreq(streamNumbers[i],&stream.frequency);if(FAILED(result))return fail(10,streamNumbers[i]);
        if(!identity(b.vertex[i],b.identity[i],&result))return fail(11,streamNumbers[i]);
        stream.object=reinterpret_cast<uintptr_t>(b.identity[i]);
    }
    D3DVERTEXBUFFER_DESC vertex{};D3DINDEXBUFFER_DESC index{};
    result=b.vertex[0]->GetDesc(&vertex);if(FAILED(result))return fail(12);
    if(vertex.Type!=D3DRTYPE_VERTEXBUFFER)return fail(13);
    result=d->GetIndices(&b.index);if(FAILED(result))return fail(14);
    if(!b.index)return fail(15);
    result=b.index->GetDesc(&index);if(FAILED(result))return fail(16);
    if(index.Type!=D3DRTYPE_INDEXBUFFER)return fail(17);
    if(!identity(b.index,b.identity[4],&result))return fail(18);
    if(!identity(b.declaration,b.identity[5],&result))return fail(19);
    b.values.vertex={vertex.Size,vertex.Usage,uint32_t(vertex.Pool),uint32_t(vertex.Format),vertex.FVF};
    b.values.index={index.Size,index.Usage,uint32_t(index.Pool),uint32_t(index.Format),0};
    b.values.indexObject=reinterpret_cast<uintptr_t>(b.identity[4]);
    if(diagnostic){diagnostic->valid|=4;diagnostic->inputs=b.values;}
    ScopeCopyRanges ranges;
    if(!(idle?idleBufferRanges(b.values,std::span(b.elements).first(b.count),ranges,diagnostic?&diagnostic->rangeChecks:nullptr):
              scopeBufferRanges(b.values,std::span(b.elements).first(b.count),ranges)))return fail(20);
    result=d->GetVertexShader(&b.shader);if(FAILED(result))return fail(21);
    if(!b.shader)return fail(22);
    if(!identity(b.shader,b.identity[6],&result))return fail(23);
    result=d->GetVertexShaderConstantF(8,b.uvRows[0].data(),2);if(FAILED(result))return fail(24);
    for(const auto &row:b.uvRows)for(float value:row)if(!std::isfinite(value))return fail(25);
    return true;
}
// Shader objects expose no bytecode mutator; same canonical identity in the
// second snapshot pins the program admitted here. All calls precede VB locks.
static bool boundProgram(bool idle=false) {
    auto *shader=probe.bindings[0].shader;
    UINT size=0;
    if (!shader || FAILED(shader->GetFunction(nullptr,&size)) || size<8 ||
        size%sizeof(uint32_t) || size>sizeof(probe.program) || (idle && size>sizeof(probe.idle.program))) return false;
    const UINT expected=size;
    if (FAILED(shader->GetFunction(probe.program.data(),&size)) || size!=expected) return false;
    probe.programWords = size/sizeof(uint32_t);
    return idle || scopeUvProgram(std::span(probe.program).first(probe.programWords));
}
// Optional color admission is independent of the existing geometry/UV path.
// All COM output owners live in ProbeOwner before calls, including partial
// getter results and the extra MRT probe. No getters execute while VB is locked.
static bool boundColor(IDirect3DDevice9 *d,BoundColor &b,bool program) {
    if (!probe.raster.opaqueMode || !currentScopeQueryBoundary() ||
        !scopeGpuEyeOwner(d,b.identity[3],b.identity[4],b.width,b.height) ||
        FAILED(d->GetRenderTarget(0,&b.target)) || !b.target || !identity(b.target,b.identity[1]) ||
        FAILED(d->GetDepthStencilSurface(&b.depth)) || !b.depth || !identity(b.depth,b.identity[2]) ||
        b.identity[1]!=b.identity[3] || b.identity[2]!=b.identity[4] ||
        FAILED(d->GetViewport(&b.viewport))) return false;
    const auto &vp=b.viewport;
    if (vp.X || vp.Y || vp.Width!=b.width || vp.Height!=b.height ||
        !std::isfinite(vp.MinZ) || !std::isfinite(vp.MaxZ) ||
        vp.MinZ!=probe.raster.nearDepth || vp.MaxZ!=probe.raster.farDepth ||
        vp.MinZ<0 || vp.MaxZ>1 || vp.MaxZ<=vp.MinZ) return false;
    D3DSURFACE_DESC target{},depth{};
    if (FAILED(b.target->GetDesc(&target)) || FAILED(b.depth->GetDesc(&depth))) return false;
    for (const auto &surface:{target,depth})
        if (surface.Type!=D3DRTYPE_SURFACE || surface.Width!=b.width || surface.Height!=b.height ||
            surface.MultiSampleType!=D3DMULTISAMPLE_NONE || surface.MultiSampleQuality) return false;
    if (!(target.Usage&D3DUSAGE_RENDERTARGET) || !(depth.Usage&D3DUSAGE_DEPTHSTENCIL)) return false;
    D3DCAPS9 caps{};
    if (FAILED(d->GetDeviceCaps(&caps)) || !caps.NumSimultaneousRTs || caps.NumSimultaneousRTs>4 ||
        caps.MaxUserClipPlanes>6) return false;
    for (DWORD i=1;i<caps.NumSimultaneousRTs;++i) {
        const HRESULT result=d->GetRenderTarget(i,&b.extra);
        const bool extra=b.extra!=nullptr;
        release(b.extra);
        if (extra || (FAILED(result) && result!=D3DERR_NOTFOUND)) return false;
    }
    for (size_t i=0;i<colorStates.size();++i)
        if (FAILED(d->GetRenderState(colorStates[i],&b.states[i]))) return false;
    const auto &s=b.states;
    if (s[0]!=D3DZB_TRUE || s[1]!=TRUE || s[2]!=D3DCMP_LESSEQUAL || s[3] || s[4] || s[5] || s[6] ||
        (s[7]&7)!=7 || (s[7]&~15u) || s[8]!=D3DFILL_SOLID || s[9]<D3DCULL_NONE || s[9]>D3DCULL_CCW ||
        s[10]>1 || (s[11]&~((1u<<caps.MaxUserClipPlanes)-1)) || s[14]!=TRUE || s[15]>1 ||
        !std::isfinite(std::bit_cast<float>(s[12])) || !std::isfinite(std::bit_cast<float>(s[13]))) return false;
    if (s[10] && (FAILED(d->GetScissorRect(&b.scissor)) || b.scissor.left<0 || b.scissor.top<0 ||
        b.scissor.right<=b.scissor.left || b.scissor.bottom<=b.scissor.top ||
        uint32_t(b.scissor.right)>b.width || uint32_t(b.scissor.bottom)>b.height)) return false;
    for (unsigned i=0;i<b.clipPlanes.size();++i) if (s[11]&(1u<<i)) {
        if (FAILED(d->GetClipPlane(i,b.clipPlanes[i].data()))) return false;
        for (float value:b.clipPlanes[i]) if (!std::isfinite(value)) return false;
    }
    if (FAILED(d->GetPixelShader(&b.shader)) || !b.shader || !identity(b.shader,b.identity[0])) return false;
    if (program) {
        UINT size=0;
        if (FAILED(b.shader->GetFunction(nullptr,&size)) || size<8 || size%4 || size>sizeof(probe.colorProgram)) return false;
        const UINT expected=size;
        if (FAILED(b.shader->GetFunction(probe.colorProgram.data(),&size)) || size!=expected ||
            !scopeColorProgram(std::span(probe.colorProgram).first(size/4))) return false;
    }
    b.valid=true;
    return true;
}
static bool sameColor(const BoundColor &a,const BoundColor &b) noexcept {
    return a.valid && b.valid && a.width==b.width && a.height==b.height &&
        std::equal(std::begin(a.identity),std::end(a.identity),std::begin(b.identity)) &&
        !std::memcmp(&a.viewport,&b.viewport,sizeof(a.viewport)) &&
        !std::memcmp(&a.scissor,&b.scissor,sizeof(a.scissor)) && a.states==b.states &&
        !std::memcmp(a.clipPlanes.data(),b.clipPlanes.data(),sizeof(a.clipPlanes));
}
static bool copySlice(bool index,ScopeByteRange range,std::span<uint8_t> destination) {
    if (range.size != destination.size() || probe.lock.outstanding()) return false;
    probe.lockedVertex=index?nullptr:probe.bindings[0].vertex[0];
    probe.lockedIndex=index?probe.bindings[0].index:nullptr;
    probe.mapped=nullptr;
    if (!probe.lock.beginLock()) return false;
    const HRESULT result=index ? probe.lockedIndex->Lock(range.offset,range.size,&probe.mapped,D3DLOCK_READONLY) :
                                 probe.lockedVertex->Lock(range.offset,range.size,&probe.mapped,D3DLOCK_READONLY);
    probe.lock.finishLock(SUCCEEDED(result));
    if (FAILED(result)) {
        probe.lockedVertex=nullptr; probe.lockedIndex=nullptr;
        return false;
    }
    const bool copied=probe.mapped != nullptr;
    if (copied) std::memcpy(destination.data(),probe.mapped,destination.size());
    return unlock() && copied && !fatalDraw.load(std::memory_order_acquire);
}
static bool hashSlices() {
    DWORD size=0,reported=0;
    if (BCryptOpenAlgorithmProvider(&probe.algorithm,BCRYPT_SHA256_ALGORITHM,nullptr,0)<0 ||
        BCryptGetProperty(probe.algorithm,BCRYPT_OBJECT_LENGTH,reinterpret_cast<PUCHAR>(&size),sizeof(size),&reported,0)<0 ||
        reported!=sizeof(size) || !size || size>probe.hashObject.size()) return false;
    const ScopeSliceBytes slices{std::span(probe.positions).first(ScopeVertices*12),
        std::span(probe.indices).first(ScopeTriangles*6),std::span(probe.weights).first(ScopeVertices*4),
        std::span(probe.localIndices).first(ScopeVertices*4),std::span(probe.uv).first(ScopeVertices*8)};
    const std::array<std::span<const uint8_t>,5> bytes{slices.positions,slices.indices,slices.weights,slices.localIndices,slices.uv};
    constexpr const char *expected[]{
        "a83d4d52827f2f95418f46c2ef8f8eca3ac594110ff35173bef4a526188a99c2",
        "9e906c5c63a118742d857f0641ab39494ed1350374ea9e132fa5745d3637781b",
        "2852dade36b5c2b533147f17dc9018ae35eff637e68d7e85fee12c05271d30ad",
        "4bcd1d733a353b8de5512ea22b608ebc74c4a818fe00663ff32ea4f7a3dd808a",
        "199e21cfca88d708d605db3ee54f35bba76191bf698c725baf4a39034480b2fd"};
    constexpr char hex[]="0123456789abcdef";
    for (unsigned i=0;i<bytes.size();++i) {
        std::array<uint8_t,32> digest{};
        if (BCryptCreateHash(probe.algorithm,&probe.hash,probe.hashObject.data(),size,nullptr,0,0)<0 ||
            BCryptHashData(probe.hash,const_cast<PUCHAR>(bytes[i].data()),ULONG(bytes[i].size()),0)<0 ||
            BCryptFinishHash(probe.hash,digest.data(),ULONG(digest.size()),0)<0) return false;
        const auto h=probe.hash; probe.hash=nullptr;
        if (BCryptDestroyHash(h)<0) return false;
        for (unsigned j=0;j<digest.size();++j)
            if (expected[i][2*j]!=hex[digest[j]>>4] || expected[i][2*j+1]!=hex[digest[j]&15]) return false;
    }
    return copyScopeCap(slices,probe.cap);
}
static bool hashIdleSlices(const ScopeCopyRanges &ranges) {
    DWORD size=0,reported=0;
    if(BCryptOpenAlgorithmProvider(&probe.algorithm,BCRYPT_SHA256_ALGORITHM,nullptr,0)<0 ||
       BCryptGetProperty(probe.algorithm,BCRYPT_OBJECT_LENGTH,reinterpret_cast<PUCHAR>(&size),sizeof(size),&reported,0)<0 ||
       reported!=sizeof(size) || !size || size>probe.hashObject.size())return false;
    const std::array<std::span<uint8_t>,5> storage{probe.positions,probe.indices,probe.weights,probe.localIndices,probe.uv};
    for(unsigned i=0;i<5;++i) {
        if(ranges.slices[i].size>storage[i].size() ||
           BCryptCreateHash(probe.algorithm,&probe.hash,probe.hashObject.data(),size,nullptr,0,0)<0 ||
           BCryptHashData(probe.hash,storage[i].data(),ranges.slices[i].size,0)<0 ||
           BCryptFinishHash(probe.hash,probe.idle.hashes[i].data(),32,0)<0)return false;
        const auto h=probe.hash;probe.hash=nullptr;if(BCryptDestroyHash(h)<0)return false;
    }
    return true;
}
static bool sameInputs(const BoundInputs &a,const BoundInputs &b) {
    return a.values==b.values && a.count==b.count && a.elements==b.elements &&
        std::equal(std::begin(a.identity),std::end(a.identity),std::begin(b.identity)) &&
        a.constantCount==b.constantCount &&
        !std::memcmp(a.constants.data(),b.constants.data(),a.constantCount*4*sizeof(float));
}
static bool collectIdleGeometry(IDirect3DDevice9 *d,const ScopeIndexedDraw &draw) {
    auto reject=[](IdleWeaponTrace::Rejection reason) {if(probe.idleTrace)probe.idleTrace->reject(reason);return false;};
    if(probe.idleTrace)probe.idleTrace->callbacks|=IdleWeaponTrace::GeometrySeen;
    if(!nativeUiDeviceCurrent(d))return reject(IdleWeaponTrace::Rejection::CollectDevice);
    ScopeCopyRanges ranges;
    IdleWeaponTrace::InputFailure inputFailure{};
    if(!boundInputs(d,probe.bindings[0],draw,true,&inputFailure)) {
        if(probe.idleTrace && probe.idleTrace->rejection==IdleWeaponTrace::Rejection::None)probe.idleTrace->inputFailure=inputFailure;
        return reject(IdleWeaponTrace::Rejection::CollectInputs);
    }
    if(!idleBufferRanges(probe.bindings[0].values,std::span(probe.bindings[0].elements).first(probe.bindings[0].count),ranges))return reject(IdleWeaponTrace::Rejection::CollectRanges);
    if(!boundProgram(true))return reject(IdleWeaponTrace::Rejection::CollectProgram);
    const std::array<std::span<uint8_t>,5> storage{probe.positions,probe.indices,probe.weights,probe.localIndices,probe.uv};
    for(unsigned i=0;i<5;++i)
        if(ranges.slices[i].size>storage[i].size() || !copySlice(i==1,ranges.slices[i],storage[i].first(ranges.slices[i].size)))return reject(IdleWeaponTrace::Rejection::CollectSlice);
    if(!hashIdleSlices(ranges))return reject(IdleWeaponTrace::Rejection::CollectHash);
    if(!boundInputs(d,probe.bindings[1],draw,true))return reject(IdleWeaponTrace::Rejection::CollectRebind);
    if(!sameInputs(probe.bindings[0],probe.bindings[1]))return reject(IdleWeaponTrace::Rejection::CollectChanged);
    IdleRasterCopy now;IdleWeaponTrace *trace=nullptr;
    if(!currentIdleRaster(now,trace) || trace!=probe.idleTrace || now!=probe.idle.raster)return reject(IdleWeaponTrace::Rejection::CollectRaster);
    if(!nativeUiDeviceCurrent(d) || probe.generation!=graphicsResourceGeneration() || !scopeGpuTransactionCurrent(d))return reject(IdleWeaponTrace::Rejection::CollectLifetime);
    probe.idle.inputs=probe.bindings[0].values;
    probe.idle.words=unsigned(probe.programWords);
    std::copy_n(probe.program.begin(),probe.programWords,probe.idle.program.begin());
    probe.idle.declarationCount=probe.bindings[0].count;
    probe.idle.declaration=probe.bindings[0].elements;
    probe.idle.constantCount=probe.bindings[0].constantCount;
    probe.idle.constants=probe.bindings[0].constants;
    return true;
}
static constexpr D3DSAMPLERSTATETYPE imageSamplerStates[]{D3DSAMP_ADDRESSU,D3DSAMP_ADDRESSV,D3DSAMP_ADDRESSW,
    D3DSAMP_MAGFILTER,D3DSAMP_MINFILTER,D3DSAMP_MIPFILTER,D3DSAMP_SRGBTEXTURE,D3DSAMP_MAXMIPLEVEL,
    D3DSAMP_MIPMAPLODBIAS,D3DSAMP_MAXANISOTROPY};
static constexpr DWORD imageSamplerValues[]{D3DTADDRESS_CLAMP,D3DTADDRESS_CLAMP,D3DTADDRESS_CLAMP,
    D3DTEXF_LINEAR,D3DTEXF_LINEAR,D3DTEXF_NONE,0,0,0,1};
static constexpr D3DRENDERSTATETYPE imageStates[]{D3DRS_ZWRITEENABLE,D3DRS_ZFUNC,D3DRS_COLORWRITEENABLE};
static bool restoreImageState(IDirect3DDevice9 *d) noexcept {
    if (!probe.imageChanged) return !probe.imageRestoreFailed;
    probe.imageChanged = false; // One bounded restoration attempt, including foreign unwind.
    if (probe.generation != graphicsResourceGeneration()) {
        probe.imageRestoreFailed = false;
        return true; // Retired invocation: release-only cleanup, no old shader/texture restoration.
    }
    probe.imageRestoreFailed = true;
    bool ok = SUCCEEDED(d->SetPixelShader(probe.bindings[0].color.shader));
    ok = SUCCEEDED(d->SetTexture(0,probe.originalTexture)) && ok;
    ok = SUCCEEDED(d->SetPixelShaderConstantF(0,probe.pixelConstants[0].data(),7)) && ok;
    for (unsigned i=0;i<std::size(imageSamplerStates);++i)
        ok = SUCCEEDED(d->SetSamplerState(0,imageSamplerStates[i],probe.sampler[i])) && ok;
    const DWORD values[]{probe.bindings[0].color.states[1],probe.bindings[0].color.states[2],probe.bindings[0].color.states[7]};
    for (unsigned i=0;i<std::size(imageStates);++i) ok = SUCCEEDED(d->SetRenderState(imageStates[i],values[i])) && ok;
    if (ok) {
        ok = SUCCEEDED(d->GetPixelShader(&probe.restoredShader)) &&
            probe.restoredShader == probe.bindings[0].color.shader &&
            SUCCEEDED(d->GetTexture(0,&probe.restoredTexture)) && probe.restoredTexture == probe.originalTexture;
        std::array<std::array<float,4>,7> constants{};
        ok = SUCCEEDED(d->GetPixelShaderConstantF(0,constants[0].data(),7)) &&
            !std::memcmp(constants.data(),probe.pixelConstants.data(),sizeof(constants)) && ok;
        for (unsigned i=0;i<std::size(imageSamplerStates);++i) {
            DWORD value=0;
            ok = SUCCEEDED(d->GetSamplerState(0,imageSamplerStates[i],&value)) && value == probe.sampler[i] && ok;
        }
        for (unsigned i=0;i<std::size(imageStates);++i) {
            DWORD value=0;
            ok = SUCCEEDED(d->GetRenderState(imageStates[i],&value)) && value == values[i] && ok;
        }
        release(probe.restoredTexture); release(probe.restoredShader);
    }
    probe.imageRestoreFailed = !ok;
    if (!ok) scopeGpuFault();
    return ok;
}
static bool imageCurrent(IDirect3DDevice9 *d) noexcept {
    ScopeRasterObservation current;
    return scopeGpuTransactionCurrent(d) && scopeGpuForwardingAllowed() && nativeUiDeviceCurrent(d) &&
        d->TestCooperativeLevel() == D3D_OK &&
        !probe.imageRestoreFailed && currentScopeRaster(current) && currentScopeQueryBoundary() &&
        current.hand == probe.raster.hand && scopeImagePoseMatches(probe.raster.pose,current.pose);
}
static bool prepareImage(IDirect3DDevice9 *d) {
    auto &color = probe.bindings[0].color;
    if (!color.valid || color.states[15] != FALSE ||
        !scopeGpuImage(d,probe.raster.hand,probe.sourceView,probe.image,probe.imageShader) ||
        probe.sourceView.hand != probe.raster.hand ||
        !scopeImagePoseMatches(probe.sourceView.pose,probe.raster.pose)) return false;
    ScopeOpticalFrame optic;
    ScopeImageCoordinates coordinates;
    Vec3 eye;
    if (!scopeOpticalFrame(probe.cap,probe.raster.pose.affine,optic) ||
        !scopeImageCoordinates(probe.cap,probe.raster.pose.affine,ScopeUvTransform{probe.uvRows},coordinates) ||
        !probe.sourceView.pose.imageCoordinates.valid ||
        coordinates.rows != probe.sourceView.pose.imageCoordinates.rows || !scopeImageEye(eye) ||
        !scopeShaderConstants(optic,probe.sourceView.opticalProjection,coordinates,eye,1,0,0,probe.imageConstants))
        return false;
    Vec3 target;
    if (scopeImageTarget(probe.raster.hand,target))
        scopeReticleTarget(optic,probe.sourceView.opticalProjection,target,.001f,.012f,probe.imageConstants);
    if (FAILED(d->GetTexture(0,&probe.originalTexture)) ||
        FAILED(d->GetPixelShaderConstantF(0,probe.pixelConstants[0].data(),7))) return false;
    for (unsigned i=0;i<std::size(imageSamplerStates);++i)
        if (FAILED(d->GetSamplerState(0,imageSamplerStates[i],&probe.sampler[i]))) return false;
    return imageCurrent(d) && d->TestCooperativeLevel() == D3D_OK;
}
static bool drawImage(IDirect3DDevice9 *d,D3DPRIMITIVETYPE type,INT base,UINT minimum,UINT vertices,
                      UINT start,ScopeIndexedForward forward,ScopeDrawRange range) noexcept {
    probe.imageChanged = true;
    bool ok = SUCCEEDED(d->SetPixelShader(probe.imageShader)) && SUCCEEDED(d->SetTexture(0,probe.image)) &&
        SUCCEEDED(d->SetPixelShaderConstantF(0,probe.imageConstants.rows[0].data(),7));
    for (unsigned i=0;i<std::size(imageSamplerStates);++i)
        ok = SUCCEEDED(d->SetSamplerState(0,imageSamplerStates[i],imageSamplerValues[i])) && ok;
    const DWORD values[]{FALSE,D3DCMP_EQUAL,D3DCOLORWRITEENABLE_RED|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_BLUE};
    for (unsigned i=0;i<std::size(imageStates);++i) ok = SUCCEEDED(d->SetRenderState(imageStates[i],values[i])) && ok;
    if (ok && imageCurrent(d)) ok = SUCCEEDED(forward(d,type,base,minimum,vertices,start+range.firstIndex,range.triangles));
    else ok = false;
    ok = restoreImageState(d) && ok;
    return ok && imageCurrent(d);
}
static HRESULT probeScopeDraw(IDirect3DDevice9 *d,D3DPRIMITIVETYPE type,INT base,UINT minimum,UINT vertices,
                     UINT start,UINT primitives,uintptr_t caller,ScopeIndexedForward forward) noexcept {
    if (!scopeGpuForwardingAllowed()) return D3DERR_INVALIDCALL;
    const auto gfx=reinterpret_cast<uintptr_t>(GetModuleHandleW(L"GfxD3D.dll"));
    if(probe.busy && probe.idleTrace)probe.idleTrace->reject(IdleWeaponTrace::Rejection::GpuReentry);
    if (probe.busy || !gfx || caller != gfx+0xa011) return forward(d,type,base,minimum,vertices,start,primitives);
    // All owning state exists above NativeFinally; no stack-native pointer is
    // kept through any COM call, and no allocation/native callback occurs locked.
    probe.busy=true;
    probe.cap={}; probe.raster={}; probe.idle={}; probe.idleTrace=nullptr; probe.uvRows={}; probe.program={}; probe.colorProgram={};
    probe.generation=graphicsResourceGeneration();
    probe.programWords=0;
    HRESULT result=D3DERR_INVALIDCALL;
    withNativeFinally([&] {
        bool colorCandidate=false;
        bool admitted=currentScopeRaster(probe.raster) && nativeUiDeviceCurrent(d);
        // Establish the borrowed ID1 trace before the first retained COM call,
        // so reentry during AddRef also rejects the outer observation.
        const bool idleCandidate=!probe.raster.pose.valid && currentIdleRaster(probe.idle.raster,probe.idleTrace);
        probe.device = d; d->AddRef();
        if (admitted) probe.transaction = scopeGpuTransactionBegin(d);
        if (admitted) {
            const ScopeIndexedDraw draw{uint32_t(type),base,minimum,vertices,start,primitives};
            ScopeCopyRanges ranges;
            admitted=boundInputs(d,probe.bindings[0],draw) &&
                scopeBufferRanges(probe.bindings[0].values,std::span(probe.bindings[0].elements).first(probe.bindings[0].count),ranges) && boundProgram();
            if (admitted) colorCandidate=boundColor(d,probe.bindings[0].color,true);
            const std::array<std::span<uint8_t>,5> bytes{std::span(probe.positions).first(ScopeVertices*12),
                std::span(probe.indices).first(ScopeTriangles*6),std::span(probe.weights).first(ScopeVertices*4),
                std::span(probe.localIndices).first(ScopeVertices*4),std::span(probe.uv).first(ScopeVertices*8)};
            for (unsigned i=0;admitted && i<bytes.size();++i) admitted=copySlice(i==1,ranges.slices[i],bytes[i]);
            if (probe.lock.outstanding() || fatalDraw.load(std::memory_order_acquire)) return;
            if (admitted) admitted=boundInputs(d,probe.bindings[1],draw) &&
                probe.bindings[0].values==probe.bindings[1].values &&
                probe.bindings[0].constantCount == probe.bindings[1].constantCount &&
                !std::memcmp(probe.bindings[0].constants.data(),probe.bindings[1].constants.data(),
                             probe.bindings[0].constantCount*4*sizeof(float)) &&
                probe.bindings[0].identity[5]==probe.bindings[1].identity[5] &&
                probe.bindings[0].identity[6]==probe.bindings[1].identity[6] &&
                !std::memcmp(probe.bindings[0].uvRows.data(),probe.bindings[1].uvRows.data(),sizeof(probe.uvRows)) &&
                probe.bindings[0].count==probe.bindings[1].count &&
                std::equal(probe.bindings[0].elements.begin(),probe.bindings[0].elements.begin()+probe.bindings[0].count,
                           probe.bindings[1].elements.begin()) && nativeUiDeviceCurrent(d) &&
                probe.generation==graphicsResourceGeneration() && scopeGpuRoutingCurrent(d);
            if (admitted) {
                probe.uvRows=probe.bindings[0].uvRows;
                colorCandidate=colorCandidate && boundColor(d,probe.bindings[1].color,false) &&
                    sameColor(probe.bindings[0].color,probe.bindings[1].color);
            }
            if (admitted) admitted=hashSlices(); // Hash ONLY owned copies, after all unlocks.
            colorCandidate = colorCandidate && admitted && probe.raster.capClipValid &&
                scopeCapPosition(std::span(probe.program).first(probe.programWords),
                    std::span(probe.bindings[0].constants).first(probe.bindings[0].constantCount),
                    probe.cap,probe.bindings[0].values.weights.object != 0,probe.raster.capClip);
            // Keep the first immutable identities through the whole optional
            // transaction; resample after hashing, getters and source retention.
            if (admitted && colorCandidate && probe.transaction && prepareImage(d)) {
                releaseBindings(probe.bindings[1]);
                auto &a = probe.bindings[0]; auto &b = probe.bindings[1];
                const bool unchanged = boundInputs(d,b,draw) && boundColor(d,b.color,false) &&
                    a.values == b.values && a.count == b.count && a.elements == b.elements &&
                    std::equal(std::begin(a.identity),std::end(a.identity),std::begin(b.identity)) &&
                    a.constantCount == b.constantCount &&
                    !std::memcmp(a.constants.data(),b.constants.data(),a.constantCount*4*sizeof(float)) &&
                    sameColor(a.color,b.color) && imageCurrent(d) && start <= UINT32_MAX-ScopeTriangles*3;
                if (unchanged) {
                    probe.split = true;
                    const bool rendered = executeScopeCapDraw(
                        [&](ScopeDrawRange range) noexcept {
                            return SUCCEEDED(forward(d,type,base,minimum,vertices,start+range.firstIndex,range.triangles));
                        },[&](ScopeDrawRange range) noexcept {
                            return drawImage(d,type,base,minimum,vertices,start,forward,range);
                        },[&]() noexcept { return imageCurrent(d); });
                    if (!rendered) { scopeGpuFault(); result = D3DERR_INVALIDCALL; }
                    else result = D3D_OK;
                }
            }
        }
        bool idleAdmitted=false;
        if(idleCandidate) {
            probe.transaction=scopeGpuTransactionBegin(d);
            idleAdmitted=probe.transaction && collectIdleGeometry(d,{uint32_t(type),base,minimum,vertices,start,primitives});
        }
        if(probe.idleTrace && !idleAdmitted)probe.idleTrace->reject(IdleWeaponTrace::Rejection::GpuAdmission,IdleWeaponTrace::checks({idleCandidate,probe.transaction!=0}));
        if (!scopeGpuForwardingAllowed()) return;
        if (!probe.split) result=forward(d,type,base,minimum,vertices,start,primitives);
        if(probe.idleTrace && idleAdmitted) {
            IdleRasterCopy now;IdleWeaponTrace *trace=nullptr;
            const bool current=currentIdleRaster(now,trace) && trace==probe.idleTrace && now==probe.idle.raster &&
                nativeUiDeviceCurrent(d) && probe.generation==graphicsResourceGeneration() && scopeGpuTransactionCurrent(d);
            probe.idleTrace->draw(probe.idle,SUCCEEDED(result),current);
        }
        if (probe.raster.pose.valid) {
            if (!admitted || FAILED(result) || probe.generation!=graphicsResourceGeneration()) retireScopeGeometry();
            else recordScopeGeometry(probe.raster,probe.cap,ScopeUvTransform{probe.uvRows},colorCandidate);
        }
    },[](bool aborted) noexcept { cleanup(aborted); });
    return result;
}
HRESULT scopeGpuDraw(IDirect3DDevice9 *d,D3DPRIMITIVETYPE type,INT base,UINT minimum,UINT vertices,
                     UINT start,UINT primitives,uintptr_t caller,ScopeIndexedForward forward) noexcept {
    scopeGpuOutput(d);
    if (!scopeGpuForwardingAllowed()) return D3DERR_INVALIDCALL;
    return withScopeDrawCoverage(scopeGpuRoutingCurrent(d),
        [&] { return probeScopeDraw(d,type,base,minimum,vertices,start,primitives,caller,forward); },
        [&] { return forward(d,type,base,minimum,vertices,start,primitives); });
}
} // namespace ss2vr::game
