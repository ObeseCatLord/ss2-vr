#include "common/model_tree.hpp"
#include <cstdlib>
#include <iostream>
using namespace ss2vr;
static void check(bool value, const char *message) {
    if (!value) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}
static bool equal(const Matrix34 &a, const Matrix34 &b) {
    for (unsigned i = 0; i < 12; ++i)
        if (std::abs(a.m[i] - b.m[i]) > .001f)
            return false;
    return true;
}
int main() {
    Matrix34 stretch{{2, .3f, 0, 0, 0, 3, .2f, 0, 0, 0, -4, 0}};
    auto original = affineMultiply(matrix(Pose{yaw(.7f), {3, 2, 1}}), stretch);
    Matrix34 inv;
    check(affineInverse(original, inv) && equal(affineMultiply(original, inv), matrix({})),
          "Affine inverse must include native stretch, shear, reflection and translation");
    auto desiredRigid = matrix(Pose{yaw(-.5f), {10, 1, -2}});
    Matrix34 desired;
    check(retainNativeStretch(original, desiredRigid, desired) &&
              equal(desired, affineMultiply(desiredRigid, stretch)),
          "Retargeting preserves authored stretch/shear/reflection");
    auto local = matrix(Pose{yaw(.2f), {.1f, .2f, -.4f}});
    std::vector<ModelRecord> tree{
        {-1, 1, matrix({})}, {0, 2, original}, {1, 3, affineMultiply(original, local)}, {0, 4, matrix({})}};
    auto unrelated = tree[3].world;
    check(retargetModelSubtree(tree, 1, desired) && equal(tree[1].world, desired) &&
              equal(tree[2].world, affineMultiply(desired, local)) && equal(tree[3].world, unrelated),
          "Weapon root and descendant keep local placement; unrelated model is untouched");
    auto before = tree;
    tree[2].parent = 2;
    check(!retargetModelSubtree(tree, 1, matrix({})) && equal(tree[1].world, before[1].world),
          "Parent cycles must reject the entire transform before writes");
    tree[2].parent = 999;
    check(!retargetModelSubtree(tree, 1, matrix({})), "Out-of-range parent indices are rejected");
    Matrix34 singular{};
    check(!affineInverse(singular, inv), "Singular native matrices cannot be retargeted");
    std::cout << "Native affine model-tree transform checks passed\n";
}
