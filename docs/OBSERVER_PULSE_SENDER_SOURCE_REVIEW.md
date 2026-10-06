# Observer pulse sender — Astra source acceptance

Fresh gpt-6-astra/xhigh; effective current-turn settings verified. Read-only
source review. **GO for the sender-only change; no blocking defect found.**

| Reviewed finding | Main disposition |
| --- | --- |
| Optional relay copies the same filtered frozen sample before active pulse loss | Adopt; accepted pulse/pulseZoom/historical grips remain together |
| Expanded simulation fire intersects the captured ordinary-fire mask for wire output | Adopt; physDown1/fire0/pulse1 remains ordinaryfire0, never fabricated held fire |
| Active one-use pulse is cleared once; invalid/future/stale sample has empty relay | Adopt |
| Cached same-tick return precedes freeze/recipient collection | Preserve once-per-tick relay behavior |
| Pulse bypasses50ms ordinary pose throttle | Adopt through existing send path, without a queue/protocol change |
| Native recipient mapping/binding/freshness/nonces and unreliable send unchanged | Preserve; no transport delivery guarantee |
| Historical zoom/revocation/expiry-before-first-freeze coverage was missing | Added focused optional-output codec/policy cases; checks pass |
| Native throttle/cache path is inspected but not executed offline | Explicit verification limit; no runtime/session claimed |

Production changes are confined to OrderedPosePolicy::freeze's optional relay
output and multiplayer::freezeInput. Existing callers omitting the optional
output, including lifecycle discard, retain their original behavior.

Main ran the full25 portable groups after the production change; x86 proxy/
server and x64 host builds and read-only installed/compiled artifact verification
pass. The network suite also passed UBSan. After the review, additional tests
for historical pulse zoom, revocation between receive/freeze and expiry before
first pulse freeze passed the focused network check. Production source did not
change after the review. No native/Windows/Wine/XR/network session was launched.

## Remaining multiplayer physical-melee work

Latest-value receiver replacement can still lose an accepted event before a
native consumer sees it. The existing send is unreliable. Observer retention/
one-interval consumption, exact native saw dispatch/commit, retirement and
lifecycle are not implemented by this fix. Native gesture/carry history source
remains unaccepted. No new development archive or installed-file change.
