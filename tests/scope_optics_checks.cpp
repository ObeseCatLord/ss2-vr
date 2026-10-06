#ifdef NDEBUG
#error Optical checks require assertions
#endif
#include "common/scope_optics.hpp"
#include <cassert>
#include <limits>
using namespace ss2vr;
static bool close(float a,float b,float e=1e-5f) { return std::abs(a-b)<e; }
static ScopeCapGeometry aperture() {
    ScopeCapGeometry cap;
    for (size_t i=0;i<ScopeCapVertices;++i) {
        const float a=float(i)*2*Pi/float(ScopeCapVertices);
        cap.positions[i]={.02f*std::cos(a),.3f+.02f*std::sin(a),.05f};
    }
    for (size_t i=0;i<ScopeCapEnd-ScopeCapFirst;++i) {
        cap.indices[3*i]=0; cap.indices[3*i+1]=uint16_t(i+1); cap.indices[3*i+2]=uint16_t(i+2);
    }
    cap.valid=true; return cap;
}
int main() {
    const auto nativeZoom = scopeNativeZoom(1, 1, 1, .25f, .5f);
    assert(nativeZoom.valid && nativeZoom.startMultiplier == 1 &&
           nativeZoom.endMultiplier == .25f && nativeZoom.progress == .5f);
    assert(!scopeNativeZoom(0, 1, 1, .25f, .5f).valid);
    assert(!scopeNativeZoom(1, 0, 1, .25f, .5f).valid);
    assert(!scopeNativeZoom(2, 1, 1, .25f, .5f).valid);
    assert(!scopeNativeZoom(1, 2, 1, .25f, .5f).valid);
    for (float invalid : {-1.f, 1.01f, std::numeric_limits<float>::infinity(),
                          std::numeric_limits<float>::quiet_NaN()}) {
        assert(!scopeNativeZoom(1, 1, invalid, .25f, .5f).valid);
        assert(!scopeNativeZoom(1, 1, 1, invalid, .5f).valid);
        assert(!scopeNativeZoom(1, 1, 1, .25f, invalid).valid);
    }
    assert(!scopeNativeZoom(1, 1, 0, .25f, 0).valid);
    assert(!scopeNativeZoom(1, 1, 1, 0, 0).valid);
    assert(scopeNativeZoom(1, 1, 1, .25f, 0).valid);
    assert(scopeNativeZoom(1, 1, 1, .25f, 1).valid);
    // Recover the same native base angle despite a different player's/shared
    // zoom multiplier. Each scope still uses its own observed interpolation.
    for (float degrees : {45.f,60.f,90.f,120.f,135.f}) {
        const float base = degrees*Pi/180;
        for (float shared : {1.f,.75f,.25f,.01f}) {
            const float recovered = scopeNativeBaseFov(base*shared,shared);
            assert(close(recovered,base,2e-6f));
            float magnification = 0, expected = 0;
            assert(scopeNativeMagnification(recovered,nativeZoom,magnification));
            assert(scopeAngularMagnification(base,base*.625f,expected));
            assert(close(magnification,expected,3e-6f));
        }
    }
    for (float invalid : {0.f,-1.f,std::numeric_limits<float>::infinity(),
                          std::numeric_limits<float>::quiet_NaN()}) {
        assert(scopeNativeBaseFov(invalid,1)==0);
        assert(scopeNativeBaseFov(Pi/2,invalid)==0);
    }
    assert(scopeNativeBaseFov(Pi/2,1.1f)==0);
    assert(scopeNativeBaseFov(44.f*Pi/180,1)==0);
    assert(scopeNativeBaseFov(136.f*Pi/180,1)==0);
    float invalidMagnification = 7;
    assert(!scopeNativeMagnification(Pi/2,{},invalidMagnification) && invalidMagnification==0);
    auto corruptZoom = nativeZoom; corruptZoom.progress = 2;
    assert(!scopeNativeMagnification(Pi/2,corruptZoom,invalidMagnification));
    float angularMagnification;
    assert(scopeAngularMagnification(Pi/2,Pi/4,angularMagnification));
    // Analytic45-degree half-angle result. This is not reciprocal angle scale2.
    assert(close(angularMagnification,1+std::sqrt(2.f)));
    for (float degrees : {1.f,30.f,60.f,90.f,120.f,170.f}) {
        const float base=degrees*Pi/180;
        assert(scopeAngularMagnification(base,base,angularMagnification) && angularMagnification==1);
        for (float target : {1.1f,2.f,4.f,10.f,50.f}) {
            const float narrowed=float(2*std::atan(std::tan(double(base)*.5)/double(target)));
            assert(scopeAngularMagnification(base,narrowed,angularMagnification));
            assert(close(angularMagnification,target,target*2e-6f));
        }
    }
    for (const auto angles : {std::array<float,2>{0,0}, {Pi/2,0}, {Pi,Pi/2},
                             {Pi/4,Pi/2}, {Pi/2,-Pi/4}, {Pi/2,1e-6f},
                             {std::numeric_limits<float>::infinity(),Pi/4},
                             {Pi/2,std::numeric_limits<float>::quiet_NaN()}}) {
        angularMagnification=7;
        assert(!scopeAngularMagnification(angles[0],angles[1],angularMagnification) && angularMagnification==0);
    }
    const auto cap=aperture();
    ScopeOpticalFrame f;
    assert(scopeOpticalFrame(cap,matrix({}),f));
    assert(close(f.camera.m[3],0) && close(f.camera.m[7],.3f) && close(f.camera.m[11],.05f));
    assert(close(f.radiusX,.02f) && close(f.radiusY,.02f));
    // Animated full quaternion + XYZ translation + mirror/shear/stretch.
    const Matrix34 deform{{-1.5f,.2f,0,0, 0,.7f,.3f,0, 0,0,1.2f,0}};
    const Matrix34 moved=affineMultiply(matrix({normalize({.3f,-.2f,.4f,.8f}),{2,-3,4}}),deform);
    ScopeOpticalFrame reflected;
    assert(scopeOpticalFrame(cap,moved,reflected));
    Vec3 x{reflected.camera.m[0],reflected.camera.m[4],reflected.camera.m[8]};
    Vec3 y{reflected.camera.m[1],reflected.camera.m[5],reflected.camera.m[9]};
    Vec3 z{reflected.camera.m[2],reflected.camera.m[6],reflected.camera.m[10]};
    assert(close(dot(x,x),1) && close(dot(y,y),1) && close(dot(z,z),1));
    assert(close(dot(x,y),0) && close(dot(x,z),0) && close(dot(y,z),0) && close(dot(cross(x,y),z),1));
    ScopeCapGeometry world;
    assert(scopeCapWorld(cap,moved,world));
    const Vec3 edgeNormal=cross(world.positions[1]-world.positions[0],world.positions[2]-world.positions[0]);
    assert(dot(edgeNormal,z)<0); // Reflection doesn't turn the camera backwards.
    for (auto p:world.positions) {
        const Vec3 plane=scopeTransformPoint(reflected.inverseCamera,p);
        assert(close(plane.z,0,2e-5f));
    }
    for (float translation : {100.f,1000.f,10000.f,1048576.f}) {
        for (const auto base : {matrix({}),moved}) {
            ScopeOpticalFrame reference,distant;
            assert(scopeOpticalFrame(cap,base,reference));
            auto shifted=base;
            shifted.m[3]+=translation; shifted.m[7]-=translation; shifted.m[11]+=translation;
            assert(scopeOpticalFrame(cap,shifted,distant));
            assert(close(reference.radiusX,distant.radiusX,1e-7f) &&
                   close(reference.radiusY,distant.radiusY,1e-7f));
            for (unsigned i : {0u,1u,2u,4u,5u,6u,8u,9u,10u})
                assert(close(reference.camera.m[i],distant.camera.m[i],1e-7f));
        }
    }
    ScopeOpticalProjection ordinary,zoom;
    assert(scopeOpticalProjection(f,.1f,1,ordinary));
    assert(scopeOpticalProjection(f,.1f,4,zoom));
    ScopeOpticalProjection nativeAngularZoom;
    assert(scopeAngularMagnification(Pi/2,Pi/4,angularMagnification));
    assert(scopeOpticalProjection(f,.1f,angularMagnification,nativeAngularZoom));
    assert(close(nativeAngularZoom.tangentX*angularMagnification,ordinary.tangentX));
    Fov normalFov,zoomFov;
    assert(scopeOpticalFov(ordinary,normalFov) && scopeOpticalFov(zoom,zoomFov));
    assert(close(std::tan(normalFov.right),4*std::tan(zoomFov.right)));
    // An object at the source's horizontal edge is presented at the physical
    // aperture's edge: real content magnifies 4x, not a narrowed crop's 1x.
    const float objectAngle=std::atan(zoom.tangentX);
    const Vec3 apparent{4*std::tan(objectAngle),0,-1};
    std::array<float,2> uv;
    assert(scopeImageUv(f,zoom,apparent,uv) && close(uv[0],1) && close(uv[1],.5f));
    assert(scopeImageUv(f,zoom,{0,.2f,-1},uv) && close(uv[1],0));
    assert(scopeImageUv(f,zoom,{0,0,-1},uv) && close(uv[0],.5f) && close(uv[1],.5f));
    for (float scale : {std::numeric_limits<float>::denorm_min(),1e-30f,1e-7f,1.f,1e30f}) {
        assert(scopeImageUv(f,zoom,{0,0,-scale},uv) && close(uv[0],.5f) && close(uv[1],.5f));
        if (scale>std::numeric_limits<float>::denorm_min())
            assert(scopeImageUv(f,zoom,{.1f*scale,-.05f*scale,-scale},uv) &&
                   close(uv[0],.75f) && close(uv[1],.625f));
    }
    auto malformed=zoom;
    malformed.magnification=std::numeric_limits<float>::max(); malformed.tangentX=malformed.tangentY=2;
    assert(!scopeImageUv(f,malformed,{0,0,-1},uv) && !scopeOpticalFov(malformed,zoomFov));
    auto nonRigid=f; nonRigid.inverseCamera.m[0]=2;
    assert(!scopeImageUv(nonRigid,zoom,{.1f,0,-1},uv));
    nonRigid=f; nonRigid.camera.m[0]=2; nonRigid.inverseCamera.m[0]=.5f;
    assert(!scopeImageUv(nonRigid,zoom,{.1f,0,-1},uv));
    assert(!scopeImageUv(f,zoom,{0,0,1},uv));
    assert(!scopeImageUv(f,zoom,{1,0,0},uv));
    assert(!scopeImageUv(f,zoom,{std::numeric_limits<float>::quiet_NaN(),0,-1},uv));
    assert(!scopeOpticalProjection(f,0,4,zoom) && !zoom.valid);
    assert(!scopeOpticalProjection(f,.1f,.5f,zoom));
    auto bad=cap; bad.positions[0].z+=.01f;
    assert(!scopeOpticalFrame(bad,matrix({}),f) && !f.valid);
    bad=cap; std::swap(bad.indices[0],bad.indices[1]);
    assert(!scopeOpticalFrame(bad,matrix({}),f));
    bad=cap; bad.indices[0]=ScopeCapVertices;
    assert(!scopeOpticalFrame(bad,matrix({}),f));
    assert(!scopeOpticalFrame(cap,{},f));
}
