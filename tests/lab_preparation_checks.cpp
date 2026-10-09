#include "common/lab_preparation.hpp"
#include <cassert>
#include <initializer_list>
#ifdef NDEBUG
#error Preparation checks require assertions
#endif
using namespace ss2vr;
int main() {
    for(bool nativeAbort : {false,true}) {
        LabPreparationProgress preparation;
        unsigned grant=0,select=0;
        bool intervalHealthy=true;
        assert(preparation.begin());
        preparation.stage=LabPreparationStage::Granted; // Before native entry.
        ++grant;
        assert(!preparation.begin()); // Reentered preparation cannot issue again.
        if(!nativeAbort)intervalHealthy=false; // Contained callback returns normally.
        preparation.finish(nativeAbort,intervalHealthy);
        assert(!preparation.busy && preparation.stage==LabPreparationStage::Failed);
        intervalHealthy=true; // The next native interval has a new failure flag.
        if(preparation.begin())++select;
        assert(grant==1 && select==0);
    }
    LabPreparationProgress normal;
    assert(normal.begin());normal.stage=LabPreparationStage::Granted;
    normal.finish(false,true);
    assert(normal.begin());normal.stage=LabPreparationStage::Complete;
    normal.finish(false,true);
    assert(!normal.begin());
}
