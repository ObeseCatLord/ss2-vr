#pragma once
#include "math.hpp"
namespace ss2vr {
// A sampled native lean query is not body collision or a containment test.
struct HeadProbe {
    Vec3 origin, direction;
    float length = 0;
    bool valid = false;
};
inline HeadProbe makeHeadProbe(const Pose &body, const Pose &head) {
    if (!finite(body) || !finite(head))
        return {};
    const Vec3 delta = head.p - body.p;
    const float length = std::sqrt(dot(delta, delta));
    if (!std::isfinite(length) || length < .001f || length > MaximumHeadTranslation + TrackingBoundsSlack)
        return {};
    return {body.p, delta * (1.f / length), length, true};
}
struct HeadObstruction {
    Pose body, head;
    uint32_t owner = 0, generation = 0, session = 0, reference = 0;
    uint64_t requestSequence = 0, inputSequence = 0, tickMs = 0;
    float length = 0, distance = 0;
    bool sampled = false, hit = false;
};
inline bool headDistanceConsistent(float length, float distance, bool hit) {
    // rayIsHit's strict comparison also returns false for NaN. Validate the raw
    // distance even on a miss; do not substitute the maximum for invalid data.
    return std::isfinite(length) && length >= .001f &&
           length <= MaximumHeadTranslation + TrackingBoundsSlack && std::isfinite(distance) &&
           distance >= 0 && distance <= length && (hit ? distance < length : distance == length);
}
inline HeadObstruction recordHeadObstruction(const Pose &body, const Pose &head, uint32_t owner,
                                             const Request &request, uint64_t now, const HeadProbe &probe,
                                             bool hit, float rawDistance) {
    if (!probe.valid || !owner || !finite(body) || !finite(head) ||
        !headDistanceConsistent(probe.length, rawDistance, hit))
        return {};
    return {body,
            head,
            owner,
            request.trackingGeneration,
            request.session,
            request.reference,
            request.sequence,
            request.input.sequence,
            now,
            probe.length,
            rawDistance,
            true,
            hit};
}
inline bool sameHeadQueryPose(const Pose &a, const Pose &b) {
    // Numeric equality accepts signed zero; no spatial tolerance or old-body reuse.
    return finite(a) && finite(b) && a.p.x == b.p.x && a.p.y == b.p.y && a.p.z == b.p.z && a.q.x == b.q.x &&
           a.q.y == b.q.y && a.q.z == b.q.z && a.q.w == b.q.w;
}
struct HeadVisibility {
    bool available = false;
    float value = 1;
};
inline HeadVisibility freezeHeadVisibility(const HeadObstruction &sample, const Pose &body, const Pose &head,
                                           uint32_t owner, const Request &request, uint64_t now,
                                           float fadeDepth) {
    if (!sample.sampled || !owner || sample.owner != owner || !request.sequence || !request.input.sequence ||
        !request.trackingGeneration || sample.generation != request.trackingGeneration ||
        sample.session != request.session || sample.reference != request.reference ||
        request.input.session != request.session || request.input.reference != request.reference ||
        sample.requestSequence != request.sequence || sample.inputSequence != request.input.sequence ||
        now < sample.tickMs || now - sample.tickMs > 100 || !sameHeadQueryPose(sample.body, body) ||
        !sameHeadQueryPose(sample.head, head) ||
        !headDistanceConsistent(sample.length, sample.distance, sample.hit) || !std::isfinite(fadeDepth) ||
        fadeDepth <= 0)
        return {};
    return {true,
            sample.hit ? std::clamp(1.f - (sample.length - sample.distance) / fadeDepth, 0.f, 1.f) : 1.f};
}
} // namespace ss2vr
