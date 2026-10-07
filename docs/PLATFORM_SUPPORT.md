# Windows and Linux/Proton targets

Both are required targets. The mod is still a development build: native game,
Windows executable, headset and network runtime testing has not been performed.
Offline checks cannot establish playability, headset coverage or performance.

## Shared architecture

The game proxy and server adapter are x86 Windows PE DLLs. The OpenXR host and
packaged official loader are x64 Windows PE products. The game launches the host
with an explicit executable path and inherited environment. Windows uses its
selected OpenXR runtime; Proton keeps both processes in the game's Wine prefix.
Do not launch the host manually in another prefix. IPC is pointer-free and its
layout is checked across the two architectures.

Classic D3D9 readback transports completed images to a separate D3D11 OpenXR
host. No cross-process D3D9/D3D11 shared GPU handle or D3D9Ex upgrade is assumed.
The host selects the runtime-required adapter LUID and minimum feature level,
requires the D3D11 extension and supported UNORM swapchain formats, and retains
the original request poses/FOV for image submission.

## Windows setup boundary

Use a 64-bit Windows installation with a working headset OpenXR runtime and
D3D11 driver. Keep the packaged x64 loader beside the host and both product
architectures from the same build. The installer verifies the game fingerprints
and refuses collisions with existing proxy/mod files. No automatic runtime
registration, system DLL replacement or account/security-setting change occurs.

Module/system paths are read into bounded growing UTF-16 buffers. Failures and
truncation are rejected rather than consumed as filenames. This fixes startup,
configuration, fingerprint and logging path handling for long installation paths
and Wine drive mappings. The proxy loads the real D3D9 library from the system
location with a restricted dependency search and rejects a self-alias.
See Microsoft's [module-path contract](https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulefilenamew)
and [system-directory contract](https://learn.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getsystemdirectoryw).

## Proton setup boundary

Use a Proton build with wineopenxr and its matching DXVK components. Valve's
D3D11 OpenXR bridge requires DXVK interop and translates the graphics binding to
Vulkan. Forcing WineD3D/OpenGL is not the supported route for this host. Its
format mapping includes the BGRA/RGBA UNORM formats requested here.
[Valve's bridge implementation](https://github.com/ValveSoftware/Proton/blob/proton_11.0/wineopenxr/openxr_loader.c).

The existing Steam launch recipe remains:

```
WINEDLLOVERRIDES="d3d9=n,b" PRESSURE_VESSEL_IMPORT_OPENXR_1_RUNTIMES=1 %command%
```

The native headset runtime must be running and reachable from Steam's container,
and the game's prefix must have its appropriate Windows-side wineopenxr runtime
registration. Do not substitute a native Linux `.so` manifest for a Windows
runtime manifest or blindly change global runtime selection. Container/runtime
setup differs by distribution and headset; inspect the chosen setup before
changing paths or permissions. The host now logs a Wine-specific diagnostic when
the D3D11 extension or session creation fails, without logging paths or tokens.

`VR_OVERRIDE` and xrizer configure OpenVR, while this host uses OpenXR directly.
The launch recipe and upstream implementation establish a supported design
route, not verified operation of this mod on a specific Proton/runtime/headset.

## Offline evidence and remaining runtime work

Portable tests exercise path failure, nontermination, embedded nulls, repeated
truncation, required-size returns, long paths and directory parsing. Cross-builds
and ABI/product checks cover x86 proxy/server and x64 host/loader. They do not
execute a loader, graphics API, native hook or headset frame.

Eventual runtime acceptance still needs the complete installation/startup path,
real stereo/controller input, scope/UI output, loss/reset/restart behavior and
performance on both Windows and the selected Proton/native-runtime combination.
Those tests require a separate change to the current no-runtime-testing scope.

Installer preflight also applies Windows filename rules when run on Linux:
reserved device names, trailing-dot/space aliases, case-colliding payloads,
noncanonical paths, alternate streams and the installer's own receipt path are
rejected before mutation. See [Microsoft's filename rules](https://learn.microsoft.com/en-us/windows/desktop/fileio/naming-a-file).
Copied payloads are rehashed before the receipt is written. A copy-integrity
failure removes only the attempted payload and newly created empty directories,
so a normal retry can proceed; existing game files remain outside ownership.
Synthetic fixtures cover these failures and retry, without using a game install.
This is not a guarantee against a separate process maliciously changing directory
links or files concurrently; stop such activity before installing or removing.

GitHub Actions is disabled at the owner's request; validation stays local.
Configure `SS2VR_COMPONENT=core` in Debug and Release, build, then run CTest.
These portable helper checks use no proprietary game inputs, Windows executable,
Wine, graphics API or headset. Passing them is offline regression evidence only,
not a native mod build or platform runtime certification. Native PE/ABI checks
continue separately with private owned inputs.

Rollback also records an exclusively created destination before reopening its
source. A source-open failure after preflight therefore cannot leave an empty
payload file blocking the next installation. A synthetic failure-and-retry
fixture covers this boundary on the installer itself.
