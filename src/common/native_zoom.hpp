#pragma once
#include <array>
#include <cstdint>
namespace ss2vr {
// Adapter provenance only. Native IsZooming/time/damage/audio stay native.
struct NativeZoomClaim {
    void *weapon = nullptr, *owner = nullptr;
    uint32_t weaponHandle = 0, ownerHandle = 0;
    unsigned hand = 2;
    uint64_t serial = 0, revision = 0;
    bool effect = false, leftAudio = false, blocked = false, deleting = false, teardown = false;
};
// Current native Delete routing is ephemeral and independent of an older
// effect's cleanup provenance after FetchOwner reassociation.
struct NativeZoomDeleteRouting {
    void *owner=nullptr;
    uint32_t ownerHandle=0;
    unsigned hand=2;
};
inline bool nativeZoomPreserveDelete(const NativeZoomDeleteRouting &right,const NativeZoomClaim &left) noexcept {
    return right.hand==1 && right.owner && right.ownerHandle && left.serial && left.hand==0 &&
        right.owner==left.owner && right.ownerHandle==left.ownerHandle && left.effect &&
        !left.blocked && !left.teardown && !left.deleting;
}
class NativeZoomClaims {
  public:
    static constexpr unsigned Capacity = 128;
    // Caller serializes access; no lock may span a native callback.
    NativeZoomClaim find(void *weapon) const noexcept {
        for(const auto &entry:entries_)
            if(entry.serial && entry.weapon==weapon) return entry;
        return {};
    }
    NativeZoomClaim reserve(void *weapon,void *owner,uint32_t wh,uint32_t oh,unsigned hand) noexcept {
        if(!weapon || !owner || !wh || !oh || hand>=2) return {};
        NativeZoomClaim *slot=nullptr;
        for(auto &entry:entries_)
            if(entry.serial && entry.weapon==weapon) {
                if(entry.weaponHandle!=wh || entry.owner!=owner || entry.ownerHandle!=oh || entry.hand!=hand ||
                    entry.blocked || entry.deleting || entry.teardown) return {};
                slot=&entry; break;
            }
        if(!slot && serial_!=UINT64_MAX)
            for(auto &entry:entries_)
                if(!entry.serial) {
                    entry={weapon,owner,wh,oh,hand,++serial_,1};
                    slot=&entry;break;
                }
        if(!slot) return {}; // Never evict a live source's cleanup obligation.
        slot->effect=true;
        return *slot;
    }
    void change(uint64_t serial,bool deletion,bool takeover,bool abort) noexcept {
        if(auto *entry=bySerial(serial)) {
            if(entry->revision!=UINT64_MAX) ++entry->revision;
            else entry->blocked=true;
            entry->deleting|=deletion;
            entry->blocked|=abort || deletion;
            if(takeover) entry->effect=false;
        }
    }
    void ownerDeleting(void *owner) noexcept {
        for(auto &entry:entries_)
            if(entry.serial && entry.owner==owner) {
                entry.teardown=entry.blocked=true;
                if(entry.revision!=UINT64_MAX) ++entry.revision;
            }
    }
    void audioStarted(uint64_t serial,uint64_t revision) noexcept {
        if(auto *entry=bySerial(serial);entry && entry->revision==revision)
            entry->leftAudio=true;
    }
    void sourceDeleted(uint64_t serial) noexcept {
        if(auto *entry=bySerial(serial)) *entry={};
    }
    bool token(const NativeZoomClaim &claim) const noexcept {
        const auto live=find(claim.weapon);
        return claim.serial && live.serial==claim.serial && live.revision==claim.revision;
    }
  private:
    NativeZoomClaim *bySerial(uint64_t serial) noexcept {
        for(auto &entry:entries_) if(serial && entry.serial==serial) return &entry;
        return nullptr;
    }
    std::array<NativeZoomClaim,Capacity> entries_{};
    uint64_t serial_=0;
};
enum class NativeZoomQuery { Effect, PreserveCrossDelete };
inline bool nativeZoomPhaseAllowed(bool nested,bool current,bool preparation,bool execution,
                                   NativeZoomQuery purpose) noexcept {
    return !nested && current && (execution || (preparation && purpose==NativeZoomQuery::PreserveCrossDelete));
}
inline int nativeZoomStepPredicate(unsigned kind,int original,const NativeZoomClaim &captured,
                                  const NativeZoomClaim &live,bool revoked,bool sourceRevoked) noexcept {
    const bool sameSource=captured.serial && captured.serial==live.serial;
    if(sourceRevoked || (captured.serial && (!sameSource || live.deleting))) return 0;
    const bool current=sameSource && captured.revision==live.revision;
    if(kind==5) return sameSource && live.leftAudio ? 1:original;
    if(revoked || (sameSource && (!current || live.blocked || live.teardown))) return 0;
    if(kind==3) return current && live.effect ? 1:original;
    if(kind==4) return current && live.effect && live.hand==0 ? 1:original;
    return original;
}
inline bool nativeZoomStateAllowed(int state,bool active,uint32_t deleted) noexcept {
    return !(deleted&2) && (state==1 || state==7 || (active && (state==4 || state==8)));
}
} // namespace ss2vr
