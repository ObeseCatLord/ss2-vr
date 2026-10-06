#pragma once
#define WIN32_LEAN_AND_MEAN
#include "protocol.hpp"
#include <string>
#include <windows.h>
namespace ss2vr {
inline uint32_t trackingEpoch(const Shared &shared) {
    return static_cast<uint32_t>(InterlockedCompareExchange(
        reinterpret_cast<volatile LONG *>(const_cast<uint32_t *>(&shared.trackingGeneration)), 0, 0));
}
inline void publishTrackingEpoch(Shared &shared, uint32_t epoch) {
    // Unsigned monotonic publication also protects against callbacks completing out of order.
    uint32_t observed = trackingEpoch(shared);
    while (epoch > observed) {
        uint32_t previous = static_cast<uint32_t>(
            InterlockedCompareExchange(reinterpret_cast<volatile LONG *>(&shared.trackingGeneration),
                                       static_cast<LONG>(epoch), static_cast<LONG>(observed)));
        if (previous == observed)
            break;
        observed = previous;
    }
}
struct Channel {
    HANDLE mapping = nullptr, mutex = nullptr, ready = nullptr;
    Shared *shared = nullptr;
    std::wstring token;
    ~Channel() {
        close();
    }
    Channel() = default;
    Channel(const Channel &) = delete;
    Channel &operator=(const Channel &) = delete;
    bool open(const std::wstring &id, bool create) {
        token = id;
        auto n = L"Local\\SS2VR-" + id;
        mapping = create ? CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0,
                                              sizeof(Shared), n.c_str())
                         : OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, n.c_str());
        if (!mapping)
            return false;
        shared = static_cast<Shared *>(MapViewOfFile(mapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Shared)));
        if (!shared) {
            close();
            return false;
        }
        mutex = create ? CreateMutexW(nullptr, FALSE, (n + L"-lock").c_str())
                       : OpenMutexW(SYNCHRONIZE | MUTEX_MODIFY_STATE, FALSE, (n + L"-lock").c_str());
        ready = create ? CreateEventW(nullptr, FALSE, FALSE, (n + L"-ready").c_str())
                       : OpenEventW(SYNCHRONIZE | EVENT_MODIFY_STATE, FALSE, (n + L"-ready").c_str());
        if (!mutex || !ready) {
            close();
            return false;
        }
        if (create) {
            ZeroMemory(shared, sizeof(Shared));
            shared->magic = Magic;
            shared->abi = Abi;
            shared->bytes = sizeof(Shared);
            shared->gamePid = GetCurrentProcessId();
        }
        return shared->magic == Magic && shared->abi == Abi && shared->bytes == sizeof(Shared);
    }
    bool lock(DWORD ms = 0) {
        if (!mutex)
            return false;
        auto result = WaitForSingleObject(mutex, ms);
        if (result == WAIT_ABANDONED) {
            if (shared) {
                shared->error = 1;
                shared->shutdown = 1;
            }
            ReleaseMutex(mutex);
            return false;
        }
        return result == WAIT_OBJECT_0;
    }
    void unlock() {
        ReleaseMutex(mutex);
    }
    void close() {
        if (shared)
            UnmapViewOfFile(shared);
        shared = nullptr;
        for (auto h : {mapping, mutex, ready})
            if (h)
                CloseHandle(h);
        mapping = mutex = ready = nullptr;
    }
};
struct Lock {
    Channel &c;
    bool held;
    explicit Lock(Channel &x, DWORD ms = 0) : c(x), held(c.lock(ms)) {}
    ~Lock() {
        if (held)
            c.unlock();
    }
    explicit operator bool() const {
        return held;
    }
};
} // namespace ss2vr
