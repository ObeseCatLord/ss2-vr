# Astra retained-shot zoom codec source review

Explicit/effective Astra/xhigh verified. Final **GO for the bounded codec component** after a returned-sample admission fix. This does not enable native zoom; the current producer still sends zero zoom fields.

| Finding | Disposition |
|---|---|
| P1 rejected primaryfire still retained historical zoom override | Fixed. Ordered freeze now clears returned fire/pulse/pulseZoom for each release-unadmitted hand while retaining desired zoom. Active history remains one-time; ACK/credit unchanged. Astra final GO. |
| Required rejected-history regression missing | Added both directions for each hand with no release witness, opposite-hand desired preserved, one ACK and no replay. |
| Simultaneous opposite-hand histories and cancellation | Added. Cancelling left preserves right's unzoomed historical override; expiry resumes held intent. |
| Generation/capability/expiry nonzero contexts | Added. Astra found capability test's generation mismatch could mask the intended assertion; main matched generations. Added fresh nonzero pulseZoom expiry scenario separately. |
| Wire/read/write/mask admission | GO. Field order matches, desired/pulse bits bounded, valid/unblocked tracked hand/type13/equip constraints, mixedwire4 rejected for every kind. Native actual weapon identity remains an adapter requirement. |
| Existing invalidation boundary | GO. One production helper clears affected-hand fire/pulse/desiredzoom/pulsezoom in frozen/pending/active native peer records under existinglock; physical held/release witnesses unchanged. |

All11 offline groups passed after production fix. Focused network checks after nonblocking test-isolation improvements remain main verification. Affected x86 products rebuilt; final artifact/ABI records refreshed separately. Source uses wire5, IPC6 remains unchanged because no host Zoom action/button semantics are introduced by this component. Immutable0.2.5 package remainswire4/unchanged. No new queue, zoom timer, damage state machine, runtime launch or native gameplay callback was added.
