#include "common/network.hpp"
#include "common/controls.hpp"
#include "common/intent_boundary.hpp"
#include <cstdlib>
#include <iostream>
#include <limits>
using namespace ss2vr;
using namespace ss2vr::network;

static void check(bool value, const char *what) {
    if (!value) {
        std::cerr << what << '\n';
        std::exit(1);
    }
}
static Pose pose(float x, float y, float z) {
    return {{0, 0, 0, 1}, {x, y, z}};
}
static PosePacket validPose(uint32_t sequence = 1, uint32_t generation = 1) {
    PosePacket p{};
    p.clientNonce = 11;
    p.serverNonce = 22;
    p.sequence = sequence;
    p.trackingGeneration = generation;
    p.head = pose(.1f, .2f, .3f);
    p.grip[0] = pose(.4f, 0, 0);
    p.grip[1] = pose(-.4f, 0, 0);
    p.validMask = 7;
    p.primarySampleEligibleMask = HandMask;
    p.primaryNeutralSampleMask = HandMask;
    p.zoomSampleEligibleMask = HandMask;
    p.intentEpoch[0] = p.intentEpoch[1] = 1;
    p.primaryInputGeneration[0] = p.primaryInputGeneration[1] = 1;
    p.zoomInputGeneration[0] = p.zoomInputGeneration[1] = 1;
    p.releasedSerial[0] = p.releasedSerial[1] = 1;
    p.zoomReleasedSerial[0] = p.zoomReleasedSerial[1] = 1;
    p.requestedWeapon[0] = -1;
    p.requestedWeapon[1] = -1;
    p.nativeWeaponId[0] = 2;
    p.nativeWeaponId[1] = -1;
    return p;
}
static std::string encodePose(const PosePacket &p) {
    Message m{};
    m.kind = Kind::Pose;
    m.pose = p;
    std::string wire;
    check(encode(m, wire), "valid pose encodes");
    return wire;
}
int main() {
    Message hello{};
    hello.kind = Kind::Hello;
    hello.hello.nonce = 0x1122334455667788ull;
    std::string wire;
    check(encode(hello, wire) && wire.starts_with(CarrierTag) && wire.size() <= MaxCarrierAscii,
          "hello carrier is bounded and tagged");
    Message decoded{};
    check(parse(wire, decoded) == ParseResult::Valid && decoded.kind == Kind::Hello &&
              decoded.hello.nonce == hello.hello.nonce,
          "little-endian hello round trips");
    std::vector<uint8_t> oldVersion;
    check(detail::unbase64(std::string_view(wire).substr(CarrierTag.size()), oldVersion),
          "valid hello has a native wire payload");
    oldVersion[0] = 4;
    check(parse(std::string(CarrierTag) + detail::base64(oldVersion), decoded) == ParseResult::Malformed,
          "Wire4 cannot negotiate without retained-shot zoom context");
    auto p = validPose();
    p.physicalDownMask = 1;
    p.primaryNeutralSampleMask = uint8_t(HandMask & ~p.physicalDownMask & p.primarySampleEligibleMask);
    p.fireMask = 1;
    p.releasedSerial[0] = 4;
    wire = encodePose(p);
    check(parse(wire, decoded) == ParseResult::Valid && decoded.pose.sequence == 1 &&
              decoded.pose.grip[0].p.x == .4f && decoded.pose.fireMask == 1,
          "pose round trips through base64");
    Ack epochAck{11, 22, 7, {3, 9}};
    Message ackMessage{};
    ackMessage.kind = Kind::Ack;
    ackMessage.ack = epochAck;
    check(encode(ackMessage, wire) && parse(wire, decoded) == ParseResult::Valid &&
              decoded.ack.intentEpoch[0] == 3 && decoded.ack.intentEpoch[1] == 9,
          "ack epochs round trip without changing its credit token");
    Relay relay{p, 99, 7};
    Message relayMessage{};
    relayMessage.kind = Kind::Relay;
    relayMessage.relay = relay;
    check(encode(relayMessage, wire) && parse(wire, decoded) == ParseResult::Valid &&
              decoded.relay.subjectAvatar == 99 && decoded.relay.subjectIncarnation == 7,
          "relay adds server subject identity");
    p.pulseMask = 1;
    wire = encodePose(p);
    check(parse(wire, decoded) == ParseResult::Valid && decoded.pose.pulseMask == 1,
          "one-step physical pulse round trips with the reliable pose");
    p = validPose();
    p.pulseMask = 1;
    wire = encodePose(p);
    check(parse(wire, decoded) == ParseResult::Valid && !decoded.pose.fireMask && decoded.pose.pulseMask == 1,
          "released short tap remains a pulse rather than a held fire level");
    check(outboundTargetHandle(true, 37, 91) == 91 && outboundTargetHandle(false, 37, 91) == 37,
          "client outbound policy keeps the mapped server wire handle even though it is not local");
    check(!outboundHandlesValid(false, 91) && outboundHandlesValid(true, 91),
          "failed local source resolve rejects send without resolving the mapped server handle");
    check(parse("ordinary engine chat", decoded) == ParseResult::NotMod, "untagged chat is not consumed");
    check(parse("~SS2VR1~not-base64", decoded) == ParseResult::Malformed,
          "tagged invalid base64 is consumed as malformed");
    check(parse(std::string(CarrierTag) + "AQEAAAAA", decoded) == ParseResult::Malformed,
          "truncated tagged payload is malformed");
    auto oversized = std::string(CarrierTag) + std::string(MaxCarrierAscii, 'A');
    check(parse(oversized, decoded) == ParseResult::Malformed, "oversized tagged carrier is rejected");
    p = validPose();
    p.fireMask = 1;
    p.physicalDownMask = 0;
    p.primaryNeutralSampleMask = uint8_t(HandMask & ~p.physicalDownMask & p.primarySampleEligibleMask);
    Message invalid{};
    invalid.kind = Kind::Pose;
    invalid.pose = p;
    check(!encode(invalid, wire), "fire requires a physical down witness");
    p = validPose();
    p.primarySampleEligibleMask = 4;
    invalid.pose = p;
    check(!encode(invalid, wire), "unknown primary eligibility bits are malformed");
    p = validPose();
    p.zoomSampleEligibleMask = 4;
    invalid.pose = p;
    check(!encode(invalid, wire), "unknown zoom eligibility bits are malformed");
    p = validPose();
    p.primaryNeutralSampleMask = 4;
    invalid.pose = p;
    check(!encode(invalid, wire), "unknown primary neutral bits are malformed");
    p = validPose();
    p.physicalDownMask = 1;
    invalid.pose = p;
    check(!encode(invalid, wire), "a raw neutral witness cannot also be physically held");
    p = validPose();
    p.primarySampleEligibleMask = 2;
    invalid.pose = p;
    check(!encode(invalid, wire), "an inactive primary action cannot contribute neutral evidence");
    p = validPose();
    p.nativeWeaponId[0] = 13;
    p.zoomMask = p.zoomPhysicalDownMask = 1;
    p.intentEpoch[0] = 7;
    p.intentEpoch[1] = 8;
    p.releasedSerial[0] = 17;
    p.primaryInputGeneration[0] = 19;
    p.zoomReleasedSerial[0] = 23;
    p.zoomInputGeneration[0] = 29;
    wire = encodePose(p);
    check(parse(wire, decoded) == ParseResult::Valid && decoded.pose.intentEpoch[0] == 7 &&
              decoded.pose.primaryInputGeneration[0] == 19 && decoded.pose.zoomReleasedSerial[0] == 23 &&
              decoded.pose.zoomInputGeneration[0] == 29 && decoded.pose.zoomPhysicalDownMask == 1 &&
              decoded.pose.zoomSampleEligibleMask == HandMask && decoded.pose.primarySampleEligibleMask == HandMask &&
              decoded.pose.primaryNeutralSampleMask == HandMask,
          "wire6 preserves raw action eligibility, streams, serials, and epochs");
    std::vector<uint8_t> truncatedPose;
    check(detail::unbase64(std::string_view(wire).substr(CarrierTag.size()), truncatedPose),
          "wire6 pose has a decodable payload");
    truncatedPose.pop_back();
    check(parse(std::string(CarrierTag) + detail::base64(truncatedPose), decoded) == ParseResult::Malformed,
          "codec loss in wire6 action fields is malformed rather than defaulted");
    p.zoomPhysicalDownMask = 0;
    invalid.pose = p;
    check(!encode(invalid, wire), "held zoom requires active raw-down evidence in the transport original");
    p = validPose();
    p.head.p.x = .751f;
    invalid.pose = p;
    check(!encode(invalid, wire), "head body-relative translation is bounded");
    p = validPose();
    p.head.p = {.2f, -1.1f, .3f};
    p.grip[0].p = p.head.p + Vec3{-.4f, -.4f, -.5f};
    p.grip[1].p = p.head.p + Vec3{.4f, -.4f, -.5f};
    wire = encodePose(p);
    check(parse(wire, decoded) == ParseResult::Valid && decoded.pose.head.p.y == -1.1f &&
              decoded.pose.grip[0].p.y == -1.5f,
          "Deep crouch and head-relative controller poses survive multiplayer replication");
    p.head.p.y = -HeadDownLimit - .01f;
    invalid.pose = p;
    check(!encode(invalid, wire), "Headset vertical travel remains bounded independently of horizontal lean");
    p = validPose();
    p.grip[0].p = {HeadHorizontalLimit + HandReachLimit + .01f, 0, 0};
    invalid.pose = p;
    check(!encode(invalid, wire),
          "Server rejects grips outside the reachable head volume even inside the outer sphere");
    p = validPose();
    p.validMask = 1;
    p.grip[0].p = p.head.p + Vec3{HandReachLimit + .01f, 0, 0};
    invalid.pose = p;
    check(!encode(invalid, wire), "Inactive hands retain the tighter current-head bound");
    p = validPose();
    p.grip[0].p = {HeadHorizontalLimit + .93f, -HeadDownLimit - .93f, 0};
    invalid.pose = p;
    check(!encode(invalid, wire), "Envelope corner distance rejects excessive diagonal reach");
    p = validPose();
    p.head.p = {HeadHorizontalLimit, -HeadDownLimit, 0};
    p.grip[0].p = p.head.p + Vec3{0, -HandReachLimit, 0};
    p.grip[1].p = p.head.p;
    wire = encodePose(p);
    check(parse(wire, decoded) == ParseResult::Valid, "Valid poses at the full crouch/reach boundary encode");
    p.validMask = 1;
    p.grip[0].p = p.grip[1].p = p.head.p;
    wire = encodePose(p);
    check(parse(wire, decoded) == ParseResult::Valid,
          "Canonical inactive hands do not discard deep head tracking");
    for (float nonfinite :
         {std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::infinity()}) {
        p = validPose();
        p.grip[0].p.z = nonfinite;
        invalid.pose = p;
        check(!encode(invalid, wire), "Nonfinite controller positions cannot enter the tracking envelope");
    }
    p = validPose();
    const Pose calibration{yaw(.63f), {2, 1, -4}};
    const Pose limitHead{
        normalize(multiply(yaw(.7f), Quat{.2f, 0, .1f, .97f})),
        {HeadHorizontalLimit * std::sqrt(.5f), -HeadDownLimit, HeadHorizontalLimit * std::sqrt(.5f)}};
    for (float turn : {-.7f, 1.6f, Pi}) {
        p = validPose();
        p.head = bodyHeadTracking(calibration, turn, compose(calibration, limitHead));
        for (unsigned hand = 0; hand != 2; ++hand) {
            Pose grip = limitHead;
            grip.p.x += hand ? HandReachLimit : -HandReachLimit;
            p.grip[hand] = bodyHandTracking(calibration, turn, compose(calibration, limitHead),
                                            compose(calibration, grip));
        }
        wire = encodePose(p);
        check(
            parse(wire, decoded) == ParseResult::Valid,
            "Production rig transforms survive turn/recenter rounding at simultaneous head and reach limits");
    }
    p = validPose();
    p.grip[0].q.w = .8f;
    invalid.pose = p;
    check(!encode(invalid, wire), "out of range quaternion is rejected");
    p = validPose();
    p.grip[0].q.w = 1.01f;
    wire = encodePose(p);
    check(parse(wire, decoded) == ParseResult::Valid && decoded.pose.grip[0].q.w == 1.0f,
          "small quaternion drift is normalized");
    p = validPose();
    p.requestedWeapon[0] = 14;
    invalid.pose = p;
    check(!encode(invalid, wire), "reserved weapon id is rejected");
    PeerState peer{};
    check(peer.bindCapability(11, 22), "peer binds nonzero handshake capability");
    p = validPose(1);
    p.validMask = 7;
    p.physicalDownMask = 1;
    p.primaryNeutralSampleMask = uint8_t(HandMask & ~p.physicalDownMask & p.primarySampleEligibleMask);
    p.fireMask = 1;
    p.releasedSerial[0] = 5;
    check(peer.acceptPose(p, 1000) && !peer.physicalFireAllowed(0),
          "held state cannot arm after capability bind");
    p = validPose(2);
    p.validMask = 7;
    p.releasedSerial[0] = 6;
    check(peer.acceptPose(p, 1010) && peer.physicalFireAllowed(0),
          "new serial while physically released arms hand");
    check(!peer.acceptPose(p, 1020), "sequence is monotonic within a generation");
    check(peer.fresh(1210) && !peer.fresh(1211), "caller supplied local tick enforces 200ms freshness");
    peer.invalidateTracking();
    p = validPose(3, 2);
    p.validMask = 5;
    p.releasedSerial[0] = 7; // hand invalid: not a release.
    check(peer.acceptPose(p, 1300) && !peer.physicalFireAllowed(0),
          "tracking invalidity cannot arm physical release");
    p = validPose(4, 2);
    p.validMask = 7;
    p.releasedSerial[0] = 8;
    check(peer.acceptPose(p, 1310) && peer.physicalFireAllowed(0),
          "valid physical release re-arms after tracking invalidation");
    peer.invalidateWeaponGeneration(9);
    p = validPose(5, 2);
    p.intentEpoch[0] = 2;
    p.validMask = 7;
    p.wheelOrEquipBlockedMask = 1;
    p.releasedSerial[0] = 8;
    check(peer.acceptPose(p, 1320) && !peer.physicalFireAllowed(0),
          "wheel/equipment gating is not a release witness");
    p = validPose(6, 2);
    p.intentEpoch[0] = 2;
    p.validMask = 7;
    p.releasedSerial[0] = 9;
    check(peer.acceptPose(p, 1330) && peer.physicalFireAllowed(0),
          "new released serial re-arms after weapon generation change");
    p = validPose(7, 1);
    check(!peer.acceptPose(p, 1340), "old generation cannot replace current tracking");
    p = validPose(6, 3);
    check(!peer.acceptPose(p, 1340), "new generation cannot bypass sequence ordering");
    p = validPose(7, 2);
    p.intentEpoch[0] = 2;
    p.physicalDownMask = p.fireMask = 1;
    p.primaryNeutralSampleMask = uint8_t(HandMask & ~p.physicalDownMask & p.primarySampleEligibleMask);
    p.releasedSerial[0] = 9;
    check(peer.acceptPose(p, 1700) && !peer.physicalFireAllowed(0),
          "expired stream requires new physical release");

    // Action activity is not inferred from hand tracking. An inactive sample
    // may be coalesced out of transport, so its next active held sample cannot
    // establish a release without a new active raw-up witness.
    PeerState actions{};
    check(actions.bindCapability(11, 22), "action-admission peer binds capability epochs");
    auto action = validPose(1);
    action.releasedSerial[0] = action.zoomReleasedSerial[0] = 1;
    check(actions.acceptPose(action, 1800) && actions.physicalFireAllowed(0) && actions.zoomAllowed(0),
          "independent active raw-up samples arm primary and zoom");
    action = validPose(2);
    action.physicalDownMask = action.fireMask = action.zoomPhysicalDownMask = 1;
    action.primaryNeutralSampleMask = uint8_t(HandMask & ~action.physicalDownMask & action.primarySampleEligibleMask);
    action.primarySampleEligibleMask &= uint8_t(~1u);
    action.primaryNeutralSampleMask &= uint8_t(~1u);
    action.zoomSampleEligibleMask &= uint8_t(~1u);
    ++action.primaryInputGeneration[0];
    ++action.zoomInputGeneration[0];
    check(actions.acceptPose(action, 1810) && !action.fireMask && !actions.physicalFireAllowed(0) &&
              !actions.zoomAllowed(0),
          "inactive raw-held actions revoke admission without fabricating a release");
    action = validPose(3);
    action.primaryInputGeneration[0] = 2;
    action.zoomInputGeneration[0] = 2;
    action.physicalDownMask = action.fireMask = action.zoomPhysicalDownMask = action.zoomMask = 1;
    action.primaryNeutralSampleMask = uint8_t(HandMask & ~action.physicalDownMask & action.primarySampleEligibleMask);
    action.nativeWeaponId[0] = 13;
    check(actions.acceptPose(action, 1820) && !action.fireMask && !action.zoomMask,
          "coalesced inactive-to-active held samples remain blocked until a real neutral");
    action = validPose(4);
    action.primaryInputGeneration[0] = 2;
    action.zoomInputGeneration[0] = 2;
    action.releasedSerial[0] = action.zoomReleasedSerial[0] = 2;
    check(actions.acceptPose(action, 1830) && actions.physicalFireAllowed(0) && actions.zoomAllowed(0),
          "new active raw-up serials establish each action's own neutral");
    action = validPose(5);
    action.primaryInputGeneration[0] = 2;
    action.zoomInputGeneration[0] = 2;
    action.releasedSerial[0] = action.zoomReleasedSerial[0] = 2;
    action.physicalDownMask = action.fireMask = action.zoomPhysicalDownMask = action.zoomMask = 1;
    action.primaryNeutralSampleMask = uint8_t(HandMask & ~action.physicalDownMask & action.primarySampleEligibleMask);
    action.nativeWeaponId[0] = 13;
    check(actions.acceptPose(action, 1840) && action.fireMask == 1 && action.zoomMask == 1,
          "admitted primary and zoom held levels remain independent after neutral");
    action = validPose(6);
    action.primaryInputGeneration[0] = 2;
    action.zoomInputGeneration[0] = 3;
    action.releasedSerial[0] = 2;
    action.zoomReleasedSerial[0] = 2;
    action.physicalDownMask = action.fireMask = action.zoomPhysicalDownMask = action.zoomMask = 1;
    action.primaryNeutralSampleMask = uint8_t(HandMask & ~action.physicalDownMask & action.primarySampleEligibleMask);
    action.nativeWeaponId[0] = 13;
    check(actions.acceptPose(action, 1850) && action.fireMask == 1 && !action.zoomMask &&
              actions.physicalFireAllowed(0) && !actions.zoomAllowed(0),
          "a zoom stream change revokes only zoom admission while primary remains live");

    PeerState epochs{};
    check(epochs.bindCapability(11, 22) && epochs.intentEpoch[0] == 1 && epochs.intentEpoch[1] == 1,
          "successful bind starts nonzero per-hand epochs");
    epochs.invalidateWeaponGeneration(9, 1);
    epochs.invalidateWeaponGeneration(9, 1);
    check(epochs.intentEpoch[0] == 3 && epochs.intentEpoch[1] == 1,
          "repeated same-generation invalidations advance only their affected hand");
    epochs.intentEpoch[0] = UINT32_MAX;
    epochs.invalidateWeaponGeneration(9, 1);
    epochs.invalidateWeaponGeneration(9, 1);
    check(!epochs.intentEpoch[0], "epoch exhaustion fails closed without reusing a value");
    PeerState staleEpoch{};
    check(staleEpoch.bindCapability(11, 22), "stale-epoch trace binds capability");
    auto zeroEpoch = validPose(1);
    zeroEpoch.intentEpoch[0] = 0;
    zeroEpoch.releasedSerial[0] = 99;
    zeroEpoch.zoomReleasedSerial[0] = 99;
    check(staleEpoch.acceptPose(zeroEpoch, 1860) && !staleEpoch.physicalFireAllowed(0) &&
              !staleEpoch.zoomAllowed(0) && !staleEpoch.observedReleasedSerial[0] &&
              !staleEpoch.observedZoomReleasedSerial[0],
          "zero epoch filters all intents before it can poison either release baseline");
    auto currentEpoch = validPose(2);
    currentEpoch.releasedSerial[0] = currentEpoch.zoomReleasedSerial[0] = 1;
    check(staleEpoch.acceptPose(currentEpoch, 1870) && staleEpoch.physicalFireAllowed(0) &&
              staleEpoch.zoomAllowed(0),
          "current epoch establishes neutral after discarded stale raw input");
    ConsumptionCredits credits;
    check(credits.grant({11, 22, 1}) && !credits.grant({11, 22, 2}) &&
              credits.hasOutstandingCapability(11, 22),
          "one capability cannot submit another pose before native consumption");
    check(credits.grant({12, 23, 1}) && credits.grant({13, 24, 1}) && credits.grant({14, 25, 1}) &&
              credits.full() && !credits.grant({15, 0, 0}),
          "four recovery capabilities exhaust aggregate credit including Hello grants");
    Ack consumed{11, 22, 1};
    check(credits.retire(consumed) && credits.count() == 3 && !credits.retire({11, 22, 3}),
          "only the exact completed pose token returns credit");
    check(!credits.retire({13, 23, 1}) && credits.count() == 3,
          "an ACK from another capability cannot free reliable credit");
    PendingIntents pending;
    auto leftTap = validPose();
    leftTap.physicalDownMask = leftTap.fireMask = 1;
    leftTap.primaryNeutralSampleMask = uint8_t(HandMask & ~leftTap.physicalDownMask & leftTap.primarySampleEligibleMask);
    check(pending.retain(leftTap, 1, 100), "quick left tap is retained while snapshot credit is full");
    auto rightTap = leftTap;
    rightTap.physicalDownMask = rightTap.fireMask = 2;
    rightTap.primaryNeutralSampleMask = uint8_t(HandMask & ~rightTap.physicalDownMask & rightTap.primarySampleEligibleMask);
    check(pending.retain(rightTap, 2, 101) && pending.active(0, 101) && pending.active(1, 101),
          "overlapping hand taps retain one immutable intent per hand");
    check(pending.mask(101) == 3, "retained taps become a bounded aggregate pulse mask");
    auto rightWheel = validPose();
    rightWheel.wheelOrEquipBlockedMask = 2;
    pending.observeSnapshot(rightWheel);
    check(pending.active(0, 102) && !pending.active(1, 102),
          "right wheel snapshot cancels only its own pending tap");
    check(!pending.active(0, 300), "unsubmitted quick taps expire within a bounded window");
    pending.requireNeutral(1);
    check(!pending.retain(leftTap, 1, 301), "recovery rejects retained held input before neutral");
    auto neutral = validPose();
    neutral.releasedSerial[0] = 3;
    pending.observeSnapshot(neutral);
    check(pending.retain(leftTap, 1, 302), "real neutral permits the next left tap");

    // Drive the same credit, immutable intent, and receive/freeze/finish methods
    // used by the adapter, including a real delayed receive/consumption window.
    ConsumptionCredits window;
    OrderedPosePolicy stream;
    check(stream.validation.bindCapability(11, 22), "ordered stream binds capability");
    Ack ack{};
    auto latest = validPose(1);
    latest.releasedSerial[0] = latest.releasedSerial[1] = 1;
    check(window.grant({11, 22, 1}) && stream.receive(latest, 1000, ack),
          "first neutral snapshot occupies the native consumption slot");
    check(!stream.finishInterval(ack) && window.hasOutstandingCapability(11, 22),
          "arrival alone cannot return consumption credit");
    PendingIntents taps;
    taps.observeSnapshot(latest);
    auto press = latest;
    press.physicalDownMask = press.fireMask = 1;
    press.primaryNeutralSampleMask = uint8_t(HandMask & ~press.physicalDownMask & press.primarySampleEligibleMask);
    press.grip[0] = pose(.65f, .1f, 0);
    check(taps.retain(press, 1, 1010), "left tap retained behind outstanding native pose");
    auto otherPress = press;
    otherPress.physicalDownMask = otherPress.fireMask = 2;
    otherPress.primaryNeutralSampleMask = uint8_t(HandMask & ~otherPress.physicalDownMask & otherPress.primarySampleEligibleMask);
    otherPress.grip[1] = pose(-.6f, .2f, 0);
    check(taps.retain(otherPress, 2, 1011), "right tap overlaps without rewriting left intent");
    latest = validPose(2);
    latest.releasedSerial[0] = latest.releasedSerial[1] = 2;
    latest.grip[0] = pose(.2f, 0, 0);
    latest.grip[1] = pose(-.2f, 0, 0);
    taps.observeSnapshot(latest);
    check(!window.grant({11, 22, 2}), "release remains buffered while current capability is full");
    PosePacket sample{};
    check(stream.freeze(1050, sample) && !sample.fireMask && window.count() == 1,
          "delayed neutral reaches native interval without early ACK");
    check(stream.finishInterval(ack) && window.acknowledge(ack, 11, 22) && !window.acknowledge(ack, 11, 22),
          "completed exact token refreshes once; replay cannot refresh capability");
    const auto pulse = taps.apply(latest, 1060);
    check(pulse == 3 && !latest.fireMask && latest.grip[0].p.x == .65f && latest.grip[1].p.x == -.6f &&
              latest.nativeWeaponId[0] == press.nativeWeaponId[0] &&
              latest.trackingGeneration == press.trackingGeneration,
          "released taps carry each original grip, weapon ID and tracking generation");
    check(window.grant({11, 22, 2}) && stream.receive(latest, 1070, ack),
          "buffered taps enter the reliable stream only after prior consumption");
    taps.consume(pulse); // Native send success, exactly as in submit.
    auto unchanged = latest;
    check(!taps.apply(unchanged, 1071) && !unchanged.pulseMask,
          "successful native submission consumes both local intents exactly once");
    check(stream.freeze(1080, sample) && sample.fireMask == 3 && !sample.physicalDownMask,
          "delayed released taps fire for their consumption interval");
    check(stream.finishInterval(ack) && window.acknowledge(ack, 11, 22),
          "tap credit retires only at completed simulation boundary");
    check(stream.freeze(1090, sample) && !sample.fireMask && !stream.finishInterval(ack),
          "released tap cannot become held or produce another consumption ACK");
    latest = validPose(3);
    latest.physicalDownMask = latest.fireMask = 1;
    latest.primaryNeutralSampleMask = uint8_t(HandMask & ~latest.physicalDownMask & latest.primarySampleEligibleMask);
    latest.releasedSerial[0] = 2;
    check(window.grant({11, 22, 3}) && stream.receive(latest, 1100, ack) && stream.freeze(1110, sample) &&
              sample.fireMask == 1,
          "authorized held level enters the native cadence interval");
    check(stream.finishInterval(ack) && window.acknowledge(ack, 11, 22) && stream.freeze(1120, sample) &&
              sample.fireMask == 1,
          "fresh held level remains eligible across native intervals");
    latest = validPose(4);
    latest.releasedSerial[0] = 3;
    check(window.grant({11, 22, 4}) && stream.receive(latest, 1130, ack) && stream.freeze(1140, sample) &&
              !sample.fireMask,
          "next ordered physical release stops held native firing");
    check(stream.finishInterval(ack) && window.acknowledge(ack, 11, 22),
          "release consumption returns its exact reliable credit");

    // Codec -> retained taps -> native interval -> relay -> exact consumption.
    // The two presses deliberately come from different physical head positions.
    OrderedPosePolicy movingRig;
    ConsumptionCredits movingCredit;
    PendingIntents movingTaps;
    auto neutralRig = validPose(1);
    neutralRig.head = neutralRig.grip[0] = neutralRig.grip[1] = pose(0, 0, 0);
    neutralRig.nativeWeaponId[1] = 2;
    neutralRig.releasedSerial[0] = neutralRig.releasedSerial[1] = 1;
    check(movingRig.validation.bindCapability(11, 22) && movingCredit.grant({11, 22, 1}),
          "Moving rig binds the original capability and one consumption credit");
    wire = encodePose(neutralRig);
    check(parse(wire, decoded) == ParseResult::Valid && movingRig.receive(decoded.pose, 2000, ack) &&
              movingRig.freeze(2001, sample) && movingRig.finishInterval(ack) &&
              movingCredit.acknowledge(ack, 11, 22),
          "Encoded neutral snapshot arms both hands through the production receiver");
    movingTaps.observeSnapshot(neutralRig);
    auto leftPress = neutralRig;
    leftPress.grip[0] = pose(HandReachLimit, 0, 0);
    leftPress.physicalDownMask = leftPress.fireMask = 1;
    leftPress.primaryNeutralSampleMask = uint8_t(HandMask & ~leftPress.physicalDownMask & leftPress.primarySampleEligibleMask);
    check(movingTaps.retain(leftPress, 1, 2010), "Boundary left tap keeps its original grip");
    auto rightPress = neutralRig;
    rightPress.head.p.x = .1f;
    rightPress.grip[1].p.x = .1f - HandReachLimit;
    rightPress.physicalDownMask = rightPress.fireMask = 2;
    rightPress.primaryNeutralSampleMask = uint8_t(HandMask & ~rightPress.physicalDownMask & rightPress.primarySampleEligibleMask);
    check(movingTaps.retain(rightPress, 2, 2011), "Boundary right tap uses its own later head position");
    auto movingRelease = neutralRig;
    movingRelease.sequence = 2;
    movingRelease.head.p.x = -.1f;
    movingRelease.grip[0].p = movingRelease.grip[1].p = movingRelease.head.p;
    movingRelease.releasedSerial[0] = movingRelease.releasedSerial[1] = 2;
    check(movingTaps.apply(movingRelease, 2020) == 3 &&
              !handTranslationValid(movingRelease.head.p, movingRelease.grip[0].p),
          "Latest head and independently retained grips reproduce the former codec failure");
    wire = encodePose(movingRelease);
    check(parse(wire, decoded) == ParseResult::Valid && decoded.pose.grip[0].p.x == leftPress.grip[0].p.x &&
              decoded.pose.grip[1].p.x == rightPress.grip[1].p.x && movingCredit.grant({11, 22, 2}) &&
              movingRig.receive(decoded.pose, 2030, ack),
          "Head motion cannot suppress a retained tap or the other hand's outgoing update");
    movingTaps.consume(3);
    PosePacket frozenRelay;
    check(movingRig.freeze(2031, sample, &frozenRelay) && sample.fireMask == 3 &&
              frozenRelay.fireMask == 0 && frozenRelay.pulseMask == 3 &&
              frozenRelay.physicalDownMask == 0 && movingRig.active.pulseMask == 0,
          "Frozen observer relay retains both accepted taps without fabricating held fire");
    relayMessage.relay = {frozenRelay, 99, 7};
    check(encode(relayMessage, wire) && parse(wire, decoded) == ParseResult::Valid &&
              decoded.relay.pose.pulseMask == 3 && !decoded.relay.pose.fireMask &&
              decoded.relay.pose.grip[0].p.x == leftPress.grip[0].p.x &&
              decoded.relay.pose.grip[1].p.x == rightPress.grip[1].p.x,
          "Accepted released taps survive observer relay encoding with their historical grips");
    check(movingRig.freeze(2032, sample, &frozenRelay) && !sample.pulseMask &&
              !sample.fireMask && !frozenRelay.pulseMask && !frozenRelay.fireMask,
          "Next freeze cannot re-emit an already consumed observer pulse");
    check(movingRig.finishInterval(ack) && movingCredit.acknowledge(ack, 11, 22) &&
              !movingCredit.acknowledge(ack, 11, 22),
          "Retained rig taps return their exact consumption credit only once");
    // A retained pulse can coexist with a new physical hold that has no live
    // fire admission. The pulse must not become persistent ordinary relay fire.
    for (uint8_t ordinaryFire : {uint8_t(0), uint8_t(1)}) {
        OrderedPosePolicy relayRig;
        auto baseline = validPose(1);
        check(relayRig.validation.bindCapability(11, 22) &&
                  relayRig.receive(baseline, 3000, ack) && relayRig.freeze(3000, sample) &&
                  relayRig.finishInterval(ack), "Relay mixed-pulse baseline admits neutral");
        auto mixed = baseline;
        mixed.sequence = 2;
        mixed.physicalDownMask = mixed.pulseMask = 1;
        mixed.primaryNeutralSampleMask = 2;
        mixed.fireMask = ordinaryFire;
        check(relayRig.receive(mixed, 3010, ack) &&
                  relayRig.freeze(3010, sample, &frozenRelay) && sample.fireMask == 1 &&
                  frozenRelay.fireMask == ordinaryFire && frozenRelay.physicalDownMask == 1 &&
                  frozenRelay.pulseMask == 1,
              "Relay preserves ordinary admission independently of same-hand historical pulse");
        relayMessage.relay = {frozenRelay, 99, 7};
        check(encode(relayMessage, wire) && parse(wire, decoded) == ParseResult::Valid &&
                  decoded.relay.pose.fireMask == ordinaryFire && decoded.relay.pose.pulseMask == 1,
              "Mixed pulse relay survives real wire codec without inventing held fire");
        check(!relayRig.freeze(3010 + MaxPoseAgeMs + 1, sample, &frozenRelay) &&
                  !frozenRelay.sequence && !frozenRelay.fireMask && !frozenRelay.pulseMask,
              "Expired interval emits no relay snapshot or pulse");
    }
    for (unsigned scenario = 0; scenario != 3; ++scenario) {
        OrderedPosePolicy relayRig;
        auto baseline = validPose(1);
        baseline.nativeWeaponId[0] = 13;
        check(relayRig.validation.bindCapability(11, 22) &&
                  relayRig.receive(baseline, 4000, ack) && relayRig.freeze(4000, sample) &&
                  relayRig.finishInterval(ack), "Historical zoom relay has genuine neutral baseline");
        auto historical = baseline;
        historical.sequence = 2;
        historical.pulseMask = historical.pulseZoomMask = 1;
        check(relayRig.receive(historical, 4010, ack), "Historical zoom pulse reaches pending slot");
        if (scenario == 1)
            relayRig.validation.invalidateWeaponGeneration(2, 1);
        const uint64_t freezeTime = scenario == 2 ? 4010 + MaxPoseAgeMs + 1 : 4010;
        const bool frozen = relayRig.freeze(freezeTime, sample, &frozenRelay);
        if (scenario == 0) {
            relayMessage.relay = {frozenRelay, 99, 7};
            check(frozen && !frozenRelay.fireMask && frozenRelay.pulseMask == 1 &&
                      frozenRelay.pulseZoomMask == 1 && !frozenRelay.zoomMask &&
                      encode(relayMessage, wire) && parse(wire, decoded) == ParseResult::Valid &&
                      decoded.relay.pose.pulseZoomMask == 1,
                  "Released historical zoom remains a pulse context through frozen relay codec");
        } else if (scenario == 1) {
            check(frozen && !frozenRelay.fireMask && !frozenRelay.pulseMask &&
                      !frozenRelay.pulseZoomMask && !frozenRelay.zoomMask,
                  "Revocation between receive and freeze cannot revive observer pulse or zoom");
        } else {
            check(!frozen && !frozenRelay.sequence && !frozenRelay.fireMask &&
                      !frozenRelay.pulseMask && !frozenRelay.pulseZoomMask,
                  "Expiry before first pulse freeze emits an empty observer snapshot");
        }
    }
    movingRelease.sequence = 3;
    movingRelease.grip[0].p = movingRelease.grip[1].p = movingRelease.head.p;
    movingRelease.releasedSerial[0] = movingRelease.releasedSerial[1] = 3;
    check(!movingTaps.apply(movingRelease, 2040), "Successful send does not replay retained rig taps");
    wire = encodePose(movingRelease);
    check(parse(wire, decoded) == ParseResult::Valid && movingCredit.grant({11, 22, 3}) &&
              movingRig.receive(decoded.pose, 2041, ack) && movingRig.freeze(2042, sample) &&
              !sample.fireMask && movingRig.finishInterval(ack) && movingCredit.acknowledge(ack, 11, 22),
          "The next encoded neutral update flows normally after historical tap consumption");

    // Recovery retains old aggregate debt while requiring new physical neutral.
    check(window.grant({11, 22, 5}), "old-capability pose remains in native transport");
    taps.requireNeutral();
    check(!taps.retain(press, 1, 1200), "recovery cannot retain an already held trigger");
    check(window.grant({31, 0, 0}) && !window.grant({31, 0, 0}),
          "recovery allows only one Hello for the new nonce");
    check(stream.validation.bindCapability(31, 32), "server rotates capability on recovery Hello");
    check(window.acknowledge({31, 32, 0}, 31, 0) && !window.acknowledge({31, 32, 0}, 31, 32),
          "initial server nonce requires a matching Hello credit; duplicate cannot refresh");
    auto old = validPose(5);
    check(!stream.receive(old, 1210, ack) && ack.clientNonce == 11 && ack.acceptedSequence == 5 &&
              !window.acknowledge(ack, 31, 32) && window.count() == 0,
          "old queued pose is explicitly discarded; ACK retires old debt without reviving state");
    latest = validPose(1, 2);
    latest.clientNonce = 31;
    latest.serverNonce = 32;
    latest.physicalDownMask = latest.fireMask = 1;
    latest.primaryNeutralSampleMask = uint8_t(HandMask & ~latest.physicalDownMask & latest.primarySampleEligibleMask);
    check(stream.receive(latest, 1220, ack) && stream.freeze(1230, sample) && !sample.fireMask,
          "new capability cannot arm held input before a genuine neutral");
    check(stream.finishInterval(ack), "blocked held snapshot is still processed and ACKed");
    latest.sequence = 2;
    latest.physicalDownMask = latest.fireMask = 0;
    latest.primaryNeutralSampleMask = uint8_t(HandMask & ~latest.physicalDownMask & latest.primarySampleEligibleMask);
    latest.releasedSerial[0] = 4;
    taps.observeSnapshot(latest);
    check(stream.receive(latest, 1240, ack) && stream.freeze(1250, sample) &&
              stream.validation.physicalFireAllowed(0) && stream.finishInterval(ack),
          "new-generation real neutral arms only after processing");

    PendingIntents cancellation;
    check(cancellation.retain(press, 1, 1300) && cancellation.retain(otherPress, 2, 1301),
          "cancellation scenario retains original taps for both hands");
    auto changed = validPose();
    changed.nativeWeaponId[1] = 3;
    check(cancellation.apply(changed, 1310) == 1 && changed.grip[0].p.x == .65f &&
              changed.nativeWeaponId[1] == 3 && changed.grip[1].p.x == -.4f,
          "right weapon change cancels right intent while preserving left tap and right equipment");
    cancellation = {};
    check(cancellation.retain(press, 1, 1311), "epoch trace retains a primary pulse");
    changed = press;
    ++changed.intentEpoch[0];
    check(!cancellation.apply(changed, 1312), "pending intent cannot cross an epoch replacement");
    changed.trackingGeneration = 2;
    check(!cancellation.apply(changed, 1320), "new tracking generation cancels old immutable taps");
    check(!cancellation.retain(press, 1, 1321), "old held generation cannot rearm canceled tap");
    cancellation = {};
    check(cancellation.retain(press, 1, 1400), "expiry scenario retains a tap");
    latest = validPose();
    check(!cancellation.apply(latest, 1600), "expired tap cannot enter a newly available credit");

    // A full server slot, replay, rate discard, or stale capability returns the
    // submitted token, and delayed stale arrival never gains new freshness.
    OrderedPosePolicy blocked;
    blocked.validation.bindCapability(11, 22);
    auto first = validPose(1);
    check(blocked.receive(first, 2000, ack), "server stores one immutable pending snapshot");
    auto second = validPose(2);
    check(!blocked.receive(second, 2001, ack) && ack.acceptedSequence == 2 && blocked.pending.sequence == 1 &&
              blocked.validation.lastSequence == 1,
          "full server slot discards exact new token without changing pending validation");
    check(!blocked.freeze(2201, sample) && blocked.finishInterval(ack) && ack.acceptedSequence == 1,
          "expired queued snapshot is discarded at native boundary without refreshing its age");
    check(!blocked.receive(second, 2202, ack, false) && ack.acceptedSequence == 2,
          "rate-rejected snapshot explicitly returns its own token");
    check(window.grant({31, 32, 3}) && !window.acknowledge({31, 32, 3}, 31, 32, false) && !window.count(),
          "late exact ACK retires credit but cannot revive an expired capability lease");
    check(!window.acknowledge({31, 32, 99}, 31, 32),
          "unexpected current-nonce ACK cannot refresh capability");

    OrderedPosePolicy delayedReplace;
    check(delayedReplace.validation.bindCapability(11, 22), "replacement trace binds its capability");
    auto replaceNeutral = validPose(1);
    replaceNeutral.releasedSerial[0] = replaceNeutral.zoomReleasedSerial[0] = 1;
    check(delayedReplace.receive(replaceNeutral, 2250, ack) && delayedReplace.freeze(2251, sample) &&
              delayedReplace.finishInterval(ack),
          "replacement trace first establishes real release witnesses");
    auto delayedOld = validPose(2);
    delayedOld.physicalDownMask = delayedOld.fireMask = delayedOld.zoomPhysicalDownMask = delayedOld.zoomMask = 1;
    delayedOld.primaryNeutralSampleMask = uint8_t(HandMask & ~delayedOld.physicalDownMask & delayedOld.primarySampleEligibleMask);
    delayedOld.nativeWeaponId[0] = 13;
    check(delayedReplace.receive(delayedOld, 2252, ack), "old-epoch held packet enters the receive slot");
    delayedReplace.validation.invalidateWeaponGeneration(77, 1);
    check(delayedReplace.freeze(2253, sample) && !sample.fireMask && !sample.zoomMask &&
              delayedReplace.finishInterval(ack) && ack.intentEpoch[0] == 2,
          "receive/freeze recheck drops a replaced hand while preserving its exact consumption token");
    auto replacementNeutral = validPose(3);
    replacementNeutral.intentEpoch[0] = 2;
    replacementNeutral.releasedSerial[0] = replacementNeutral.zoomReleasedSerial[0] = 2;
    check(delayedReplace.receive(replacementNeutral, 2254, ack) && delayedReplace.freeze(2255, sample) &&
              delayedReplace.finishInterval(ack),
          "post-replacement active raw-up samples re-arm the new epoch");
    auto replacementHeld = replacementNeutral;
    replacementHeld.sequence = 4;
    replacementHeld.physicalDownMask = replacementHeld.fireMask = replacementHeld.zoomPhysicalDownMask =
        replacementHeld.zoomMask = 1;
    replacementHeld.primaryNeutralSampleMask = uint8_t(HandMask & ~replacementHeld.physicalDownMask & replacementHeld.primarySampleEligibleMask);
    replacementHeld.nativeWeaponId[0] = 13;
    check(delayedReplace.receive(replacementHeld, 2256, ack) && delayedReplace.freeze(2257, sample) &&
              sample.fireMask == 1 && sample.zoomMask == 1,
          "new-epoch held intent follows its own neutral rather than the delayed old packet");
    OrderedPosePolicy firstRetained;
    firstRetained.validation.bindCapability(11, 22);
    auto firstRetainedPose = validPose(1);
    firstRetainedPose.nativeWeaponId[0] = 13;
    firstRetainedPose.pulseMask = firstRetainedPose.pulseZoomMask = 1;
    check(firstRetained.receive(firstRetainedPose, 2270, ack) && firstRetained.freeze(2271, sample) &&
              sample.fireMask == 1 && sample.pulseZoomMask == 1 && intervalZoomMask(sample) == 1,
          "The first post-ACK real neutral may admit a retained released shot without losing its zoom");
    auto consumedZoomShot = sample;
    firstRetained.validation.zoomReleaseObserved[0] = false;
    firstRetained.validation.filterWeaponIntents(consumedZoomShot);
    check(!consumedZoomShot.fireMask && !consumedZoomShot.pulseMask && !consumedZoomShot.pulseZoomMask,
          "Use-boundary zoom revocation cannot downgrade an already composed historical shot");
    // Drive the real raw producer, hysteresis gate, ACK timestamp boundary,
    // retained intent, and native interval policy together. Clear/down latches
    // in the dead band must not launder a cumulative pre-boundary release.
    for (unsigned hand = 0; hand < 2; ++hand) {
        const uint8_t bit = uint8_t(1u << hand), other = uint8_t(HandMask ^ bit);
        for (bool streamChange : {false, true}) {
            OrderedPosePolicy causal;
            check(causal.validation.bindCapability(11, 22), "Causal producer trace binds native capability");
            auto initial = validPose(1);
            check(causal.receive(initial, 3000, ack) && causal.freeze(3001, sample) && causal.finishInterval(ack),
                  "Both raw primary streams begin admitted from actual neutral");
            PendingIntents retained;
            retained.observeSnapshot(initial);
            TriggerGate nativeGate;
            bool physical = false;
            uint32_t serial = 1;
            nativeGate.update(0, true);
            for (int n = 0; n < 6; ++n)
                check(samplePrimaryNeutral(0, true, true, physical, serial), "Pre-boundary release samples accumulate");
            IntentInputBoundary boundary;
            check(boundary.install(1, 10, 2990), "Initial local ACK boundary installs");
            if (!streamChange) {
                causal.validation.invalidateWeaponGeneration(12, bit);
                check(boundary.install(2, 10, 3002), "Replacement ACK installs the affected hand's new epoch");
                retained.requireNeutral(bit);
            }
            uint32_t carrierSequence = 1;
            uint64_t inputSequence = 10, tick = 3002;
            auto produce = [&](float trigger) {
                auto result = validPose(++carrierSequence);
                ++inputSequence;
                ++tick;
                result.intentEpoch[hand] = boundary.echo(inputSequence, tick, tick);
                result.primaryInputGeneration[hand] = streamChange ? 2 : 1;
                const bool neutralSample = samplePrimaryNeutral(trigger, true, true, physical, serial);
                result.primaryNeutralSampleMask &= uint8_t(~bit);
                if (neutralSample) result.primaryNeutralSampleMask |= bit;
                result.releasedSerial[hand] = serial;
                if (physical) result.physicalDownMask |= bit;
                if (nativeGate.update(trigger, true)) result.fireMask |= bit;
                return result;
            };
            auto deadBand = produce(.4f);
            retained.observeSnapshot(deadBand);
            check(!deadBand.physicalDownMask && !(deadBand.primaryNeutralSampleMask & bit) &&
                      causal.receive(deadBand, tick, ack) && causal.freeze(tick, sample) &&
                      !causal.validation.physicalFireAllowed(hand) &&
                      causal.validation.physicalFireAllowed(1 - hand) && causal.finishInterval(ack),
                  "Post-boundary dead-band input cannot reuse a pre-boundary release; other hand remains admitted");
            auto stillHeld = produce(.8f);
            check((stillHeld.fireMask & bit) && !retained.retain(stillHeld, bit, tick),
                  "An armed native gate cannot retain a shot without a new explicit raw neutral");
            auto clientHeld = stillHeld;
            retained.filterPrimaryIntents(clientHeld);
            check(!(clientHeld.fireMask & bit) &&
                      !retained.currentPrimaryIntent(clientHeld, hand, clientHeld.intentEpoch[hand],
                                                     clientHeld.primaryInputGeneration[hand]),
                  "The actual submit/use helpers suppress client held intent as well as retained pulses");
            check(causal.receive(stillHeld, tick, ack) && causal.freeze(tick, sample) &&
                      !(sample.fireMask & bit) && !(sample.pulseMask & bit) && causal.finishInterval(ack),
                  "The server rejects the .4 to .8 trace after either an epoch or logical stream boundary");
            auto newNeutral = produce(.1f);
            retained.observeSnapshot(newNeutral);
            check(causal.receive(newNeutral, tick, ack) && causal.freeze(tick, sample) &&
                      causal.validation.physicalFireAllowed(hand) && causal.finishInterval(ack),
                  "A genuinely new below-threshold sample restores only the affected admission");
            auto newPress = produce(.8f);
            check(retained.retain(newPress, bit, tick) && retained.apply(newPress, tick) == bit &&
                      causal.receive(newPress, tick, ack) && causal.freeze(tick, sample) &&
                      (sample.fireMask & bit) && (sample.pulseMask & bit) && causal.finishInterval(ack),
                  "New raw neutral followed by press restores native held and retained shot behavior");
            retained.filterPrimaryIntents(newPress);
            check(retained.currentPrimaryIntent(newPress, hand, newPress.intentEpoch[hand],
                                                newPress.primaryInputGeneration[hand]),
                  "Client native use accepts its exact admitted held intent");
            auto released = produce(.1f);
            retained.observeSnapshot(released);
            check(!retained.currentPrimaryIntent(released, hand, released.intentEpoch[hand],
                                                 released.primaryInputGeneration[hand]) &&
                      retained.currentPrimaryIntent(released, hand, released.intentEpoch[hand],
                                                    released.primaryInputGeneration[hand], false) &&
                      nativeCommandQuery(0, 1, 2, true) == 1,
                  "An admitted ordinary physical release remains a native release edge without active fire");
            auto cachedHeld = newPress;
            retained.requireNeutral(bit); // ACK arrives AFTER native snapshot/command publication.
            check(!retained.currentPrimaryIntent(cachedHeld, hand, cachedHeld.intentEpoch[hand],
                                                 cachedHeld.primaryInputGeneration[hand]),
                  "An intervening ACK immediately revokes a cached native fire intent before consumption");
            check(!retained.currentPrimaryIntent(released, hand, released.intentEpoch[hand],
                                                  released.primaryInputGeneration[hand], false),
                  "ACK revocation also rejects a stale cached release context");
            check((newPress.primaryNeutralSampleMask & other) == other,
                  "Current neutral witness remains independently attributed to the opposite hand");
        }
    }
    PendingIntents cachedNeutral;
    cachedNeutral.requireNeutral(1);
    auto cachedZero = validPose();
    cachedZero.intentEpoch[0] = 0;
    cachedNeutral.observeSnapshot(cachedZero);
    auto heldAfterZero = validPose(2);
    heldAfterZero.intentEpoch[0] = 2;
    heldAfterZero.physicalDownMask = heldAfterZero.fireMask = 1;
    heldAfterZero.primaryNeutralSampleMask = 2;
    check(!cachedNeutral.retain(heldAfterZero, 1, 3100),
          "An epoch-zero cached neutral cannot arm retained input in a new epoch");
    auto staleCopied = validPose(), staleReturned = staleCopied;
    PeerState liveAdmission;
    liveAdmission.bindCapability(11, 22);
    liveAdmission.invalidateWeaponGeneration(1, 1);
    liveAdmission.filterWeaponIntents(staleReturned);
    check(staleCopied.intentEpoch[0] == staleReturned.intentEpoch[0] &&
              !currentIntentSample(staleCopied, staleReturned, 0, liveAdmission.intentEpoch[0]) &&
              currentIntentSample(staleCopied, staleReturned, 1, liveAdmission.intentEpoch[1]),
          "Matching stale muzzle echoes cannot match live admission; unaffected tracking remains valid");
    staleCopied.intentEpoch[0] = staleReturned.intentEpoch[0] = 2;
    check(currentIntentSample(staleCopied, staleReturned, 0, liveAdmission.intentEpoch[0]) &&
              !staleCopied.fireMask && !staleReturned.fireMask,
          "Current-epoch tracked muzzle input is eligible independently of primary fire");
    ++staleReturned.sequence;
    check(!currentIntentSample(staleCopied, staleReturned, 0, liveAdmission.intentEpoch[0]),
          "A native callback cannot replace the exact captured authority sample");
    PeerState retiredStream;
    for (unsigned hand = 0; hand < 2; ++hand) {
        const uint8_t bit = uint8_t(1u << hand);
        PeerState riderBoundary;
        riderBoundary.bindCapability(11, 22);
        auto request = validPose();
        request.requestedWeapon[0] = 12;
        request.requestedWeapon[1] = 13;
        auto copied = request, pending = request, active = request;
        riderBoundary.invalidateWeaponGeneration(2, bit);
        cancelWeaponRequests(copied, bit);
        cancelWeaponRequests(pending, bit);
        cancelWeaponRequests(active, bit);
        check(copied.requestedWeapon[hand] == -1 && pending.requestedWeapon[hand] == -1 &&
                  active.requestedWeapon[hand] == -1 && copied.requestedWeapon[1 - hand] >= 0,
              "Rider/equipment boundaries cancel copied and retained equip requests per hand");
        riderBoundary.filterWeaponIntents(request);
        check(request.requestedWeapon[hand] == -1 && request.requestedWeapon[1 - hand] >= 0 &&
                  !currentIntentSample(copied, request, hand, riderBoundary.intentEpoch[hand]),
              "A fresh delayed old-epoch equip request cannot execute after dismount");
        request.intentEpoch[hand] = riderBoundary.intentEpoch[hand];
        request.requestedWeapon[hand] = 16;
        copied = request;
        riderBoundary.filterWeaponIntents(request);
        check(request.requestedWeapon[hand] == 16 &&
                  currentIntentSample(copied, request, hand, riderBoundary.intentEpoch[hand]),
              "A genuinely current-epoch weapon selection remains available without a firing requirement");
    }
    retiredStream.bindCapability(11, 22);
    auto streamNeutral = validPose(1);
    streamNeutral.primaryInputGeneration[0] = streamNeutral.zoomInputGeneration[0] = 2;
    streamNeutral.releasedSerial[0] = streamNeutral.zoomReleasedSerial[0] = 2;
    check(retiredStream.acceptPose(streamNeutral, 2280), "Current logical streams establish their own real neutral");
    auto oldStream = validPose(2);
    oldStream.physicalDownMask = oldStream.fireMask = oldStream.zoomPhysicalDownMask = oldStream.zoomMask = 1;
    oldStream.primaryNeutralSampleMask = uint8_t(HandMask & ~oldStream.physicalDownMask & oldStream.primarySampleEligibleMask);
    oldStream.nativeWeaponId[0] = 13;
    oldStream.releasedSerial[0] = oldStream.zoomReleasedSerial[0] = 99;
    check(retiredStream.acceptPose(oldStream, 2281) && !oldStream.fireMask && !oldStream.zoomMask &&
              retiredStream.observedPrimaryInputGeneration[0] == 2 && retiredStream.observedReleasedSerial[0] == 2 &&
              retiredStream.observedZoomInputGeneration[0] == 2 && retiredStream.observedZoomReleasedSerial[0] == 2,
          "A retired stream with a newer carrier sequence cannot revive itself or poison current release baselines");
    PendingIntents aggregateTap;
    ConsumptionCredits debt;
    check(debt.grant({41, 42, 1}) && debt.grant({43, 44, 1}) && debt.grant({45, 46, 1}) &&
              debt.grant({47, 48, 1}) && debt.full() && aggregateTap.retain(press, 1, 2300),
          "tap remains retained with all four aggregate recovery credits occupied");
    auto freed = validPose();
    freed.clientNonce = 47;
    freed.serverNonce = 48;
    check(!debt.acknowledge({41, 42, 1}, 47, 48) && !debt.full() && !debt.grant({47, 48, 2}) &&
              aggregateTap.apply(freed, 2310) == 1,
          "old ACK frees aggregate debt while current capability remains outstanding and tap immutable");
    check(debt.acknowledge({47, 48, 1}, 47, 48) && debt.grant({47, 48, 2}) && freed.grip[0].p.x == .65f,
          "retained original tap can send only after its current-capability consumption ACK");
    aggregateTap.consume(1);
    check(!aggregateTap.apply(freed, 2311), "aggregate-bound tap is consumed only once");
    auto wheelTap = validPose();
    check(aggregateTap.retain(press, 1, 2400) && aggregateTap.retain(otherPress, 2, 2401),
          "wheel scenario begins with both immutable taps");
    wheelTap.wheelOrEquipBlockedMask = 2;
    check(aggregateTap.apply(wheelTap, 2410) == 1 && wheelTap.grip[0].p.x == .65f,
          "right wheel cancels its own pending tap without moving left grip");
    wheelTap.wheelOrEquipBlockedMask = 0;
    wheelTap.physicalDownMask = wheelTap.fireMask = 2;
    wheelTap.primaryNeutralSampleMask = uint8_t(HandMask & ~wheelTap.physicalDownMask & wheelTap.primarySampleEligibleMask);
    check(aggregateTap.apply(wheelTap, 2420) == 1 && !aggregateTap.retain(wheelTap, 2, 2420),
          "closing right wheel while held cannot rearm or replay canceled right tap");
    OrderedPosePolicy replaced;
    replaced.validation.bindCapability(11, 22);
    check(replaced.receive(first, 2500, ack) && replaced.freeze(2510, sample) &&
              replaced.finishInterval(ack) && ack.clientNonce == 11 && ack.serverNonce == 22,
          "HELLO replacing a staged interval explicitly discards the original immutable token");
    replaced = OrderedPosePolicy{};
    replaced.validation.bindCapability(31, 32);
    check(!replaced.finishInterval(ack), "new HELLO peer has no old consumption token to acknowledge");
    check(replaced.receive(latest, 2520, ack) == false,
          "old-capability snapshot remains rejected after staged peer replacement");
    replaced = OrderedPosePolicy{};
    replaced.validation.bindCapability(11, 22);
    check(replaced.receive(first, 2530, ack) && replaced.discardPending(ack) && ack.clientNonce == 11 &&
              ack.acceptedSequence == 1 && !replaced.discardPending(ack),
          "HELLO replacing an unprepared packet discards its exact token once");
    // Transport ownership is distinct from whether the wrapper returned:
    // pre-call failure can revoke, but an interrupted native call is ambiguous.
    for (const auto phase : {TransportPhase::BeforeCall, TransportPhase::EnteredCall,
                             TransportPhase::ReturnedCall}) {
        ConsumptionCredits transport;
        const ConsumptionToken selected{11,22,11}, unrelated{31,32,12};
        check(transport.grant(selected) && transport.grant(unrelated), "handoff credits granted");
        revokeUnsubmitted(transport, selected, phase);
        check(transport.count() == (phase == TransportPhase::BeforeCall ? 1u : 2u) &&
                  transport.hasOutstandingCapability(31,32),
              "only a definitely unsubmitted exact token is revoked");
        if (phase != TransportPhase::BeforeCall)
            check(transport.acknowledge({11,22,11},11,22),
                  "ambiguous/returned submission retains its exact server ACK ownership");
    }
    // An ACK is observed before handoff, without losing its existing slot.
    OrderedPosePolicy unsentAck;
    check(unsentAck.validation.bindCapability(11,22), "unsent ACK stream binds");
    auto unsentPose = validPose(1);
    check(unsentAck.receive(unsentPose,2550,ack) && unsentAck.freeze(2551,sample) &&
              unsentAck.peekInterval(ack) && unsentAck.awaitingConsumption,
          "peeking an outgoing ACK retains the actual receive-slot ownership");
    const Ack selectedAck = ack;
    unsentAck.abortInterval();
    check(unsentAck.peekInterval(ack) && ack.acceptedSequence == selectedAck.acceptedSequence &&
              !unsentAck.hasActive && unsentAck.awaitingConsumption,
          "a failed ACK send leaves a discard token in the same inactive slot");
    check(!unsentAck.confirmInterval({11,22,2}) &&
              !unsentAck.confirmInterval({31,32,1}) && unsentAck.awaitingConsumption,
          "wrong sequence or replacement capability cannot confirm another ACK");
    check(unsentAck.confirmInterval(selectedAck) && !unsentAck.confirmInterval(selectedAck),
          "confirmed handoff retires the exact slot once, without replay");
    OrderedPosePolicy deletedPending;
    check(deletedPending.validation.bindCapability(11,22) &&
              deletedPending.receive(unsentPose,2552,ack) &&
              deletedPending.hasPending && !deletedPending.awaitingConsumption &&
              deletedPending.freeze(2553,sample), "lifecycle pending occupies only one original slot");
    deletedPending.abortInterval();
    check(!deletedPending.hasPending && deletedPending.peekInterval(ack) &&
              ack.acceptedSequence == 1 && !deletedPending.hasActive &&
              deletedPending.confirmInterval(ack),
          "lifecycle discard promotes pending into the original active slot without gameplay");

    // Interrupted native work never retries a partially executed shot. The same
    // immutable credit token is eventually discarded; no extra queue is used.
    OrderedPosePolicy interrupted;
    ConsumptionCredits interruptedCredit;
    check(interrupted.validation.bindCapability(11, 22), "abort stream binds");
    auto beforeAbort = validPose(1);
    beforeAbort.nativeWeaponId[0] = beforeAbort.nativeWeaponId[1] = 13;
    check(interrupted.receive(beforeAbort, 2600, ack) &&
              interrupted.freeze(2601, sample) && interrupted.finishInterval(ack),
          "abort stream first arms native weapons");
    auto firingBeforeAbort = beforeAbort;
    firingBeforeAbort.sequence = 2;
    firingBeforeAbort.physicalDownMask = firingBeforeAbort.fireMask = 3;
    firingBeforeAbort.primaryNeutralSampleMask = 0;
    firingBeforeAbort.zoomPhysicalDownMask = firingBeforeAbort.zoomMask = 3;
    firingBeforeAbort.pulseMask = firingBeforeAbort.pulseZoomMask = 3;
    check(interruptedCredit.grant({11, 22, 2}) &&
              interrupted.receive(firingBeforeAbort, 2602, ack) &&
              interrupted.freeze(2603, sample) && sample.fireMask == 3 &&
              intervalZoomMask(sample) == 3, "abort begins with real admitted shot/zoom");
    interrupted.abortInterval();
    check(!interrupted.hasActive && interrupted.awaitingConsumption &&
              !interrupted.validation.fresh(2604) &&
              !interrupted.freeze(2604, sample) && !sample.fireMask &&
              !sample.pulseMask && !sample.zoomMask && !sample.pulseZoomMask &&
              interruptedCredit.count() == 1,
          "abort blocks replay while retaining the exact outstanding discard credit");
    auto delayedNeutral = beforeAbort;
    delayedNeutral.sequence = 3;
    delayedNeutral.releasedSerial[0] = delayedNeutral.releasedSerial[1] = 2;
    delayedNeutral.zoomReleasedSerial[0] = delayedNeutral.zoomReleasedSerial[1] = 2;
    check(!interrupted.receive(delayedNeutral, 2605, ack),
          "an outstanding interrupted token still bounds the receive slot");
    check(interrupted.finishInterval(ack) && ack.acceptedSequence == 2 &&
              ack.intentEpoch[0] == 2 && ack.intentEpoch[1] == 2 &&
              interruptedCredit.acknowledge(ack, 11, 22) &&
              !interrupted.finishInterval(ack),
          "normal boundary discards exactly once using current rearm epochs");
    check(interrupted.receive(delayedNeutral, 2606, ack) &&
              interrupted.freeze(2607, sample) &&
              !interrupted.validation.physicalFireAllowed(0) &&
              !interrupted.validation.zoomAllowed(0) && interrupted.finishInterval(ack),
          "delayed pre-interruption neutral cannot rearm at an old epoch");
    auto afterAbort = delayedNeutral;
    afterAbort.sequence = 4;
    afterAbort.intentEpoch[0] = afterAbort.intentEpoch[1] = 2;
    check(interrupted.receive(afterAbort, 2608, ack) &&
              interrupted.freeze(2609, sample) && interrupted.finishInterval(ack) &&
              interrupted.validation.physicalFireAllowed(0) &&
              interrupted.validation.zoomAllowed(0),
          "a new current-epoch physical neutral permits recovery");
    auto resumedShot = firingBeforeAbort;
    resumedShot.sequence = 5;
    resumedShot.intentEpoch[0] = resumedShot.intentEpoch[1] = 2;
    check(interrupted.receive(resumedShot, 2610, ack) &&
              interrupted.freeze(2611, sample) && sample.fireMask == 3 &&
              intervalZoomMask(sample) == 3 && interrupted.finishInterval(ack),
          "ordinary native input resumes after the fresh neutral boundary");
    interrupted.abortInterval(); // An interrupted held interval has no new token.
    check(!interrupted.freeze(2612, sample) && !interrupted.finishInterval(ack),
          "aborting a repeated held interval fabricates no credit token");
    // Historical shot context must survive credit pressure in BOTH directions:
    // a zoomed tap after release, and an unzoomed tap after newly holding zoom.
    for (unsigned hand = 0; hand < 2; ++hand) {
        const uint8_t bit = uint8_t(1u << hand), other = uint8_t(3u ^ bit);
        for (bool historicallyZoomed : {false, true}) {
            PendingIntents retainedZoom;
            auto shot = validPose();
            shot.nativeWeaponId[0] = shot.nativeWeaponId[1] = 13;
            shot.physicalDownMask = shot.fireMask = bit;
            shot.primaryNeutralSampleMask = uint8_t(HandMask & ~shot.physicalDownMask & shot.primarySampleEligibleMask);
            shot.zoomMask = historicallyZoomed ? bit : 0;
            shot.zoomPhysicalDownMask = shot.zoomMask;
            check(retainedZoom.retain(shot, bit, 3000), "native shot retains its zoom context");
            auto newest = shot;
            newest.sequence = 2;
            newest.physicalDownMask = newest.fireMask = 0;
            newest.primaryNeutralSampleMask = uint8_t(HandMask & ~newest.physicalDownMask & newest.primarySampleEligibleMask);
            newest.zoomMask = uint8_t(other | (historicallyZoomed ? 0 : bit));
            newest.zoomPhysicalDownMask = newest.zoomMask;
            newest.grip[hand].p.x += .1f;
            newest.releasedSerial[hand] = 2;
            check(retainedZoom.apply(newest, 3010) == bit &&
                      newest.pulseZoomMask == (historicallyZoomed ? bit : 0) &&
                      newest.grip[hand].p.x == shot.grip[hand].p.x,
                  "historical grip and zoom travel together without replacing desired zoom");
            const auto desired = newest.zoomMask;
            wire = encodePose(newest);
            check(parse(wire, decoded) == ParseResult::Valid && decoded.pose.zoomMask == desired &&
                      decoded.pose.pulseZoomMask == newest.pulseZoomMask,
                  "both zoom contexts round trip through native carrier");
            auto retry = newest;
            check(retainedZoom.apply(retry, 3011) == bit && retry.pulseZoomMask == newest.pulseZoomMask,
                  "failed native send retains identical historical zoom context");
            OrderedPosePolicy zoomServer;
            zoomServer.validation.bindCapability(11, 22);
            auto armed = newest;
            armed.sequence = 1;
            armed.physicalDownMask = armed.fireMask = armed.pulseMask = armed.zoomMask =
                armed.pulseZoomMask = armed.zoomPhysicalDownMask = 0;
            armed.primaryNeutralSampleMask = uint8_t(HandMask & ~armed.physicalDownMask & armed.primarySampleEligibleMask);
            armed.releasedSerial[0] = armed.releasedSerial[1] = 1;
            armed.zoomReleasedSerial[0] = armed.zoomReleasedSerial[1] = 1;
            check(zoomServer.receive(armed, 3011, ack) && zoomServer.freeze(3011, sample) &&
                      zoomServer.finishInterval(ack),
                  "fresh active raw-up samples arm primary and zoom before retained history arrives");
            check(zoomServer.receive(decoded.pose, 3012, ack) && zoomServer.freeze(3013, sample) &&
                      sample.fireMask == bit && intervalZoomMask(sample) == uint8_t(other | newest.pulseZoomMask),
                  "authoritative pulse interval uses historical zoom even when latest level is opposite");
            check(zoomServer.finishInterval(ack) && ack.acceptedSequence == 2 &&
                      zoomServer.freeze(3014, sample) && !sample.pulseMask && !sample.pulseZoomMask &&
                      intervalZoomMask(sample) == desired,
                  "next native interval resumes latest desired zoom without retaining a tap");
            retainedZoom.consume(bit);
            check(!retainedZoom.apply(retry, 3015) && !retry.pulseZoomMask && retry.zoomMask == desired,
                  "consumed zoom context cannot replay or clear unrelated held zoom");
            retainedZoom = {};
            check(retainedZoom.retain(shot, bit, 3020), "blocked scenario retains zoom context");
            retry.wheelOrEquipBlockedMask = bit;
            retry.zoomMask &= uint8_t(~bit);
            check(!retainedZoom.apply(retry, 3021) && !retry.pulseZoomMask,
                  "wheel cancellation removes historical shot and its zoom context together");

            OrderedPosePolicy unarmedZoomServer;
            unarmedZoomServer.validation.bindCapability(11, 22);
            auto unarmed = newest;
            unarmed.releasedSerial[0] = unarmed.releasedSerial[1] = 0;
            auto zoomNeutral = armed;
            zoomNeutral.primarySampleEligibleMask = 0;
            zoomNeutral.primaryNeutralSampleMask = 0;
            check(unarmedZoomServer.receive(zoomNeutral, 3099, ack) &&
                      unarmedZoomServer.freeze(3099, sample) && unarmedZoomServer.finishInterval(ack),
                  "zoom can establish its own release admission while primary remains unarmed");
            check(unarmedZoomServer.receive(unarmed, 3100, ack) &&
                      unarmedZoomServer.freeze(3101, sample) && !sample.fireMask &&
                      !sample.pulseMask && !sample.pulseZoomMask &&
                      sample.zoomMask == desired && intervalZoomMask(sample) == desired,
                  "rejected shot cannot override either hand's current zoom intent");
            check(unarmedZoomServer.finishInterval(ack) && ack.acceptedSequence == unarmed.sequence &&
                      !unarmedZoomServer.finishInterval(ack) &&
                      unarmedZoomServer.freeze(3102, sample) && !sample.fireMask &&
                      !sample.pulseMask && !sample.pulseZoomMask && sample.zoomMask == desired,
                  "rejected shot is acknowledged once and its zoom context never replays");
        }
    }
    PendingIntents oppositeHistories;
    auto twoShots = validPose();
    twoShots.nativeWeaponId[0] = twoShots.nativeWeaponId[1] = 13;
    twoShots.fireMask = twoShots.physicalDownMask = 3;
    twoShots.primaryNeutralSampleMask = uint8_t(HandMask & ~twoShots.physicalDownMask & twoShots.primarySampleEligibleMask);
    twoShots.zoomMask = 1;
    twoShots.zoomPhysicalDownMask = 1;
    check(oppositeHistories.retain(twoShots, 3, 4000), "two hands retain opposite zoom histories");
    auto twoNewest = twoShots;
    twoNewest.fireMask = twoNewest.physicalDownMask = 0;
    twoNewest.primaryNeutralSampleMask = uint8_t(HandMask & ~twoNewest.physicalDownMask & twoNewest.primarySampleEligibleMask);
    twoNewest.zoomMask = 2;
    twoNewest.zoomPhysicalDownMask = 2;
    check(oppositeHistories.apply(twoNewest, 4001) == 3 && twoNewest.pulseZoomMask == 1 &&
              intervalZoomMask(twoNewest) == 1,
          "simultaneous histories stay independent from opposite current desired levels");
    oppositeHistories.requireNeutral(1);
    check(oppositeHistories.apply(twoNewest, 4002) == 2 && !twoNewest.pulseZoomMask &&
              twoNewest.zoomMask == 2 && intervalZoomMask(twoNewest) == 0,
          "left cancellation preserves the right unzoomed shot's historical override");
    PendingIntents streamHistories;
    auto streamShots = twoShots;
    streamShots.zoomMask = streamShots.zoomPhysicalDownMask = 2;
    check(streamHistories.retain(streamShots, 3, 4050),
          "primary histories capture one unzoomed and one zoomed hand independently");
    auto streamChanged = streamShots;
    streamChanged.fireMask = streamChanged.physicalDownMask = 0;
    streamChanged.primaryNeutralSampleMask = uint8_t(HandMask & ~streamChanged.physicalDownMask & streamChanged.primarySampleEligibleMask);
    streamChanged.zoomMask = streamChanged.zoomPhysicalDownMask = 0;
    ++streamChanged.zoomInputGeneration[1];
    check(streamHistories.apply(streamChanged, 4051) == 1 && !streamChanged.pulseZoomMask,
          "zoom stream replacement cancels only its zoomed history and preserves opposite unzoomed primary");
    check(!oppositeHistories.apply(twoNewest, 4200) && !twoNewest.pulseZoomMask &&
              intervalZoomMask(twoNewest) == 2,
          "expired historical context restores current desired level without a retained override");
    oppositeHistories = {};
    check(oppositeHistories.retain(twoShots, 3, 4300), "tracking-generation scenario retains zoom context");
    ++twoNewest.trackingGeneration;
    check(!oppositeHistories.apply(twoNewest, 4301) && !twoNewest.pulseZoomMask,
          "tracking-generation change cancels both historical zoom contexts");
    oppositeHistories = {};
    check(oppositeHistories.retain(twoShots, 3, 4400), "capability scenario retains zoom context");
    twoNewest.trackingGeneration = twoShots.trackingGeneration;
    oppositeHistories.requireNeutral(); // Existing capability reset's retention boundary.
    check(!oppositeHistories.apply(twoNewest, 4401) && !twoNewest.pulseZoomMask,
          "capability retirement makes prior shot zoom context inaccessible");
    PendingIntents expiringZoom;
    auto zoomReleased = twoShots;
    zoomReleased.fireMask = zoomReleased.physicalDownMask = zoomReleased.zoomMask = 0;
    zoomReleased.primaryNeutralSampleMask = uint8_t(HandMask & ~zoomReleased.physicalDownMask & zoomReleased.primarySampleEligibleMask);
    check(expiringZoom.retain(twoShots, 1, 4500) && expiringZoom.apply(zoomReleased, 4501) == 1 &&
              zoomReleased.pulseZoomMask == 1,
          "expiry scenario has a live nonzero historical zoom mask");
    check(!expiringZoom.apply(zoomReleased, 4700) && !zoomReleased.pulseZoomMask &&
              !intervalZoomMask(zoomReleased),
          "nonzero historical zoom expires with its retained shot");
    for (unsigned hand = 0; hand < 2; ++hand) {
        auto zoom = validPose();
        zoom.nativeWeaponId[hand] = 13;
        zoom.zoomMask = uint8_t(1u << hand);
        zoom.zoomPhysicalDownMask = zoom.zoomMask;
        check(validatePose(zoom), "tracked sniper held zoom is valid");
        for (int failure = 0; failure < 6; ++failure) {
            auto invalidZoom = zoom;
            switch (failure) {
            case 0: invalidZoom.validMask &= uint8_t(~1u); break;
            case 1: invalidZoom.validMask &= uint8_t(~(2u << hand)); break;
            case 2: invalidZoom.wheelOrEquipBlockedMask = invalidZoom.zoomMask; break;
            case 3: invalidZoom.nativeWeaponId[hand] = 2; break;
            case 4: invalidZoom.requestedWeapon[hand] = 13; break;
            case 5: invalidZoom.zoomMask = 4; break;
            }
            check(!validatePose(invalidZoom), "invalid held zoom is rejected before native routing");
        }
        auto invalidPulse = zoom;
        invalidPulse.zoomMask = 0;
        invalidPulse.pulseZoomMask = uint8_t(1u << hand);
        check(!validatePose(invalidPulse), "historical zoom requires its matching shot pulse");
        invalidPulse.pulseMask = invalidPulse.pulseZoomMask;
        invalidPulse.nativeWeaponId[hand] = 2;
        check(!validatePose(invalidPulse), "historical zoom requires a matching native sniper type");
        auto bothHands = validPose();
        bothHands.nativeWeaponId[0] = bothHands.nativeWeaponId[1] = 13;
        bothHands.physicalDownMask = bothHands.fireMask = bothHands.pulseMask =
            bothHands.zoomMask = bothHands.pulseZoomMask = 3;
        bothHands.primaryNeutralSampleMask = uint8_t(HandMask & ~bothHands.physicalDownMask & bothHands.primarySampleEligibleMask);
        bothHands.zoomPhysicalDownMask = 3;
        invalidateWeaponIntents(bothHands, uint8_t(1u << hand));
        const auto surviving = uint8_t(3u ^ (1u << hand));
        check(bothHands.fireMask == surviving && bothHands.pulseMask == surviving &&
                  bothHands.zoomMask == surviving && bothHands.pulseZoomMask == surviving &&
                  bothHands.physicalDownMask == 3 && validatePose(bothHands),
              "native equipment invalidation retires one hand's complete context without fake release");
                  bothHands.primaryNeutralSampleMask = uint8_t(HandMask & ~bothHands.physicalDownMask & bothHands.primarySampleEligibleMask);
    }
    {
        PeerState current;
        check(current.bindCapability(11,22),"Live frozen zoom filtering binds existing capability");
        auto neutral=validPose(1);
        neutral.nativeWeaponId[0]=13;
        check(current.acceptPose(neutral,5000),"Native sniper receives actual neutral before held zoom");
        auto held=neutral;held.sequence=2;
        held.zoomMask=held.zoomPhysicalDownMask=1;
        check(current.acceptPose(held,5001) && intervalZoomMask(held)==1,
              "Captured native interval initially has admitted held zoom");
        const auto captured=held;
        auto unavailable=held;unavailable.sequence=3;
        unavailable.zoomMask=0;unavailable.zoomSampleEligibleMask&=uint8_t(~1u);
        ++unavailable.zoomInputGeneration[0];
        check(current.acceptPose(unavailable,5002),"New action loss updates live admission without weapon replacement");
        current.filterWeaponIntents(held);
        check(currentIntentSample(captured,held,0,current.intentEpoch[0]) &&
              intervalZoomMask(captured)==1 && intervalZoomMask(held)==0,
              "Same frozen sequence/epoch does not preserve zoom rejected by current action admission");
    }
    std::cout << "Portable SS2 VR network codec, zoom context and peer checks passed\n";
}
