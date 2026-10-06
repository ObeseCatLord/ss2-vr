# Remote head review disposition (in progress)

Astra/xhigh effective model and effort were independently verified by main. First GPU review is superseded by the producer review where noted. Astra source follow-up approved default-disabled landing; no game/headset/runtime testing is authorized.

| Recommendation | Disposition |
|---|---|
| Single post-DDE30 refreshed renderer palette | Adopted in source; installation is explicitly configured and default-disabled; source review approved with all four fixes. Exact normal return E2E06 only; original producer once. |
| Separate retained setter arena / GPU+CPU pointer swaps | Rejected; shared producer reaches both native paths without pointer lifetime changes. |
| Full affine head delta about native body eye | Adopted; complete XYZ and quaternion, native animation/deformation retained. No anatomical IK claim. |
| Unique runtime Head and native parent graph | Adopted; no serialized ordinal; native synthetic root and -1 map preserved. Cross-owner attachments remain native. |
| Full record/mesh/draw/map ownership chain | Adopted; back-links and non-overlapping palette intervals validate before writes. |
| Canonical source non-alias and prepare-before-write | Adopted; invocation-owned working matrices; all bodies validate before any temporary palette write. |
| One pair admission, no per-eye age expiry | Adopted; frozen bank and body eye, lifecycle checks separate from freshness. |
| Guards held through Ready | Adopted; IPC→snapshot→binding→multiplayer lock order. Raw MP lookup never reacquires lock. |
| Invalid→valid recovery history | Adopted; local Sample revision, incompatible accepted poses and explicit invalidations advance it; wire/IPC unchanged. |
| Relay recipient capability | Adopted; raw client lookup requires current local client/server nonces. |
| Native object thread ownership | Simulation boundary pins before original; lifecycle/render/commit checks refuse unknown/mismatch. Core native main-thread predicate is also required; registered physics tasks join before Step returns. Other worker/model-lifetime coverage remains unproven. |
| GPU/CPU normal, extra-pass and actual stereo acceptance | Unverified; no runtime appearance or fully equivalent IK claims. |

Portable affine, mapping, sentinel, provenance, overflow and identity checks pass. Native proxy/server compile. These do not close full thread/lifetime coverage or runtime acceptance. The goal remains active; roomscale collision, physical optics, vehicles, complete overlays and melee remain unfinished.

## Source follow-up

Initial Astra source review found four issues: unsupported remote threading poisoned native-only stereo; a server lookup re-labelled an old frozen pose with newer history; unrelated unskinned draws required irrelevant canonical evaluation; and late nesting/original exception handling was incomplete. Main corrected all four and added targeted checks through actual production helpers and Ready commit. See HEAD_ANIMATION_SOURCE_REVIEW.md. Bounded Astra follow-up approved the four fixes; see HEAD_ANIMATION_SOURCE_FOLLOWUP.md. No new blocking source defect was found.

Final full cross-build and nine offline groups pass. Static artifact verification in head-artifact-verification.json records both architectures, IPC6/wire4 and four audited internal entries, including the optional producer. No runtime was executed.

Main completed the required final native/host/loader builds and static artifact checks. This closes the bounded landing prerequisites, not runtime acceptance or complete native worker lifetime. RemoteHeadTracking remains0 in shipped configuration; no installation or runtime execution occurred.
