# Native menu pointing integration — 2026-10-03

This extends the threshold-follow LOCAL menu quad with controller ray pointing and a visible cursor. Right-hand pointing takes priority; left-hand pointing is available when the right ray misses. A selected hand's trigger presses/releases the native mouse command. Stick navigation, Use confirmation and Menu/back remain available. Pointer trigger mode suppresses the parallel keyboard Enter path.

## Verified native boundary

The fingerprinted Sam2Game menu system owns a virtual 640x480 floating-point cursor. It does not derive this cursor from Windows GetCursorPos. Moving the Windows pointer alone would not implement menu pointing.

- Internal Sam2Game0x192550 is a no-argument cdecl menu input poll. Native menu processing0x2300B0 calls it before the subsequent native menu update. It polls native bindings, dispatches mouse motion at0x192679, mouse command31 down at0x1926A2 and up at0x1926F4.
- Motion0x19E210 is cdecl `(float dx,float dy)`. It integrates native sensitivity `[Sam2Game+0x3FE200]`, clamps to640x480, updates the native cursor and calls the current menu's mouse-motion virtual+0x7C. The adapter supplies a calculated delta through this function rather than writing cursor globals.
- Command dispatcher0x19E0E0 is cdecl `(int command,int repeated)`; release0x19E170 is cdecl `(int command)`. Command0x1F is the native left mouse button. Native menu-button dispatch tests this command and the native virtual cursor for hit selection. The original dispatchers own selection and held-state bookkeeping.
- Current-menu pointer is `[Sam2Game+0x40A270]`. Native helper0x2300F0 calls its IsInteractive virtual+0x48. CMSLoading and CPopupMenuPleaseWait vtables independently establish that slot; noninteractive loading screens cannot accept VR pointing.

All entry points remain gated by the existing exact Sam2Game hash and module residency. The added menu hook joins the existing owned-hook activation/rollback transaction. It runs after the native input poll, on that same native thread, preserving ordinary inputs and the menu hierarchy. Headless authority does not initialize this adapter.

## Presentation and input ownership

The host intersects the actual submitted LOCAL quad, rejects backward/parallel/out-of-bounds rays, maps to top-left normalized image coordinates and overlays a small visible ring. It publishes only a pointer for a menu layer actually submitted in that XR frame. The native input adapter checks focus, native game foreground, current menu generation, session/reference, valid selected hand/head, finite coordinates and short freshness before mutation. Tracking/target/focus loss releases owned mouse state and requires a real neutral trigger. Hand changes and reference/session changes also disarm.

Native menu identities never cross IPC. A monotonically increasing capture generation correlates a menu image with its current native menu. The pointer also carries the exact displayed menu frame sequence and its own trigger level. Newer input may invalidate focus/tracking but cannot supply a click to old coordinates. Native cursor mutation requires the displayed frame to match the current published image; a changed image or menu generation disarms the trigger. Cursor interaction is disabled if native polling and picture capture occur on different threads. Generation exhaustion permanently disables pointing instead of reusing identities. ABI6 appends a56-byte pointer record without changing Input, Request or stereo slot layouts; matching game/host products are required.

Menus now positively override stale gameplay classification while native menus are active. The existing comfort anchor retains35° yaw-follow entry and8° residual stop, with bounded movement and distance guards. Size/readability and native behavior remain unverified in a headset. No game, Windows host, Wine or OpenXR runtime was executed.

Terra review identified missing input/frame correlation and pointer fallback eligibility. Main added the explicit sample/frame contract, shared portable eligibility function, matching native interactive/mouse-enabled checks and deterministic checks for newer trigger input, changed visual frame/menu identity, and focus loss. These checks exercise the production policy; they do not execute native dispatchers.
