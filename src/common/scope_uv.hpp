#pragma once
#include "scope_optics.hpp"

namespace ss2vr {
// Actual native VS c8/c9 for an admitted FLOAT2 source. The input's omitted
// components expand to z0/w1. Shader/material/purpose admission is external.
struct ScopeUvTransform {
    std::array<std::array<float,4>,2> rows{{{{1,0,0,0}},{{0,1,0,0}}}};
};
struct ScopeImageCoordinates {
    // dot(float3(postNativeUv,1),rows[i].xyz) -> optic-local XYZ.
    std::array<std::array<float,4>,3> rows{};
    bool valid = false;
};
// Fit an affine UV -> optic plane map from the actual remapped cap. All means
// and covariance calculations use double precision and centered coordinates;
// no large translated world point is subtracted. Original cap triangles still
// define the physical aperture, including shear/reflection.
inline bool scopeImageCoordinates(const ScopeCapGeometry &cap, const Matrix34 &affine,
                                  const ScopeUvTransform &native, ScopeImageCoordinates &out) {
    out={};
    ScopeOpticalFrame frame;
    if (!scopeOpticalFrame(cap,affine,frame)) return false;
    for (const auto &row:native.rows)
        for (float v:row) if (!std::isfinite(v)) return false;
    const double a=native.rows[0][0],b=native.rows[0][1],c=native.rows[1][0],d=native.rows[1][1];
    const double determinant=a*d-b*c, scale=a*a+b*b+c*c+d*d;
    if (!std::isfinite(scale) || scale<1e-12 || std::abs(determinant)<=scale*1e-6) return false;
    const double inverse[2][2]{{d/determinant,-b/determinant},{-c/determinant,a/determinant}};
    double uvMean[2]{}, positionMean[3]{};
    for (size_t i=0;i<ScopeCapVertices;++i) {
        for (unsigned j=0;j<2;++j) {
            if (!std::isfinite(cap.uv[i][j]) || std::abs(cap.uv[i][j])>64) return false;
            uvMean[j]+=cap.uv[i][j];
        }
        positionMean[0]+=cap.positions[i].x; positionMean[1]+=cap.positions[i].y; positionMean[2]+=cap.positions[i].z;
    }
    for (double &v:uvMean) v/=ScopeCapVertices;
    for (double &v:positionMean) v/=ScopeCapVertices;
    double uu=0,uv=0,vv=0, cov[3][2]{};
    for (size_t i=0;i<ScopeCapVertices;++i) {
        const double du=double(cap.uv[i][0])-uvMean[0],dv=double(cap.uv[i][1])-uvMean[1];
        const double p[3]{cap.positions[i].x,cap.positions[i].y,cap.positions[i].z};
        uu+=du*du; uv+=du*dv; vv+=dv*dv;
        for (unsigned r=0;r<3;++r) { cov[r][0]+=(p[r]-positionMean[r])*du; cov[r][1]+=(p[r]-positionMean[r])*dv; }
    }
    const double det=uu*vv-uv*uv;
    if (!std::isfinite(det) || uu<=1e-12 || vv<=1e-12 || det<=(uu+vv)*(uu+vv)*1e-6) return false;
    double model[3][2]{};
    for (unsigned r=0;r<3;++r) {
        model[r][0]=(cov[r][0]*vv-cov[r][1]*uv)/det;
        model[r][1]=(cov[r][1]*uu-cov[r][0]*uv)/det;
    }
    // A local UV fit is necessary even if a subsequent tiny affine scale could
    // hide inconsistent positions. No non-affine map reaches the GPU program.
    for (size_t i=0;i<ScopeCapVertices;++i) {
        const double p[3]{cap.positions[i].x,cap.positions[i].y,cap.positions[i].z};
        for (unsigned r=0;r<3;++r) {
            const double fitted=positionMean[r]+model[r][0]*(cap.uv[i][0]-uvMean[0])+model[r][1]*(cap.uv[i][1]-uvMean[1]);
            if (!std::isfinite(fitted) || std::abs(fitted-p[r])>1e-5) return false;
        }
    }
    double centered[3][2]{};
    for (unsigned r=0;r<3;++r)
        for (unsigned column=0;column<2;++column)
            for (unsigned k=0;k<3;++k)
                for (unsigned j=0;j<3;++j)
                    centered[r][column]+=double(frame.inverseCamera.m[4*r+k])*affine.m[4*k+j]*model[j][column];
    // frame center uses the existing float mean. Retain its small difference
    // from the double fit mean without introducing a world-space subtraction.
    Vec3 frameMean{};
    for (const auto p:cap.positions) frameMean=frameMean+p;
    frameMean=frameMean*(1.f/float(ScopeCapVertices));
    const double shift[3]{positionMean[0]-frameMean.x,positionMean[1]-frameMean.y,positionMean[2]-frameMean.z};
    ScopeImageCoordinates result;
    for (unsigned r=0;r<3;++r) {
        double offset=0;
        for (unsigned k=0;k<3;++k)
            for (unsigned j=0;j<3;++j)
                offset+=double(frame.inverseCamera.m[4*r+k])*affine.m[4*k+j]*shift[j];
        offset-=centered[r][0]*uvMean[0]+centered[r][1]*uvMean[1];
        const double x=centered[r][0]*inverse[0][0]+centered[r][1]*inverse[1][0];
        const double y=centered[r][0]*inverse[0][1]+centered[r][1]*inverse[1][1];
        const double z=offset-x*native.rows[0][3]-y*native.rows[1][3];
        const double coefficients[3]{x,y,z};
        for (unsigned j=0;j<3;++j) {
            if (!std::isfinite(coefficients[j]) || std::abs(coefficients[j])>4096) return false;
            result.rows[r][j]=float(coefficients[j]);
        }
    }
    // Check the narrowed coefficients against the float varying consumed by
    // the GPU. A finite double fit can still lose the lens after a huge native
    // UV offset/scale, even when every final coefficient fits in a float.
    for (size_t i=0;i<ScopeCapVertices;++i) {
        const float u=native.rows[0][0]*cap.uv[i][0]+native.rows[0][1]*cap.uv[i][1]+native.rows[0][3];
        const float v=native.rows[1][0]*cap.uv[i][0]+native.rows[1][1]*cap.uv[i][1]+native.rows[1][3];
        if (!std::isfinite(u) || !std::isfinite(v)) return false;
        const double delta[3]{double(cap.positions[i].x)-frameMean.x,
                              double(cap.positions[i].y)-frameMean.y,
                              double(cap.positions[i].z)-frameMean.z};
        for (unsigned r=0;r<3;++r) {
            double expected=0;
            for (unsigned k=0;k<3;++k)
                for (unsigned j=0;j<3;++j)
                    expected+=double(frame.inverseCamera.m[4*r+k])*affine.m[4*k+j]*delta[j];
            const float actual=result.rows[r][0]*u+result.rows[r][1]*v+result.rows[r][2];
            if (!std::isfinite(actual) || std::abs(double(actual)-expected)>1e-5) return false;
        }
    }
    result.valid=true; out=result; return true;
}
} // namespace ss2vr
