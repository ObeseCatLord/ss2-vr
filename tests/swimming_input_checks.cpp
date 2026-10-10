#include "common/swimming_input.hpp"
#include "common/settings.hpp"
#include "common/controls.hpp"
#include <cassert>
#include <limits>
using namespace ss2vr;
int main() {
    assert(!VrSettings{}.immersiveSwimming);
    for (uint32_t flags = 0; flags < 16; ++flags)
        for (uint32_t pose = 0; pose < 12; ++pose)
            assert(nativeWaterInputMode(flags, pose) ==
                ((flags & 4) && !(flags & 2) && (pose == 3 || pose == 4)));
    assert(swimmingJoystickIdle({}));
    assert(!swimmingJoystickIdle({.00001f,0,0}));
    assert(!swimmingJoystickIdle({0,1,0}));
    assert(!swimmingJoystickIdle({0,0,-1}));
    assert(!swimmingJoystickIdle({0,std::numeric_limits<float>::quiet_NaN(),0}));
    uint32_t generations[2]{1,1};
    SwimmingStrokes stroke;
    Input sample{};
    sample.session = 1; sample.reference = 2; sample.sequence = 1; sample.tickMs = 1000;
    sample.focused = sample.headValid = sample.gripValid[0] = sample.gripValid[1] = 1;
    sample.grip[0].p = {-.3f,-.2f,-.5f}; sample.grip[1].p = {.3f,-.2f,-.5f};
    auto pull = [&](float distance) {
        ++sample.sequence; sample.tickMs += 20;
        for (auto &hand : sample.grip) hand.p.z += distance;
    };
    assert(stroke.sample(sample,generations,1,1,3,1000,false)==0); // Disabled never seeds.
    assert(stroke.sample(sample,generations,1,1,3,1000,true)==0);
    pull(.03f);
    assert(stroke.sample(sample,generations,1,1,3,sample.tickMs,true)==1);
    assert(stroke.sample(sample,generations,1,1,3,sample.tickMs,true)==1); // Duplicate, no accumulation.
    pull(-.03f);
    assert(stroke.sample(sample,generations,1,1,3,sample.tickMs,true)==0); // Recovery motion is not thrust.
    pull(.03f); sample.head.p.z += .03f;
    assert(stroke.sample(sample,generations,1,1,3,sample.tickMs,true)==0); // Whole rig translation.
    pull(.03f);
    assert(stroke.sample(sample,generations,1,1,3,sample.tickMs,true)==1);
    sample.gripValid[0] = 0;
    assert(stroke.sample(sample,generations,1,1,3,sample.tickMs,true)==0);
    sample.gripValid[0] = 1; pull(.03f);
    assert(stroke.sample(sample,generations,1,1,3,sample.tickMs,true)==0); // Loss/recovery reseeds.
    pull(.03f); ++sample.reference;
    assert(stroke.sample(sample,generations,1,1,3,sample.tickMs,true)==0);
    pull(.03f);
    assert(stroke.sample(sample,generations,1,1,4,sample.tickMs,true)==0); // Surface/dive transition.
    pull(.03f); sample.buttons[1] = Wheel;
    assert(stroke.sample(sample,generations,1,1,4,sample.tickMs,true)==0);
    sample.buttons[1] = 0; pull(.03f);
    assert(stroke.sample(sample,generations,1,1,4,sample.tickMs,true)==0);
    pull(.7f);
    assert(stroke.sample(sample,generations,1,1,4,sample.tickMs,true)==0); // Tracking discontinuity.
    pull(.03f);
    assert(stroke.sample(sample,generations,1,1,4,sample.tickMs,true)==1);
    Input replay = sample; --replay.sequence; replay.tickMs -= 10;
    assert(stroke.sample(replay,generations,1,1,4,sample.tickMs,true)==0);
    assert(stroke.sample(sample,generations,1,1,4,sample.tickMs,true)==0);
    pull(.03f);
    assert(stroke.sample(sample,generations,1,1,4,sample.tickMs+101,true)==0);
    assert(stroke.sample(sample,generations,1,1,4,sample.tickMs,true)==0);
    pull(.03f);
    assert(stroke.sample(sample,generations,1,1,4,sample.tickMs,true)==1);
    assert(stroke.sample(sample,generations,1,1,4,sample.tickMs,false)==0);
    assert(stroke.sample(sample,generations,1,1,4,sample.tickMs,true)==0);
    ++sample.sequence; sample.tickMs += 20; sample.grip[0].p.z += .03f;
    assert(stroke.sample(sample,generations,1,1,4,sample.tickMs,true)==.5f);
    pull(.03f);
    assert(stroke.sample(sample,generations,2,1,4,sample.tickMs,true)==0);
    pull(.03f);
    assert(stroke.sample(sample,generations,2,2,4,sample.tickMs,true)==0);
    pull(.03f); sample.head.q.w = 2;
    assert(stroke.sample(sample,generations,2,2,4,sample.tickMs,true)==0);
    sample.head.q.w = 1;
    assert(stroke.sample(sample,generations,2,2,4,sample.tickMs,true)==0);
    pull(.03f);
    assert(stroke.sample(sample,generations,2,2,4,sample.tickMs-1,true)==0);
    // The host sees loss/recovery, but latest-value IPC delivers only valid A/C.
    // Differencing across either stream boundary must not create a stroke.
    for (unsigned lostMask : {1u,2u,3u}) {
        ActionStream streams[2];
        uint32_t epochs[2]{};
        Input current{};
        current.session=1; current.reference=2; current.sequence=1; current.tickMs=1000;
        current.focused=current.headValid=current.gripValid[0]=current.gripValid[1]=1;
        current.grip[0].p={-.3f,-.2f,-.5f}; current.grip[1].p={.3f,-.2f,-.5f};
        for(unsigned hand=0;hand<2;++hand) {
            assert(streams[hand].sample(true)); epochs[hand]=streams[hand].generation;
        }
        SwimmingStrokes missed;
        assert(missed.sample(current,epochs,1,1,3,1000,true)==0);
        for(unsigned hand=0;hand<2;++hand) {
            if(lostMask&(1u<<hand)) assert(!streams[hand].sample(false));
            assert(streams[hand].sample(true)); epochs[hand]=streams[hand].generation;
            current.grip[hand].p.z+=.03f;
        }
        current.sequence=3; current.tickMs=1020;
        assert(missed.sample(current,epochs,1,1,3,1020,true)==0);
        assert(missed.sample(current,epochs,1,1,3,1020,true)==0);
        ++current.sequence; current.tickMs+=20;
        for(auto &hand:current.grip) hand.p.z+=.03f;
        assert(missed.sample(current,epochs,1,1,3,1040,true)==1);
        // Native basis getters can run after stroke calculation; a newly
        // published stream loss must invalidate that calculated contribution.
        uint32_t latest[2]{epochs[0],epochs[1]};
        assert(swimmingGripStreamsMatch(epochs,latest));
        for(unsigned hand=0;hand<2;++hand) {
            if(lostMask&(1u<<hand)) { streams[hand].sample(false); latest[hand]=streams[hand].generation; }
        }
        assert(!swimmingGripStreamsMatch(epochs,latest));
        for(unsigned hand=0;hand<2;++hand) {
            uint32_t unavailable[2]{epochs[0],epochs[1]}; unavailable[hand]=0;
            assert(!swimmingGripStreamsMatch(unavailable,unavailable));
            assert(missed.sample(current,unavailable,1,1,3,1040,true)==0);
            assert(!missed.seeded);
        }
    }
    ActionStream exhausted;
    exhausted.generation=UINT32_MAX;
    assert(exhausted.sample(true));
    assert(!exhausted.sample(false));
    assert(exhausted.generation==0 && !exhausted.sample(true));
    uint32_t retired[2]{exhausted.generation,1};
    assert(!swimmingGripStreamsMatch(retired,retired));
    SwimmingStrokes retiredStroke;
    assert(retiredStroke.sample(sample,retired,2,2,4,sample.tickMs,true)==0 && !retiredStroke.seeded);
    const std::array<Vec3,3> identity{{{1,0,0},{0,1,0},{0,0,1}}};
    Vec3 out{7,8,9};
    assert(swimmingInputInBasis(identity, {0,.5f,-.5f}, out));
    assert(out.x == 0 && out.y == .5f && out.z == -.5f);
    for (unsigned n = 0; n < 1000; ++n) {
        const float angle = float(n) * .031f;
        const auto q = normalize(Quat{std::sin(angle*.31f), std::sin(angle*.59f), std::cos(angle*.83f), std::cos(angle)});
        const std::array<Vec3,3> basis{{rotate(q,{1,0,0}),rotate(q,{0,1,0}),rotate(q,{0,0,1})}};
        const Vec3 input{std::sin(angle), std::cos(angle*.73f), -std::cos(angle)};
        const Vec3 desired = rotate(q,input);
        assert(swimmingInputInBasis(basis, desired, out));
        assert(std::abs(out.x-input.x)<1e-5f && std::abs(out.y-input.y)<1e-5f &&
               std::abs(out.z-input.z)<1e-5f);
    }
    for (unsigned kind = 0; kind < 4; ++kind) {
        auto bad = identity;
        if (kind == 0) bad[0] = {};
        if (kind == 1) bad[0].x = -1;
        if (kind == 2) bad[1].x = .1f;
        if (kind == 3) bad[2].z = std::numeric_limits<float>::quiet_NaN();
        out = {7,8,9};
        assert(!swimmingInputInBasis(bad,{0,0,-1},out));
        assert(out.x==7 && out.y==8 && out.z==9);
    }
    assert(!swimmingInputInBasis(identity,{0,0,5},out));
    assert(!swimmingInputInBasis(identity,{0,std::numeric_limits<float>::infinity(),0},out));
}
