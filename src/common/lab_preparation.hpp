#pragma once
#include <cstdint>
#include <string_view>
namespace ss2vr {
// Private native grant/equip/save target. This does not enable weapon alignment.
struct LabPreparationTarget {
    int nativeId;
    const char *className,*saveBasename,*resourcePath,*receiptPrefix;
    bool sniperZoom;
};
inline constexpr LabPreparationTarget LabAutoShotgunTarget{
    2,"CAutoShotgunWeaponEntity","autosg-id2.sav","Temp/SS2VR/autosg-id2.sav","Lab autosg",false};
inline constexpr LabPreparationTarget LabSniperTarget{
    13,"CSniperWeaponEntity","sniper-id13.sav","Temp/SS2VR/sniper-id13.sav","Lab sniper",true};
inline const LabPreparationTarget *labPreparationTarget(int id) noexcept {
    return id==2?&LabAutoShotgunTarget:id==13?&LabSniperTarget:nullptr;
}
// Absent opt-ins are empty. A legacy alias cannot coexist with a new opt-in.
inline int labPreparationSelection(std::wstring_view legacy,std::wstring_view idle,int selected) noexcept {
    if(legacy.empty() && idle.empty())return 0;
    if(!legacy.empty())return legacy==L"1" && idle.empty() && selected==13?13:-1;
    return ((idle==L"2" && selected==2)||(idle==L"13" && selected==13))?selected:-1;
}
enum class LabPreparationStage { Waiting,Granted,Selected,SaveIssued,Complete,Failed };
struct LabPreparationProgress {
    LabPreparationStage stage=LabPreparationStage::Waiting;
    bool busy=false;
    uint64_t issuedTick=0;
    void rejectInterval() noexcept {stage=LabPreparationStage::Failed;}
    bool begin() noexcept {
        if(busy || stage==LabPreparationStage::Failed || stage==LabPreparationStage::Complete)return false;
        busy=true;return true;
    }
    void finish(bool aborted,bool nativeHealthy) noexcept {
        // A nested callback can contain failure and return normally. That
        // interval's failure must survive into every later preparation attempt.
        if(aborted || !nativeHealthy)rejectInterval();
        busy=false;
    }
};
}
