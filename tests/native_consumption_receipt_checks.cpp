#include "common/native_consumption_receipt.hpp"
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <memory>
#include <optional>
#include <type_traits>
#include <vector>
using ss2vr::NativeConsumptionRecord;
using Edge = NativeConsumptionRecord::Edge;
static void check(bool ok, const char *message) {
    if (!ok) { std::fprintf(stderr, "%s\n", message); std::exit(1); }
}
static_assert(!std::is_copy_constructible_v<NativeConsumptionRecord>);
static_assert(!std::is_copy_assignable_v<NativeConsumptionRecord>);
static_assert(!std::is_move_constructible_v<NativeConsumptionRecord>);
static_assert(ss2vr::nextConsumptionRevision(std::numeric_limits<uint64_t>::max()) == 0);
static_assert(ss2vr::nextConsumptionRevision(1) == 2);
int main() {
    NativeConsumptionRecord unknown(1), invalid(0), other(2);
    check(unknown.edge(false) == Edge::Unknown && unknown.edge(true) == Edge::Unknown,
          "Unknown consumption is not an invented low baseline");
    check(!invalid.complete(invalid.prepare(), false) && invalid.edge(true) == Edge::Unknown,
          "A missing lifetime epoch cannot establish history");
    auto first = unknown.prepare();
    check(!other.complete(first, true), "Receipt cannot cross record ownership");
    check(!unknown.complete({}, false), "Default receipt cannot establish history");
    check(unknown.complete(first, false), "A proven real completion may establish initial low");
    check(!unknown.complete(first, true), "A receipt cannot be replayed");
    check(unknown.edge(false) == Edge::None && unknown.edge(true) == Edge::Press,
          "Known low needs only a press edge");
    check(unknown.complete(unknown.prepare(), true) && unknown.edge(false) == Edge::Release,
          "Known high needs a release edge");
    for (bool innerHigh : {false, true}) {
        auto outer = unknown.prepare();
        auto inner = unknown.prepare();
        check(unknown.complete(inner, innerHigh), "Inner completed consumption commits");
        check(!unknown.complete(outer, !innerHigh), "Older outer completion cannot overwrite inner");
        check(unknown.edge(innerHigh) == Edge::None, "Inner level survives outer unwind/return");
    }
    // Preparing, then abandoning before entering native code, changes nothing.
    // A partially executed native operation instead requires retirement.
    const auto before = unknown.edge(true);
    { auto abandoned = unknown.prepare(); (void)abandoned; }
    check(unknown.edge(true) == before, "An abandoned operation publishes no speculative level");
    auto stale = unknown.prepare();
    unknown.retire();
    check(!unknown.complete(stale, true) && unknown.edge(false) == Edge::Unknown,
          "Retirement prevents stale completion and hides consumed history");
    check(!unknown.complete(unknown.prepare(), false), "Retirement cannot rearm itself");

    // Same storage can occur after a binding slot is retired/reused. The new
    // binding owner must allocate a fresh epoch; address equality is insufficient.
    alignas(NativeConsumptionRecord) unsigned char storage[sizeof(NativeConsumptionRecord)];
    auto *old = std::construct_at(reinterpret_cast<NativeConsumptionRecord *>(storage), 10);
    auto oldReceipt = old->prepare();
    old->retire();
    std::destroy_at(old);
    auto *replacement = std::construct_at(reinterpret_cast<NativeConsumptionRecord *>(storage), 11);
    check(!replacement->complete(oldReceipt, true), "Reused address does not revive an old binding epoch");
    check(replacement->edge(true) == Edge::Unknown, "Replacement starts unknown even after active old saw");
    check(replacement->complete(replacement->prepare(), false), "Fresh proven replacement completion works");
    std::destroy_at(replacement);

    // Compare randomized interleavings against an event-log model. A token
    // may publish only if no completion/retirement/rebinding superseded the
    // state it observed. Keep old tokens across rebindings and both owners.
    struct Model { uint64_t epoch, lastCompletion = 0; bool live = true, known = false, high = false; };
    struct Pending { NativeConsumptionRecord::Receipt receipt; unsigned owner; uint64_t epoch, observed;
                     bool live; };
    std::optional<NativeConsumptionRecord> records[2];
    Model models[2]{{100}, {101}};
    records[0].emplace(100); records[1].emplace(101);
    std::vector<Pending> pending;
    uint64_t nextEpoch = 102, event = 1, random = 0x9e3779b97f4a7c15ull;
    for (unsigned step = 0; step < 20000; ++step) {
        random ^= random << 13; random ^= random >> 7; random ^= random << 17;
        const unsigned owner = unsigned(random & 1), action = unsigned((random >> 1) % 8);
        auto &record = *records[owner]; auto &model = models[owner];
        if (action < 3 || pending.empty()) {
            pending.push_back({record.prepare(), owner, model.epoch, model.lastCompletion, model.live});
        } else if (action < 6) {
            const auto &token = pending[(random >> 8) % pending.size()];
            const bool high = (random >> 40) & 1;
            const bool expected = model.live && token.live && token.owner == owner &&
                token.epoch == model.epoch && token.observed == model.lastCompletion;
            check(record.complete(token.receipt, high) == expected, "Receipt disagrees with event-log admission");
            if (expected) { model.lastCompletion = event++; model.known = true; model.high = high; }
        } else if (action == 6) {
            record.retire(); model.live = false; model.known = false;
        } else {
            record.retire();
            records[owner].emplace(nextEpoch);
            models[owner] = Model{nextEpoch++};
        }
        for (unsigned i = 0; i < 2; ++i) for (bool desired : {false, true}) {
            const auto &m = models[i];
            const auto expected = !m.live || !m.known ? Edge::Unknown :
                m.high == desired ? Edge::None : desired ? Edge::Press : Edge::Release;
            check(records[i]->edge(desired) == expected, "Consumption level disagrees with event-log model");
        }
    }
    std::puts("Portable native consumption receipt checks passed; no native callback executed");
}
