# Cloud workspace source transfer — 2026-10-05

This public repository is a source transfer for continued Serious Sam 2 OpenXR
VR development. The user subsequently requested public publication and continued
work in the cloud workspace. That instruction supersedes the historical pause
for the cloud development owner; the local transfer performed no development,
build, runtime test, packaging of mod products, or deployment.

Start with the complete original `docs/HANDOFF.md` and `AGENTS.md`. Both are
byte-for-byte copies from the original accepted repository HEAD. Treat historical
status and source-review scope carefully. The goal remains incomplete and no
game/headset/network runtime verification was performed. Actual game/headset
testing remains excluded by the user. Astra investigation/review constraints
and native-engine preservation rules remain in those documents.

## Provenance and preserved work

The initial public commit imports accepted source from original branch `master`,
HEAD `ccf61c94f3cbba1027ab1fb53269efc3a16422cf` (paused handoff documentation),
with accepted-source checkpoint `828b413` and latest accepted observer-pulse
sender implementation `6acfdc7`. Original Git history and original author metadata
were not imported. Original commit references in research are provenance labels
and will not resolve in this snapshot repository. No source license was changed.

`handoff/unaccepted-melee.patch` separately preserves three modified and four
previously untracked files. It is unaccepted WIP, not merged into source. Read
`handoff/WIP_STATUS.md`. `handoff/CONTENT_MANIFEST.json` records accepted files,
transfer additions, and the expected hashes of those seven WIP files after patch
application. The manifest excludes itself to avoid a recursive hash.

## Dependencies and portable work

Use CMake >=3.24, Git, Python 3, a native C++20 compiler, Clang targeting
`i686-pc-windows-msvc`, and i686/x86_64 MinGW-w64 compilers and binutils.
CMake fetches MinHook `c3fcafdc10146beb5919319d0683e44e3c30d537` and OpenXR SDK
`75c53b6e853dc12c7b3c771edc9c9c841b15faaa`. `tools/requirements.txt` pins
`pefile==2024.8.26`. Capstone is needed by some optional native-inspection tools;
optional experimental shader tooling uses a native Linux vkd3d 1.17 compiler.
Dependency caches and existing local builds were not transferred.

To configure portable checks after resuming development:

```sh
cmake -S . -B build-core -DSS2VR_COMPONENT=core -DCMAKE_BUILD_TYPE=Debug
cmake --build build-core -j4
ctest --test-dir build-core --output-on-failure
```

The native core CMake tests use ordinary Linux tools and synthetic fixtures.
`python3 tools/build.py --jobs 4` also cross-builds game/host products and fetches
pinned dependencies when no local dependency path is supplied. These commands
were not run by the transfer agent. Network access and suitable toolchains must
be provided by the consuming cloud environment.

## Exclusions and missing evidence

No installed game binaries, DLLs, executables, meshes, textures, media, `.gro`
archives, saves, or private runtime configs are included. There are no `deps/`,
`build*/`, or `dist/` trees, old mod package archives, credentials, analysis scratch
dumps, or raw session telemetry. Project `config/` files are mod-owned templates.
Original MIT and third-party notices are retained. Native address/call-flow
research, compatibility fingerprints and derived measurements are retained as
project evidence; they are not runtime verification or proprietary file payloads.

Native verification tools using `--game` require separately available owned game
binaries/assets; those checks cannot be rerun from this public repository alone.
Historical `/tmp` scratch references are not transferable evidence. The original
README mentions immutable distribution archives, which remain local and are not
published here. This repository is a development source transfer, not a playable
release. Local PC paths do not identify cloud workspace paths. If a separate
Library transfer is later used, retain its identity/version and materialize and
verify bytes in the consuming executor's own workspace.
