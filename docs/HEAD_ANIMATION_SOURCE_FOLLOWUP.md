# Bounded source follow-up

Disposition: GO for default-disabled head landing and active pair-coherence fixes, subject to main's pending native build/artifact checks. All four previously identified source blockers are closed in the inspected current source. No new blocking defect found within the follow-up scope.

| Previous finding | Disposition | Current evidence |
| --- | --- | --- |
| Permanent remote-thread failure rejects later native-only pairs | Closed | remote_render.cpp:185 derives admission from immutable bodyValid entries; presentation_identity.hpp:8 skips the owner predicate for empty banks, preserves admitted invalidity; remote_render.cpp:262 gates native array reads independently. clearBinding preserves frozen admission. |
| Server raw guard re-stamps frozen pose | Closed | multiplayer.cpp:687 preserves the frozen revision and invalidates on mismatch using validFrozenPresentationRevision. Commit retains binding and MP guards through Ready. |
| Unrelated/static/headless draws require canonical evaluation | Closed | remote_render.cpp:429 resolves/caches exact owners before maps/evaluation, returns for unrelated/empty mappings, and requires canonical nonalias only after a Changed result. Scratch preparation precedes native writes. |
| Late nesting and swallowed original exception | Closed | head_palette.hpp:42 faults after late invalidation; remote_render.cpp:519 fences native writes; palettePass catches only the postPalette adapter lambda, allowing original producer exceptions to propagate with helper RAII cleanup. |

Verification: read current production branches and tests; independently compiled tests/head_checks.cpp against current headers with g++ C++20, O0, assertions enabled, output exclusively in reviewer scratch. Execution exit 0. Tests exercise production helpers and commitNativeFrame; this is not native integration/runtime validation.

Default false remains in settings, INI fallback and shipped INI. engine.cpp:1575 passes the setting; remote_render.cpp:592 installs DDE30 only when enabled.

Remaining gates: main's pending native builds and exact-byte artifact checks. Unhooked worker/model/deletion lifetime, normals/extra passes/attached-child appearance remain unverified and do not acquire proof from this review. Keep default 0 under the accepted conditional policy. No further thread investigation, subagents, production writes or game/host/Wine/runtime execution performed.
