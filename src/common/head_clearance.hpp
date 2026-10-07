#pragma once
#include "math.hpp"

namespace ss2vr {
inline bool validHeadClearance(const HeadClearance& c) {
    if(c.reserved)return false;
    if(c.mode==HeadClearanceMode::Disabled||c.mode==HeadClearanceMode::Opaque)
        return !c.tickMs&&c.centre.x==0&&c.centre.y==0&&c.centre.z==0&&
               c.clearRadius==0&&c.headRadius==0&&c.nearZ==0;
    return c.mode==HeadClearanceMode::Clear&&c.tickMs&&
        std::isfinite(c.centre.x)&&std::isfinite(c.centre.y)&&std::isfinite(c.centre.z)&&
        std::isfinite(c.clearRadius)&&c.clearRadius>0&&c.clearRadius<=.5f&&
        std::isfinite(c.headRadius)&&c.headRadius>0&&c.headRadius<=c.clearRadius&&
        std::isfinite(c.nearZ)&&c.nearZ>0&&c.nearZ<=c.clearRadius;
}
inline bool headClearanceAllows(const HeadClearance& c,const Request& image,
                                 const Input& latest,const Pose (&eyes)[2],uint64_t now) {
    if(!validHeadClearance(c))return false;
    if(c.mode!=HeadClearanceMode::Clear)return true;
    if(c.centre.x!=image.input.head.p.x||c.centre.y!=image.input.head.p.y||c.centre.z!=image.input.head.p.z||
       c.tickMs<image.input.tickMs||now<c.tickMs||now-c.tickMs>100||!latest.headValid||!latest.focused||!finite(latest.head)||
       latest.session!=image.session||latest.reference!=image.reference)return false;
    const auto distance=[&](Vec3 p) {
        return std::hypot(double(p.x)-c.centre.x,double(p.y)-c.centre.y,double(p.z)-c.centre.z);
    };
    if(distance(latest.head.p)+c.headRadius>c.clearRadius)return false;
    for(unsigned h=0;h<2;++h) {
        const auto f=image.fov[h];
        if(!finite(eyes[h])||!std::isfinite(f.left)||!std::isfinite(f.right)||!std::isfinite(f.up)||
           !std::isfinite(f.down)||f.left>=f.right||f.down>=f.up||
           std::abs(f.left)>=1.5f||std::abs(f.right)>=1.5f||std::abs(f.up)>=1.5f||std::abs(f.down)>=1.5f)
            return false;
        const double x=std::max(std::abs(std::tan(double(f.left))),std::abs(std::tan(double(f.right))));
        const double y=std::max(std::abs(std::tan(double(f.down))),std::abs(std::tan(double(f.up))));
        // A sphere encloses every orientation of the cached image's near-plane
        // corners around the current tracked eye; rotation cannot spend margin
        // that was only checked for one previous view orientation.
        const double extent=double(c.nearZ)*std::sqrt(1+x*x+y*y)+.0005;
        if(!std::isfinite(extent)||distance(eyes[h].p)+extent>c.clearRadius)return false;
    }
    return true;
}
} // namespace ss2vr
