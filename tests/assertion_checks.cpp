#include <cassert>
#include <cstdio>

// Do not use assert to test the harness itself: a build that accidentally
// defines NDEBUG must fail this check rather than silently passing it.
int main() {
    int evaluated = 0;
    assert(++evaluated == 1);
    if (evaluated != 1) {
        std::fputs("Portable checks require active assertions in every build configuration\n", stderr);
        return 1;
    }
    return 0;
}
