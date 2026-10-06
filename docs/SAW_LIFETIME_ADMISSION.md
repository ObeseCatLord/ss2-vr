# Saw lifetime admission: current static evidence

Root inspection, 2026-10-05. Private pinned Sam2Game.dll only; no native code
executed. This records a distinction required before connecting gesture input.

The ordinary circular-saw constructor at RVA164680 calls the base default
constructor498A0 at1646A1. The base writes primary release field38=1 and
stateB0=1; the derived constructor clears sound stateE8 at16471D. This proves
default-construction initialization, not the state of a weapon first observed
in a selected-hand slot.

The saw copy constructor165DA0 instead calls base copy43FF0 at165DAB. The base
copies owner28, primary release38 and stateB0 from its source. The derived
constructor copies sound stateE8 at165DFE/165E04. Base assignment43E40 also
copies38 andB0. A new pointer, handle, selection or tracking generation therefore
cannot establish that native primary was consumed low. Likewise stateB0 orE8
alone cannot substitute for a completed canonical release.

## Consequence for connection

Keep newly admitted and replaced bindings unknown. A possible narrow initial
policy is to wait for an actual original canonical primary release completion,
then require fresh quiet gesture arming. This avoids a constructor hook solely
to guess initial state, but is not yet implemented or accepted as the finished
interaction: it would require the user to use and release the trigger first.
It does not eliminate release-boundary, binding-lifetime or observer proofs.

Current Authority rows are copied out and replaced wholesale by saveAuthority.
The noncopyable consumption record must not be inserted into those sample rows
or serialized with input. Stable ownership must be retained separately from
sample replacement within the existing owner, with explicit retirement on
weapon, player, incarnation, carry and world changes. No new registry is added.

The canonical target classifier and held-only overlay remain inactive. Their
portable tests establish manual-history isolation and target-set classification;
they do not establish native dispatch or lifetime. The existing engine callers
leave the gesture overlay empty.
