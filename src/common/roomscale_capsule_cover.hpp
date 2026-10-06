#pragma once
#include "roomscale_primitive_query.hpp"
#include <array>

namespace ss2vr::roomscale {
struct CoverSphere { Vector centre{}; float radius=0; };
struct CapsuleCover {
    static constexpr unsigned MaximumCells=32;
    std::array<CoverSphere,MaximumCells+2> sphere{};
    unsigned count=0;
    bool valid=false;
};

inline float outwardFloat(double value) {
    if (!std::isfinite(value)||value<=0||value>std::numeric_limits<float>::max()) return 0;
    float result=static_cast<float>(value);
    if (double(result)<value) result=std::nextafter(result,std::numeric_limits<float>::infinity());
    return std::isfinite(result)&&result>0?result:0;
}

// Cover the ACTUAL descriptor, in its local Y-axis frame. No guessed player
// dimensions or body resize: only query spheres are conservatively enlarged.
// The caller supplies a maximum relative query-radius increase. Failure is not
// permission to use fewer spheres or relax that budget. At most34 native sphere
// queries are required; the caller still owns transforms, query and settlement.
inline CapsuleCover coverCapsule(const Primitive& actual,float relativeRadiusBudget) {
    using namespace detail;
    CapsuleCover rejected;
    if (!arithmeticSupported()||actual.kind!=2||!std::isfinite(actual.width)||actual.width<=0||
        !std::isfinite(actual.height)||actual.height<actual.width||
        !std::isfinite(relativeRadiusBudget)||relativeRadiusBudget<0||relativeRadiusBudget>1) return rejected;
    const double r=double(actual.width)*.5;
    const double maximumRadius=relativeRadiusBudget==0?r:
        plus(exact(r),times(exact(r),exact(relativeRadiusBudget))).lo;
    auto accept=[&](double needed) {
        const float value=outwardFloat(needed);
        return value&&double(value)<=maximumRadius?value:0.f;
    };
    if (actual.width==actual.height) {
        const auto radius=accept(r);
        if (!radius) return rejected;
        rejected.sphere[0]={{0,0,0},radius};rejected.count=1;rejected.valid=true;
        return rejected;
    }
    // An enlarged spine encloses the exact descriptor even when subtraction
    // rounds. Cover that enclosing capsule, rather than assuming cancellation
    // was exact for very disparate finite dimensions.
    const auto halfSpine=minus(exact(double(actual.height)*.5),exact(r));
    if (!valid(halfSpine)||halfSpine.hi<=0) return rejected;
    const double extent=halfSpine.hi;
    const float end=static_cast<float>(extent);
    if (!std::isfinite(end)) return rejected;
    const double endError=up(std::abs(double(end)-extent));
    const float endpointRadius=accept(plus(exact(r),exact(endError)).hi);
    if (!endpointRadius) return rejected;
    for(unsigned cells=1;cells<=CapsuleCover::MaximumCells;++cells) {
        CapsuleCover result;
        result.sphere[0]={{0,-end,0},endpointRadius};
        result.sphere[1]={{0,end,0},endpointRadius};result.count=2;
        const auto halfCell=dividePositive(exact(extent),exact(cells));
        bool fits=true;
        for(unsigned i=0;i<cells;++i) {
            const auto factor=minus(dividePositive(exact(2*i+1),exact(cells)),exact(1));
            const auto centre=times(exact(extent),factor);
            const float y=static_cast<float>(extent*(double(2*i+1)/cells-1));
            if (!valid(centre)||!std::isfinite(y)) { fits=false; break; }
            const double error=std::max(up(std::abs(double(y)-centre.lo)),up(std::abs(double(y)-centre.hi)));
            const auto halfGap=plus(exact(halfCell.hi),exact(error));
            const auto needed=length({exact(r),halfGap,exact(0)});
            const float radius=valid(needed)?accept(needed.hi):0;
            if (!radius) { fits=false; break; }
            result.sphere[result.count++]={{0,y,0},radius};
        }
        if (fits) { result.valid=true; return result; }
    }
    return rejected;
}
// Translate an upright local cover to the native world float grid. Each query
// sphere grows by a bound on centre quantization; a distant/poorly representable
// position cannot silently shrink the covered avatar. maximumRadius is an
// absolute caller-owned budget, not permission to resize the native body.
inline CapsuleCover placeUprightCover(const CapsuleCover& local,Vector worldOrigin,
                                      float maximumRadius) {
    using namespace detail;
    CapsuleCover result;
    if (!arithmeticSupported()||!local.valid||!local.count||local.count>local.sphere.size()||
        !std::isfinite(worldOrigin.x)||!std::isfinite(worldOrigin.y)||!std::isfinite(worldOrigin.z)||
        !std::isfinite(maximumRadius)||maximumRadius<=0) return result;
    for(unsigned i=0;i<local.count;++i) {
        const auto& sphere=local.sphere[i];
        if (sphere.centre.x!=0||sphere.centre.z!=0||!std::isfinite(sphere.centre.y)||
            !std::isfinite(sphere.radius)||sphere.radius<=0) return {};
        if (sphere.centre.y==0) {
            if (sphere.radius>maximumRadius) return {};
            result.sphere[result.count++]={worldOrigin,sphere.radius};
            continue;
        }
        const auto exactY=plus(exact(worldOrigin.y),exact(sphere.centre.y));
        const float y=static_cast<float>(double(worldOrigin.y)+double(sphere.centre.y));
        if (!valid(exactY)||!std::isfinite(y)) return {};
        const double error=std::max(up(std::abs(double(y)-exactY.lo)),
                                    up(std::abs(double(y)-exactY.hi)));
        const auto radius=outwardFloat(plus(exact(sphere.radius),exact(error)).hi);
        if (!radius||radius>maximumRadius) return {};
        result.sphere[result.count++]={{worldOrigin.x,y,worldOrigin.z},radius};
    }
    result.valid=true;
    return result;
}
} // namespace ss2vr::roomscale
