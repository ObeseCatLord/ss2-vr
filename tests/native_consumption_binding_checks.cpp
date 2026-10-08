#include "common/native_consumption_binding.hpp"
#include <cassert>
#include <limits>
#include <type_traits>
#ifdef NDEBUG
#error Native consumption binding tests require active assertions
#endif
using namespace ss2vr;
static_assert(!std::is_copy_constructible_v<NativeConsumptionBinding>);
static_assert(!std::is_move_constructible_v<NativeConsumptionBinding>);
int main() {
    using Edge=NativeConsumptionBinding::Edge;
    const NativeConsumptionBindingKey dispatchKey{11,22,1};
    assert(nativeConsumptionDispatchMatches(dispatchKey,1,true,3,dispatchKey,1,true,3));
    assert(!nativeConsumptionDispatchMatches({11,33,1},1,true,3,dispatchKey,1,true,3));
    assert(!nativeConsumptionDispatchMatches(dispatchKey,2,true,3,dispatchKey,1,true,3));
    assert(!nativeConsumptionDispatchMatches(dispatchKey,1,false,3,dispatchKey,1,true,3));
    assert(!nativeConsumptionDispatchMatches(dispatchKey,1,true,4,dispatchKey,1,true,3));
    // A getter after early admission completes a real stop with unchanged
    // topology. Final production fence must withdraw G without withdrawing
    // manual demand or suppressing a release of a known High.
    assert(nativeGestureFenceCurrent(true,false,true,10,12,12,12,12));
    const bool stalePhysical = true;
    const bool finalPhysical = stalePhysical && nativeGestureFenceCurrent(false,true,true,12,13,13,12,12);
    assert(!finalPhysical);
    NativeConsumptionBinding stopped;
    assert(stopped.activate(dispatchKey,1));
    assert(stopped.complete(dispatchKey,stopped.prepare(dispatchKey),false));
    assert(nativeGestureReconcileEdge(stopped.edge(dispatchKey,finalPhysical),true,finalPhysical,false)==Edge::None);
    assert(nativeGestureReconcileEdge(stopped.edge(dispatchKey,true),true,finalPhysical,false)==Edge::Press);
    assert(!nativeGestureFenceCurrent(true,false,true,12,13,13,12,12));
    assert(nativeGestureFenceCurrent(true,false,true,12,14,14,14,14));
    assert(!nativeGestureFenceCurrent(true,false,false,12,14,14,14,14));
    // Production decision + real receipt storage, controlled callback returns.
    // This exercises first-use accounting, not native contact/sound behavior.
    for (unsigned role = 0; role < 3; ++role) {
        const NativeConsumptionBindingKey key{11,22,role % 2};
        NativeConsumptionBinding fresh;
        assert(fresh.activate(key,1));
        unsigned presses=0,releases=0;
        bool inFlight=false;
        const auto decide = [&](bool manual,bool physical,bool admitted=true) {
            return nativeGestureReconcileEdge(fresh.edge(key,manual||physical),admitted,physical,inFlight);
        };
        assert(decide(false,false)==Edge::None); // Unknown never becomes Low.
        assert(decide(true,false)==Edge::None); // Await actual manual callback.
        assert(decide(false,true,false)==Edge::None); // Failed owner admission.
        assert(!nativeGestureQuietWitness(10,11,11,10,10));
        assert(nativeGestureQuietWitness(10,12,12,12,12));
        assert(decide(false,true)==Edge::Press);
        const auto press=fresh.prepare(key);
        inFlight=true;
        ++presses; // Enter controlled original press, not yet completed.
        assert(decide(false,true)==Edge::None); // Reentry cannot redispatch.
        assert(fresh.edge(key,false)==Edge::Unknown); // No speculative High/Low.
        assert(fresh.complete(key,press,true));
        inFlight=false;
        assert(decide(false,true)==Edge::None);
        assert(decide(true,true)==Edge::None);
        assert(decide(true,false)==Edge::None); // Manual keeps shared High.
        assert(decide(false,true)==Edge::None); // Gesture keeps shared High.
        assert(decide(false,false)==Edge::Release);
        ++releases;
        assert(fresh.complete(key,fresh.prepare(key),false));
        assert(decide(false,false)==Edge::None);
        assert(presses==1 && releases==1);
        // Native stop fences through its sample. A later quiet cannot lend
        // permission to a delayed pulse stamped with the old quiet.
        assert(!nativeGestureQuietWitness(20,21,21,12,12));
        assert(nativeGestureQuietWitness(20,22,22,22,22));
        assert(!nativeGestureQuietWitness(20,19,19,12,12));
        const auto stale=fresh.prepare(key);
        fresh.retire();assert(fresh.activate(key,2));
        assert(fresh.edge(key,false)==Edge::Unknown);
        assert(!fresh.complete(key,stale,true));
        // Actual nested completion supersedes outer first-press receipt.
        const auto outer=fresh.prepare(key),inner=fresh.prepare(key);
        assert(fresh.complete(key,inner,false));
        assert(!fresh.complete(key,outer,true));
        assert(!fresh.retire(key,outer));
        assert(fresh.edge(key,false)==Edge::None);
    }
    // Same-generation native high20 -> stop50 -> reset-fenced quiet60 ->
    // quiet70 arms. A delayed pulse40/quiet30 may not borrow that later arming.
    assert(nativeGestureQuietWitness(60, 70, 70, 70, 70));
    assert(!nativeGestureQuietWitness(60, 40, 40, 30, 30));
    // QuietN captured before an intervening stop is fenced through N during
    // preparation. MovingN+1 still carries quietN; only later quiet can rearm.
    assert(!nativeGestureQuietWitness(100, 100, 100, 100, 100));
    assert(!nativeGestureQuietWitness(100, 101, 101, 100, 100));
    assert(nativeGestureQuietWitness(100, 102, 102, 102, 102));
    assert(!nativeGestureQuietWitness(100, 102, 102, 103, 102));
    assert(!nativeGestureQuietWitness(100, 102, 102, 102, 103));
    assert(!nativeGestureQuietWitness(100, 102, 102, 102, 0));
    const NativeConsumptionBindingKey right{11,22,1},left{11,33,0};
    NativeConsumptionBinding owner;
    assert(!owner.activate({},1));assert(!owner.activate(right,0));
    assert(!owner.activate({11,22,2},1));assert(!owner.activate({0,22,1},1));
    assert(!owner.activate({11,0,1},1));
    assert(owner.activate(right,1));assert(owner.matches(right));
    assert(owner.edge(right,false)==Edge::Unknown); // No guessed initial low.
    auto low=owner.prepare(right);assert(owner.complete(right,low,false));
    assert(owner.edge(right,true)==Edge::Press);
    auto high=owner.prepare(right);assert(owner.complete(right,high,true));
    assert(owner.edge(right,false)==Edge::Release);
    assert(!owner.activate(right,2)&&!owner.activate(left,2));
    assert(owner.edge(right,false)==Edge::Release); // Sample refresh cannot erase high.
    assert(!owner.complete(left,owner.prepare(right),false));
    assert(!owner.complete(right,owner.prepare(left),false));
    auto outer=owner.prepare(right),inner=owner.prepare(right);
    assert(owner.complete(right,inner,false));
    assert(!owner.complete(right,outer,true));
    assert(!owner.retire(right,outer));
    assert(owner.edge(right,false)==Edge::None); // Completed inner observation survives.
    auto stale=owner.prepare(right);owner.retire();owner.retire();
    assert(!owner.matches(right)&&!owner.complete(right,stale,true));
    assert(!owner.activate(right,1)&&!owner.activate(right,0));
    assert(owner.activate(right,2));
    assert(owner.edge(right,false)==Edge::Unknown);
    assert(!owner.complete(right,stale,true)); // Same address/key, newer lifetime.
    assert(!owner.retire(right,stale));
    NativeConsumptionBinding other;assert(other.activate(right,2));
    assert(!other.complete(right,owner.prepare(right),true));
    assert(!other.retire(right,owner.prepare(right)));
    auto aborted=owner.prepare(right);
    assert(!owner.retire(left,aborted));
    assert(owner.retire(right,aborted));
    assert(!owner.retire(right,aborted)&&!owner.matches(right));
    owner.retire();assert(owner.activate(left,3));
    assert(!owner.complete(right,owner.prepare(left),true));
    owner.retire();assert(owner.activate(left,std::numeric_limits<uint64_t>::max()));
    owner.retire();assert(!owner.activate(left,std::numeric_limits<uint64_t>::max()));
    assert(!owner.activate(left,1)); // No epoch wrap/resurrection.
}
