#pragma once
namespace ss2vr {
// Lexical cancellation only. No native resource, traversal, movement, or
// successful-query ownership is supplied by this decision function.
enum class OptionalQueryDecision : unsigned { original, cancel, clearBranch };
constexpr OptionalQueryDecision optionalQueryDecision(bool *unavailable,
                                                      bool replacementPending,
                                                      unsigned gateKind) noexcept {
    if (!unavailable) return OptionalQueryDecision::original;
    if (*unavailable) return OptionalQueryDecision::cancel;
    if (gateKind>1 || (gateKind==0 && replacementPending)) {
        *unavailable=true;
        return OptionalQueryDecision::cancel;
    }
    return gateKind==0?OptionalQueryDecision::clearBranch:OptionalQueryDecision::original;
}
// Admit only a pinned, inspected virtual target inside an optional query.
// Unknown shapes cancel the query; they are never treated as empty space.
constexpr OptionalQueryDecision optionalQueryTargetDecision(bool* unavailable,
                                                            bool supported) noexcept {
    if (!unavailable) return OptionalQueryDecision::original;
    if (*unavailable || !supported) {
        *unavailable=true;
        return OptionalQueryDecision::cancel;
    }
    return OptionalQueryDecision::original;
}
} // namespace ss2vr
