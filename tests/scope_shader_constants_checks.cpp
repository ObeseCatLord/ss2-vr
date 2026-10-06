#include "common/scope_shader_constants.hpp"
#include <cassert>
#include <limits>
using namespace ss2vr;
static bool close(float a, float b) { return std::abs(a-b) < 2e-5f; }
int main() {
    ScopeOpticalFrame frame;
    frame.camera = frame.inverseCamera = matrix({});
    frame.radiusX = frame.radiusY = .02f;
    frame.valid = true;
    ScopeOpticalProjection projection;
    assert(scopeOpticalProjection(frame, .1f, 4, projection));
    ScopeImageCoordinates coordinates;
    coordinates.rows = {{{.04f,0,-.02f,0},{0,-.04f,.02f,0},{0,0,0,0}}};
    coordinates.valid = true;
    ScopeShaderConstants constants;
    for (Vec3 eye : {Vec3{0,0,.1f}, Vec3{.01f,-.005f,.1f}, Vec3{-.005f,.004f,.2f}}) {
        assert(scopeShaderConstants(frame, projection, coordinates, eye, .7f, .001f, .01f, constants));
        assert(constants.valid && constants.rows[5][0] == .7f);
        for (float u : {0.f,.25f,.5f,.75f,1.f}) for (float v : {0.f,.5f,1.f}) {
            // Evaluate the HLSL constant interface separately from the builder.
            const Vec3 aperture{.04f*u-.02f, -.04f*v+.02f, 0};
            const Vec3 ray = aperture-eye;
            const float shaderU = (constants.rows[0][0]*u+constants.rows[0][1]*v+
                constants.rows[0][2]-constants.rows[3][0])/constants.rows[3][2]*constants.rows[4][0]+constants.rows[4][2];
            const float shaderV = (constants.rows[1][0]*u+constants.rows[1][1]*v+
                constants.rows[1][2]-constants.rows[3][1])/constants.rows[3][2]*constants.rows[4][1]+constants.rows[4][3];
            std::array<float,2> expected;
            assert(scopeImageUv(frame, projection, ray, expected));
            assert(close(shaderU,expected[0]) && close(shaderV,expected[1]));
        }
    }
    assert(scopeShaderConstants(frame,projection,coordinates,{0,0,.1f},1,0,0,constants));
    assert(constants.rows[6][2]==0); // Missing native aim is explicitly disabled.
    assert(scopeReticleTarget(frame,projection,{1,0,-40},.001f,.012f,constants));
    assert(close(constants.rows[6][0],.75f) && close(constants.rows[6][1],.5f));
    assert(constants.rows[6][2]==.001f && constants.rows[6][3]==.012f);
    assert(scopeReticleTarget(frame,projection,{0,1,-40},.001f,.012f,constants));
    assert(close(constants.rows[6][0],.5f) && close(constants.rows[6][1],.25f));
    for (Vec3 bad : {Vec3{0,0,1},Vec3{0,0,0},Vec3{10,0,-1},
                    Vec3{0,0,std::numeric_limits<float>::quiet_NaN()}}) {
        assert(!scopeReticleTarget(frame,projection,bad,.001f,.012f,constants));
        assert((constants.rows[6]==std::array<float,4>{}));
    }
    const Pose placement{normalize({.2f,-.3f,.4f,.8f}), {10,-20,30}};
    frame.camera = matrix(placement); frame.inverseCamera = matrix(inverse(placement));
    const Vec3 relative{.01f,-.005f,.1f};
    const auto eye = placement.p+rotate(placement.q,relative);
    assert(scopeShaderConstants(frame,projection,coordinates,eye,1,0,0,constants));
    assert(close(constants.rows[3][0],relative.x) && close(constants.rows[3][1],relative.y) &&
           close(constants.rows[3][2],relative.z));
    assert(scopeReticleTarget(frame,projection,placement.p+rotate(placement.q,{1,0,-40}),.001f,.012f,constants));
    assert(close(constants.rows[6][0],.75f) && close(constants.rows[6][1],.5f));
    frame.camera = frame.inverseCamera = matrix({});
    for (Vec3 badEye : {Vec3{0,0,0},Vec3{0,0,-1},Vec3{0,0,std::numeric_limits<float>::infinity()}}) {
        assert(!scopeShaderConstants(frame,projection,coordinates,badEye,1,0,0,constants));
        assert(!constants.valid && constants.rows[4][0] == 0);
    }
    for (float visibility : {-1.f,1.1f,std::numeric_limits<float>::quiet_NaN()})
        assert(!scopeShaderConstants(frame,projection,coordinates,{0,0,.1f},visibility,0,0,constants));
    assert(!scopeShaderConstants(frame,projection,coordinates,{0,0,.1f},1,.02f,.01f,constants));
    assert(!scopeShaderConstants(frame,projection,coordinates,{0,0,.1f},1,0,.6f,constants));
    coordinates.rows[2][0] = std::numeric_limits<float>::quiet_NaN();
    assert(!scopeShaderConstants(frame,projection,coordinates,{0,0,.1f},1,0,0,constants));
}
