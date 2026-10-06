#pragma once
#include <cstdint>
#include <windows.h>
namespace ss2vr::game::menus {
using HookInstaller = bool (*)(HMODULE, uint32_t, void *, void **);
bool initialize(HMODULE sam, HookInstaller install);
bool active();
uint32_t captureGeneration();
bool pointerAvailable();
} // namespace ss2vr::game::menus
