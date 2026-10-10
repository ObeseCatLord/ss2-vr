#pragma once
#include "ride_render_observation.hpp"
#include "rider.hpp"
#include "world_submission.hpp"

namespace ss2vr {
struct RideGripRig {
    RiderIdentity rider;
    uint32_t producer=0,generation=0;
    uint64_t revision=0,graphics=0;
    Pose anchor{},origin{};
    float turn=0;
};
struct RideGripSurface {
    std::array<Vec3,35> positions{};
    std::array<uint16_t,90> indices{};
    uint32_t vertices=0;
};
struct RideGripFrame {
    Request request;
    RideGripRig rig;
    RideRenderIdentity render;
    uint32_t configuration=0,file=0,resource=0,parameter=0;
    std::array<uint32_t,3> stretch{};
    RideHandleProfile profile=RideHandleProfile::Unknown;
    std::array<RideGripSurface,2> surfaces;
    bool valid=false;
};
// Two native transport slots and one activated value copy; no new queue,
// native pointer lease, or geometry lifetime beyond the submitted source age.
struct RideGripFrames {
    std::array<RideGripFrame,2> pending{};
    RideGripFrame active;
    uint32_t next=0,continuity=0;
    void add(const RideGripFrame &frame) {
        if(!frame.valid)return;
        pending[next]=frame;next=(next+1)%pending.size();
    }
    bool submitted(const WorldSubmission &receipt,uint32_t producer,uint64_t now,RideGripFrame &out) {
        out={};
        if(!receipt.active || !receipt.continuity || continuity!=receipt.continuity)active={};
        continuity=receipt.continuity;
        if(!receipt.active || !receipt.continuity)return false;
        if(!active.valid || !submittedWorldMatches(receipt,producer,active.request,now)) {
            active={};
            for(const auto &frame:pending)
                if(frame.valid && frame.rig.producer==producer &&
                   submittedWorldMatches(receipt,producer,frame.request,now)) {active=frame;break;}
        }
        if(!active.valid)return false;
        out=active;return true;
    }
};
inline bool rideGripFrameCurrent(const RideGripFrame &frame,const RideGripRig &rig,const Input &input,uint64_t now) {
    return frame.valid && rig.rider.seated() && frame.rig.rider==rig.rider &&
        rig.producer && frame.rig.producer==rig.producer && frame.rig.generation==rig.generation &&
        frame.rig.revision==rig.revision && frame.rig.graphics==rig.graphics &&
        !std::memcmp(&frame.rig.origin,&rig.origin,sizeof(Pose)) && frame.rig.turn==rig.turn &&
        frame.request.session==input.session && frame.request.reference==input.reference &&
        frame.request.input.tickMs && now>=frame.request.input.tickMs &&
        now-frame.request.input.tickMs<=100;
}
} // namespace ss2vr
