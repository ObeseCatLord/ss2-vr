#include "common/hook_transaction.hpp"
#include <array>
#include <cstdlib>
#include <iostream>

static void check(bool result, const char *message) {
    if (!result) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}
int main() {
    std::array<int, 3> tokens{};
    std::vector<void *> owned{&tokens[0], &tokens[1], &tokens[2]};
    unsigned disabled = 0, removed = 0;
    bool ok = ss2vr::rollbackHooks(
        owned,
        [&](void *p) {
            ++disabled;
            return p != &tokens[1];
        },
        [&](void *) {
            ++removed;
            return true;
        });
    check(!ok && disabled == 3 && removed == 0 && owned.size() == 3,
          "A failed disable must retain every original trampoline and try all disables");
    disabled = removed = 0;
    ok = ss2vr::rollbackHooks(
        owned,
        [&](void *) {
            ++disabled;
            return true;
        },
        [&](void *p) {
            check(disabled == 3, "Removal started before all disables");
            ++removed;
            return p != &tokens[1];
        });
    check(!ok && removed == 3 && owned.size() == 1 && owned[0] == &tokens[1],
          "Removal failure must preserve ownership of only the disabled unresolved hook");
    ok = ss2vr::rollbackHooks(owned, [](void *) { return true; }, [](void *) { return true; });
    check(ok && owned.empty(), "Completed rollback must release every owned hook");
    std::cout << "Partial hook activation rollback checks passed\n";
}
