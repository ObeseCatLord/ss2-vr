#include "common/native_primary_dispatch.hpp"
#include "common/native_consumption_receipt.hpp"
#include <cstdio>
#include <cstdlib>
using namespace ss2vr;
static void check(bool ok, const char *what) {
    if (!ok) { std::fprintf(stderr, "%s\n", what); std::exit(1); }
}
static_assert(nativePrimarySingleTarget(0, 10, 0, false, 10));
static_assert(!nativePrimarySingleTarget(1, 10, 0, false, 10));
static_assert(nativePrimarySingleTarget(1, 0, 20, false, 20));
static_assert(nativePrimarySingleTarget(0, 0, 20, false, 20));
static_assert(!nativePrimarySingleTarget(0, 0, 20, true, 20));
static_assert(!nativePrimarySingleTarget(0, 10, 20, false, 10));
static_assert(!nativePrimarySingleTarget(0, 10, 20, false, 20));
static_assert(nativePrimarySingleTarget(0, 10, 20, true, 10));
static_assert(nativePrimarySingleTarget(1, 10, 20, true, 20));
static_assert(!nativePrimarySingleTarget(0, 10, 10, true, 10));
static_assert(!nativePrimarySingleTarget(2, 10, 20, true, 10));
static_assert(!nativePrimarySingleTarget(3, 10, 20, true, 20));
int main() {
    auto coupled = nativePrimaryReleaseTargets(0, 10, 20, false);
    check(coupled.count == 2 && coupled.primary[0] == 10 && coupled.primary[1] == 20,
          "Coupled release keeps BOTH native targets in native order");
    auto alternate = nativePrimaryReleaseTargets(1, 10, 0, false);
    check(alternate.primaryLane && alternate.count == 0 && alternate.alternative == 10,
          "Right-only semantic-one release is alternative, not primary");
    for (unsigned semantic : {2u, 3u, 4u, ~0u}) {
        const auto other = nativePrimaryReleaseTargets(semantic, 10, 20, true);
        check(!other.primaryLane && !other.count && !other.alternative,
              "Tertiary/unknown native lanes cannot enter primary reconciliation");
    }
    // This is a metadata trace using production helpers, NOT execution of a
    // native saw. The actual press/release and lifetime boundaries remain gates.
    NativeConsumptionRecord consumed(1);
    check(consumed.complete(consumed.prepare(), false), "Trace begins with a proven real low completion");
    unsigned press = 0, release = 0;
    const auto completeAtNativeBoundary = [&](bool high) {
        const auto edge = consumed.edge(high);
        if (edge == NativeConsumptionRecord::Edge::None) return;
        check(edge != NativeConsumptionRecord::Edge::Unknown, "Unknown history cannot be guessed");
        check(nativePrimarySingleTarget(0, 10, 0, false, 10), "Exact canonical target coverage required");
        auto receipt = consumed.prepare();
        if (high) ++press; else ++release;
        check(consumed.complete(receipt, high), "Trace models only successful actual completion");
    };
    completeAtNativeBoundary(true);  // initial high/fire
    completeAtNativeBoundary(false); // 20ms release
    completeAtNativeBoundary(true);  // 40ms high even if native held query is skipped
    completeAtNativeBoundary(false); // 60ms release still required in recoil
    completeAtNativeBoundary(false); // duplicate low is not another callback
    check(press == 2 && release == 2, "Recoil does not erase a required second release or duplicate a low");
    std::puts("Canonical target classification and receipt trace passed; no native callbacks executed");
}
