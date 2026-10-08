# Native background-controls prerequisite

Current direction: individual weapon-firing tests are user-operated in VR. This
work observes native movement submission without firing, replacing no native
input, focus, physics or transport mechanism.

Astra/xhigh reviewed the current producer boundary against the owned fingerprinted
Sam2Game/Engine/Core binaries. Current-turn local model/effort tags were verified;
independent backend routing introspection remains unavailable. Source evidence is
not a runtime movement receipt.

| Recommendation | Disposition |
| --- | --- |
| Do not override exclusive input for return192583 | Adopted. The native192550 path is menu/system input, called under a nonzero menu; it cannot establish walking |
| Do not override gameplay return256CD | Adopted. Its true branch enables native device polling/system bindings and potentially desktop cursor warping |
| Observe native inactive producerF2DD6/returnF2DDC first | Adopted. Private lab trace records a finite120 calls per route with arguments and bidirectional binding observation; behavior is unchanged |
| Consider movement-only adaptation of existing inactive producer | Conditional, not enabled. Requires observed live caller/owner and separate source review; native pause/modal restrictions and exactly one original controls call remain |
| Reuse shared existing stick basis rather than cached command samples | Adopted design constraint. Inactive native polling cannot make sampledControls fresh |
| Keep origin settlement separate | Adopted. Controls submission or aggregate body displacement cannot certify causal roomscale settlement |

Ordinary local controls returnF35D3 is already the swimming/control wrapper's
audited producer. Native inactive controller virtual10 resolvesF2D50, retains
simIsPaused atF2D87 and submits native look with an initially zero-filled movement
vector and zero fire through the same brain virtual384/ClientAction route. The
static verifier checks the exact caller, pause guard, initial image zero-fill and
original native forwarding. Runtime tracing must establish actual arguments;
zero-fill alone does not prove later global-vector immutability.

Run200920 observed14 actual inactive submissions with valid local brain/player
binding, native zero move/fire and fresh focused XR input while the owned native
window was minimized and Core foreground/exclusive flags were both zero. The
scene hash matched the verified Jungle stream;307 complete neutral stereo pairs
preceded the diagnostic. Normal shutdown completed with no cleanup errors. No
movement or firing was injected. This establishes the native route/owner occurrence,
not movement adaptation or roomscale attribution.

A movement-only candidate is now written behind the explicit private-lab
SS2VR_LAB_BACKGROUND_MOVE opt-in plus tracing. Source review and actual
movement/cessation evidence were subsequently obtained
for the bounded private fixture below. It remains default-off in production. It uses the
shared existing horizontal stick convention, preserves the native zero-fire and
look arguments and reuses the original single forwarding call. It enables no
hardware polling. Native pause/modal/input-enable restrictions remain mandatory.

The private trace copies input identity/origin and observes handles synchronously
before the original native call. It accesses no borrowed receiver afterward,
retains no new native ownership and does not enable the alternate caller for
swimming or movement. Lab tracing requires the explicit process opt-in. Both
triggers remain neutral for the owner movement prerequisite.

Acceptance for a later movement adapter includes actual native body movement
then cessation under neutral, unchanged coordinate origin, zero movement when
admission is lost, and no newly enabled desktop hardware polling/menu processing.
Multiplayer host-local, remote-client and authority consumers still require their
separate ownership/publication proof. No new movement RPC or physics replacement
is proposed.

## Reviewed non-firing body proof

Astra's initial source NO-GO caught flight admitted by the water-only predicate,
a potentially waiting placement getter after admission, and model placement being
accepted as a physics-root receipt. Adopted repairs explicitly reject flight,
collect native observations before final admission, recheck fresh copied input
after all waiting lookups, and require unchanged mechanism/root handles. Scalar
logging runs only after normal original completion, with no receiver post-read.
The normal native forwarding remains once-only.

The harness review also caught an undispatched movement mode, a truncated or early
cessation window, consumption inferred from wall-clock timestamps, permissive
direction checks and NaN comparison acceptance. All were repaired: both modes
dispatch; full dwell and release-endpoint coverage are mandatory; input sequence
and identity define consumption; four axes and the complete native movement vector
must match; three movement components/turn must be finite. Native trace saturation
and final neutral-release failure are fatal to the probe.

Conditional source GO was limited to this default-off single-player experiment.
Reviewed engine SHA256 is858a7eb60a69c85e40707273a6a7e6ce4cf70a27fc10aac94629d1cc344a8419;
harness SHA256 is81d284c38534d9cefa32181a3278a87b9e21ac56467595239fb84f4cc2eac775.
The owner's compiled client/server forwarding gates pass, including server Python-O;
four mutated ABI controls reject widened fire, bad look/fire slots and bad cleanup.

Actual run204618 uses compiled source8395c0bfff5e321ded198520d7cd853d62c0a9ce2458c713bb41c22f94218cca.
The verified Jungle stream is10562049bytes with SHA256
106f9e287110988720cc717aaaa22190cafcf90fb184fd5a83e9d467d93165df.
It recorded320 complete neutral native stereo pairs, then a held0.3 left-stick
forward demand and neutral release under fresh focused XR input while native
foreground/exclusive remained zero. Fifty-four accepted receipts retained the same
brain/player/mechanism/root, session/reference/generation, origin and turn.
The physics-body horizontal displacement was2.216559m; drift over the fully
bracketed final250ms was0.005253m. Both triggers and native firing counters stayed
zero. Normal shutdown completed with no cleanup errors.

This proves the fixture's native walking submission, movement and stopping.
It does not establish background jump/use/turn/firing, admission-loss behavior,
multiplayer movement or causal roomscale-origin settlement. The adapter remains a
private-lab prerequisite, not completed general VR input or production enablement.
