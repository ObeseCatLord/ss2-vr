#include "common/physical_gesture.hpp"
#include <cstdio>
#include <cstdlib>
using namespace ss2vr;
static void check(bool ok, const char *message) {
    if (!ok) { std::fprintf(stderr, "%s\n", message); std::exit(1); }
}
int main() {
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
        return gesture.sample(input, binding, 1, input.tickMs, allowed);
    };
    check(!sample(0).eligible, "Context-change sample cannot arm motion");
    check(!sample(0).eligible, "First pose only seeds a velocity baseline");
    const auto quiet = sample(0);
    check(quiet.eligible && quiet.quiet && !quiet.down, "Fresh quiet observation arms a gesture");
    const auto swing = sample(.06f);
    check(swing.eligible && swing.down && gesture.current(swing, input, binding, 1, input.tickMs), "Fresh tracked movement produces gesture high");
    check(input.trigger[0] == 0, "Gesture never fabricates a manual trigger");
    auto changed = input; changed.focused = 0;
    check(!gesture.current(swing, changed, binding, 1, input.tickMs), "Focus loss rejects even a copied same-sequence sample");
    changed = input; ++changed.primaryInputGeneration[0];
    check(!gesture.current(swing, changed, binding, 1, input.tickMs), "Action replacement invalidates a copied sample");
    check(!gesture.current(swing, input, binding, 1, input.tickMs + 201), "Cached high expires without another producer call");
    auto otherBinding = binding; ++otherBinding.rig;
    check(!gesture.current(swing, input, otherBinding, 1, input.tickMs), "Rig replacement cannot borrow old motion");
    auto repeat = gesture.sample(input, binding, 1, input.tickMs, true);
    check(repeat.down && !repeat.fresh && !repeat.quiet, "Cached swing is not a new motion/neutral witness");
    input.trigger[0] = .9f;
    check(!sample(.12f, false).eligible && input.trigger[0] == .9f,
          "Carry/equip suppression removes gesture without editing manual input");
    check(!gesture.current(swing, input, binding, 1, input.tickMs), "Lost permission revokes earlier gesture provenance");
    check(!sample(.18f).eligible && !sample(.24f).eligible,
          "Continuing movement after interruption cannot arm without quiet");
    check(sample(.24f).quiet, "New quiet motion can rearm after interruption");
    auto fresh = sample(.30f);
    check(fresh.down, "New swing after a genuine quiet observation is admitted");
    auto oldInput = input; --oldInput.sequence; --oldInput.tickMs;
    check(!gesture.sample(oldInput, binding, 1, input.tickMs, true).eligible &&
          !gesture.current(fresh, input, binding, 1, input.tickMs), "Out-of-order input revokes captured high");
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
    check(!gesture.sample(input, otherBinding, 1, input.tickMs, true).eligible &&
          !gesture.current(beforeInvalid, input, binding, 1, input.tickMs), "Invalid hand retires cached gesture provenance");
    std::puts("Independent physical gesture provenance checks passed; no native input/history changed");
}
