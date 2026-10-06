#pragma once
#include "math.hpp"
#include <span>
#include <vector>
namespace ss2vr {
inline bool finiteMatrix(const Matrix34 &m) {
    for (float value : m.m)
        if (!std::isfinite(value))
            return false;
    return true;
}
inline Matrix34 affineMultiply(const Matrix34 &a, const Matrix34 &b) {
    Matrix34 result{};
    for (unsigned row = 0; row < 3; ++row)
        for (unsigned column = 0; column < 4; ++column) {
            for (unsigned k = 0; k < 3; ++k)
                result.m[row * 4 + column] += a.m[row * 4 + k] * b.m[k * 4 + column];
            if (column == 3)
                result.m[row * 4 + column] += a.m[row * 4 + 3];
        }
    return result;
}
inline bool affineInverse(const Matrix34 &a, Matrix34 &out) {
    if (!finiteMatrix(a))
        return false;
    Vec3 x{a.m[0], a.m[4], a.m[8]}, y{a.m[1], a.m[5], a.m[9]}, z{a.m[2], a.m[6], a.m[10]};
    Vec3 c0 = cross(y, z), c1 = cross(z, x), c2 = cross(x, y);
    float det = dot(x, c0);
    float scale = std::sqrt(dot(x, x) * dot(y, y) * dot(z, z));
    if (!std::isfinite(scale) || scale < 1e-12f || std::abs(det) <= scale * 1e-6f)
        return false;
    c0 = c0 * (1 / det);
    c1 = c1 * (1 / det);
    c2 = c2 * (1 / det);
    Vec3 t{a.m[3], a.m[7], a.m[11]};
    out = {{c0.x, c0.y, c0.z, -dot(c0, t), c1.x, c1.y, c1.z, -dot(c1, t), c2.x, c2.y, c2.z, -dot(c2, t)}};
    return finiteMatrix(out);
}
// Keep native stretch/shear/reflection when replacing the attachment's rigid
// world pose. Gram-Schmidt defines a proper rigid frame; all residual authored
// affine geometry remains in inverse(frame)*native rather than being discarded.
inline bool retainNativeStretch(const Matrix34 &native, const Matrix34 &desiredRigid, Matrix34 &desired) {
    if (!finiteMatrix(native) || !finiteMatrix(desiredRigid))
        return false;
    Vec3 x{native.m[0], native.m[4], native.m[8]}, y{native.m[1], native.m[5], native.m[9]};
    float length = std::sqrt(dot(x, x));
    if (length < 1e-6f)
        return false;
    x = x * (1 / length);
    y = y - x * dot(x, y);
    length = std::sqrt(dot(y, y));
    if (length < 1e-6f)
        return false;
    y = y * (1 / length);
    Vec3 z = cross(x, y);
    Matrix34 frame{{x.x, y.x, z.x, native.m[3], x.y, y.y, z.y, native.m[7], x.z, y.z, z.z, native.m[11]}};
    Matrix34 inv;
    if (!affineInverse(frame, inv))
        return false;
    desired = affineMultiply(desiredRigid, affineMultiply(inv, native));
    return finiteMatrix(desired);
}
struct ModelRecord {
    int32_t parent = -1;
    uintptr_t instance = 0;
    Matrix34 world{};
};
inline bool ancestorContains(std::span<const ModelRecord> records, size_t child, size_t ancestor,
                             bool &contains) {
    contains = false;
    for (size_t hops = 0; hops < records.size(); ++hops) {
        if (child >= records.size())
            return false;
        contains |= child == ancestor;
        int32_t parent = records[child].parent;
        if (parent == -1)
            return true;
        if (parent < 0)
            return false;
        child = size_t(parent);
    }
    return false;
}
// Validate and calculate every write before changing even one native record.
inline bool retargetModelSubtree(std::span<ModelRecord> records, size_t root, const Matrix34 &desired) {
    if (root >= records.size() || !finiteMatrix(desired))
        return false;
    Matrix34 inv;
    if (!affineInverse(records[root].world, inv))
        return false;
    Matrix34 delta = affineMultiply(desired, inv);
    std::vector<std::pair<size_t, Matrix34>> changes;
    for (size_t i = 0; i < records.size(); ++i) {
        bool descendant;
        if (!ancestorContains(records, i, root, descendant))
            return false;
        if (descendant) {
            auto world = affineMultiply(delta, records[i].world);
            if (!finiteMatrix(world))
                return false;
            changes.emplace_back(i, world);
        }
    }
    for (const auto &[i, world] : changes)
        records[i].world = world;
    return true;
}
} // namespace ss2vr
