# Scope scene cut: ordering and native list evidence

Root inspection and helper implementation, 2026-10-05. No auxiliary view, source
texture capture or magnified native draw is enabled by this checkpoint.

## Selected direction supersedes the narrow helper

The later sections of SCOPE_SCENE_SOURCE_AUDIT.md already establish a stronger
native gun-free collection and pre-bloom callback-command route. Flares at90000
are a concrete counterexample to general first-gun capture. The new narrow
first-gun helper/tests were therefore removed before any native integration;
their tested ordering alone did not justify another source path. Keep the
native list evidence below, but implement the existing selected capture-command
rankAFFFF route rather than maintain parallel capture policies.

## Native boundary

Pinned Engine CViewRenCmd::Execute155FF0 reads a command-array owner at view+10.
The array's data pointer is+4 and count is+8. At156033 it sorts pointers with
155940; that comparator compares command+8 masked to24 bits, then command+C.
Execution reads each pointer and invokes virtual+4 at15606E, returning156071.
The loop rereads the array/count after each invocation. A future source reader
must not treat the original pointer/count as an immutable lease by default.

Pinned Sam2Game's ordinary weapon command has table2A4720, weapon pointer+10,
and sort key8FF00. Its Render call returns4BF3A. Stock weapon-target coverage
is documented in SCOPE_STOCK_COMMAND_COVERAGE.md. Merely knowing that key does
not establish that all desired scene work preceded the gun.

Engine constructors install these command tables: Effect219124, Obj219144,
Test21914C and View218F94. These are class-identity observations, not blanket
permission to classify their descendants/callbacks as safe source content.
Unknown classes must not silently be labeled scene work.

## Historical bounded ordering helper (removed)

scopeSceneCut consumes copied, already-proven command classifications from one
current sorted root. It accepts only a scene prefix followed by one or two
unique local guns, entered at the first gun. Unknown classes, missing/aliased
weapon tokens, later scene work, a third gun, or invocation at the second gun
decline. Its bounded count is a safety limit, not a claim of native capacity.

This is deliberately stricter than assuming every command after8FF00 is a gun.
It prevents a source cut that silently omits a later classified world command.
An empty prefix is only an ordering result, not a claim that a framebuffer was
cleared or contains valid pixels. Tests cover both hand orders, every short
four-kind ordering and invalid/bounded inputs.

The helper was never connected. Native classification, list lifetime through
capture, source target/format/viewport/alias checks, color representation and
auxiliary-view ownership still belong to the live source adapter. Do not enable
a first-successful-sniper retry: latch the first actual ordinary gun attempt
before tracking/geometry admission, and keep rejection terminal for that view.
