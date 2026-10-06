#pragma once
#include "model_tree.hpp"
#include <array>
#include <cstring>

namespace ss2vr {
// The geometry is copied from admitted live slices. No owned game asset is
// compiled into the mod. These constants describe the audited stock layout.
constexpr size_t ScopeVertices = 884, ScopeTriangles = 928;
constexpr size_t ScopeCapFirst = 901, ScopeCapEnd = 923;
constexpr size_t ScopeCapVertices = 24, ScopeCapIndices = (ScopeCapEnd - ScopeCapFirst) * 3;
struct ScopeCapGeometry {
    std::array<Vec3, ScopeCapVertices> positions{};
    std::array<uint16_t, ScopeCapIndices> indices{};
    bool valid = false;
    std::array<std::array<float,2>,ScopeCapVertices> uv{}; // Raw bound coordinates, not post-VS UV.
};
enum class ScopeRasterStatus { Unrelated, Rejected, Observed };
inline void rejectScopeGeometry(ScopeCapGeometry &geometry, bool &rejected) {
    geometry = {}; rejected = true;
}
inline void applyScopeRasterStatus(ScopeRasterStatus status, ScopeCapGeometry &geometry, bool &rejected) {
    if (status == ScopeRasterStatus::Rejected) rejectScopeGeometry(geometry,rejected);
}
inline bool acceptScopeGeometry(ScopeCapGeometry &geometry, bool &rejected, const ScopeCapGeometry &incoming) {
    if (rejected || !incoming.valid || (geometry.valid && (geometry.indices != incoming.indices ||
        std::memcmp(geometry.positions.data(),incoming.positions.data(),sizeof(incoming.positions)) ||
        geometry.uv != incoming.uv))) {
        rejectScopeGeometry(geometry,rejected); return false;
    }
    geometry = incoming; return true;
}
struct ScopeSliceBytes {
    std::span<const uint8_t> positions, indices, weights, localIndices;
    std::span<const uint8_t> uv; // Required raw FLOAT2 coordinates from the same current draw.
};
inline bool scopeSliceSizes(const ScopeSliceBytes &slices) {
    return slices.positions.size() == ScopeVertices * 12 &&
           slices.indices.size() == ScopeTriangles * 3 * 2 &&
           slices.weights.size() == ScopeVertices * 4 && slices.localIndices.size() == ScopeVertices * 4 &&
           slices.uv.size() == ScopeVertices*8;
}
// Call ONLY after exact hashes of every supplied slice match. Keeping the parser
// separate lets offline checks exercise malformed bounds and winding without
// bypassing the production fingerprint gate.
inline bool copyScopeCap(const ScopeSliceBytes &slices, ScopeCapGeometry &out) {
    out = {};
    if (!scopeSliceSizes(slices)) return false;
    std::array<uint16_t, ScopeCapVertices> sourceIndices{};
    size_t used = 0;
    ScopeCapGeometry next;
    for (size_t i = 0; i < ScopeCapIndices; ++i) {
        uint16_t source = 0;
        std::memcpy(&source, slices.indices.data() + (ScopeCapFirst * 3 + i) * 2, 2);
        if (source >= ScopeVertices) return false;
        size_t mapped = 0;
        while (mapped < used && sourceIndices[mapped] != source) ++mapped;
        if (mapped == used) {
            if (used == ScopeCapVertices) return false;
            sourceIndices[used++] = source;
            std::memcpy(&next.positions[mapped], slices.positions.data() + size_t(source) * 12, 12);
            const auto p = next.positions[mapped];
            if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z)) return false;
            std::memcpy(next.uv[mapped].data(),slices.uv.data()+size_t(source)*8,8);
            if (!std::isfinite(next.uv[mapped][0]) || !std::isfinite(next.uv[mapped][1])) return false;
        }
        next.indices[i] = uint16_t(mapped);
    }
    if (used != ScopeCapVertices) return false;
    for (size_t i = 0; i < ScopeCapIndices; i += 3) {
        const Vec3 a = next.positions[next.indices[i]], b = next.positions[next.indices[i+1]],
                   c = next.positions[next.indices[i+2]];
        const auto normal = cross(b-a, c-a);
        if (!std::isfinite(dot(normal,normal)) || dot(normal,normal) < 1e-18f) return false;
    }
    next.valid = true;
    out = next;
    return true;
}
inline bool scopeCapWorld(const ScopeCapGeometry &cap, const Matrix34 &affine, ScopeCapGeometry &out) {
    ScopeCapGeometry next = cap; // Permit an in-place transform without destroying its input.
    out = {};
    Matrix34 inverse;
    if (!next.valid || !affineInverse(affine, inverse)) return false;
    for (auto &p : next.positions) {
        const Vec3 source = p;
        p = {affine.m[0]*source.x + affine.m[1]*source.y + affine.m[2]*source.z + affine.m[3],
             affine.m[4]*source.x + affine.m[5]*source.y + affine.m[6]*source.z + affine.m[7],
             affine.m[8]*source.x + affine.m[9]*source.y + affine.m[10]*source.z + affine.m[11]};
        if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z)) return false;
    }
    out = next; // Keep actual winding: reflection remains in transformed points.
    return true;
}
} // namespace ss2vr
