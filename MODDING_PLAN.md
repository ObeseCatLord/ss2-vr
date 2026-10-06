# Modding route

Installed Steam app 204340 uses native PE32 Serious Engine 2, not Unreal, Unity or Serious Engine 1. Five target binary fingerprints are recorded in docs/installed-build.json. Stock native dual-wield inventory/rendering methods exist; no verified Engine 2 C++ SDK/source was found.

Preserve native game simulation, inventory and renderer. Use a narrow x86 D3D9 proxy/game-hook adapter and one x64 OpenXR/D3D11 host, with pinned MinHook/OpenXR dependencies and CPU stereo transport. Astra's xhigh review rejected command-graph replay; the final design rebuilds native render preparation/collection per eye and scopes independent current-weapon input/pose hooks. Unknown builds fail closed. Renderer gates and remaining evidence are explicit.

Read docs/RESEARCH.md, docs/ASTRA_REVIEW.md, docs/REVIEW_DISPOSITION.md, docs/FINAL_PLAN.md and docs/IMPLEMENTATION_STATUS.md. Actual game/headset testing and showcase recording were explicitly excluded by the user. Compile/offline verification and packaging are complete; runtime correctness is unverified. No publication or installed-game deployment is part of the delivered changes.
