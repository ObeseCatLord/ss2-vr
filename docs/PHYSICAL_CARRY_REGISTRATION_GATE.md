# Native history identity and registration — Astra Max disposition

Fresh gpt-6-astra/max; current routing and Core/Engine/Sam pins verified.
Static owned binary investigation only, no edits/runtime. No manual-history
architecture is approved by these findings. Native exports name the player
class CPlayerPuppetEntity.

| Verified identity path | Main disposition |
| --- | --- |
| CoreB7A0 non-registering pointer-to-existing-handle search | Not a pure read: successful B490/B580 searches reorder hashes and it does not lock itself; not suitable for scalar helper |
| CoreC450 hvPointerToHandle | Registers on miss and takes native mutexBB790; current pointerHandle binding cannot be reused as pure prebinding |
| CoreC540 hvPointerToHandle_internal | Also registers on miss; suffix is not lookup-only |
| CoreB370 hvHandleToPointer | Non-registering registry-read-only validation apart from synchronization; validate an already-known handle outside scalar helper |
| Handle format generation<<24\|slot | Existing native registry owns allocation identity; no new pointer-only identity scheme |
| CEntity+4 initializedFFFFFFFF atEngine595E7; getter58D40; copied at207C/20DE | EntityID cannot distinguish copy source/destination lifetimes |
| CorememFree20D70 calls hvDismissPointer20D98; clears slotC033 and advances generationC03C/C042 | Establishes allocation registration lifetime; not unproved placement reconstruction without dismissal |

Ordinary factorySam104190 allocates at1041C4 and default-constructs at1041D9.
The normal allocator clears the registration marker atCore20A39→6FEC0.
Default player construction reaches the zero348 store before real registration.
AtF91EE/F91FB, the actual player pointer goes to helper2241C0; that helper calls
CoreC450 at2241D0, receives the handle at2241D6, stores it athelper+4 at2241D9,
and returns toF9200. HelperABI is thiscall, one player pointer argument, ret4;
the helper is attached atplayer+9A0.

The +9A0 helper is copied atassignment289E4 and copy constructor2AA0D. Its
indirect handle can therefore refer to the source; it is not unconditional
destination identity. Require native handle-to-pointer equality to the target.

| Native348 store | Identity availability / smallest justified boundary |
| --- | --- |
| A41FF(default constructor, EBX=dest) | Fresh target unregistered at store; registration2241D6 is later. A4206 prebinding not proved |
| D465(assignment, EBX=source, EDI=dest) | Existing registered/validated source and target can be prebound attyped285C0 wrapper; no unconditional guarantee for arbitrary inputs. D46B remains immediate commit candidate |
| F29E(copy constructor, ESI=source, EBX=dest) | Fresh destination registration not proved; typed2A610 proves pointer correspondence only. Copied +9A0 does not create or retarget lifetime |

Existing Authority handle→CoreB370→target equality is the justified validation
route, outside scalar instructions. Main spot-check confirms current
copyAuthority invokes pointerHandle and its binding is CoreC450. Reusing it
unchanged would register rather than merely validate.

The native generic copy index33920 stores a type/pointer pair; destination-only
mdPostCopy does not prove direct copy-constructor lifetime correspondence.
Missing registration/provenance stays unknown. No added registry or raw348
write follows from this evidence. Main prioritizes the independently reviewed
saw-level stop proof because success would eliminate this storage requirement.
