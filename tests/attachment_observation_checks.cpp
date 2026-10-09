#include "common/attachment_observation.hpp"
#include <cassert>
#include <limits>
using namespace ss2vr;
int main() {
    Matrix34 m=matrix(Pose{Quat{0,0,0,1},Vec3{1,2,3}});
    AttachmentObservation a;a.begin(7,0xabcdef);a.complete(1,m);
    assert(a.admitted(7) && a.ident==0xabcdef);
    assert(!a.admitted(0) && !a.admitted(8));
    for(unsigned i=0;i<12;++i)assert(a.absolute.m[i]==m.m[i]);
    a.begin(7,1);a.complete(1,m);assert(!a.admitted(7)); // Duplicate never accepted.
    for(int result : {0,-1,2}) {
        AttachmentObservation failed;failed.begin(7,1);failed.complete(result,m);
        assert(!failed.copied && !failed.admitted(7));
    }
    for(bool aborted : {false,true}) {
        AttachmentObservation failed;failed.begin(7,1);
        failed.aborted=aborted;failed.nested=!aborted;failed.complete(1,m);
        assert(!failed.copied && !failed.admitted(7));
    }
    AttachmentObservation bad;bad.begin(7,1);m.m[5]=std::numeric_limits<float>::quiet_NaN();
    bad.complete(1,m);assert(!bad.admitted(7));
    AttachmentObservation empty;assert(!empty.admitted(7));
}
