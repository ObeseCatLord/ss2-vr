#pragma once

#include "model_tree.hpp"
#include <span>
#include <vector>

namespace ss2vr {
// Views of native renderer metadata, never a persistent replacement skeleton.
struct PaletteBone {
    int32_t owner = -1, parent = -1;
    uint32_t name = 0;
    bool named = false;
};
struct PaletteMap {
    int32_t draw = -1, bone = -1;
};
enum class HeadPaletteResult { Invalid, Unchanged, Changed };

// One native producer invocation, including non-render and nested passthrough.
// Nested native work can rebuild globals: decline the entire outer adaptation.
template<class Original, class Adapt, class Fault>
inline void withFreshNativePalette(bool renderCaller, bool &active, bool &invalidated,
                                   Original &&original, Adapt &&adapt, Fault &&fault) {
    const bool nested = active;
    if (!nested) {
        active = true;
        invalidated = false;
    } else {
        invalidated = true;
    }
    struct Restore {
        bool &active;
        bool nested;
        ~Restore() { if (!nested) active = false; }
    } restore{active, nested};
    original();
    if (!renderCaller)
        return;
    if (nested || invalidated)
        fault();
    else {
        adapt(); // Adapter must check invalidated immediately before its writes.
        if (invalidated)
            fault();
    }
}

inline bool identityTrackingDelta(const Pose &delta) {
    return delta.p.x == 0 && delta.p.y == 0 && delta.p.z == 0 && delta.q.x == 0 && delta.q.y == 0 &&
           delta.q.z == 0 && (delta.q.w == 1 || delta.q.w == -1);
}
// Native producer has already refreshed the palette. Calculate every result in
// separate invocation-owned memory; caller applies only after complete success.
inline HeadPaletteResult retargetHeadPalette(std::span<const PaletteBone> bones,
                                             std::span<const PaletteMap> mappings,
                                             std::span<const int32_t> drawOwners,
                                             std::span<const Matrix34> worlds, int32_t owner,
                                             uint32_t headName, const Pose &eye, Pose delta,
                                             std::span<const Matrix34> native,
                                             std::vector<Matrix34> &output) {
    output.clear();
    if (!finite(eye) || !finite(delta))
        return HeadPaletteResult::Invalid;
    float norm2 = delta.q.x * delta.q.x + delta.q.y * delta.q.y + delta.q.z * delta.q.z + delta.q.w * delta.q.w;
    if (!std::isfinite(norm2) || norm2 < .95f * .95f || norm2 > 1.05f * 1.05f)
        return HeadPaletteResult::Invalid;
    delta.q = normalize(delta.q);
    if (identityTrackingDelta(delta))
        return HeadPaletteResult::Unchanged;
    if (owner <= 0 || size_t(owner) >= worlds.size() || bones.empty() ||
        mappings.size() != native.size())
        return HeadPaletteResult::Invalid;
    size_t head = bones.size();
    for (size_t i = 0; i < bones.size(); ++i) {
        const auto &bone = bones[i];
        if (bone.owner < 0 || size_t(bone.owner) >= worlds.size() || bone.parent < -1 ||
            (bone.parent >= 0 && size_t(bone.parent) >= bones.size()) ||
            (!bone.named && !(i == 0 && bone.owner == 0 && bone.parent == -1)))
            return HeadPaletteResult::Invalid;
        if (bone.owner == owner && bone.named && bone.name == headName) {
            if (head != bones.size())
                return HeadPaletteResult::Invalid;
            head = i;
        }
    }
    // Linear-time graph validation. Sentinels terminate; no recursion or guessed
    // skeleton ordering. Cross-owner attachments remain native in this adapter.
    std::vector<uint8_t> state(bones.size(), 0), selected(bones.size(), 0);
    std::vector<size_t> path;
    for (size_t i = 0; i < bones.size(); ++i) {
        if (state[i] == 2)
            continue;
        path.clear();
        int32_t node = int32_t(i);
        while (node >= 0 && state[size_t(node)] == 0) {
            state[size_t(node)] = 1;
            path.push_back(size_t(node));
            node = bones[size_t(node)].parent;
        }
        if (node >= 0 && state[size_t(node)] == 1)
            return HeadPaletteResult::Invalid;
        for (auto it = path.rbegin(); it != path.rend(); ++it) {
            const auto &bone = bones[*it];
            selected[*it] = bone.owner == owner &&
                            (*it == head || (bone.parent >= 0 && selected[size_t(bone.parent)]));
            state[*it] = 2;
        }
    }
    bool any = false;
    for (const auto &map : mappings) {
        if (map.draw < 0 || size_t(map.draw) >= drawOwners.size() ||
            drawOwners[size_t(map.draw)] < 0 || size_t(drawOwners[size_t(map.draw)]) >= worlds.size() ||
            map.bone < -1 || (map.bone >= 0 && size_t(map.bone) >= bones.size()))
            return HeadPaletteResult::Invalid;
        any |= map.bone >= 0 && drawOwners[size_t(map.draw)] == owner && selected[size_t(map.bone)];
    }
    if (!any)
        return HeadPaletteResult::Unchanged;
    Matrix34 inverseWorld, inverseEye;
    if (!affineInverse(worlds[size_t(owner)], inverseWorld) || !affineInverse(matrix(eye), inverseEye))
        return HeadPaletteResult::Invalid;
    const Matrix34 worldDelta = affineMultiply(matrix(eye), affineMultiply(matrix(delta), inverseEye));
    const Matrix34 modelDelta = affineMultiply(inverseWorld, affineMultiply(worldDelta, worlds[size_t(owner)]));
    output.assign(native.begin(), native.end());
    for (size_t i = 0; i < mappings.size(); ++i)
        if (mappings[i].bone >= 0 && drawOwners[size_t(mappings[i].draw)] == owner &&
            selected[size_t(mappings[i].bone)]) {
            output[i] = affineMultiply(modelDelta, native[i]);
            if (!finiteMatrix(output[i])) {
                output.clear();
                return HeadPaletteResult::Invalid;
            }
        }
    return HeadPaletteResult::Changed;
}
} // namespace ss2vr
