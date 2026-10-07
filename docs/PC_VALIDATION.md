# PC validation — 2026-10-07

## Source and isolation

PC development begins from public commit
`8d172189aa0fbddadc1c1786eedbc38db8368487`, tree
`07b2508387d1a367107673020e6162eba9e17fd4`. Both queued source checkpoints are
present. The complete28-commit history passes the empty-email gate. Actions is
disabled; workflow files are absent. No old ancestry was imported.

The original installed-game checkout, seven unaccepted melee files and user data
are untouched. A separate complete game copy and new Proton prefix are used for
runtime checks. The original config was snapshotted with `um backup` under the
local name `ss2vr-pc-original-config-20261007`. Private paths, logs, crash dumps,
screenshots and game assets remain outside public source. Installer preflight,
installation and hash-checked removal operate only on the isolated lab copy.

## Local build and offline results

GNU MinGW16.2.0 builds the x86 proxy/server and x64 host/official loader. Clang
22.1.8 builds the existing native-finally C boundary. All59 portable groups pass
locally in Debug and Release with pinned Python dependencies. An initial failure
was missing Capstone in the test environment; pinned dependencies and an absolute
PYTHONPATH fix the environment without changing source or weakening the check.

Actual product/layout verification passes for IPC9/wire6. Initial local build
input fingerprint: `ac725760615945bf6b81947bb6d0f68dda69e3f3f40092a5ffac5fddb51fbb07`.
This identifies source/build inputs, not runtime success. Product SHA256 values:

| Product | SHA256 |
|---|---|
| x86 d3d9.dll | `cc9f75315df70f780a94e8d7dbcb481086c819c357fd1213ce6d63a786b6c74b` |
| x86 SS2VRServer.dll | `9bb0b93c1e548b6a3b28eff0c15d26d2f112cc5f364ab1f6cb95a84ae9de2a23` |
| x64 ss2vr_host.exe | `e0a1cd299815c4e164ec443d54515c11f46667dffe616fca8d1eddb1fa3802ed` |
| x64 official loader | `bb011caa82528c541a73967ce6408f82198ff4fd0358b38b54884719d863bd1d` |

A matching development package was staged privately and passed collision/hash
preflight before installation into the lab. It is incomplete and not a playable
release. Later source edits require rebuilding all products before repackaging.

## First actual Proton startup observation

Proton Hotfix `hotfix-20260828-ptr-x86_64` (prefix11.0-100), a new prefix and an
isolated1280x720 Xvfb display were used with the native D3D9 proxy override.
The native game reached Direct3D context creation, then displayed its crash
dialog and wrote a minidump. The captured image was inspected. No VR host log
was produced. This is a startup **failure**, not a headset or rendering pass.

The exact lab game PID was stopped; the collision-refusing installer verified
and removed only unchanged mod-owned payload. An unmodified control launch in
the same lab/prefix is being compared. Astra is examining the preserved first-run
logs and native boundary. No cause is claimed before that comparison/evidence.

No active Monado/SteamVR/WiVRn service or USB headset was observed at initial
inventory. A Vulkan-capable RTX4090 is available. Hardware/runtime availability
is being clarified; no HMD/controller frame, network session, vehicle drive or
scope/UI visual acceptance has occurred. A virtual display launch cannot prove
headset comfort, performance or tracking.

## Remaining goal

Multiplayer body/origin settlement, physical melee consumption/lifetime and
observer delivery, wider vehicles including actual wheel grabbing, remote-head
worker/model lifetime/enablement and final full-project review/package readiness
remain unfinished. Inactive helpers are not completed features. The rejected
melee patch remains unapplied and native160/348 remain manual-only.
