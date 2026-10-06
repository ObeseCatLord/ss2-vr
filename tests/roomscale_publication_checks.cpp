#include "common/roomscale_publication.hpp"
#include "common/rig_revision.hpp"
#include <cassert>
using namespace ss2vr;
using namespace ss2vr::roomscale;
int main() {
    using A=PublicationAction;
    assert(publicationAction(false,true,false,true,true)==A::ignore);
    assert(publicationAction(false,false,true,false,false)==A::ignore);
    assert(publicationAction(true,false,false,false,false)==A::retireWithoutOrigin);
    assert(publicationAction(true,true,true,false,false)==A::retireWithoutOrigin);
    assert(publicationAction(true,true,true,true,true)==A::retireWithoutOrigin);
    assert(publicationAction(true,true,false,true,true)==A::publishOrigin);
    assert(publicationAction(true,true,false,false,true)==A::quarantine);
    assert(publicationAction(true,true,false,true,false)==A::quarantine);
    assert(publicationAction(true,true,false,false,false)==A::quarantine);
    RigRevision rig;
    const auto ticket=rig.begin(0);assert(ticket);
    assert(publicationAction(rig.current()==ticket,true,false,false,true)==A::quarantine);
    assert(!rig.usable(0)&&!rig.usable(ticket)); // No false success from an ambiguous mutation.
    const auto recovered=rig.recoverForNewOrigin();
    assert(publicationAction(rig.current()==ticket,true,false,true,true)==A::ignore);
    assert(!rig.finish(ticket)&&rig.usable(recovered));
}
