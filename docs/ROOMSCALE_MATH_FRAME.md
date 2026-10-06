# Bounded roomscale arithmetic frame

The inactive additional-query adapter requires nearest rounding, at least
binary64 x87 precision, masked exceptions and gradual underflow. Native game
control state is not assumed to satisfy these requirements. The ordinary
collision path and checked native body commit must retain their caller's mode.

`runRoomscaleMathFrame` saves an aligned 512-byte FX image and installs x87
control `0x027f` and MXCSR `0x1f80` only inside the explicitly supplied extra
math/query body. It checks both control words before and after the body. The
x86 product need not define `__SSE__`, so the MXCSR check is explicit. Ordinary
exception flags are allowed; altered rounding, precision or masking rejects the
scope. The helpers carry a narrow SSE2 target attribute, rather than changing
the game's compiler-wide FP target. The owner checks OS-reported SSE2 support
before using them.

The saved image and flags live above the existing native-finally frame. Cleanup
clears native TLS and restores the image on normal return, contained GNU error,
or native unwinding that crosses the frame. Native exceptions are not swallowed
or converted. An exception caught entirely inside native code does not cross
this boundary; this adapter cannot certify such a query's completeness. Nested
entry marks both the existing and attempted scope failed. Wrong-thread/null
entry is refused before changing FP state.

This frame is not connected to gameplay. Its future caller must already own the
query phase, exclude gameplay and body-commit callbacks from its body, and bind
query/resource failure to movement refusal. Saving FP state is not a worker or
lifetime lock, resource cleanup, collision proof, or native unwind runtime test.

## Offline evidence

All 44 portable groups pass in Debug and Release. The production helper fixture
passes ASan/UBSan with leak checking disabled in the workspace. It exercises
hostile precision/rounding/FTZ/DAZ, installed interval arithmetic, an x87 stack
sentinel and restoration after ordinary C++ exceptions. It does not emulate
Windows SEH. Both x86 products compile/link. The compiled ABI verifier checks
aligned outer image storage, native TLS, SSE2/owner gates, save/install order,
both mode checks, leaf FX helpers and cleanup restoration. It passes normally
and with Python optimization; replacing FXRSTOR with FXSAVE is rejected.

No Windows, Wine, game, headset or OpenXR process was run. Roomscale body movement
remains inactive until geometry, ownership and origin/replication settlement are
connected.
