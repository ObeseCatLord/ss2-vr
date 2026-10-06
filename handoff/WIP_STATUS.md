# Preserved unaccepted physical-melee work

This patch preserves seven local worktree files at transfer on 2026-10-05. It is
not merged into the accepted source snapshot and is not an approved implementation
or release. The historical handoff reports native gesture/carry integration as
NO-GO. Compilation or portable-check results do not override those gates.

Read `docs/HANDOFF.md` and the referenced physical-melee reviews before using it.
The patch applies to the accepted source snapshot originating at
`ccf61c94f3cbba1027ab1fb53269efc3a16422cf`. Its accepted-source checkpoint is
`828b413`; the later commit records the paused handoff.

Modified files:

- `CMakeLists.txt`
- `src/common/controls.hpp`
- `src/game/engine.cpp`

Previously untracked files:

- `src/common/gameplay_primary.hpp`
- `src/common/physical_motion.hpp`
- `tests/gameplay_primary_checks.cpp`
- `tests/physical_motion_checks.cpp`

If needed, inspect with `git apply --stat handoff/unaccepted-melee.patch` and
check with `git apply --check handoff/unaccepted-melee.patch` from the repository
root. Applying it changes source files and is a separate development decision.
The transfer did not apply it to the accepted source or modify the original tree.
