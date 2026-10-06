#pragma once
#include "head_palette.hpp"
#include <cstdint>
#include <span>
#include <vector>
namespace ss2vr {
struct PaletteModelRange { int32_t first = -1, count = 0; };
struct PaletteMeshRange { int32_t owner = -1, first = -1, count = 0; };
struct PaletteDrawRange { int32_t mesh = -1, first = -1, count = 0; };
// -1 is ordinary visibility absence; -2 is ambiguous native ownership.
inline int32_t paletteBodyOwner(std::span<const uintptr_t> instances, uintptr_t body) {
    int32_t owner = -1;
    if (!body)
        return owner;
    for (size_t i = 1; i < instances.size(); ++i)
        if (instances[i] == body) {
            if (owner != -1)
                return -2;
            owner = int32_t(i);
        }
    return owner;
}
inline bool validPaletteSpan(int32_t first, int32_t count, size_t size) {
    if (count < 0)
        return false;
    if (!count && first == -1)
        return true; // Native empty-range sentinel.
    return first >= 0 && size_t(first) <= size && size_t(count) <= size - size_t(first);
}
inline bool disjointMatrixStorage(uintptr_t a, size_t countA, uintptr_t b, size_t countB) {
    if (!a || !b || countA > UINTPTR_MAX / sizeof(Matrix34) || countB > UINTPTR_MAX / sizeof(Matrix34))
        return false;
    const size_t bytesA = countA * sizeof(Matrix34), bytesB = countB * sizeof(Matrix34);
    if (a > UINTPTR_MAX - bytesA || b > UINTPTR_MAX - bytesB)
        return false;
    return a + bytesA <= b || b + bytesB <= a;
}
// Resolve native back-links, not pointer proximity or a current-draw global.
// On failure no ownership view is usable and caller makes zero palette writes.
inline bool paletteDrawOwners(std::span<const PaletteModelRange> models,
                              std::span<const PaletteMeshRange> meshes,
                              std::span<const PaletteDrawRange> draws,
                              std::span<const PaletteMap> maps, std::vector<int32_t> &owners) {
    owners.clear();
    if (models.empty())
        return false;
    std::vector<int32_t> meshOwners(meshes.size(), -1), drawOwners(draws.size(), -1);
    std::vector<uint8_t> mapped(maps.size(), 0);
    for (size_t model = 0; model < models.size(); ++model) {
        const auto range = models[model];
        if (!validPaletteSpan(range.first, range.count, meshes.size()))
            return false;
        for (int32_t j = 0; j < range.count; ++j) {
            size_t i = size_t(range.first) + size_t(j);
            if (meshOwners[i] != -1 || meshes[i].owner != int32_t(model))
                return false;
            meshOwners[i] = int32_t(model);
        }
    }
    for (size_t mesh = 0; mesh < meshes.size(); ++mesh) {
        const auto range = meshes[mesh];
        if (meshOwners[mesh] < 0 || !validPaletteSpan(range.first, range.count, draws.size()))
            return false;
        for (int32_t j = 0; j < range.count; ++j) {
            size_t i = size_t(range.first) + size_t(j);
            if (drawOwners[i] != -1 || draws[i].mesh != int32_t(mesh))
                return false;
            drawOwners[i] = meshOwners[mesh];
        }
    }
    for (size_t draw = 0; draw < draws.size(); ++draw) {
        const auto range = draws[draw];
        if (drawOwners[draw] < 0 || !validPaletteSpan(range.first, range.count, maps.size()))
            return false;
        for (int32_t j = 0; j < range.count; ++j) {
            size_t i = size_t(range.first) + size_t(j);
            if (mapped[i] || maps[i].draw != int32_t(draw))
                return false;
            mapped[i] = 1;
        }
    }
    for (uint8_t present : mapped)
        if (!present)
            return false;
    owners = std::move(drawOwners);
    return true;
}
} // namespace ss2vr
