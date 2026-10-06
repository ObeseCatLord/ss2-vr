# Astra source review — findings and fixes

Effective gpt-6-astra/xhigh verified independently by main. Read-only native/source inspection and offline reproductions; no runtime, production writes or subagents. Initial verdict: NO-GO for combined source until four bounded fixes. The post-producer architecture and default-disabled lifetime disposition were accepted.

| Priority / finding | Main fix and targeted evidence |
|---|---|
| P1 permanent remote-thread failure also rejects later native-only local pairs | Lazy pair admission checks support only when bank has admitted remote bindings. Existing admitted bank remains immutable/sticky-invalid. Production policy plus actual Ready commit checks cover rejection followed by empty-bank publication with no owner/object query. Remote adapter remains unsupported. |
| P1 server raw lookup relabels old frozen pose with newer revision | Preserve frozen sample's revision; return invalid while it differs from current peer history. New compatible samples retain history; genuine next simulation freeze owns its new revision. Zero/exhausted revision rejected. |
| P1 unrelated static/unskinned draw rejects head pair | Find relevant exact body record first, cache unique owners, return native for unrelated or empty map draws. Require canonical non-alias only after at least one mapped Head result is prepared. Headless/unmapped LOD stays native. Actual candidate malformed ownership still makes zero writes and invalidates pair. |
| P2 late nesting permits writes; original exceptions swallowed | Producer exception propagation and RAII cleanup preserved; catch only plugin adaptation. Nested invalidity is checked at actual adapter's final write fence and faults outer transaction. Production helper checks original-once, nested original/adaptation, zero writes, original throw, cleanup and unchanged canonical bytes. |

The bounded Astra follow-up approved all four fixes; final builds and artifact checks pass. See HEAD_ANIMATION_SOURCE_FOLLOWUP.md. Unhooked model/deletion-worker lifetime, normals, extra passes and actual stereo appearance remain missing evidence. Head option must stay default-disabled.
