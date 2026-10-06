# Astra shared native entry source review

Effective Astra/xhigh verified. GO for the inspected compile-only MinGW x86 entries, not native zoom enablement. Saved flags/registers, selected EAX/ECX, original EAX at ECX site, aligned FX storage/helper call, original ESP and COFF trampoline relocation were approved. Boundary audit eight records matched; Activate preserves its13-byte stolen-window distinction. Preserve one shared macro.

| Finding | Disposition |
|---|---|
| P2 toolchain guard admits non-MinGW/Clang although underscore COFF symbols assumed | Fixed: require GCC, MinGW, Windows, x86; explicitly exclude Clang. |
| P3 brief claimed tool/header hashes but verifier/fixture source hashes absent | Fixed: artifact now records verifier/fixture source hashes as well as header/object hashes. |
| EDI owner subject not emitted by two ESI fixtures | Added third compile-only EDI-subject instance of exact same macro and verifier branch. |
| Helper floating-point environment needs precise characterization | FXSAVE/FNINIT/FLDCW gives empty x87 stack with native control settings and MXCSR; legacy FX only, no AVX upper-state guarantee. noexcept is a type contract; future helper must contain C++ exceptions/native ownership and never unwind naked frame. |

No fixture is linked into the mod or executed. Actual native helpers, production entry emission, MinHook relocation, outside/indirect incoming branches and lifecycle remain distinct integration gates. No native predicate hooks installed. Bounded Astra follow-up verified all corrections and all three emitted entries: final GO within this compile-only scope.
