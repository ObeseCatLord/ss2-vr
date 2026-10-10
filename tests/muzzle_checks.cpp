#include "common/muzzle.hpp"
#include "common/rider.hpp"
#include <cstdlib>
#include <iostream>
#include <limits>
using namespace ss2vr;
static void check(bool ok, const char *message) {
    if (!ok) { std::cerr << message << '\n'; std::exit(1); }
}
struct Observation {
    Pose placement{};
    int native = 0, attachment = 0, retarget = 0;
    bool calibrated = true;
};
static Observation dispatch(bool accepted, bool sniper = true, bool nested = false) {
    Observation result;
    const Pose view{{}, {0, 1.6f, 0}}, muzzle{{}, {.15f, 1.3f, -.4f}};
    const Pose model{{}, {.1f, 1.3f, -.1f}}, tracked{yaw(.7f), {3, 2, -9}};
    invokeNativeMuzzle(sniper, accepted, nested,
        [&] { ++result.native; result.placement = view; },
        [&] { ++result.attachment; result.placement = muzzle; },
        [&] {
            ++result.retarget;
            result.placement = retargetCalibratedMuzzle({}, result.placement, model.p, tracked);
            result.calibrated = true;
        }, &result.calibrated);
    return result;
}
int main() {
    const Pose native{{}, {.04658699f,.35394251f,-.59738034f}};
    const Pose tracked{yaw(.7f),{3,2,-9}};
    const Vec3 charge{.1f,.02f,0},alignment{-.02f,0,.003f},root{.03f,-.01f,.005f};
    const auto expected=tracked.p+rotate(tracked.q,native.p-root+charge+alignment);
    const auto unbounded=retargetCalibratedMuzzle({},native,root+charge,tracked,charge,alignment);
    check(dot(unbounded.p-expected,unbounded.p-expected)<1e-12f,"Native reach changed vector/charge/alignment");
    const auto limited=retargetShot({},native,root+charge,tracked,.5f,charge,alignment);
    check(dot(limited.p-unbounded.p,limited.p-unbounded.p)>.01f,"Long native vector still truncated");
    check(limited.q.x==unbounded.q.x && limited.q.y==unbounded.q.y &&
          limited.q.z==unbounded.q.z && limited.q.w==unbounded.q.w,"Reach changed native aim orientation");
    const Pose shortNative{{},{0,0,-.1f}};
    const auto shortBounded=retargetShot({},shortNative,{},tracked);
    const auto shortPreserved=retargetCalibratedMuzzle({},shortNative,{},tracked);
    check(shortBounded.p.x==shortPreserved.p.x && shortBounded.p.y==shortPreserved.p.y &&
          shortBounded.p.z==shortPreserved.p.z,"Short native vector changed");
    // Exercise the production conversion for independent hands, longer native
    // reach and a moved/rotated native camera. The camera must cancel out of the
    // attachment vector, while authored aim, charge and alignment survive once.
    const Pose camera{yaw(-.6f), {24, 3, -15}};
    const Pose movedCamera{yaw(.9f), {-13, 5, 7}};
    const Pose attachment{yaw(.15f), {.2f, -.05f, -1.4f}};
    const Vec3 modelRoot{-.1f, .02f, .3f};
    for (unsigned hand = 0; hand < 2; ++hand) {
        const Pose controller{yaw(hand ? -.4f : .8f), {hand ? 2.f : -2.f, 1.2f, -3}};
        const auto result = retargetCalibratedMuzzle(camera, compose(camera, attachment),
                                                    modelRoot + charge, controller, charge, alignment);
        const auto moved = retargetCalibratedMuzzle(movedCamera, compose(movedCamera, attachment),
                                                   modelRoot + charge, controller, charge, alignment);
        const Pose expectedPose{multiply(controller.q, attachment.q),
            controller.p + rotate(controller.q, attachment.p - modelRoot + alignment + charge)};
        check(dot(result.p - expectedPose.p, result.p - expectedPose.p) < 1e-10f,
              "Calibrated long muzzle lost native reach/charge/alignment");
        check(dot(moved.p - result.p, moved.p - result.p) < 1e-10f,
              "Native camera movement changed the controller muzzle");
        const auto aim = rotate(result.q, {0, 0, -1});
        const auto expectedAim = rotate(expectedPose.q, {0, 0, -1});
        check(dot(aim - expectedAim, aim - expectedAim) < 1e-10f,
              "Native attachment aim was changed by reach preservation");
    }
    Input input;
    input.headValid = input.handValid[0] = input.handValid[1] = 1;
    for (unsigned hand = 0; hand < 2; ++hand) {
        check(eligibleLocalMuzzle(input, hand, true, true, true, true), "Valid local hand rejected");
        auto lost = input; lost.handValid[hand] = 0;
        auto rejected = dispatch(eligibleLocalMuzzle(lost, hand, true, true, true, true));
        check(rejected.native == 1 && rejected.attachment == 0 && rejected.retarget == 0 &&
              !rejected.calibrated && rejected.placement.p.y == 1.6f, "Lost local hand adapted");
        auto invalid = input; invalid.hand[hand].p.x = std::numeric_limits<float>::quiet_NaN();
        check(!eligibleLocalMuzzle(invalid, hand, true, true, true, true), "Nonfinite local aim accepted");
    }
    check(!eligibleLocalMuzzle(input, 2, true, true, true, true), "Invalid hand index accepted");
    check(!eligibleLocalMuzzle(input, 0, false, true, true, true) &&
          !eligibleLocalMuzzle(input, 0, true, false, true, true) &&
          !eligibleLocalMuzzle(input, 0, true, true, false, true) &&
          !eligibleLocalMuzzle(input, 0, true, true, true, false), "Local lifecycle gate bypassed");
    network::PosePacket pose;
    pose.validMask = 7; pose.nativeWeaponId[0] = pose.nativeWeaponId[1] = 13;
    MuzzleBinding binding{10,10,10,4,4,30,30,30,31,13,0,true,true,true,true,1000,900};
    check(eligibleAuthorityMuzzle(pose, binding), "Valid native authoritative hand rejected");
    for (unsigned hand = 0; hand < 2; ++hand) {
        auto b = binding; b.hand = hand;
        auto p = pose; p.validMask &= uint8_t(~(2u << hand));
        auto rejected = dispatch(eligibleAuthorityMuzzle(p, b));
        check(rejected.native == 1 && rejected.retarget == 0 && !rejected.calibrated,
              "Invalid authoritative hand adapted");
    }
    for (int failure = 0; failure < 12; ++failure) {
        auto b = binding;
        switch (failure) {
        case 0: b.sampleIncarnation++; break;
        case 1: b.peerAvatar++; break;
        case 2: b.sampleAvatar++; break;
        case 3: b.otherHandle = b.handle; break;
        case 4: b.currentHandle++; break;
        case 5: b.boundHandle++; break;
        case 6: b.receivedMs = 799; break;
        case 7: b.receivedMs = 1001; break;
        case 8: b.peerValid = false; break;
        case 9: b.sampleValid = false; break;
        case 10: b.alive = false; break;
        case 11: b.nativeId = 12; break;
        }
        bool accepted = eligibleAuthorityMuzzle(pose, b);
        auto rejected = dispatch(accepted);
        check(!accepted && rejected.native == 1 && rejected.attachment == 0 && rejected.retarget == 0 &&
              !rejected.calibrated && rejected.placement.p.y == 1.6f,
              "Rejected authoritative context changed getter, placement or calibration");
    }
    auto local = dispatch(eligibleLocalMuzzle(input, 0, true, true, true, true));
    auto authority = dispatch(eligibleAuthorityMuzzle(pose, binding));
    check(local.native == 0 && local.attachment == 1 && local.retarget == 1 && local.calibrated,
          "Eligible sniper did not use exactly one native attachment getter");
    check(local.placement.p.x == authority.placement.p.x && local.placement.p.y == authority.placement.p.y &&
          local.placement.p.z == authority.placement.p.z, "Local/server reference semantics differ");
    auto laser = dispatch(true), bullet = dispatch(true);
    check(laser.placement.p.x == bullet.placement.p.x && laser.placement.p.z == bullet.placement.p.z &&
          laser.attachment == bullet.attachment, "Bullet and laser getter semantics diverged");
    auto nested = dispatch(true, true, true);
    check(nested.native == 1 && nested.attachment == 0 && nested.retarget == 0 && !nested.calibrated,
          "Nested dispatch retargeted or invoked two getters");
    auto ordinary = dispatch(true, false);
    check(ordinary.native == 1 && ordinary.attachment == 0 && ordinary.retarget == 1,
          "Ordinary weapon's original reference changed");
    auto desktop = dispatch(false);
    check(desktop.native == 1 && desktop.retarget == 0 && !desktop.calibrated,
          "Desktop placement was adapted");
    for (bool generationChange : {false, true}) {
        const RiderIdentity captured{10, 0, 0, 0, false};
        RiderIdentity live = captured;
        uint32_t generation = 3;
        Pose output{{}, {1, 2, 3}};
        const Pose nativeOutput = output;
        bool calibrated = true;
        invokeNativeMuzzle(false, true, false,
            [&] {
                if (generationChange)
                    ++generation;
                else
                    live = {10, 20, 5, 3, true};
            }, [] {}, [&] {
                if (!sameHandheldRig(captured, live, 3, generation))
                    return;
                output.p = {8, 9, 10};
                calibrated = true;
            }, &calibrated);
        check(output.p.x == nativeOutput.p.x && output.p.y == nativeOutput.p.y &&
                  output.p.z == nativeOutput.p.z && !calibrated,
              "Native getter lifecycle changes preserve native placement and cannot publish VR calibration");
    }
    std::cout << "Native muzzle eligibility, dispatch, rejection and reference parity passed\n";
}
