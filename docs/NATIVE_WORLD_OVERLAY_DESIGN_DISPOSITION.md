# Native world-marker design disposition

Main verified and adopted Astra's conditional design GO. Final implementation plan:

| Recommendation | Disposition |
|---|---|
| Incremental per-eye phase; delete parent replay | Adopted. Keep current Render3D hook/original callback; extend same stereo loop with optional post-render callback scoped to native parent returnFEA8A. Desktop/fallback/nested paths never invoke added phase. |
| Copied actual eye matrices; dead root excluded | Adopted. Copy executed native world view/full adjusted P, use existing immutable eye/request identity; never dereference/reactivate retired root after Render3D. |
| Match original color stage and graphics setup | Adopted. After original Render3D completes, original ortho/blend501/disable raster calls and original navigation/objective draws; no shader/UI model replacement. |
| Ordinal binding and native ABI | Adapted. Actual names exist with complete PE parsing; reuse named S lookup and existing artifact symbol verification. Preserve actual virtual fade dispatch, native reference/scalar arguments and validate compiled caller ABI. |
| Narrow admission/target/callback guards | Adopted. Reuse current frame/rider/native thread/pair-fault data and actual bridge surfaces; require live world-info and drawport/viewport match, revalidate around native getters/draws. No Snapshot lock across native callbacks. |

Scope is world markers only. Existing comfort-following HUD/menu/wheels, native parent listener/crosshair/flat overlay ownership, transport/IPC7/wire6/physics/weapons remain. Native flat overlay capture/readability remains unfinished. No user decision or runtime launch is required. Source review follows completed implementation/offline checks; no completion inferred from this plan.
