#pragma once
#include <array>
#include <cstdint>
namespace ss2vr {
// Exported read-only package metadata. It is not part of shared memory or the
// network protocol, and its parser never needs to execute a Windows product.
struct BuildContract {
    char magic[16];
    uint32_t schema,component,ipcAbi,wireVersion;
    uint32_t inputBytes,requestBytes,uiBytes,slotBytes,sharedBytes;
    uint32_t versionMajor,versionMinor,versionPatch;
    std::array<char,64> sourceFingerprint;
};
static_assert(sizeof(BuildContract)==128);
}
