# Astra cleanup/ABI follow-up

Fresh effective gpt-6-astra/xhigh verified. Read-only bounded final verification, no edits/agents/runtime or broader review. Verdict: GO; both source findings closed.

WeaponViewPass records graphics setup before rejecting projection/view/depth substitution. cleanupRequired no longer depends on successful stage advancement. Native depth callback also records responsibility before callback-availability short circuit, and cleanup consumes that responsibility. Regression fails atstage0, reaches rejected setup/placement and requires cleanup; failure before anysetup and untouched native return require none.

Compiled ABI verifier matches each of12 source words to its exact destination store and restricts intervening instructions to the unrelated boolean immediate store at stack30, excluding arithmetic/register reassignment. No additional fixes arise from these findings. Reviewer did not rerun final build/tests or verifier; main completed final x86/x64 builds, ten offline groups, exact native artifact and strengthened compiled ABI checks separately. Native GPU/wall-occlusion/runtime behavior remains unverified.
