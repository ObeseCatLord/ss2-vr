#pragma once
#include <cstdint>
namespace ss2vr {
// Only for fingerprinted native builtin programs already admitted by device
// identity. The backend declares TEXCOORD0 into v0, with its position buffer
// in stream0. These numbers are D3D9 declaration ABI tags, not a general alias.
inline bool nativeUiPositionInput(uint32_t stream,uint32_t offset,uint32_t type,
                                  uint32_t method,uint32_t usage,uint32_t index) {
    return stream==0 && offset==0 && type==2 && method==0 && usage==5 && index==0;
}
// D3D9 triangle topology values. Device-side static assertions verify the ABI.
// Point/line pixel footprints do not follow the admitted panel homography.
inline bool nativeUiTriangleTopology(uint32_t topology,uint32_t fill) {
    return topology>=4 && topology<=6 && fill==3; // D3DFILL_SOLID
}
// Keep explicit lock ownership until confirmed retirement. The caller may
// attempt one bounded cleanup, then quarantine the surface until device reset.
template<class Surface,class Unlock,class Reject>
bool retireNativeUiLock(Surface *&owned,Unlock unlock,Reject reject) {
    if (!owned) return true;
    if (!unlock(owned)) { reject(); return false; }
    owned=nullptr;
    return true;
}
// Called after all potentially waiting UI uploads, immediately before counting
// and submitting. Session loss suppresses EVERY layer, including wheels/HUD.
template<class Layers>
bool finalNativeUiLayers(bool quit,bool loss,Layers &layers,bool world) {
    if (quit || loss) { layers.clear(); return false; }
    return world;
}
} // namespace ss2vr
