#include "common/protocol.hpp"
using namespace ss2vr;
#ifdef _MSC_VER
#pragma section(".ss2abi", read)
__declspec(allocate(".ss2abi"))
#else
__attribute__((section(".ss2abi"),used))
#endif
extern const uint32_t ss2vr_layout[]={Magic,Abi,sizeof(Input),sizeof(Request),sizeof(Ui),sizeof(Slot),sizeof(Shared),offsetof(Shared,latest),offsetof(Shared,slot),offsetof(Shared,menu)};
