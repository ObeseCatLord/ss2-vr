# Matching source and compiled products — 2026-10-07

The packager previously hardcoded IPC8 while current source/products use IPC9.
It also checked PE architecture without proving the three products came from the
same build inputs. The current source corrects both problems. No historical
archive is modified, and this is not a finished-device-test release.

Each game, server and host product exports one read-only 128-byte
ss2vrBuildContract value. It records the component, IPC/wire versions, five shared
structure sizes, project version and SHA256 build-input fingerprint. It does not
change the shared-memory or multiplayer wire layout or execute at runtime.

CMake hashes sorted source-relative paths and content hashes for CMakeLists.txt
and files under src/ and cmake/. Input changes/additions/removals trigger
reconfiguration. The fingerprint excludes checkout location, timestamps, Git
metadata, author identity, docs and tests. Dependency revisions/configuration
appear in the hashed CMake inputs; this is not an authenticity signature or proof
that an externally supplied dependency checkout is trustworthy.

The Python verifier reads the exported value from bounded read-only PE data. It
checks architecture/component identity, schema, versions, layout agreement and
agreement with current build inputs. Packaging refuses old products without a
contract, mismatched architectures/layouts and stale or mixed source fingerprints
before staging. It validates the staged source/products again before writing the
manifest, catching a changed source snapshot during copying. Runtime config and
payload integrity remain covered by the existing file hashes.

The manifest's ABI and wire values come from verified compiled metadata. New
development package names include the project version and fingerprint prefix;
existing directories/archives are never overwritten. An alternate --output path
must also be fresh. The scope remains incomplete-development-checkpoint and
runtime_verified remains false. Installers still refuse collisions with existing
game/mod files. No installation, game, host, Wine or headset run is performed by
these checks.

Synthetic fixtures cover valid x86/x64 exports, stale ABI/wire/version/source,
mixed layouts, malformed metadata, wrong component/machine, missing exports,
writable sections and source additions/changes. Full source checks also compare
actual game/server/host contracts, and all56 portable Debug/Release groups pass.
A local staging smoke test exercises the new manifest without deployment.
