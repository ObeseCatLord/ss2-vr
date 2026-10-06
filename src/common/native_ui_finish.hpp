#pragma once
#include <cstdint>
namespace ss2vr {
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
