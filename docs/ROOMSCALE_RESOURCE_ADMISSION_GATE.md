# Resource-ready admission — Astra disposition

Fresh gpt-6-astra/xhigh; current routing and Engine/Core pins verified. Read-only
static native inspection. **UNPROVEN complete ready/stable admission**, with
concrete counterexamples to using IsReplacementReady as pump exclusion.

Engine World dispatch2911E uses receiver2F1B3C, testing receiver+4 bit0 at29114.
Model dispatchDA421 uses modelInstance+18 configuration, testingbit0 atDA419.
Concrete World207410 andModelConfiguration2095B4 slot0C bothreach1BF786→Core
AcquireReplacement3E560. They overwrite their pointer slots andAddRef/RemRef
after replacement. ModelRenderable15C4D0 takesinstance+5C at15C4DB andcalls
DA280 at15C613; observingtheWorld doesnotcoverallencounteredmodels.

Core3E560 checks current/mainthread IDs3E564/3E56B, equality3E570, clearsbit0
3E574 andpumpsglobaltasks3E578. Thereisnootherreadinesscheck. Proxy47200
tests+0C bit8, createsloadingtask47090 whenneeded, alwayswaits46D80 at47258,
andmaypump46E25 onmainthread whileevent+28 isunsignaled. Afterwaiting itreads
resource+60 andmayinvokeitsreplacement47274 ifresource+4 bit0isset.

| Examined predicate / owner | Exact consequence |
| --- | --- |
| Resource IsReplacementReady1DC80 | Alwaysreturns1; World/modelslot14 reachesitthrough1BF77A evenwhenreplacementflagwouldpump |
| Proxy IsReplacementReady46E70 | Onlyzerotimeeventtest68CF0; BindLoadedResource47010 assigns+60 before signal47064, notclear resourceflag |
| simIsLoading1B4760 / loaderIsPrepared44A20 | Loaderflag, not exhaustivequeryreceiverreadiness |
| GetChangeCount3E690 | Twowordcounter; taskregistration setsreplacementbit independently |
| wldGetCurrent1B79C0 / configurationgetterD2D70 | Canresolvereplacementthrough0C at1B79D6/D2DAA; notcallbackfree preflightgetters |
| GetAllModelInstances15D100 | One renderable'sinstance, not completeworldquerytargets |
| AddRef48500 / RemRef48530 | ShortrefcountmutexBEFB4, ret4; no retainedflag/target/receiver-setguard |
| Loader-thread AddPostLoadTask46880 | Queues469B3 andsetsresourcebit0 at469B9 underqueue mutexBEF3C; refcount doesn'tprohibitit |

Positive fact: bit0clear at the two actual native branch sites skips replacement
dispatch. A prior observation cannotproveitstaysclear for everydynamicreceiver.
Anemptyqueue also doesn'texcludelaterloader-threadenqueue. No actual nested
querywasdemonstrated, andno globalqueue/nativeflagsuppressionisapproved.

Main rejects a ready-getter/preflight-only shortcut. The next architecture
comparison must consider a narrow failure path for the **mod-owned query** at
actual replacement-dispatch sites, versus requiring full callback closure or
an existing stable native ownership extent. It may not change stock query
behavior, fabricate resource pointers/hits, or bypass normal native cleanup.
Exact normal-failure control flow and complete dispatch coverage are unverified.
No source query/body-movement/physics/protocol change follows from this result.
