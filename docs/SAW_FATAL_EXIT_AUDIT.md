# Saw release: bounded fatal-exit follow-up

2026-10-05, root cloud inspection of user-supplied owned files. This is a new
static finding, not an Astra acceptance or completed native integration review.
The user asked root to do the development itself; the local task only transfers
files. Original normal-allocation-success findings remain distinct.

## Reproducible boundary facts

Core.dll SHA256 is
`7a1bd56b9bfa3edfbb23f4d3c96490e40a7c0b031b85e797e1af91b2ba3cf207`.
The allocator's default callback at RVA20920 calls fatal reporting at RVA44f0.
The old handoff's label `F44F0` must not be interpreted as RVAf44f0.

- Fatal reporting invokes its optional configured callback at RVA451c, before
  calling conExit(1) at RVA457a.
- conExit at RVA41e0 stores the exit code, invokes the optional pre-termination
  callback at RVA41fd, then walks registered callbacks in descending index order
  at RVA421a with their registered arguments.
- It invokes the optional post-termination callback at RVA4234 before sysExit at
  RVA423a. sysExit RVA6afb0 calls the imported MSVCR71.dll `exit` function at
  RVA6afb7. This is not a verified direct ExitProcess call or callback-free path.
- The exported registration/setter boundaries are RVA4930/41c0 for list add/remove,
  RVA3370/3390 for pre/post termination, and RVA33b0 for fatal error handling.
  Their presence is not evidence that a handler is installed in a live session.

Run the new read-only verifier with pefile and capstone installed:

    python3 tools/verify_saw_exit_boundary.py --game /path/to/owned/game

The verifier emits derived metadata only. It checks the fingerprint before
interpreting offsets, verifies exact native call sites and the imported exit
identity, and explicitly does not certify callback-target closure. Normal and
Python-optimized invocations agree. A deliberately incorrect fingerprint is
refused. No disassembly payload or proprietary binary is committed.

## Bounded registration search and disposition

The inspected Sam2.exe, Engine.dll and Sam2Game.dll import tables do not import
these registration functions. A Core.dll executable-section scan found no direct
call candidates to the five setters/add/remove functions. This does NOT rule out
function pointers, dynamic lookup, other loaded DLLs, extensions or live state.
It is not proof that the callback lists and slots stay empty.

The supplied executable matches the research fingerprint
`727901f161133ff653fcdc196858335b991b743e67deb448c03c808e5b33e28b`.
Engine.dll and Sam2Game.dll also match the recorded research fingerprints.

**Disposition: native reconciliation remains gated.** Do not treat the eventual
CRT exit as proof that no callback can reenter before the saw base-stop receipt.
Do not add a global shutdown hook, suppress native callbacks, or invent lifecycle
state merely to hide this uncertainty. The next inspection is limited to actual
registration sites in the remaining installed modules. If the required closure
expands into arbitrary application shutdown, reopen the design rather than
silently escalating to a whole-program audit. Copied-active bindings, retirement,
manual carry isolation and observer consumption remain separate unresolved gates.

## Remaining-module import survey

The user subsequently supplied the remaining 41 installed DLLs privately. All
41 sizes and hashes match their transfer manifest. None of their import tables
names the five callback registration/setter APIs. Together with the first three
DLLs and Sam2.exe, that is 45 inspected native files. This rules out a direct
named import in this supplied set, not a dynamic lookup, reflected invocation,
CRT destructor callback, arbitrary extension, or live callback registration.
The native gate is still unproved; no further files are requested on that basis.
