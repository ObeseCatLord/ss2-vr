#include "common/scope_draw_order.hpp"
#include <array>
#include <cassert>
#include <cstdio>
using namespace ss2vr;
struct Draw { ScopeDrawRange range; bool image; };
struct Fixture {
    std::array<Draw, 4> draws{};
    unsigned count = 0, checks = 0, failCall = 0, failCheck = 0;
    bool draw(ScopeDrawRange range, bool image) noexcept {
        assert(count < draws.size());
        draws[count++] = {range, image};
        return count != failCall;
    }
    bool run() noexcept {
        return executeScopeCapDraw(
            [&](ScopeDrawRange range) noexcept { return draw(range, false); },
            [&](ScopeDrawRange range) noexcept { return draw(range, true); },
            [&]() noexcept { return ++checks != failCheck; });
    }
};
int main() {
    Fixture success;
    assert(success.run() && success.count == 4 && success.checks == 5);
    assert(success.draws[0].range == ScopePrefixDraw && !success.draws[0].image);
    assert(success.draws[1].range == ScopeCapDraw && !success.draws[1].image);
    assert(success.draws[2].range == ScopeCapDraw && success.draws[2].image);
    assert(success.draws[3].range == ScopeSuffixDraw && !success.draws[3].image);
    std::array<unsigned, ScopeTriangles> native{}, image{};
    for (auto call : success.draws) {
        assert(call.range.firstIndex % 3 == 0);
        for (unsigned n = 0; n < call.range.triangles; ++n) {
            const auto triangle = call.range.firstIndex / 3 + n;
            assert(triangle < ScopeTriangles);
            ++(call.image ? image[triangle] : native[triangle]);
        }
    }
    for (unsigned t = 0; t < ScopeTriangles; ++t) {
        assert(native[t] == 1);
        assert(image[t] == unsigned(t >= 901 && t < 923));
    }
    for (unsigned failure = 1; failure <= 4; ++failure) {
        Fixture f; f.failCall = failure;
        assert(!f.run() && f.count == failure);
        for (unsigned i = 0; i < f.count; ++i)
            assert(f.draws[i].range == success.draws[i].range &&
                   f.draws[i].image == success.draws[i].image);
    }
    for (unsigned failure = 1; failure <= 5; ++failure) {
        Fixture f; f.failCheck = failure;
        assert(!f.run() && f.checks == failure && f.count == failure - 1);
    }
    // In particular an uncertain RGB bind/draw/restore cannot proceed to the
    // suffix, retry the cap, or repaint the full original object.
    Fixture restoration; restoration.failCall = 3;
    assert(!restoration.run() && restoration.count == 3);
    std::puts("Ordered scope cap execution checks passed; no native GPU draw executed");
}
