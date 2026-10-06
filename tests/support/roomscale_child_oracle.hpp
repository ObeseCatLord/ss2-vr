#pragma once
#include "common/math.hpp"
#include <array>
#include <cmath>
using ss2vr::Pose;
namespace ss2vr_test {
// Independent scalar oracle with a directed binary32 rounding step after EVERY
// operation. The production bound must also enclose a wide-significand route.
struct Arithmetic {
    int direction=0; bool wide=false;
    long double round(long double x) const {
        if(wide)return x;
        float r=static_cast<float>(x);
        if(direction==-1 && (long double)r>x)r=std::nextafter(r,-INFINITY);
        if(direction==1 && (long double)r<x)r=std::nextafter(r,INFINITY);
        return r;
    }
    long double add(long double a,long double b)const {return round(a+b);}
    long double sub(long double a,long double b)const {return round(a-b);}
    long double mul(long double a,long double b)const {return round(a*b);}
};
std::array<long double,7> oracle(Pose p,Pose l,Arithmetic f) {
    const auto [x,y,z,w]=p.q;const auto [u,v,t,s]=l.q;
    const auto twice=[&](long double a,long double b) {auto r=f.mul(a,b);return f.add(r,r);};
    const auto a=twice(x,x),b=twice(x,y),c=twice(x,z),d=twice(y,y),e=twice(y,z),ff=twice(z,z);
    const auto g=twice(w,x),h=twice(w,y),i=twice(w,z);
    const long double m[3][3]={{f.sub(1,f.add(ff,d)),f.sub(b,i),f.add(h,c)},
                            {f.add(i,b),f.sub(1,f.add(a,ff)),f.sub(e,g)},
                            {f.sub(c,h),f.add(g,e),f.sub(1,f.add(d,a))}};
    std::array<long double,7> result{
        f.sub(f.add(f.add(f.mul(w,u),f.mul(y,t)),f.mul(x,s)),f.mul(z,v)),
        f.add(f.add(f.sub(f.mul(w,v),f.mul(x,t)),f.mul(z,u)),f.mul(y,s)),
        f.add(f.sub(f.add(f.mul(x,v),f.mul(w,t)),f.mul(y,u)),f.mul(z,s)),
        f.sub(f.sub(f.sub(f.mul(w,s),f.mul(x,u)),f.mul(y,v)),f.mul(z,t)),0,0,0};
    for(unsigned row=0;row<3;++row) {
        const auto first=row==0?f.mul(m[row][2],l.p.z):f.mul(m[row][0],l.p.x);
        const auto second=row==0?f.mul(m[row][0],l.p.x):f.mul(m[row][2],l.p.z);
        auto value=f.add(f.add(first,second),f.mul(m[row][1],l.p.y));
        // x/y native intermediates spill before adding parent translation.
        if(row<2)value=f.round(value);
        const float parent[3]={p.p.x,p.p.y,p.p.z};
        result[row+4]=f.round(f.add(value,parent[row]));
    }
    return result;
}
} // namespace ss2vr_test
