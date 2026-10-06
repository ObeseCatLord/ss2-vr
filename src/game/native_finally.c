// Compile this small C boundary with a Microsoft x86 target. The surrounding
// C++ remains MinGW/DWARF. No proprietary code or Windows SDK is required here.
#if !defined(_WIN32) || !defined(_M_IX86) || !defined(_MSC_VER)
#error Native finally requires a Microsoft-compatible Windows x86 compiler
#endif
__declspec(dllimport) int __cdecl _abnormal_termination(void);
typedef void (__cdecl *NativeBody)(void *);
typedef void (__cdecl *NativeCleanup)(void *, int);
void __cdecl ss2vrNativeFinally(NativeBody body, NativeCleanup cleanup, void *context) {
    __try { body(context); }
    __finally { cleanup(context, _abnormal_termination()); }
}
