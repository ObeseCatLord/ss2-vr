#pragma once
#include <type_traits>

// Instruction-level native gates need a stronger boundary than a C++ function
// detour: the native frame still owns all registers, flags and FP state. These
// definitions emit only an entry and trampoline cell; they install no hooks.
#if !defined(__GNUC__) || defined(__clang__) || !defined(__i386__) || !defined(__MINGW32__) || !defined(_WIN32)
#error "Native predicate entries require the verified MinGW x86 toolchain"
#endif

#define SS2VR_ENTRY_STRING_(value) #value
#define SS2VR_ENTRY_STRING(value) SS2VR_ENTRY_STRING_(value)
namespace ss2vr::game {
using NativePredicateCallback = int (__attribute__((cdecl)) *)(int, void *, unsigned) noexcept;
}

// pushal saves EDI at0, ESI at4, ECX at24 and EAX at28 relative to its final ESP.
// helper must be a C-linkage, cdecl, noexcept function returning an int:
//     helper(originalTestedValue, nativeSubject, kind)
// It must contain exceptions; unwinding through a naked frame is unsupported.
// The one selected saved register receives its result. All other state is
// restored before the original relocated native instruction executes.
#define SS2VR_X86_PREDICATE_ENTRY(name, helper, resultSlot, subjectSlot, kind) \
    static_assert(std::is_same_v<decltype(&helper), ss2vr::game::NativePredicateCallback>); \
    static_assert((resultSlot == 0 || resultSlot == 24 || resultSlot == 28) && (subjectSlot == 0 || subjectSlot == 4)); \
    extern "C" { __attribute__((used)) void *name##_original = nullptr; } \
    extern "C" __attribute__((naked, used)) void name() { \
        __asm__( \
            "pushfl\n\t" \
            "pushal\n\t" \
            "cld\n\t" \
            "movl %esp, %ebp\n\t" \
            "andl $-16, %esp\n\t" \
            "subl $512, %esp\n\t" \
            "fxsave (%esp)\n\t" \
            "fninit\n\t" \
            "fldcw (%esp)\n\t" \
            "subl $4, %esp\n\t" \
            "pushl $" SS2VR_ENTRY_STRING(kind) "\n\t" \
            "pushl " SS2VR_ENTRY_STRING(subjectSlot) "(%ebp)\n\t" \
            "pushl " SS2VR_ENTRY_STRING(resultSlot) "(%ebp)\n\t" \
            "call _" SS2VR_ENTRY_STRING(helper) "\n\t" \
            "addl $16, %esp\n\t" \
            "movl %eax, " SS2VR_ENTRY_STRING(resultSlot) "(%ebp)\n\t" \
            "fxrstor (%esp)\n\t" \
            "movl %ebp, %esp\n\t" \
            "popal\n\t" \
            "popfl\n\t" \
            "jmp *_" SS2VR_ENTRY_STRING(name) "_original\n\t"); \
    }
