# Scope GPU input geometry — Astra review

This source follow-up extends the existing exact physical Scope invocation and
per-eye observation bank. It adds no magnified image, shader substitution,
simulation/inventory/RPC change, resource cache or new native hook. The immutable
0.2.11 archive predates this work. Full immersion remains unfinished.

Explicit/effective Astra/xhigh design review ran two turns. Initial NO-GO caught
a contradiction between forwarding every native draw and drawing safely after
an indeterminate buffer Unlock. The follow-up retained the minimal actual-DIP
adapter, narrowed concurrency to the existing single-thread native draw contract
and required fatal containment before live acquisition. COM ownership pins object
lifetime; no protection against arbitrary concurrent writers is claimed.

The reader uses the existing exact GfxD3D returnA011, current physical native
model/surface/palette affine, actual COM bindings and exact static managed buffer
metadata. Four bounded READONLY copies are sequential, with fixed owned storage.
All hashes run after unlock/release; all four stock slice hashes must match
before copying actual cap geometry. Commit follows a successful original draw
and fresh invocation/affine identity. `contentVerified` remains false: loaded
geometry is distinct from complete skeleton/material/render-purpose admission.

Foreign acquisition obligations and COM outputs live in explicit TLS ownership
above NativeFinally. Successful Lock with null output still requires Unlock;
pending Lock or attempted Unlock followed by abnormal exit remains uncertain.
Failed/indeterminate Unlock is a fatal rendering condition, not ordinary decline.
The current and later intercepted draw families refuse forwarding, the entire
pair faults, and one uncertain resource reference remains retained until process
termination. Managed resources survive Reset, so Reset/deviceLost/resource epoch
changes cannot clear this latch. Ordinary declines preserve native forwarding.

| Astra recommendation | Main disposition |
|---|---|
| Prefer existing DIP COM input ownership over CPU-table reader | Adopted; no native CPU lock/table reader or ownership registry added. |
| Treat uncertain Unlock separately from diagnostic failure | Adopted: process-lifetime draw stop, retained uncertain reference, no retries or automatic recovery. |
| Reuse native main-thread/device contract | Adopted; PUREDEVICE/MULTITHREADED declined, no native callbacks while locked, fresh COM/context checks. |
| Require complete draw interception before acquisition | Fixed using existing `uiRoutingReady`, current device/creation-thread accessor before probe and in revalidation; no new interception layer. |
| Retire geometry on a later recognized rejected Scope pass | Fixed with Unrelated/Rejected/Observed classification and existing invocation rejection latch; unrelated surfaces preserve evidence. |
| Geometry cannot revive after rejection in the same invocation | Production accept/reject helper and focused regression; next invocation/eye uses original lifecycle reset. |
| Keep metadata reports bounded by asset fingerprint | Adopted; graph25scalars and argument-object/class/version assertions, false live/substitution flags. |
| Commit only after original draw succeeds | Adopted; failed forwarding or changed resource epoch retires tentative geometry. |

The first source pass found the hook-coverage and same-invocation rejection bugs
above. Two follow-up Astra/xhigh turns closed both findings and accepted the
query-ordering/retired comparison-key refinement: **bounded SOURCE GO**. Final
cross-build and all17 portable groups pass; the private owned-cap check, actual
DIP caller/argument/stack shape, artifact/IPC and compiled-finally checks pass.
Exact source/product hashes and verified review settings are recorded in
scope-gpu-source-checks.json. No Windows code, game, XR runtime,
headset or network session is executed. The source/binary checks establish the
bounded adapter, not optical rendering, driver behavior or tested playability.
