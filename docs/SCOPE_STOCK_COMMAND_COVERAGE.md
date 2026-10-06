# Stock first-gun command coverage

Root static inspection, 2026-10-05; pinned Sam2Game.dll, no native execution.
Reproduce with tools/verify_weapon_render_coverage.py using privately supplied
game files. No proprietary bytes are emitted or included in the repository.

All 18 exported stock weapon default constructors assign the classified table
recorded by the verifier. Their AddRenderingCommand slot1F0 is4BF40. The
adjacent Render slot1F4 is4C740 for seventeen classes and171F20 for Sniper.
An independent scan of all aligned non-code references to4BF40 yields exactly
these 18 tables, with no additional unclassified occurrence. This includes
CircularSawBlades and SeriousBomb as well as the ordinary inventory guns.

The common command constructor4BF40 stores the weapon at command+10, installs
table2A4720 and sort key8FF00. Execute4BDE0 resolves that weapon's owner, obtains
the native camera, builds twelve matrix words, and calls weapon slot1F4 at
4BF34, returning4BF3A. The current Base/Sniper Render hooks therefore cover the
classified stock command's actual render targets regardless of which hand
renders first. There is no need to add a separate common-command hook merely
to cover another stock weapon override in this pinned module.

## Remaining admission

This narrows the earlier unknown override set, but does not enable scope-image
capture. A capture latch must occur at the first actual ordinary local gun
entry before tracking/geometry admission, including failed admission. It must
not wait for the first successful sniper draw or revive after rejection.
External or replaced vtables remain unsupported and require live identity and
target checks. The static constructor/table scan is not a live binding proof.

The intended world content preceding the cut, source target/color equivalence,
auxiliary-view query/cache lifetime, actual zoom mapping and safe cap replacement
still need integration evidence. No magnified image is currently rendered.
