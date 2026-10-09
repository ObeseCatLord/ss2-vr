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
    LabPreparationProgress waitingPresentation;
    waitingPresentation.stage=LabPreparationStage::Selected;
    waitingPresentation.issuedTick=123; // Selection already issued exactly once.
    for(unsigned tick=0;tick<3;++tick) {
        assert(waitingPresentation.begin());
        // Initial presentation-busy deferral never advances or reissues action.
        waitingPresentation.finish(false,true);
        assert(!waitingPresentation.busy && waitingPresentation.stage==LabPreparationStage::Selected &&
               waitingPresentation.issuedTick==123);
    }
    assert(waitingPresentation.begin());
    waitingPresentation.finish(false,false); // A contained native failure is terminal even while waiting.
    assert(!waitingPresentation.begin());
    LabPreparationProgress beforeEntry;
    beforeEntry.stage=LabPreparationStage::Granted;
    beforeEntry.rejectInterval(); // Original simulation failed before adapter entry.
    assert(!beforeEntry.begin());
    LabPreparationProgress nested;
    assert(nested.begin());nested.stage=LabPreparationStage::Granted;
    nested.rejectInterval(); // Preserve busy until the issuing callback unwinds.
    assert(nested.busy && !nested.begin());
    nested.finish(false,true);
    assert(!nested.busy && !nested.begin());
}
