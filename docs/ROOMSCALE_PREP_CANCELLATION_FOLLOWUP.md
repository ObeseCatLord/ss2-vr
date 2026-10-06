# Model-preparation cancellation follow-up

Root static inspection, 2026-10-05. Pinned Engine.dll only; no runtime or hooks.
This follows the exact E1810 skeleton branch named in the handoff rather than
reopening a global loader-task audit.

## Additional coverage requirement

E1720 has six replacement virtual calls: E17AB (configuration), E1810
(skeleton), E1A51 (mesh), E1D68 (selected associated resource), E1F60 and E2003
(child configuration paths). E1D68 was absent from the earlier abbreviated
site list. It follows a resource+4 bit0 test at E1D5E, then stores the returned
replacement and performs AddRef/RemRef. Its resource type/descendants have not
been closed. Ignoring it would not establish complete pump exclusion.

Two recursive E1720 calls occur at E1F96 and E2036. The parent ignores their
return values and continues its loops. Returning minus one from a cancelled
child therefore does not propagate cancellation on its own.

## Cleanup path distinction

E1720 has no local SEH frame in its inspected body. Its early minus-one epilogue
is E18A1..E18AA. That does not prove that every proposed cancellation site has
the same stack/x87 state or that a partially populated scratch row can be used.

Its outer preparation caller E23F0 calls E1720 at E246D. At E2472 it inspects
global counts, not the returned index. Jumping to E2639 is insufficient:
E2639 calls DBC90, the native model-record pass, before profiler/SEH cleanup.
A cancelled partial model must not enter that pass. The actual cleanup-only
suffix begins E263E, restores the profiler state and SEH frame, and returns at
E2663. Reaching it from E2472 would also need the original five arguments
removed from the stack; it cannot be a blind branch replacement.

The caller DA280 then calls DDE30 and DA170 before DAD90. Both former calls
must be skipped for a cancelled partial preparation. DAD90 is a scalar reset of
the model/mesh/draw/bone/map/palette counts and evaluation pointer, with no
native callbacks in its inspected body. Its caller's DA4F9..DA522 suffix restores
the three saved model-mode globals and SEH frame. The three preparation arguments
are still on the stack at DA4E1 and are normally cleared together with the later
DA170 argument at DA4EF. A cancellation adapter must account for that difference.

## Current decision

Do not implement the previous two-site shortcut or treat a local miss as a
successful whole query. A purpose-bound cancellation design now requires at
least six preparation replacement boundaries, both recursive returns, the
outer preparation return and the model-query cleanup transition, in addition
to world/configuration boundaries. That is materially larger than the proposed
two-site slice. No source hook is enabled by this inspection.

Before implementation, compare that bounded multi-site adapter with an existing
native query-owned extent. Invocation provenance, every cancellation site's
stack/x87 state, full cleanup and rejection of earlier partial hits remain
required. Body volume, primitive overlap, movement acceptance and multiplayer
origin settlement are separate unresolved requirements.
