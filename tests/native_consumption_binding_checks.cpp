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
    using Edge=NativeConsumptionBinding::Edge;
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
