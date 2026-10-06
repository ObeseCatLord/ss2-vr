#pragma once
#include "roomscale_primitive_query.hpp"
#include "math.hpp"
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
// Enclose the affine image of every local sphere under a COPIED actual matrix.
// This is a mathematical cover, not permission to invent the native transform.
// The caller must establish that this matrix describes the queried hull and
// that all captured hulls are included. No native query or pose is changed.
using AffineEnvelope=std::array<detail::Interval,12>;
inline CapsuleCover placeAffineEnvelope(const CapsuleCover& local,const AffineEnvelope& matrix,
                                        float maximumRadius) {
    using namespace detail;
    CapsuleCover result;
    if (!arithmeticSupported()||!local.valid||!local.count||local.count>local.sphere.size()||
        !std::isfinite(maximumRadius)||maximumRadius<=0) return result;
    for(auto v:matrix) if(!valid(v)) return result;
    std::array<IntervalVector,3> rows{};
    for(unsigned r=0;r<3;++r) for(unsigned c=0;c<3;++c)
        rows[r][c]=matrix[r*4+c];
    const IntervalVector crossRows{
        minus(times(rows[1][1],rows[2][2]),times(rows[1][2],rows[2][1])),
        minus(times(rows[1][2],rows[2][0]),times(rows[1][0],rows[2][2])),
        minus(times(rows[1][0],rows[2][1]),times(rows[1][1],rows[2][0]))};
    const auto determinant=inner(rows[0],crossRows);
    if (!valid(determinant)||(determinant.lo<=0&&determinant.hi>=0)) return result;
    // ||A||_2^2 = lambda_max(A^T A) <= ||A^T A||_infinity. Interval
    // products/sums and outward sqrt enclose rounding in the spectral bound.
    double squaredScale=0;
    for(unsigned r=0;r<3;++r) {
        auto rowSum=exact(0);
        for(unsigned c=0;c<3;++c) {
            auto dotProduct=exact(0);
            for(unsigned k=0;k<3;++k)
                dotProduct=plus(dotProduct,times(rows[k][r],rows[k][c]));
            if(!valid(dotProduct)) return {};
            rowSum=plus(rowSum,exact(std::max(std::abs(dotProduct.lo),std::abs(dotProduct.hi))));
        }
        if(!valid(rowSum)) return {};
        squaredScale=std::max(squaredScale,rowSum.hi);
    }
    const double scale=up(std::sqrt(squaredScale));
    if(!std::isfinite(scale)||scale<=0) return {};
    for(unsigned i=0;i<local.count;++i) {
        const auto& sphere=local.sphere[i];
        if(!detail::finite(convert(sphere.centre))||!std::isfinite(sphere.radius)||sphere.radius<=0)
            return {};
        const auto input=interval(convert(sphere.centre));
        std::array<float,3> centre{};
        IntervalVector errors{};
        for(unsigned r=0;r<3;++r) {
            const auto world=plus(inner(rows[r],input),matrix[r*4+3]);
            if(!valid(world)) return {};
            centre[r]=static_cast<float>(world.lo*.5+world.hi*.5);
            if(!std::isfinite(centre[r])) return {};
            errors[r]=minus(world,exact(centre[r]));
        }
        const auto error=length(errors);
        if(!valid(error)) return {};
        const auto needed=plus(times(exact(sphere.radius),exact(scale)),exact(error.hi));
        const float radius=valid(needed)?outwardFloat(needed.hi):0;
        if(!radius||radius>maximumRadius) return {};
        result.sphere[result.count++]={{centre[0],centre[1],centre[2]},radius};
    }
    result.valid=true;
    return result;
}
inline CapsuleCover placeAffineCover(const CapsuleCover& local,const ss2vr::Matrix34& matrix,
                                     float maximumRadius) {
    AffineEnvelope exactMatrix{};
    for(unsigned i=0;i<12;++i) exactMatrix[i]=detail::exact(matrix.m[i]);
    return placeAffineEnvelope(local,exactMatrix,maximumRadius);
}
} // namespace ss2vr::roomscale
