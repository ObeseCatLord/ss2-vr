#pragma once
#include "scope_pose.hpp"
#include "scope_geometry.hpp"

namespace ss2vr {
// Values copied from the actual bound inputs of one DIP. Object identities are
// borrowed comparison keys only; this structure neither owns nor pins COM.
struct ScopeStreamInput {
    uintptr_t object = 0;
    uint32_t offset = 0, stride = 0, frequency = 0;
    bool operator==(const ScopeStreamInput &) const = default;
};
struct ScopeBufferDescription {
    uint32_t size = 0, usage = 0, pool = 0, format = 0, fvf = 0;
    bool operator==(const ScopeBufferDescription &) const = default;
};
struct ScopeDeclarationElement {
    uint16_t stream = 0, offset = 0;
    uint8_t type = 0, method = 0, usage = 0, usageIndex = 0;
    bool operator==(const ScopeDeclarationElement &) const = default;
};
struct ScopeIndexedDraw {
    uint32_t topology = 0;
    int32_t base = 0;
    uint32_t minimum = 0, vertices = 0, start = 0, primitives = 0;
    bool operator==(const ScopeIndexedDraw &) const = default;
};
struct ScopeBufferInputs {
    ScopeSurfaceLayout surface{};
    ScopeIndexedDraw draw{};
    ScopeStreamInput positions{}, localIndices{}, weights{}, uv{};
    ScopeBufferDescription vertex{}, index{};
    uintptr_t indexObject = 0;
    bool softwarePositions = false;
    bool operator==(const ScopeBufferInputs &) const = default;
};
struct ScopeByteRange {
    uint32_t offset = 0, size = 0;
    bool operator==(const ScopeByteRange &) const = default;
};
struct ScopeCopyRanges {
    // Position, triangle indices, weights, local bone indices, raw UV. The last two
    // remain in the shared VB even when a single-weight shader omits stream 6.
    std::array<ScopeByteRange,5> slices{};
    bool weightsActive = false;
};
inline bool scopeByteRange(uint32_t offset, uint32_t size, uint32_t capacity) {
    return size && uint64_t(offset) + size <= capacity;
}
// Metadata admission only. Actual context, COM ownership, unlock completion
// and five content hashes are separate mandatory gates at the live boundary.
inline bool scopeBufferRanges(const ScopeBufferInputs &in,
                              std::span<const ScopeDeclarationElement> declaration,
                              ScopeCopyRanges &out) {
    out = {};
    const ScopeSurfaceLayout stock{884,928, {{{0,0x85,0}, {0,0x87,0},
                                            {151520,0x80,0}, {155056,0x80,0}}}};
    if (in.surface != stock || in.softwarePositions || in.draw.topology != 4 ||
        in.draw.base || in.draw.minimum || in.draw.vertices != ScopeVertices ||
        in.draw.start || in.draw.primitives != ScopeTriangles ||
        !in.positions.object || in.localIndices.object != in.positions.object ||
        in.uv.object != in.positions.object || !in.indexObject ||
        in.vertex.size != 212128 || in.vertex.usage || in.vertex.pool != 1 ||
        in.vertex.format != 100 || in.vertex.fvf ||
        in.index.size != 19038 || in.index.usage || in.index.pool != 1 || in.index.format != 101 ||
        in.positions.offset || in.positions.stride != 12 || in.positions.frequency != 1 ||
        in.localIndices.offset != 155056 || in.localIndices.stride != 4 || in.localIndices.frequency != 1 ||
        in.uv.offset != 181824 || in.uv.stride != 8 || in.uv.frequency != 1 ||
        declaration.empty() || declaration.size() > 65)
        return false;
    bool position = false, local = false, weights = false, uv = false, ended = false;
    std::array<bool,4> entries{};
    for (size_t i = 0; i < declaration.size(); ++i) {
        const auto e = declaration[i];
        if (e.stream == 0xff) {
            // Exact D3DDECL_END, including its position at the end of the copy.
            if (i+1 != declaration.size() || e.offset || e.type != 17 || e.method || e.usage || e.usageIndex)
                return false;
            ended = true;
            break;
        }
        if (e.stream > 15) return false;
        // A duplicate relevant semantic would make the shader's actual input
        // ambiguous, even when the expected stream itself has the right type.
        if (e.usage == 5 && (e.usageIndex == 0 || e.usageIndex == 3 || e.usageIndex == 5 || e.usageIndex == 6) &&
            e.stream != e.usageIndex) return false;
        if (e.stream != 0 && e.stream != 3 && e.stream != 5 && e.stream != 6) continue;
        bool &entry = entries[e.stream == 0 ? 0 : e.stream == 5 ? 1 : e.stream == 6 ? 2 : 3];
        if (entry) return false;
        entry = true;
        // Unused entries are permitted for absent stream 6 only. They do not
        // establish an active weight input; duplicate relevant entries decline.
        bool &seen = e.stream == 0 ? position : e.stream == 5 ? local : e.stream == 6 ? weights : uv;
        if (e.type == 17) {
            if (e.stream != 6) return false;
            continue;
        }
        if (seen || e.offset || e.method || e.usage != 5 || e.usageIndex != e.stream ||
            e.type != (e.stream == 0 ? 2 : e.stream == 3 ? 1 : 8)) return false;
        seen = true;
    }
    if (!ended || !position || !local || !uv) return false;
    if (weights && (in.weights.object != in.positions.object || in.weights.offset != 151520 ||
                    in.weights.stride != 4 || in.weights.frequency != 1)) return false;
    ScopeCopyRanges next{{{{0,10608},{0,5568},{151520,3536},{155056,3536},{181824,7072}}}, weights};
    for (size_t i = 0; i < next.slices.size(); ++i) {
        const auto range = next.slices[i];
        if (!scopeByteRange(range.offset, range.size, i == 1 ? in.index.size : in.vertex.size)) return false;
    }
    out = next;
    return true;
}
} // namespace ss2vr
