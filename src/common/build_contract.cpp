#include "build_contract.hpp"
#include "network.hpp"
#include "build_contract_generated.hpp"
namespace {
constexpr ss2vr::BuildContract contract() {
    ss2vr::BuildContract result{"SS2VR_BUILD_V1",1,SS2VR_BUILD_COMPONENT,
        ss2vr::Abi,ss2vr::network::WireVersion,
        sizeof(ss2vr::Input),sizeof(ss2vr::Request),sizeof(ss2vr::Ui),
        sizeof(ss2vr::Slot),sizeof(ss2vr::Shared),
        SS2VR_VERSION_MAJOR,SS2VR_VERSION_MINOR,SS2VR_VERSION_PATCH,{}};
    constexpr char fingerprint[]=SS2VR_SOURCE_FINGERPRINT;
    static_assert(sizeof(fingerprint)==65);
    for(unsigned i=0;i<result.sourceFingerprint.size();++i)result.sourceFingerprint[i]=fingerprint[i];
    return result;
}
}
extern "C" __declspec(dllexport) constinit const ss2vr::BuildContract ss2vrBuildContract=contract();
