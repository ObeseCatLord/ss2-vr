# Native saw dispatch and receipt boundary — Astra disposition

Current continuation: the completed task-2 investigation supports a bounded,
uniquely mapped saw adapter on stable binding/normal-allocation-success routes;
no demonstrated fatal reentry justifies another broad shutdown census. Native
integration, initial/copy admission and observer delivery remain open. Earlier
proof decisions follow as historical evidence.

Receipt cleanup now has a conditional retirement operation. An ambiguous callback
can retire only its captured owner, lifetime epoch and observation revision;
an older outer abort cannot erase a completed inner observation or a replacement
at the same address. Actual lifetime teardown retains unconditional retirement.
Portable tests cover these cases and20,000 event-log interleavings, including
stale/cross-owner conditional retirement. Full products/61 Debug groups and
artifact contracts pass on source
`03e67658a13b1e9bcab50e3165afdb1822f6ee8c4598eecbf78ab938541a1994`.
These helpers remain inactive: this is not physical-melee or native callback
acceptance. Individual firing tests belong to the user in VR.

Fresh gpt-6-astra/xhigh; current routing and Sam/Engine/Core pins verified.
Static read-only proof. **UNPROVEN final ordering**, with a useful completed
base-stop boundary at165498. No native hook is enabled.

## Exact canonical input reference

7FFC0 is voidthiscall(player,semanticButton), ret4, forwarding to player408
with(button,0). Stock408 is DoAttack101F00, two explicitarguments/ret8.
7FFE0 has the one-argument/ret4 ABI and tail-jumps through40C toStopAttackFA770.
Operator capturesflip8E58D, convertsrawprimaryindices8E670..8E686, then calls
press/release at8E6E9/8E72D. Forraw0/1, semantic=raw XOR capturedflip; other
lanes staynative. A hand enum is not a dispatch argument; never flip twice.

Let R/L be resolved800/804 and U be validgame+comboenabled+player9BC.

| Semantic | Press afternonidle | Release afterbookkeeping |
| --- | --- | --- |
|0| No weapon-start | R204; additionallyL204 when!U |
|1| IfR exists/Labsent:R200; otherwise no weapon-start | L204 ifL exists; otherwiseR208 |
|Other| Native separatebehavior | No primary weaponrelease |

R-only alternative saw200→48FE0 sets2C=1; alternative208→49000 clearsit.
These are not primaryrelease. Suppressing a callback requires its whole native
target set, not just one saw. Synthetic dispatch must revalidate the original
raw-button→post-flip semantic binding.

## Completed base release and candidate adapter

Press101F25 invokes340→7F0F0, writingplayer148/14C. Under stablebinding,
semantic0 andsemantic1 withanexistingL have no subsequent weapon-start callback;
normal originalcallbackreturn is the press-completion candidate.

ReleaseFA77C firstcalls7F450, conditionallyclearing1C8 whenC0==6 andmarking
nonidle. Selected204 enters165490; call165493→48FF0 writes38=1 andtail-jumps
through1F8→4CAA0. State8 remains8; state4 requestsstate1 through1B0→4A070.
Successfulreturn is165498, before sound3 call16549C→164F80. Sound processing
alreadyinvokesresources164FA5 beforeplay1650A0 andlaterE8write1650AC/1650FB.
The receipt therefore must not claim soundcompletion.

The smallestcandidate is a typed voidthiscall(weapon) wrapper aroundoriginal
48FF0, committing aftersuccessfulreturn onlyunderexactpreboundreceipt context
andincomingreturn165498. This avoids relocating the soundcall. Complete
entry48FF0..48FF9 is9bytes; normalpressentry7FFC0..7FFC6 is6bytes; releaseentry
7FFE0..7FFE5 is5bytes. These are static candidate windows, not linked ABI proof
or source approval. Inline165498..1654A1 is9bytes and includes a relative call
to164F80, making it the larger alternative.

## One earlier unresolved edge

State4→SetState1 callsGetAnimQueue4A0D8,NewClearState4A0E6,GetIdleAnim4A0F7,
andPlayAnimation4A11E whenvalid. It writesB0=1 onlyat4A134 or4A3DB oninvalid
animation. Slot210 at4A12B isplainRET1C3240, but that alone doesn'tclose the
animation work. Same-binding operator/base-step reentry beforeB0write could
reachstate4Fire4EF3F whilea pendingreceipt suppressesduplicatereconciliation.
No such reentry is demonstrated; it mustbe established orexcluded onthisactual
path before sourceintegration. Animmutableinputancestor doesn'tstop native
state4 from firingbeforeheld-low, and stale/replacementbindings can'tborrow it.

## Main disposition and next proof

Adopt canonical semantic dispatch and complete target-set coverage. Prefer the
base-release wrapper at48FF0 over inline soundcall displacement. Preserve native
160/348, combat and fourlanes; do not add a mirrored command-history registry.
The20low/40unqueriedhigh/60low reference still requires canonicalpress/release
receipts, native manual callback deduplication and completed-innercommit survival.
No statewrite or repeatedStopAttack is a substitute.

Next inspectonly the synchronous animation/get-idle path before4A134/4A3DB,
including actual indirect/resource callbacks ifpresent. Do not infer reentry
fromthewordanimation or demand whole-program closure withouta reachable edge.
Bothhands, initial/copy/replacementretirement and observerconsumption remain
required beyondthisstablebindingproof. No native/runtime/session wasrun.
