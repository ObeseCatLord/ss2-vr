// Compile-only ABI fixture using the exact shared entry definition. It is never
// executed and is not linked into the mod. Real native policy remains a gate.
#include "game/x86_predicate_entry.hpp"
extern "C" __attribute__((noinline)) int predicateAbiHelper(int value, void *, unsigned) noexcept {
    return value;
}
SS2VR_X86_PREDICATE_ENTRY(predicateEax, predicateAbiHelper, 28, 4, 1)
SS2VR_X86_PREDICATE_ENTRY(predicateEcx, predicateAbiHelper, 24, 4, 2)
SS2VR_X86_PREDICATE_ENTRY(predicateOwner, predicateAbiHelper, 28, 0, 3)
SS2VR_X86_PREDICATE_ENTRY(predicateEdi, predicateAbiHelper, 0, 4, 4)
