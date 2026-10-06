#pragma once
#include "controls.hpp"
#include "physical_motion.hpp"

namespace ss2vr {
// Gesture provenance is independent of manual trigger/history. The caller
// supplies a stable native weapon identity and permission; this object never
// edits Input, command bytes, retained manual samples or native history.
struct PhysicalGestureSample {
    bool eligible = false, down = false, fresh = false, quiet = false;
    uint32_t generation = 0;
};
struct PhysicalGestureBinding {
    uint32_t owner = 0, weapon = 0, rig = 0;
    unsigned hand = 2;
    bool operator==(const PhysicalGestureBinding &) const = default;
};
struct PhysicalGestureInput {
    PhysicalMotion motion;
    TriggerGate gate;
    ActionStream stream;
    InputSampleBoundary boundary;
    PhysicalGestureBinding binding;
    uint32_t producer = 0, action = 0, session = 0, reference = 0;
    uint64_t sequence = 0, tickMs = 0;
    bool configured = false;
    PhysicalGestureSample last;

    bool current(const PhysicalGestureSample &captured, const Input &input,
                 const PhysicalGestureBinding &native, uint32_t inputProducer, uint64_t now) const noexcept {
        return stream.active && captured.eligible && captured.generation == stream.generation &&
            captured.generation == last.generation && captured.down == last.down &&
            input.sequence == sequence && input.tickMs == tickMs &&
            input.session == session && input.reference == reference && binding == native &&
            producer == inputProducer && native.hand < 2 && input.focused && input.headValid &&
            input.handValid[native.hand] && input.gripValid[native.hand] &&
            primaryActionEligible(input, native.hand) && action == input.primaryInputGeneration[native.hand] &&
            !recenterHeld(input) && input.tickMs <= now && now - input.tickMs <= 200 &&
            PhysicalMotion::valid(input.head) && PhysicalMotion::valid(input.hand[native.hand]) &&
            PhysicalMotion::valid(input.grip[native.hand]) && std::isfinite(input.trigger[native.hand]) &&
            input.trigger[native.hand] >= 0 && input.trigger[native.hand] <= 1;
    }
    PhysicalGestureSample sample(const Input &input, const PhysicalGestureBinding &native,
                                 uint32_t inputProducer, uint64_t now, bool allowed) {
        const auto hand = native.hand;
        if (hand >= 2) {
            if (stream.active) stream.invalidate();
            gate = {}; last = {}; motion.reset(); configured = false;
            return {};
        }
        const bool changed = !configured || binding != native || producer != inputProducer ||
            action != input.primaryInputGeneration[hand] || session != input.session ||
            reference != input.reference;
        if (changed) {
            stream.invalidate(); motion.reset(); gate = {}; last = {};
            boundary = {input.session, input.reference, input.sequence, now, inputProducer};
            binding = native; producer = inputProducer; action = input.primaryInputGeneration[hand];
            session = input.session; reference = input.reference; configured = true;
            sequence = tickMs = 0;
        }
        allowed = allowed && native.owner && native.weapon && native.rig && inputProducer && input.session && input.reference &&
            input.focused && input.headValid && input.handValid[hand] && input.gripValid[hand] &&
            primaryActionEligible(input, hand) && !recenterHeld(input) &&
            PhysicalMotion::valid(input.head) && std::isfinite(input.trigger[hand]) &&
            input.trigger[hand] >= 0 && input.trigger[hand] <= 1 &&
            PhysicalMotion::valid(input.hand[hand]) && PhysicalMotion::valid(input.grip[hand]) &&
            input.sequence && input.tickMs && input.tickMs <= now && now - input.tickMs <= 200;
        if (!allowed) {
            if (stream.active) stream.invalidate();
            motion.reset(); gate = {}; last = {};
            boundary = {input.session, input.reference, input.sequence, now, inputProducer};
            return {};
        }
        if (!boundary.permits(input, now, inputProducer) || !stream.sample(true)) return {};
        if (input.sequence == sequence && input.tickMs == tickMs) {
            auto repeat = last; repeat.fresh = repeat.quiet = false;
            return repeat;
        }
        if (input.sequence <= sequence || input.tickMs <= tickMs) {
            if (stream.active) stream.invalidate();
            gate = {}; last = {};
            return {};
        }
        const auto move = motion.sample(weaponTracking(input, hand), input.sequence, input.tickMs, now, true);
        sequence = input.sequence; tickMs = input.tickMs;
        if (!move.known || !move.fresh) {
            if (last.eligible && stream.active) stream.invalidate();
            gate = {}; last = {}; return {};
        }
        // Quiet is a genuinely fresh velocity observation, not merely a low
        // result caused by interruption. Hysteresis alone cannot rearm a swing.
        if (!gate.armed && !move.quiet) return last = {};
        const bool high = gate.update(move.down ? 1.f : 0.f, true);
        return last = {true, high, true, move.quiet, stream.generation};
    }
};
} // namespace ss2vr
