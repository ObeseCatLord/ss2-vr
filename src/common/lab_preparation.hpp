#pragma once
#include <cstdint>
namespace ss2vr {
enum class LabPreparationStage { Waiting,Granted,Selected,SaveIssued,Complete,Failed };
struct LabPreparationProgress {
    LabPreparationStage stage=LabPreparationStage::Waiting;
    bool busy=false;
    uint64_t issuedTick=0;
    bool begin() noexcept {
        if(busy || stage==LabPreparationStage::Failed || stage==LabPreparationStage::Complete)return false;
        busy=true;return true;
    }
    void finish(bool aborted,bool nativeHealthy) noexcept {
        // A nested callback can contain failure and return normally. That
        // interval's failure must survive into every later preparation attempt.
        if(aborted || !nativeHealthy)stage=LabPreparationStage::Failed;
        busy=false;
    }
};
}
