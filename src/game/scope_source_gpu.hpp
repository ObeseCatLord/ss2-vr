#pragma once
#include <d3d9.h>

namespace ss2vr::game {
using ScopeSourceCopy = HRESULT (*)(IDirect3DDevice9 *, IDirect3DSurface9 *, IDirect3DSurface9 *) noexcept;
using ScopeSourceCurrent = bool (*)(void *) noexcept;
// Copy into a distinct, caller-owned single-level render-target texture. The
// caller must already admit the scene cut, native view purpose and source color
// representation, and retain device/source/destination above NativeFinally.
// copy must use the bridge's original full-surface, unscaled, unfiltered copy;
// current must check the same frame/view/resource owner without native calls,
// backed by a sticky interference guard: final pointer equality alone does not
// prove that no intervening draw, clear, resolve or source overwrite occurred.
// This function changes no bindings or render state. False makes the candidate
// image unusable, including partial copies. It grants no image-draw permission.
// Device health is checked at capture; the frame owner must still reject later
// device loss/reset before sampling or publishing the image.
// Called by the admitted native pre-bloom command in the frozen source view.
bool captureScopeSource(IDirect3DDevice9 *device, IDirect3DSurface9 *expectedSource,
                        IDirect3DTexture9 *destination, UINT width, UINT height,
                        ScopeSourceCopy copy, ScopeSourceCurrent current, void *context) noexcept;
} // namespace ss2vr::game
