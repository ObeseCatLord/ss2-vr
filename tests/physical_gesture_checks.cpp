#include "common/physical_gesture.hpp"
#include <cstdio>
#include <cstdlib>
using namespace ss2vr;
static void check(bool ok, const char *message) {
    if (!ok) { std::fprintf(stderr, "%s\n", message); std::exit(1); }
}
int main() {
    for (unsigned hand : {0u,1u}) {
        Input recovered{};
        recovered.session=recovered.reference=recovered.focused=recovered.headValid=1;
        recovered.handValid[hand]=recovered.gripValid[hand]=1;
        recovered.primaryActiveMask=1u<<hand; recovered.primaryInputGeneration[hand]=1;
        PhysicalGestureBinding target{10,20,1,hand};
        PhysicalGestureInput missed;
        ActionStream poses;
        check(poses.sample(true), "Initial grip stream available");
        PhysicalGestureSample quiet{};
        for(unsigned n=1;n<=3;++n) {
            recovered.sequence=n; recovered.tickMs=980+n*20;
            quiet=missed.sample(recovered,poses.generation,target,1,recovered.tickMs,true);
        }
        check(quiet.eligible && quiet.quiet && !quiet.down, "Pre-loss fresh quiet arms the fixture");
        check(!poses.sample(false) && poses.sample(true), "Host sees loss/recovery omitted by consumer");
        check(!missed.current(quiet,recovered,poses.generation,target,1,recovered.tickMs),
              "Grip epoch change invalidates even an otherwise identical captured input");
        recovered.sequence=5; recovered.tickMs=1060; recovered.grip[hand].p.x=.03f;
        auto recovery=missed.sample(recovered,poses.generation,target,1,recovered.tickMs,true);
        check(!recovery.eligible && !recovery.quiet && missed.stream.generation!=quiet.generation,
              "Skipped grip loss cannot generate high or reuse old quiet");
        check(!missed.sample(recovered,poses.generation,target,1,recovered.tickMs,true).eligible,
              "Duplicate recovery cannot supply a causal quiet witness");
        for(unsigned n=6;n<=7;++n) {
            recovered.sequence=n; recovered.tickMs+=20; recovered.grip[hand].p.x+=.03f;
            check(!missed.sample(recovered,poses.generation,target,1,recovered.tickMs,true).eligible,
                  "Continued recovery movement cannot rearm before fresh quiet");
        }
        ++recovered.sequence; recovered.tickMs+=20;
        quiet=missed.sample(recovered,poses.generation,target,1,recovered.tickMs,true);
        check(quiet.eligible && quiet.quiet && !quiet.down, "New uninterrupted quiet rearms");
        ++recovered.sequence; recovered.tickMs+=20; recovered.grip[hand].p.x+=.03f;
        const auto high=missed.sample(recovered,poses.generation,target,1,recovered.tickMs,true);
        check(high.eligible && high.down && missed.current(high,recovered,poses.generation,target,1,recovered.tickMs),
              "Subsequent real movement qualifies independently for either hand");
        check(!missed.current(high,recovered,0,target,1,recovered.tickMs) &&
              !missed.current(high,recovered,poses.generation+1,target,1,recovered.tickMs),
              "Zero or late replaced grip epoch cannot publish or consume a captured high");
        check(!missed.sample(recovered,0,target,1,recovered.tickMs,true).eligible && !missed.motion.seeded,
              "Zero grip generation rejects and retires motion history");
        check(recovered.trigger[hand]==0 && recovered.primaryInputGeneration[hand]==1,
              "Grip loss never fabricates a trigger or changes its independent identity");
    }
    Input input{};
    input.session = input.reference = input.focused = input.headValid = 1;
    input.handValid[0] = input.handValid[1] = input.gripValid[0] = input.gripValid[1] = 1;
    input.primaryActiveMask = 3;
    input.primaryInputGeneration[0] = input.primaryInputGeneration[1] = 1;
    PhysicalGestureBinding binding{10, 20, 1, 0};
    PhysicalGestureInput gesture;
    uint64_t sequence = 0, time = 1000;
    const auto sample = [&](float position, bool allowed = true) {
        input.sequence = ++sequence; input.tickMs = time; time += 30;
        input.hand[0].p.x = input.grip[0].p.x = position;
        return gesture.sample(input, 1, binding, 1, input.tickMs, allowed);
    };
    check(!sample(0).eligible, "Context-change sample cannot arm motion");
    check(!sample(0).eligible, "First pose only seeds a velocity baseline");
    const auto quiet = sample(0);
    check(quiet.eligible && quiet.quiet && !quiet.down, "Fresh quiet observation arms a gesture");
    const auto swing = sample(.06f);
    check(swing.eligible && swing.down && gesture.current(swing, input, 1, binding, 1, input.tickMs), "Fresh tracked movement produces gesture high");
    check(input.trigger[0] == 0, "Gesture never fabricates a manual trigger");
    auto changed = input; changed.focused = 0;
    check(!gesture.current(swing, changed, 1, binding, 1, input.tickMs), "Focus loss rejects even a copied same-sequence sample");
    changed = input; ++changed.primaryInputGeneration[0];
    check(!gesture.current(swing, changed, 1, binding, 1, input.tickMs), "Action replacement invalidates a copied sample");
    check(!gesture.current(swing, input, 1, binding, 1, input.tickMs + 201), "Cached high expires without another producer call");
    auto otherBinding = binding; ++otherBinding.rig;
    check(!gesture.current(swing, input, 1, otherBinding, 1, input.tickMs), "Rig replacement cannot borrow old motion");
    auto repeat = gesture.sample(input, 1, binding, 1, input.tickMs, true);
    check(repeat.down && !repeat.fresh && !repeat.quiet, "Cached swing is not a new motion/neutral witness");
    input.trigger[0] = .9f;
    check(!sample(.12f, false).eligible && input.trigger[0] == .9f,
          "Carry/equip suppression removes gesture without editing manual input");
    check(!gesture.current(swing, input, 1, binding, 1, input.tickMs), "Lost permission revokes earlier gesture provenance");
    check(!sample(.18f).eligible && !sample(.24f).eligible,
          "Continuing movement after interruption cannot arm without quiet");
    check(sample(.24f).quiet, "New quiet motion can rearm after interruption");
    auto fresh = sample(.30f);
    check(fresh.down, "New swing after a genuine quiet observation is admitted");
    auto oldInput = input; --oldInput.sequence; --oldInput.tickMs;
    check(!gesture.sample(oldInput, 1, binding, 1, input.tickMs, true).eligible &&
          !gesture.current(fresh, input, 1, binding, 1, input.tickMs), "Out-of-order input revokes captured high");
    check(sample(.30f).quiet && sample(.36f).down, "Fresh quiet/new movement recovers without replay");
    binding.owner = 11;
    check(!sample(.42f).eligible, "Owner replacement cannot inherit gesture state");
    check(!sample(.42f).eligible && sample(.42f).quiet, "Replacement establishes its own baseline");
    check(sample(.48f).down, "Replacement can admit its own fresh gesture");
    input.session = 2; sequence = 0;
    check(!sample(0).eligible && !sample(0).eligible && sample(0).quiet && sample(.06f).down,
          "A new source session can restart its sequence after an explicit boundary");
    time += 201;
    check(!sample(.07f).eligible, "Long pose gap does not invent a release/velocity observation");
    check(sample(.07f).quiet && sample(.13f).down, "Gap requires new quiet before another swing");
    input.buttons[0] = Button::Recenter;
    check(!sample(.19f).eligible && input.trigger[0] == .9f, "Recenter revokes gestures while preserving raw trigger");
    input.buttons[0] = 0;
    check(!sample(.19f).eligible && sample(.19f).quiet && sample(.25f).down, "Quiet recovery after recenter");
    const auto beforeInvalid = gesture.last;
    otherBinding = binding; otherBinding.hand = 2;
    check(!gesture.sample(input, 1, otherBinding, 1, input.tickMs, true).eligible &&
          !gesture.current(beforeInvalid, input, 1, binding, 1, input.tickMs), "Invalid hand retires cached gesture provenance");
    std::puts("Independent physical gesture provenance checks passed; no native input/history changed");
}
