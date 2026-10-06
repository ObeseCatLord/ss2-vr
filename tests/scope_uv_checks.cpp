#ifdef NDEBUG
#error Scope UV checks require assertions
#endif
#include "common/scope_uv.hpp"
#include <cassert>
#include <limits>
using namespace ss2vr;
static bool close(float a,float b,float tolerance=2e-5f) { return std::abs(a-b)<tolerance; }
static ScopeCapGeometry aperture() {
    ScopeCapGeometry cap;
    for (size_t i=0;i<ScopeCapVertices;++i) {
        const float a=float(i)*2*Pi/float(ScopeCapVertices);
        const float x=.02f*std::cos(a), y=.02f*std::sin(a);
        cap.positions[i]={x,.3f+y,.05f};
        cap.uv[i]={.9f+5*x,.78f-10*y};
    }
    for (size_t i=0;i<ScopeCapEnd-ScopeCapFirst;++i) {
        cap.indices[3*i]=0; cap.indices[3*i+1]=uint16_t(i+1); cap.indices[3*i+2]=uint16_t(i+2);
    }
    cap.valid=true; return cap;
}
static Vec3 mapped(const ScopeImageCoordinates &map,float u,float v) {
    return {map.rows[0][0]*u+map.rows[0][1]*v+map.rows[0][2],
            map.rows[1][0]*u+map.rows[1][1]*v+map.rows[1][2],
            map.rows[2][0]*u+map.rows[2][1]*v+map.rows[2][2]};
}
// Independent centered reference: multiply only the native linear affine,
// then project onto optical basis columns. No world-coordinate subtraction.
static Vec3 reference(const ScopeCapGeometry &cap,const Matrix34 &affine,const ScopeOpticalFrame &frame,Vec3 p) {
    Vec3 mean{}; for (const auto v:cap.positions) mean=mean+v;
    mean=mean*(1.f/float(ScopeCapVertices));
    const Vec3 delta=p-mean;
    const Vec3 moved{affine.m[0]*delta.x+affine.m[1]*delta.y+affine.m[2]*delta.z,
                     affine.m[4]*delta.x+affine.m[5]*delta.y+affine.m[6]*delta.z,
                     affine.m[8]*delta.x+affine.m[9]*delta.y+affine.m[10]*delta.z};
    return {dot(moved,{frame.camera.m[0],frame.camera.m[4],frame.camera.m[8]}),
            dot(moved,{frame.camera.m[1],frame.camera.m[5],frame.camera.m[9]}),
            dot(moved,{frame.camera.m[2],frame.camera.m[6],frame.camera.m[10]})};
}
int main() {
    const auto cap=aperture();
    ScopeImageCoordinates map;
    assert(scopeImageCoordinates(cap,matrix({}),{},map) && map.valid);
    auto center=mapped(map,.9f,.78f);
    assert(close(center.x,0) && close(center.y,0) && close(center.z,0));
    assert(close(mapped(map,1,.78f).x,.02f));
    assert(close(mapped(map,.9f,.58f).y,.02f));
    const Matrix34 deformation{{-1.5f,.2f,0,0,0,.7f,.3f,0,0,0,1.2f,0}};
    const Matrix34 posed=affineMultiply(matrix({normalize({.3f,-.2f,.4f,.8f}),{2,-3,4}}),deformation);
    // Native material UV rotation/shear/stretch/offset; z coefficients are
    // arbitrary because the admitted FLOAT2 expands z0/w1.
    ScopeUvTransform native;
    native.rows[0]={2,.4f,12,.3f}; native.rows[1]={-.5f,1.5f,-27,-.2f};
    assert(scopeImageCoordinates(cap,posed,native,map));
    ScopeOpticalFrame frame;
    assert(scopeOpticalFrame(cap,posed,frame));
    for (size_t i=0;i<ScopeCapVertices;++i) {
        const auto uv=cap.uv[i];
        const float u=2*uv[0]+.4f*uv[1]+.3f, v=-.5f*uv[0]+1.5f*uv[1]-.2f;
        const auto actual=mapped(map,u,v);
        const auto expected=reference(cap,posed,frame,cap.positions[i]);
        assert(close(actual.x,expected.x) && close(actual.y,expected.y) && close(actual.z,expected.z));
    }
    auto distant=posed; distant.m[3]+=1048576; distant.m[7]-=1048576; distant.m[11]+=1048576;
    ScopeImageCoordinates farMap;
    assert(scopeImageCoordinates(cap,distant,native,farMap));
    assert(farMap.rows==map.rows); // The coefficients contain no world translation.
    // Perspective interpolation: unequal clip W preserves the affine cap map
    // through the native UV varying; screen-linear interpolation is insufficient.
    const unsigned triangle[]{cap.indices[0],cap.indices[1],cap.indices[2]};
    const double bary[]{.2,.3,.5}, clipW[]{.15,.4,.8};
    double denominator=0; float u=0,v=0; Vec3 position{};
    for (unsigned i=0;i<3;++i) denominator+=bary[i]/clipW[i];
    for (unsigned i=0;i<3;++i) {
        const float weight=float((bary[i]/clipW[i])/denominator);
        const auto uv=cap.uv[triangle[i]];
        u+=weight*(2*uv[0]+.4f*uv[1]+.3f); v+=weight*(-.5f*uv[0]+1.5f*uv[1]-.2f);
        position=position+cap.positions[triangle[i]]*weight;
    }
    const auto actual=mapped(map,u,v);
    const auto expected=reference(cap,posed,frame,position);
    assert(close(actual.x,expected.x) && close(actual.y,expected.y) && close(actual.z,expected.z));
    auto offset=native; offset.rows[0][3]=4096; offset.rows[1][3]=-4096;
    assert(!scopeImageCoordinates(cap,posed,offset,map) && !map.valid);
    auto badlyConditioned=native; badlyConditioned.rows[0]={1,0,0,0};
    badlyConditioned.rows[1]={0,1e-7f,0,0};
    assert(!scopeImageCoordinates(cap,posed,badlyConditioned,map));
    auto bad=cap; for (auto &uv:bad.uv) uv[1]=1e-5f*(uv[1]-.78f);
    assert(!scopeImageCoordinates(bad,posed,native,map));
    bad=cap; bad.valid=false;
    assert(!scopeImageCoordinates(bad,posed,native,map) && !map.valid);
    bad=cap; for (auto &uv:bad.uv) uv={1,1};
    assert(!scopeImageCoordinates(bad,posed,native,map));
    bad=cap; bad.uv[0][0]+=.01f;
    assert(!scopeImageCoordinates(bad,posed,native,map));
    bad=cap; bad.uv[0][0]=std::numeric_limits<float>::quiet_NaN();
    assert(!scopeImageCoordinates(bad,posed,native,map));
    auto singular=native; singular.rows[1]=singular.rows[0];
    assert(!scopeImageCoordinates(cap,posed,singular,map));
    singular=native; singular.rows[0][3]=std::numeric_limits<float>::max();
    assert(!scopeImageCoordinates(cap,posed,singular,map));
}
