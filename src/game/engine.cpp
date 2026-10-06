#include "common/controls.hpp"
#include "common/native_zoom.hpp"
#include "common/native_primary_projection.hpp"
#include "common/frame_policy.hpp"
#include "common/head_comfort.hpp"
#include "common/hook_transaction.hpp"
#include "common/lasers.hpp"
#include "common/muzzle.hpp"
#include "common/win_settings.hpp"
#include "common/winproc.hpp"
#include "common/winpath.hpp"
#include "common/weapon_view.hpp"
#include "common/world_markers.hpp"
#include "game.hpp"
#include "menu.hpp"
#include "multiplayer.hpp"
#include "native_tracking.hpp"
#include "native_memory.hpp"
#include "native_finally.hpp"
#include "x86_predicate_entry.hpp"
#include "remote_render.hpp"
#include "scope_observer.hpp"
#include "scope_views.hpp"
#include "common/scope_position_program.hpp"
#include <MinHook.h>
#include <array>
#include <atomic>
#include <cstring>
#include <vector>
#ifdef _MSC_VER
#include <intrin.h>
#endif
namespace ss2vr::game {
std::atomic<bool> hooksReady = false;
using VoidThis = void(__thiscall *)(void *);
using IntThis = int(__thiscall *)(void *);
using HandInt = int(__thiscall *)(void *, int);
using HandlePointer = void *(__cdecl *)(uint32_t);
using PointerHandle = uint32_t(__cdecl *)(void *);
using PoseGet = Pose *(__thiscall *)(void *, Pose *);
using ViewOrigin = Pose *(__thiscall *)(void *, Pose *, int);
using ProjGet = Matrix44 *(__thiscall *)(void *, Matrix44 *);
using MatrixPose = Pose *(__cdecl *)(Pose *, const Matrix34 &);
using SetWeapon = void(__thiscall *)(void *, int, int, int);
using Toggle = void(__thiscall *)(void *, int);
using InventoryInt = int(__thiscall *)(void *, int);
using Frustum = Matrix44 *(__cdecl *)(Matrix44 *, float, float, float, float);
using WeaponAbs = int(__thiscall *)(void *, const Matrix34 &, Matrix34 &);
using RenderWeapon = void(__thiscall *)(void *, Matrix34);
using NativeFire = int(__thiscall *)(void *, float);
static NativeFire originalNativeFire = nullptr;
static VrSettings settings;
static std::atomic<uint32_t> fireFeedback[2]{}, damageFeedback{0};
struct Box1 {
    float min, max;
};
using ViewPrepare = void(__thiscall *)(void *, const Matrix34 &, const Matrix44 &, const Box1 &, uint32_t);
static ViewPrepare originalViewPrepare = nullptr;
static uintptr_t rootPrepareReturn = 0;
static uintptr_t rootProjectionReturn = 0, nativeFrustumReturn = 0, playerFovGetter = 0;
static PoseGet originalCamera = nullptr, originalShot = nullptr, originalSniperShot = nullptr;
static ViewOrigin baseViewOrigin = nullptr;
static PoseGet nativeBodyPlacement = nullptr;
static const uint32_t *invalidSeatIdent = nullptr;
static ProjGet originalProjection = nullptr;
static VoidThis originalRender = nullptr, originalStep = nullptr, originalDelete = nullptr,
                originalWeaponDelete = nullptr, originalSniperDelete = nullptr;
static VoidThis originalSimulationStep = nullptr, originalEntityStep = nullptr;
static void *(__cdecl *currentSimulation)() = nullptr, *(__cdecl *currentWorld)() = nullptr;
static VoidThis originalViewExecute = nullptr;
using RayInit = void(__cdecl *)();
using RayFloat = void(__cdecl *)(float);
struct NativeRay {
    Vec3 origin, direction;
};
static RayInit originalRayInit = nullptr;
static void(__cdecl *setRay)(const NativeRay &) = nullptr;
static RayFloat rayMaximum = nullptr, rayMinimum = nullptr, rayRadius = nullptr;
static int(__cdecl *checkRay)() = nullptr, (__cdecl * rayHit)() = nullptr;
static float(__cdecl *hitDistance)() = nullptr;
static void(__cdecl *rayCategory)(uint32_t) = nullptr, (__cdecl * thickCategory)(uint32_t) = nullptr;
static void(__cdecl *rayAvatar)(void *) = nullptr, (__cdecl * rayMechanism)(void *) = nullptr;
static void(__cdecl *rayFluids)(int) = nullptr;
static void *(__thiscall *getMechanism)(void *) = nullptr;
static void(__cdecl *drawLine)(const Vec3 &, const Vec3 &, uint32_t, uint32_t) = nullptr;
static RayInit *enableDepth = nullptr, *disableDepth = nullptr, *enableDepthWrite = nullptr,
               *disableDepthWrite = nullptr;
static int *depthEnabled = nullptr, *depthWriting = nullptr;
static RayInit *enableBlend = nullptr, *disableBlend = nullptr, *enableAlpha = nullptr,
               *disableAlpha = nullptr;
static int *blending = nullptr, *alphaTesting = nullptr, *depthComparison = nullptr;
using RasterComparison = void(__cdecl *)(int);
static RasterComparison *setDepthComparison = nullptr;
static RayInit ortho = nullptr;
static VoidThis activateView = nullptr;
static void **currentCanvas = nullptr;
static uintptr_t allowedRayReturn = 0;
static uintptr_t laserEngineBase = 0, laserTurretVtable = 0;
using ModelInstanceGet = void *(__thiscall *)(void *);
using AttachmentGet = int(__cdecl *)(void *, uint32_t, Matrix34 &);
using AttachmentIdent = uint32_t *(__thiscall *)(void *, uint32_t *);
using ShootDirection = Vec3 *(__thiscall *)(void *, Vec3 *);
static ModelInstanceGet laserModelInstance = nullptr;
static AttachmentGet laserAttachment = nullptr;
static uint32_t laserIdleAttachment = 0;
static uint32_t bulletCategory = 0;
static VoidThis originalOperatorFiring = nullptr;
static HandInt originalFireButtonPressed = nullptr;
using WeaponButton = int32_t(__thiscall *)(void *, uint32_t);
using GameInfoGet = uint32_t *(__cdecl *)(uint32_t *);
static WeaponButton nativeWeaponButton = nullptr;
static IntThis nativeFlipButtons = nullptr, nativeComboWeapons = nullptr;
static GameInfoGet nativeGameInfo = nullptr;
static uintptr_t primaryOperatorReturn = 0, primaryHeldReturn = 0;
static IntThis originalSniperAlternativePress = nullptr;
static int(__cdecl *nativeMainThread)() = nullptr;
static uintptr_t playerAlternativePressReturn = 0;
static WeaponAbs originalWeaponAbs = nullptr;
static RenderWeapon originalWeaponRender = nullptr, originalSniperRender = nullptr;
static Frustum originalFrustum = nullptr;
using MatrixInverse = Matrix34 *(__cdecl *)(Matrix34 *, const Matrix34 &);
using DepthRange = void(__cdecl *)(float, float);
using ProjectionSet = void(__cdecl *)(const Matrix44 &);
static MatrixInverse originalMatrixInverse = nullptr;
static DepthRange originalDepthRange = nullptr, *nativeDepthRange = nullptr;
static ProjectionSet *nativeProjectionSet = nullptr;
static Matrix44 *nativeCurrentProjection = nullptr, *nativeAdjustedProjection = nullptr;
static Matrix34 *nativeCurrentView = nullptr;
static float *nativeDepthNear = nullptr, *nativeDepthFar = nullptr;
static uint32_t *nativeCachedMatrices = nullptr;
static uintptr_t weaponFrustumReturn = 0, weaponInverseReturn = 0, weaponDepthReturn = 0,
                 weaponPlacementReturn = 0, weaponRestoreReturn = 0, rootDepthReturn = 0,
                 ordinaryWeaponRenderReturn = 0;
static DepthRange expectedDepthRange = nullptr;
static ProjectionSet expectedProjectionSet = nullptr;
static HandlePointer resolve = nullptr;
static PointerHandle pointerHandle = nullptr;
static IntThis isLocal = nullptr, isDual = nullptr, isNetricsa = nullptr, health = nullptr, armor = nullptr;
static IntThis isAlive = nullptr, thirdPerson = nullptr;
static IntThis originalThirdPerson = nullptr;
static uintptr_t mountedAvatarReturn = 0, mountedClampReturn = 0;
static uintptr_t markerParentReturn = 0;
static int32_t *uiVertexHandle = nullptr, *uiPixelHandle = nullptr, *uiSimplePrograms = nullptr;
static uint32_t *uiPrograms = nullptr;
using MarkerDraw = void(__thiscall *)(void *, const Matrix34 &, const Matrix44 &,
                                     const WorldMarkerDimensions &, float);
using MarkerFade = float(__thiscall *)(void *);
static MarkerDraw nativeNavigation = nullptr, nativeObjectives = nullptr;
static void *(__cdecl *nativeDrawPort)() = nullptr, *(__cdecl *nativeWorldInfo)() = nullptr;
static void(__cdecl *nativeBlendType)(int) = nullptr;
static int(__cdecl *singlePlayer)() = nullptr;
static SetWeapon setWeapon = nullptr;
static Toggle toggleDual = nullptr;
static HandInt canChange = nullptr;
static InventoryInt inInventory = nullptr, ammo = nullptr;
static MatrixPose matrixPose = nullptr;
static SRWLOCK snapshotLock = SRWLOCK_INIT;
struct Calibration {
    bool valid = false;
    uint32_t handle = 0;
    Pose nativeModelLocal;
    uint64_t tickMs = 0;
};
struct Snapshot {
    void *player = nullptr;
    RiderIdentity rider;
    uint32_t handle[2]{};
    uint32_t intentEpoch[2]{}; // Captured client admission; not a restamped input.
    uint32_t playerHandle = 0;
    Input input;
    InputSampleBoundary interruption;
    uint32_t inputProducer = 0;
    Pose origin;
    float turn = 0;
    bool initialized = false, fire[2]{}, use = false, jump = false, sprint = false;
    bool selecting[2]{}, zoom[2]{};
    uint32_t generation = 0, networkGeneration = 0;
    Ui ui;
};
static Snapshot current;
static std::atomic<DWORD> simulationThread{0};
static LaserAim laserAim[2];
static HeadObstruction headObstruction;
static SRWLOCK laserLock = SRWLOCK_INIT;
static TrackingEpochs generations, networkGenerations;
// Called while holding snapshotLock; publish invalidation before native state changes.
static uint32_t advanceEpochUnlocked() {
    uint32_t epoch = generations.advance();
    if (channel.shared)
        publishTrackingEpoch(*channel.shared, epoch ? epoch : ExhaustedEpoch);
    return epoch;
}
static void advanceGeneration(Snapshot &s) {
    AcquireSRWLockExclusive(&snapshotLock);
    s.generation = advanceEpochUnlocked();
    ReleaseSRWLockExclusive(&snapshotLock);
}
static Calibration calibration[2];
static void clearCalibration(int h) {
    AcquireSRWLockExclusive(&snapshotLock);
    calibration[h] = {};
    ReleaseSRWLockExclusive(&snapshotLock);
}
static WeaponWheel wheels[2];
static TriggerGate gates[2];
static HeldZoomInput zoomInputs[2];
static uint32_t lastButtons[2]{};
static bool turnLatched = false;
static int pendingSelection[2]{-1, -1};
static uint64_t selectionTick[2]{}, physicalSequence = 0;
static bool physicalDown[2]{};
static uint32_t releasedSerial[2]{};
static uint32_t zoomReleasedSerial[2]{}, primaryInputGeneration[2]{};
static void *sniperVtable = nullptr;
static uintptr_t sniperBaseStepReturn = 0, sniperCrossDeleteReturn = 0;
static VoidThis originalSniperStep = nullptr, originalBaseWeaponStep = nullptr;
static VoidThis originalZoomActivate = nullptr, originalZoomDeactivate = nullptr;
static IntThis nativeZoomFlag = nullptr, nativeBaseAlternativePress = nullptr, nativeOwnWeapons = nullptr;
static VoidThis nativeBaseAlternativeRelease = nullptr;
static NativeFire originalSniperFire = nullptr;
static Toggle originalSniperPutDown = nullptr;
struct ZoomManagerFrame {
    void *manager=nullptr,*simulation=nullptr,*world=nullptr;
    ZoomManagerFrame *previous=nullptr;
    bool current=false,preparation=false,execution=false;
    uint32_t frozenOwner=0;
};
static thread_local ZoomManagerFrame *zoomManagerFrame=nullptr;
static bool nativeSniper(void *weapon) {
    return weapon && *reinterpret_cast<void **>(weapon) == sniperVtable;
}

static network::PosePacket lastNetworkPose;

static Snapshot copySnapshot() {
    AcquireSRWLockShared(&snapshotLock);
    auto s = current;
    ReleaseSRWLockShared(&snapshotLock);
    return s;
}
static bool primaryLocalRecognized(const Snapshot &snapshot, void *subject, uint32_t handle) noexcept {
    return nativePrimaryLocalRecognized(subject, handle, snapshot.player, snapshot.playerHandle,
                                       snapshot.initialized);
}
static bool fresh(const Input &i) {
    return i.focused && i.headValid && GetTickCount64() - i.tickMs < 200 && finite(i.head);
}
static bool vrSession(const Snapshot &s) {
    return hooksReady.load(std::memory_order_acquire) && s.initialized && validTrackingEpoch(s.generation) &&
           channel.shared && trackingEpoch(*channel.shared) == s.generation && s.input.session &&
           GetTickCount64() - s.input.tickMs < 1000;
}
static bool local(void *p) {
    return p && isLocal && isLocal(p);
}
static bool trackingEligible(void *p) {
    RiderIdentity rider;
    return nativeInputHealthy() && hooksReady.load(std::memory_order_acquire) && local(p) &&
           (singlePlayer() || multiplayer::server() || multiplayer::negotiatedLocal()) && isAlive(p) &&
           !isNetricsa(p) && !menus::active() && readNativeRider(p, rider) &&
           (rider.seated() || (rider.handheld() && !thirdPerson(p)));
}
static bool trackingAnchor(void *p, Pose &out) {
    if (!trackingEligible(p))
        return false;
    return nativeTrackingAnchor(p, out);
}
bool readNativeRider(void *player, RiderIdentity &out) {
    out = {};
    if (!player || !resolve || !pointerHandle || !invalidSeatIdent)
        return false;
    RiderIdentity value;
    value.player = pointerHandle(player);
    if (!value.player || resolve(value.player) != player)
        return false;
    const auto *bytes = static_cast<const uint8_t *>(player);
    memcpy(&value.ride, bytes + 0x544, 4);
    memcpy(&value.state, bytes + 0x548, 4);
    memcpy(&value.seat, bytes + 0x54c, 4);
    value.seatValid = value.seat != *invalidSeatIdent;
    if (value.ride && !resolve(value.ride))
        return false;
    if (value.state == 3 && !value.seated())
        return false;
    out = value;
    return true;
}
bool nativeRiderCurrent(void *player, const RiderIdentity &expected) {
    RiderIdentity live;
    return expected.player && resolve && resolve(expected.player) == player &&
           readNativeRider(player, live) && live == expected;
}
bool nativeTrackingAnchor(void *player, Pose &out, const RiderIdentity *expected) {
    RiderIdentity identity;
    if (!baseViewOrigin || !nativeBodyPlacement || !readNativeRider(player, identity) ||
        (expected && identity != *expected))
        return false;
    Pose view, body;
    baseViewOrigin(player, &view, 0);
    if (!nativeRiderCurrent(player, identity))
        return false;
    uint32_t heightBits;
    memcpy(&heightBits, static_cast<uint8_t *>(player) + 0x8c0, 4);
    if (identity.seated()) {
        // Reject the getter's native identity fallback when no native pose
        // source exists. These are the exact borrowed sources it reads.
        uint32_t mechanismHandle = 0, modelHandle = 0;
        memcpy(&mechanismHandle, static_cast<uint8_t *>(player) + 0x114, 4);
        memcpy(&modelHandle, static_cast<uint8_t *>(player) + 0x120, 4);
        void *mechanism = mechanismHandle ? resolve(mechanismHandle) : nullptr;
        if (mechanism) {
            uint32_t rootHandle = 0;
            memcpy(&rootHandle, static_cast<uint8_t *>(mechanism) + 0x38, 4);
            if (!rootHandle || !resolve(rootHandle))
                return false;
        } else if (!modelHandle || !resolve(modelHandle))
            return false;
        nativeBodyPlacement(player, &body);
        if (!nativeRiderCurrent(player, identity))
            return false;
    }
    Pose result;
    if (!nativeRiderAnchor(identity, view, heightBits, body, result) ||
        !nativeRiderCurrent(player, identity))
        return false;
    out = result;
    return true;
}
static uint32_t nativeHandle(void *p, int hand) {
    return *reinterpret_cast<uint32_t *>(static_cast<uint8_t *>(p) + (hand == 1 ? 0x800 : 0x804));
}
// Authoritative samples use native player/weapon identity and never depend on
// an eye render, IPC channel, or render-populated calibration.
struct Authority {
    uint32_t player = 0, hand[2]{}, weaponGeneration = 0;
    RiderIdentity rider;
    multiplayer::Sample sample;
};
static std::array<Authority, 18> authorities;
static SRWLOCK authorityLock = SRWLOCK_INIT;
static Authority copyAuthority(void *player) {
    Authority result;
    const uint32_t handle = player ? pointerHandle(player) : 0;
    AcquireSRWLockShared(&authorityLock);
    for (const auto &entry : authorities)
        if (handle && entry.player == handle) {
            result = entry;
            break;
        }
    ReleaseSRWLockShared(&authorityLock);
    return result;
}
static void saveAuthority(const Authority &value) {
    bool held = false;
    withNativeFinally([&] {
        AcquireSRWLockExclusive(&authorityLock);
        held = true;
        Authority *destination = nullptr;
        for (auto &entry : authorities)
            if (entry.player == value.player) {
                destination = &entry;
                break;
            }
        if (!destination)
            for (auto &entry : authorities)
                if (!entry.player || !resolve(entry.player)) {
                    destination = &entry;
                    break;
                }
        if (destination)
            *destination = value;
    }, [&](bool aborted) noexcept {
        if (held) {
            held = false;
            ReleaseSRWLockExclusive(&authorityLock);
        }
        if (aborted)
            nativeInputFailed();
    });
}
static bool bodyAnchor(void *player, Pose &pose) {
    return nativeTrackingAnchor(player, pose);
}
static bool nativeWeaponReference(void *player, void *weapon, Pose &body, Pose &camera, Pose &modelPose) {
    RiderIdentity rider;
    if (!readNativeRider(player, rider) || !rider.handheld())
        return false;
    if (!bodyAnchor(player, body))
        return false;
    originalCamera(player, &camera);
    if (!nativeRiderCurrent(player, rider) || !finite(camera))
        return false;
    auto cameraMatrix = matrix(camera);
    Matrix34 model;
    if (!originalWeaponAbs(weapon, cameraMatrix, model))
        return false;
    matrixPose(&modelPose, model);
    return nativeRiderCurrent(player, rider) && finite(modelPose);
}
static void suppressHandheld(network::PosePacket &pose) {
    network::invalidateWeaponIntents(pose, network::HandMask);
    pose.requestedWeapon[0] = pose.requestedWeapon[1] = -1;
    pose.wheelOrEquipBlockedMask |= network::HandMask;
}
static bool currentWeaponSample(const multiplayer::Sample &, const multiplayer::Sample &, unsigned);
static void authorityStepBody(void *player) {
    if (singlePlayer() || !multiplayer::server())
        return;
    auto value = copyAuthority(player);
    value.player = pointerHandle(player);
    value.sample = multiplayer::freeze(player);
    if (!value.sample.negotiated)
        return;
    if(zoomManagerFrame && zoomManagerFrame->preparation)
        zoomManagerFrame->frozenOwner=value.player;
    uint8_t changed = 0;
    RiderIdentity rider;
    const bool riderValid = readNativeRider(player, rider);
    if (value.rider.player && (!riderValid || value.rider != rider))
        changed = network::HandMask;
    value.rider = rider;
    for (unsigned hand = 0; hand < 2; ++hand) {
        uint32_t handle = nativeHandle(player, hand);
        if (value.hand[hand] != handle)
            changed |= uint8_t(1u << hand);
        value.hand[hand] = handle;
    }
    if (changed) {
        if (value.weaponGeneration != UINT32_MAX)
            ++value.weaponGeneration;
        multiplayer::invalidateWeapons(player, value.weaponGeneration, changed);
        network::invalidateWeaponIntents(value.sample.pose, changed);
        network::cancelWeaponRequests(value.sample.pose, changed);
    }
    if (!riderValid || !rider.handheld())
        suppressHandheld(value.sample.pose);
    Pose body;
    if (!isAlive(player) || !bodyAnchor(player, body)) {
        multiplayer::invalidatePlayer(player);
        value.sample.valid = false;
    }
    if (value.sample.valid && riderValid && rider.handheld()) {
        bool pending = false;
        uint8_t equipped = 0;
        for (unsigned hand = 0; hand < 2; ++hand) {
            if (!nativeRiderCurrent(player, rider)) {
                suppressHandheld(value.sample.pose);
                value.sample.valid = false;
                break;
            }
            int requested = value.sample.pose.requestedWeapon[hand];
            auto weapon = value.hand[hand] ? resolve(value.hand[hand]) : nullptr;
            int actual = weapon ? *reinterpret_cast<int *>(static_cast<uint8_t *>(weapon) + 0xb4) : -1;
            if (requested >= 0 && requested != actual &&
                inInventory(static_cast<uint8_t *>(player) + 0x8f8, requested)) {
                pending = true;
                if (canChange(player, hand) && nativeRiderCurrent(player, rider) &&
                    currentWeaponSample(value.sample, multiplayer::authority(player), hand)) {
                    setWeapon(player, requested, hand, 1);
                    equipped |= uint8_t(1u << hand);
                }
            }
        }
        if (value.sample.valid && nativeRiderCurrent(player, rider) && !isDual(player) &&
            !value.sample.pose.physicalDownMask && !pending && canChange(player, 0) &&
            canChange(player, 1) && nativeRiderCurrent(player, rider)) {
            toggleDual(player, 0);
            equipped = 3;
        }
        if (equipped) {
            multiplayer::invalidateWeapons(player, ++value.weaponGeneration, equipped);
            network::invalidateWeaponIntents(value.sample.pose, equipped);
        }
    }
    saveAuthority(value);
}
static void authorityStep(void *player) {
    auto *frame=zoomManagerFrame;
    const auto previous=frame ? frame->frozenOwner:0;
    withNativeFinally([&] {
        if(frame) frame->frozenOwner=0;
        authorityStepBody(player);
    },[&](bool aborted) noexcept {
        if(frame) frame->frozenOwner=previous;
        if(aborted) nativeInputFailed();
    });
}
static void authorityAfterStep(void *player) {
    if (singlePlayer() || !multiplayer::server())
        return;
    auto value = copyAuthority(player);
    if (!value.sample.negotiated)
        return;
    uint8_t changed = 0;
    RiderIdentity rider;
    const bool riderValid = readNativeRider(player, rider);
    if (!riderValid || value.rider != rider)
        changed = network::HandMask;
    value.rider = rider;
    for (unsigned hand = 0; hand < 2; ++hand)
        if (value.hand[hand] != nativeHandle(player, hand))
            changed |= uint8_t(1u << hand);
    if (!isAlive(player)) {
        multiplayer::invalidatePlayer(player);
        value.sample.valid = false;
    } else if (changed) {
        multiplayer::invalidateWeapons(player, ++value.weaponGeneration, changed);
        network::invalidateWeaponIntents(value.sample.pose, changed);
        network::cancelWeaponRequests(value.sample.pose, changed);
    }
    if (!riderValid || !rider.handheld())
        suppressHandheld(value.sample.pose);
    saveAuthority(value);
}
static bool currentWeaponSample(const multiplayer::Sample &captured, const multiplayer::Sample &live,
                                unsigned hand) {
    const auto now = GetTickCount64();
    return hand < 2 && captured.valid && live.negotiated && live.valid &&
           captured.avatar == live.avatar && captured.incarnation == live.incarnation &&
           now >= captured.receivedMs && now - captured.receivedMs <= network::MaxPoseAgeMs &&
           network::currentIntentSample(captured.pose, live.pose, hand, live.liveIntentEpoch[hand]);
}
static void *livePlayer(const Snapshot &s) {
    void *p = s.playerHandle ? resolve(s.playerHandle) : nullptr;
    return p == s.player ? p : nullptr;
}
static bool aliased(const Snapshot &s) {
    if (!s.handle[0] || !s.handle[1])
        return false;
    return s.handle[0] == s.handle[1] || resolve(s.handle[0]) == resolve(s.handle[1]);
}
static int handOf(void *weapon, const Snapshot &s) {
    if (!s.rider.handheld() || !nativeRiderCurrent(s.player, s.rider))
        return -1;
    void *player = livePlayer(s);
    if (!player || !weapon || aliased(s))
        return -1;
    for (int h = 0; h < 2; h++)
        if (s.handle[h] && nativeHandle(player, h) == s.handle[h] && resolve(s.handle[h]) == weapon)
            return h;
    return -1;
}
static bool localWeaponCurrent(void *weapon, const Snapshot &captured, unsigned hand) {
    const auto live = copySnapshot();
    return sameHandheldRig(captured.rider, live.rider, captured.generation, live.generation) &&
           live.playerHandle == captured.playerHandle && vrSession(captured) && livePlayer(captured) &&
           trackedHandCurrent(captured.input, live.input, hand) && handOf(weapon, captured) == int(hand);
}
static void refreshHand(Snapshot &s, void *p, int h) {
    uint32_t handle = nativeHandle(p, h);
    if (!s.rider.handheld()) {
        s.handle[h] = handle;
        return;
    }
    if (s.handle[h] != handle) {
        gates[h] = TriggerGate{};
        pendingSelection[h] = -1;
        clearCalibration(h);
        s.fire[h] = s.zoom[h] = false;
        advanceGeneration(s);
    }
    s.handle[h] = handle;
}
static void refreshOwnership(Snapshot &s, void *p) {
    for (int h = 0; h < 2; h++) {
        refreshHand(s, p, h);
        s.selecting[h] = pendingSelection[h] >= 0;
        s.ui.currentWeapon[h] = -1;
        s.ui.currentAmmo[h] = 0;
        void *weapon = s.handle[h] ? resolve(s.handle[h]) : nullptr;
        if (s.rider.handheld() && weapon) {
            s.ui.currentWeapon[h] = *reinterpret_cast<int *>(static_cast<uint8_t *>(weapon) + 0xb4);
            s.ui.currentAmmo[h] = ammo(static_cast<uint8_t *>(p) + 0x8f8, s.ui.currentWeapon[h]);
        }
    }
    if (s.rider.handheld() && aliased(s)) {
        for (int h = 0; h < 2; h++) {
            s.fire[h] = false;
            gates[h] = {};
            clearCalibration(h);
        }
    }
    s.ui.trackingGeneration = s.generation;
}
static void publishSnapshot(const Snapshot &s) {
    AcquireSRWLockExclusive(&snapshotLock);
    if (!channel.shared ||
        !mayPublishSnapshot(s.generation, trackingEpoch(*channel.shared), current.generation)) {
        ReleaseSRWLockExclusive(&snapshotLock);
        return; // A deletion/transition superseded this copied snapshot.
    }
    current = s;
    ReleaseSRWLockExclusive(&snapshotLock);
    Lock l(channel);
    if (l && trackingEpoch(*channel.shared) == s.generation) {
        channel.shared->ui = s.ui;
        for (int h = 0; h < 2; ++h)
            channel.shared->ui.fireSequence[h] = fireFeedback[h].load(std::memory_order_relaxed);
        channel.shared->ui.damageSequence = damageFeedback.load(std::memory_order_relaxed);
    }
}
static void update(void *p) {
    if (!hooksReady.load(std::memory_order_acquire) || !channel.shared || !local(p))
        return;
    simulationThread.store(GetCurrentThreadId(), std::memory_order_relaxed);
    auto s = copySnapshot();
    Input input = s.input;
    uint32_t inputProducer = s.inputProducer;
    {
        Lock l(channel);
        if (l) {
            input = channel.shared->latest;
            inputProducer = channel.shared->hostPid;
        }
        if (l && !channel.shared->rendererReady)
            input.session = input.focused = 0;
    }
    RiderIdentity rider;
    const bool riderValid = readNativeRider(p, rider);
    bool changed = s.player != p || s.inputProducer != inputProducer ||
                   s.input.session != input.session || s.input.reference != input.reference ||
                   s.rider != rider;
    if (changed) {
        const auto interruption = s.interruption;
        s = Snapshot{};
        s.interruption = interruption;
        s.player = p;
        s.playerHandle = pointerHandle(p);
        s.rider = rider;
        s.networkGeneration = networkGenerations.advance();
        advanceGeneration(s);
        for (int h = 0; h < 2; h++) {
            wheels[h] = WeaponWheel{};
            pendingSelection[h] = -1;
            gates[h] = TriggerGate{};
            clearCalibration(h);
        }
        turnLatched = false;
    }
    s.inputProducer = inputProducer;
    bool enabled = riderValid && fresh(input) && s.interruption.permits(input, GetTickCount64(), inputProducer) &&
                   trackingEligible(p) && nativeRiderCurrent(p, rider);
    if (enabled != bool(s.ui.gameplay)) {
        s.networkGeneration = networkGenerations.advance();
        advanceGeneration(s);
    }
    enabled = enabled && validTrackingEpoch(s.generation);
    s.input = input;
    const int previousHealth = s.ui.health;
    const bool hadHealth = s.ui.tickMs != 0;
    s.ui = {};
    s.ui.tickMs = GetTickCount64();
    s.ui.gameplay = enabled ? 1 : 0;
    s.ui.health = health(p);
    s.ui.armor = armor(p);
    if (enabled && hadHealth && s.ui.health < previousHealth)
        damageFeedback.fetch_add(1, std::memory_order_relaxed);
    if (enabled && !s.initialized) {
        s.origin = {yaw(yawAngle(input.head.q)), input.head.p};
        s.initialized = true;
    }
    bool resettingControls = enabled && recenterHeld(input);
    bool recenter = resettingControls && !((lastButtons[0] | lastButtons[1]) & Button::Recenter);
    if (recenter) {
        s.origin = {yaw(yawAngle(input.head.q)), input.head.p};
        s.turn = 0;
        s.networkGeneration = networkGenerations.advance();
        pendingSelection[0] = pendingSelection[1] = -1;
        advanceGeneration(s);
        for (auto &gate : gates)
            gate = TriggerGate{};
        for (auto &w : wheels)
            w.cancel();
    }
    float ax = input.axis[1][0];
    bool turningAllowed = enabled && !resettingControls && !(input.buttons[1] & Button::Wheel);
    if (std::abs(ax) < .3f || !turningAllowed)
        turnLatched = false;
    if (turningAllowed && std::abs(ax) > .75f && !turnLatched) {
        s.turn += (ax > 0 ? -1 : 1) * Pi / 6;
        turnLatched = true;
        s.networkGeneration = networkGenerations.advance();
        advanceGeneration(s);
    }
    for (int h = 0; h < 2; h++) {
        if (!nativeRiderCurrent(p, rider))
            return;
        if (primaryInputGeneration[h] != input.primaryInputGeneration[h]) {
            primaryInputGeneration[h] = input.primaryInputGeneration[h];
            gates[h] = {};
        }
        refreshHand(s, p, h);
        void *weapon = s.handle[h] ? resolve(s.handle[h]) : nullptr;
        auto &ui = s.ui.wheel[h];
        for (int id = 0; id < int(WeaponCount); id++)
            if (id != 14 && inInventory(static_cast<uint8_t *>(p) + 0x8f8, id)) {
                ui.weapon[ui.count] = id;
                ui.ammo[ui.count] = ammo(static_cast<uint8_t *>(p) + 0x8f8, id);
                ui.count++;
            }
        bool valid = enabled && !resettingControls && input.handValid[h] && finite(input.hand[h]);
        bool wheelAllowed = valid && rider.handheld() && !(input.blockedWheels & (1u << h));
        int selected = wheels[h].update((input.buttons[h] & Button::Wheel) != 0, input.axis[h][0],
                                        input.axis[h][1], ui.weapon, ui.count, wheelAllowed);
        ui.open = wheels[h].open;
        ui.hover = wheels[h].hover;
        const bool fireAllowed = rider.seated() || (rider.handheld() && wheelAllowed &&
            !wheels[h].open && pendingSelection[h] < 0 && weapon != nullptr);
        s.fire[h] = gates[h].update(input.trigger[h], valid && primaryActionEligible(input, unsigned(h)) &&
            fireAllowed);
        // Native selection validates the inventory again and touches only the requested hand.
        if (!valid || !rider.handheld())
            pendingSelection[h] = -1;
        else if (selected >= 0) {
            pendingSelection[h] = selected;
            selectionTick[h] = GetTickCount64();
        }
        if (pendingSelection[h] >= 0 && !inInventory(static_cast<uint8_t *>(p) + 0x8f8, pendingSelection[h]))
            pendingSelection[h] = -1;
        if (pendingSelection[h] >= 0 && multiplayer::remoteClient() &&
            ((weapon &&
              *reinterpret_cast<int *>(static_cast<uint8_t *>(weapon) + 0xb4) == pendingSelection[h]) ||
             GetTickCount64() - selectionTick[h] > 1500))
            pendingSelection[h] = -1;
        if (pendingSelection[h] >= 0 && !multiplayer::remoteClient() && canChange(p, h) &&
            nativeRiderCurrent(p, rider)) {
            setWeapon(p, pendingSelection[h], h, 1);
            if (!nativeRiderCurrent(p, rider))
                return; // A native lifecycle callback superseded this transaction.
            pendingSelection[h] = -1;
            s.handle[h] = nativeHandle(p, h);

            gates[h] = TriggerGate{};
            s.fire[h] = false;
            clearCalibration(h);
            advanceGeneration(s);
        }
    }
    // Use existing dual-wield policy. Retry while native weapons cannot yet transition.
    if (enabled && rider.handheld() && !multiplayer::remoteClient() && !isDual(p) && !s.fire[0] && !s.fire[1] &&
        canChange(p, 0) && canChange(p, 1) && nativeRiderCurrent(p, rider))
        toggleDual(p, 0);
    if (!nativeRiderCurrent(p, rider))
        return;
    refreshOwnership(s, p);
    for (unsigned h = 0; h < 2; ++h) {
        void *weapon = s.handle[h] ? resolve(s.handle[h]) : nullptr;
        const bool sniper = enabled && rider.handheld() && !aliased(s) && nativeSniper(weapon);
        s.zoom[h] = zoomInputs[h].sample(input,h,
            sniper && !resettingControls && !(input.blockedWheels & (1u<<h)) && !s.ui.wheel[h].open && pendingSelection[h] < 0,
            s.handle[h],s.networkGeneration,inputProducer,GetTickCount64());
    }
    s.use = enabled && !resettingControls && ((input.buttons[0] | input.buttons[1]) & Button::Use);
    uint32_t contextualButtons = 0;
    for (unsigned h=0;h<2;++h)
        contextualButtons |= contextualSniperButtons(input,h,
            enabled && rider.handheld() && !aliased(s) && nativeSniper(s.handle[h] ? resolve(s.handle[h]) : nullptr));
    s.jump = enabled && !resettingControls && (contextualButtons & Button::Jump);
    s.sprint = enabled && !resettingControls && (contextualButtons & Button::Sprint);
    lastButtons[0] = input.buttons[0];
    lastButtons[1] = input.buttons[1];
    if (!singlePlayer()) {
        network::PosePacket packet;
        packet.trackingGeneration = s.networkGeneration;
        if (s.initialized && finite(input.head))
            packet.head = bodyHeadTracking(s.origin, s.turn, input.head);
        if (enabled)
            packet.validMask = 1;
        bool newPhysicalSample = input.sequence != physicalSequence;
        for (unsigned h = 0; h < 2; ++h) {
            const bool valid = enabled && input.handValid[h] && finite(input.hand[h]);
            packet.grip[h].p = packet.head.p; // Canonical inactive-hand pose shares the current rig origin.
            const Pose grip = rider.handheld() ? calibratedGrip(input, h, s.ui.currentWeapon[h], settings)
                                               : input.hand[h];
            if (s.initialized && valid && finite(grip))
                packet.grip[h] = bodyHandTracking(s.origin, s.turn, input.head, grip);
            if (valid)
                packet.validMask |= uint8_t(2u << h);
            const bool primaryNeutral = samplePrimaryNeutral(input.trigger[h],
                input.focused && input.handValid[h] && primaryActionEligible(input, h),
                newPhysicalSample, physicalDown[h], releasedSerial[h]);
            if (valid && primaryNeutral)
                packet.primaryNeutralSampleMask |= uint8_t(1u << h);
            if (physicalDown[h])
                packet.physicalDownMask |= uint8_t(1u << h);
            if (s.fire[h] && physicalDown[h] && valid)
                packet.fireMask |= uint8_t(1u << h);
            if (!valid || s.ui.wheel[h].open || pendingSelection[h] >= 0 || recenterHeld(input))
                packet.wheelOrEquipBlockedMask |= uint8_t(1u << h);
            packet.releasedSerial[h] = releasedSerial[h];
            packet.primaryInputGeneration[h] = input.primaryInputGeneration[h];
            packet.zoomInputGeneration[h] = input.zoomInputGeneration[h];
            if (newPhysicalSample && input.focused && input.handValid[h] &&
                (input.zoomActiveMask & (1u << h)) && input.zoomInputGeneration[h] &&
                !(input.zoomDownMask & (1u << h)) && zoomReleasedSerial[h] != UINT32_MAX)
                ++zoomReleasedSerial[h];
            if (input.zoomDownMask & (1u << h))
                packet.zoomPhysicalDownMask |= uint8_t(1u << h);
            if (valid && primaryActionEligible(input, h))
                packet.primarySampleEligibleMask |= uint8_t(1u << h);
            if (valid && (input.zoomActiveMask & (1u << h)) && input.zoomInputGeneration[h])
                packet.zoomSampleEligibleMask |= uint8_t(1u << h);
            packet.zoomReleasedSerial[h] = zoomReleasedSerial[h];
            if (s.zoom[h] && valid)
                packet.zoomMask |= uint8_t(1u << h);
            packet.requestedWeapon[h] = int16_t(pendingSelection[h]);
            packet.nativeWeaponId[h] = int16_t(s.ui.currentWeapon[h]);
        }
        if (!rider.handheld())
            suppressHandheld(packet);
        physicalSequence = input.sequence;
        bool edge = packet.physicalDownMask != lastNetworkPose.physicalDownMask ||
                    packet.fireMask != lastNetworkPose.fireMask ||
                    packet.zoomMask != lastNetworkPose.zoomMask ||
                    packet.trackingGeneration != lastNetworkPose.trackingGeneration ||
                    packet.requestedWeapon[0] != lastNetworkPose.requestedWeapon[0] ||
                    packet.requestedWeapon[1] != lastNetworkPose.requestedWeapon[1];
        if (multiplayer::submit(p, packet, edge, input.sequence, input.tickMs))
            lastNetworkPose = packet;
        for (unsigned h = 0; h < 2; ++h) {
            if (s.intentEpoch[h]!=packet.intentEpoch[h]) {
                s.zoom[h]=false;
                zoomInputs[h].sample(input,h,false,s.handle[h],s.networkGeneration,inputProducer,GetTickCount64());
            }
            s.intentEpoch[h] = packet.intentEpoch[h];
            if (rider.handheld() && multiplayer::remoteClient())
                s.fire[h] = s.fire[h] && (packet.fireMask & (1u << h));
            if (rider.handheld() && multiplayer::remoteClient())
                s.zoom[h] = s.zoom[h] && (packet.zoomMask & (1u << h));
        }
    }
    publishSnapshot(s);
}
struct PreparedPlayer {
    uint32_t handle = 0;
    void *subject = nullptr;
    bool recognized = false, revoked = false, prepared = false, authoritative = false;
    NativePrimaryValue primary{};
    RiderIdentity rider;
    uint32_t brainHandle = 0, weaponHandle[2]{}, gameHandle = 0;
    void *brain = nullptr, *weapon[2]{}, *game = nullptr;
    int button[2]{-1, -1}, combo = 0, dual = 0, flip = 0;
    uint8_t hands = 0;
    // Identity of the EXISTING snapshot/frozen authority, not another pose sample.
    uint64_t sequence = 0, receivedMs = 0, clientNonce = 0, serverNonce = 0;
    uint32_t generation = 0, trackingGeneration = 0, producer = 0, session = 0, reference = 0;
    uint32_t incarnation = 0, intentEpoch[2]{}, primaryGeneration[2]{};
};
struct SimulationInterval {
    void *simulation;
    bool managerPrepared = false, networkPrepared = false;
    std::array<PreparedPlayer, 18> preparedPlayers{};
    SimulationInterval *previous;
    bool failed = false;
    bool *failure = nullptr;
};
static thread_local SimulationInterval *simulationInterval = nullptr;
static thread_local const NativePrimaryInvocation *primaryInvocation = nullptr;
static PreparedPlayer *preparedPrimary(void *subject) noexcept {
    for (auto *interval = simulationInterval; interval; interval = interval->previous)
        for (auto &entry : interval->preparedPlayers)
            if (entry.subject == subject) return &entry;
    return nullptr;
}
static void revokePrimary(void *subject) noexcept {
    // At deletion/carry invalidation, before gameplay getters or callbacks. Active invocation
    // copies remain immutable; only subsequent independent admissions are lost.
    for (auto *interval = simulationInterval; interval; interval = interval->previous)
        for (auto &entry : interval->preparedPlayers)
            if (entry.subject == subject || entry.weapon[0] == subject || entry.weapon[1] == subject)
                nativePrimaryRevoke(entry.primary, entry.revoked);
}
static uint32_t primaryField(const void *object, unsigned offset) noexcept {
    uint32_t value;
    memcpy(&value, static_cast<const uint8_t *>(object) + offset, sizeof(value));
    return value;
}
static bool primaryBindingsCurrent(const PreparedPlayer &entry) {
    // Handle conversion is the existing Core identity seam. All engine getters
    // are outside scalar helpers and are followed by these identity checks.
    if (entry.revoked || !entry.handle || resolve(entry.handle) != entry.subject ||
        entry.revoked || !entry.brainHandle || !entry.brain || resolve(entry.brainHandle) != entry.brain ||
        entry.revoked || !nativeRiderCurrent(entry.subject, entry.rider) || !entry.rider.handheld() ||
        entry.revoked || primaryField(entry.subject, 0x38c) != entry.brainHandle ||
        primaryField(entry.brain, 0x28) != entry.handle || primaryField(entry.subject, 0x564) ||
        (primaryField(entry.subject, 0x10) & 2)) return false;
    for (unsigned hand = 0; hand < 2; ++hand) {
        if (nativeHandle(entry.subject, hand) != entry.weaponHandle[hand]) return false;
        if (!entry.weaponHandle[hand]) continue;
        if (resolve(entry.weaponHandle[hand]) != entry.weapon[hand] || entry.revoked ||
            !entry.weapon[hand] || primaryField(entry.weapon[hand], 0x28) != entry.handle ||
            primaryField(entry.weapon[hand], 0xbc) != hand ||
            (primaryField(entry.weapon[hand], 0x10) & 2)) return false;
    }
    return !entry.revoked;
}
static bool primaryTopology(PreparedPlayer &entry, bool capture) {
    if (!primaryBindingsCurrent(entry) || !isAlive(entry.subject) || entry.revoked ||
        !primaryBindingsCurrent(entry)) return false;
    uint32_t gameHandle = 0;
    nativeGameInfo(&gameHandle);
    if (!primaryBindingsCurrent(entry)) return false;
    void *game = gameHandle ? resolve(gameHandle) : nullptr;
    if (!game || entry.revoked) return false;
    if (!capture && (entry.gameHandle != gameHandle || entry.game != game)) return false;
    const int combo = nativeComboWeapons(game);
    if (!primaryBindingsCurrent(entry) || resolve(gameHandle) != game) return false;
    const int dual = isDual(entry.subject);
    if (!primaryBindingsCurrent(entry)) return false;
    const int flip = nativeFlipButtons(entry.subject);
    if (!primaryBindingsCurrent(entry)) return false;
    int button[2]{-1, -1};
    for (unsigned hand = 0; hand < 2; ++hand) {
        if (!entry.weaponHandle[hand]) continue;
        button[hand] = nativeWeaponButton(entry.subject, entry.weaponHandle[hand]);
        if (!primaryBindingsCurrent(entry) || button[hand] < 0 || button[hand] > 1) return false;
    }
    uint32_t liveGame = 0;
    nativeGameInfo(&liveGame);
    if (entry.revoked || liveGame != gameHandle || resolve(gameHandle) != game ||
        !primaryBindingsCurrent(entry) ||
        (entry.hands == 3 && (!combo || !dual || button[0] == button[1]))) return false;
    if (capture) {
        entry.gameHandle = gameHandle; entry.game = game;
        entry.combo = combo; entry.dual = dual; entry.flip = flip;
        entry.button[0] = button[0]; entry.button[1] = button[1];
        return true;
    }
    // Validation only: never remap an already captured high.
    return entry.combo == combo && entry.dual == dual && entry.flip == flip &&
           entry.button[0] == button[0] && entry.button[1] == button[1];
}
static uint8_t primarySourceHands(PreparedPlayer &entry, bool capture) {
    if (entry.revoked) return 0;
    uint8_t fire = 0;
    if (entry.authoritative) {
        if (singlePlayer() || !multiplayer::server() || entry.revoked) return 0;
        const auto source = copyAuthority(entry.subject);
        const auto live = multiplayer::authority(entry.subject);
        if (entry.revoked || source.player != entry.handle || source.rider != entry.rider ||
            source.sample.avatar != entry.handle || !source.sample.negotiated || !live.negotiated) return 0;
        const auto &sample = source.sample;
        if (capture) {
            entry.sequence = sample.pose.sequence; entry.receivedMs = sample.receivedMs;
            entry.clientNonce = sample.pose.clientNonce; entry.serverNonce = sample.pose.serverNonce;
            entry.incarnation = sample.incarnation; entry.generation = source.weaponGeneration;
            entry.trackingGeneration = sample.pose.trackingGeneration;
            for (unsigned hand = 0; hand < 2; ++hand) entry.intentEpoch[hand] = sample.pose.intentEpoch[hand];
        }
        if (entry.sequence != sample.pose.sequence || entry.receivedMs != sample.receivedMs ||
            entry.clientNonce != sample.pose.clientNonce || entry.serverNonce != sample.pose.serverNonce ||
            entry.incarnation != sample.incarnation || entry.generation != source.weaponGeneration ||
            entry.trackingGeneration != sample.pose.trackingGeneration) return 0;
        for (unsigned hand = 0; hand < 2; ++hand)
            if (source.hand[hand] == entry.weaponHandle[hand] &&
                entry.intentEpoch[hand] == sample.pose.intentEpoch[hand] &&
                currentWeaponSample(sample, live, hand) &&
                (sample.pose.fireMask & live.pose.fireMask & (1u << hand))) fire |= uint8_t(1u << hand);
    } else {
        if (!primaryBindingsCurrent(entry) || !local(entry.subject) ||
            !primaryBindingsCurrent(entry)) return 0;
        const auto source = copySnapshot();
        if (source.player != entry.subject || source.playerHandle != entry.handle ||
            source.rider != entry.rider || !vrSession(source) || !fresh(source.input) ||
            !source.ui.gameplay || recenterHeld(source.input)) return 0;
        if (capture) {
            entry.sequence = source.input.sequence; entry.receivedMs = source.input.tickMs;
            entry.generation = source.generation; entry.producer = source.inputProducer;
            entry.session = source.input.session; entry.reference = source.input.reference;
            for (unsigned hand = 0; hand < 2; ++hand) {
                entry.intentEpoch[hand] = source.intentEpoch[hand];
                entry.primaryGeneration[hand] = source.input.primaryInputGeneration[hand];
            }
        }
        if (entry.sequence != source.input.sequence || entry.receivedMs != source.input.tickMs ||
            entry.generation != source.generation || entry.producer != source.inputProducer ||
            entry.session != source.input.session || entry.reference != source.input.reference) return 0;
        for (unsigned hand = 0; hand < 2; ++hand)
            if (source.handle[hand] == entry.weaponHandle[hand] && source.fire[hand] &&
                source.input.handValid[hand] && finite(source.input.hand[hand]) &&
                !source.selecting[hand] && !source.ui.wheel[hand].open &&
                entry.intentEpoch[hand] == source.intentEpoch[hand] &&
                entry.primaryGeneration[hand] == source.input.primaryInputGeneration[hand] &&
                (!multiplayer::remoteClient() || multiplayer::localPrimaryAllowed(entry.subject, hand,
                    entry.intentEpoch[hand], entry.primaryGeneration[hand], entry.sequence)))
                fire |= uint8_t(1u << hand);
    }
    return entry.revoked ? 0 : fire;
}
static bool primaryWeaponReferences(PreparedPlayer &entry, uint8_t fire) {
    if (!entry.authoritative) return true;
    for (unsigned hand = 0; hand < 2; ++hand) {
        if (!(fire & entry.hands & (1u << hand))) continue;
        Pose body, camera, model;
        if (!primaryBindingsCurrent(entry) ||
            !nativeWeaponReference(entry.subject, entry.weapon[hand], body, camera, model) ||
            !primaryBindingsCurrent(entry)) return false;
    }
    return true;
}
static void preparePrimary(PreparedPlayer &entry, bool authoritative) {
    // Recognition/neutral were reserved before update/authorityStep. Only this
    // initial preparation can introduce high; native reentry sees that neutral.
    if (!entry.recognized || entry.revoked || entry.prepared) return;
    entry.prepared = true; // One attempt, including failed/neutral preparation.
    if (!entry.handle || resolve(entry.handle) != entry.subject || entry.revoked) return;
    entry.authoritative = authoritative;
    entry.brainHandle = primaryField(entry.subject, 0x38c);
    entry.brain = entry.brainHandle ? resolve(entry.brainHandle) : nullptr;
    if (entry.revoked || !readNativeRider(entry.subject, entry.rider) || entry.revoked) return;
    for (unsigned hand = 0; hand < 2; ++hand) {
        entry.weaponHandle[hand] = nativeHandle(entry.subject, hand);
        entry.weapon[hand] = entry.weaponHandle[hand] ? resolve(entry.weaponHandle[hand]) : nullptr;
        if (entry.weaponHandle[hand]) entry.hands |= uint8_t(1u << hand);
    }
    if (!entry.hands || (entry.hands == 3 && entry.weapon[0] == entry.weapon[1]) ||
        !primaryBindingsCurrent(entry)) return;
    const uint8_t fire = primarySourceHands(entry, true) & entry.hands;
    if (!fire) return; // Neutral needs no native mapping query and cannot rise later.
    if (!primaryTopology(entry, true) || !primaryWeaponReferences(entry, fire) ||
        !nativeInputHealthy() || !primaryBindingsCurrent(entry)) return;
    const uint8_t liveFire = primarySourceHands(entry, false);
    if (!nativeInputHealthy() || !primaryBindingsCurrent(entry)) return;
    entry.primary = nativePrimaryProjection(entry.hands, fire & liveFire,
        entry.button[0], entry.button[1], entry.combo && entry.dual);
    if (entry.revoked) entry.primary.bits = 0;
}
static void refreshPrimary(PreparedPlayer &entry) {
    if (!entry.prepared || entry.revoked || !entry.primary.bits) return;
    if (!primaryBindingsCurrent(entry)) {
        nativePrimaryRevoke(entry.primary, entry.revoked);
        return;
    }
    const uint8_t fire = primarySourceHands(entry, false);
    if (!primaryTopology(entry, false) || !primaryWeaponReferences(entry, fire) ||
        !nativeInputHealthy() || !primaryBindingsCurrent(entry)) {
        nativePrimaryRevoke(entry.primary, entry.revoked);
        return;
    }
    const uint8_t liveFire = primarySourceHands(entry, false);
    if (!nativeInputHealthy() || !primaryBindingsCurrent(entry)) {
        nativePrimaryRevoke(entry.primary, entry.revoked);
        return;
    }
    entry.primary.bits &= nativePrimaryProjection(entry.hands, fire & liveFire,
        entry.button[0], entry.button[1], entry.combo && entry.dual).bits;
    if (entry.revoked) entry.primary.bits = 0;
}
static NativePrimaryValue primaryAdmission(void *subject, uintptr_t caller, bool held,
                                          NativePrimaryValue &reserved) {
    // The wrapper has already honored an immutable same-pawn ancestor. For a
    // new invocation, establish CURRENT carry ownership before any validation
    // getter or recognized-neutral reservation, even after interval preparation.
    // Native 8E6C0/8E6F6 resolves +564 before choosing prepare/throw callbacks.
    const uint32_t carryHandle = primaryField(subject, 0x564);
    const bool resolvedCarry = carryHandle && resolve(carryHandle);
    if (nativePrimaryOwnership(false, resolvedCarry) == NativePrimaryOwnership::Carry) {
        // Passthrough belongs to this carry invocation, not the prepared record.
        // Carry ending later in this interval must not resurrect its old high.
        revokePrimary(subject);
        return {};
    }
    auto *entry = preparedPrimary(subject);
    const auto snapshot = copySnapshot();
    const uint32_t handle = entry ? entry->handle : pointerHandle(subject);
    bool recognized = (entry && entry->recognized) || primaryLocalRecognized(snapshot, subject, handle);
    // No native game callbacks in knownVrAvatar. Handle conversion does not
    // acquire gameplay state or sample input.
    if (!recognized) recognized = multiplayer::knownVrAvatar(handle);
    if (nativePrimaryOwnership(recognized, resolvedCarry) != NativePrimaryOwnership::Handheld) return {};
    // Recognition is the boundary: reentry before it remains unclaimed; every
    // gameplay getter below sees this recognized-neutral reservation instead.
    reserved = NativePrimaryValue::neutral();
    auto value = reserved;
    if (entry) entry->recognized = true;
    const auto *frame = zoomManagerFrame;
    if (!entry || !entry->prepared || entry->revoked || !nativeInputHealthy() ||
        caller != (held ? primaryHeldReturn : primaryOperatorReturn) ||
        !frame || frame->previous || !frame->current || !frame->execution || frame->preparation ||
        !simulationInterval || simulationInterval->previous || !hooksReady.load(std::memory_order_acquire) ||
        !nativeMainThread || !nativeMainThread() || currentSimulation() != frame->simulation ||
        currentWorld() != frame->world) return value;
    refreshPrimary(*entry);
    if (!entry->revoked && nativeInputHealthy() && currentSimulation() == frame->simulation &&
        currentWorld() == frame->world && !entry->revoked) value = entry->primary;
    return value;
}
static void __fastcall operatorFiring(void *subject, void *) {
#ifdef _MSC_VER
    const auto caller = reinterpret_cast<uintptr_t>(_ReturnAddress());
#else
    const auto caller = reinterpret_cast<uintptr_t>(__builtin_return_address(0));
#endif
    const auto *inherited = nativePrimaryInherited(primaryInvocation, subject);
    NativePrimaryInvocation frame{subject, inherited ? inherited->value : NativePrimaryValue{},
                                  primaryInvocation, false};
    primaryInvocation = &frame; // Unclaimed until admission establishes VR ownership.
    withNativeFinally([&] {
        if (!inherited) frame.value = primaryAdmission(subject, caller, false, frame.value);
        originalOperatorFiring(subject);
    }, [&](bool aborted) noexcept {
        primaryInvocation = frame.previous;
        if (aborted) nativeInputFailed();
    });
}
static int __fastcall fireButtonPressed(void *subject, void *, int index) {
#ifdef _MSC_VER
    const auto caller = reinterpret_cast<uintptr_t>(_ReturnAddress());
#else
    const auto caller = reinterpret_cast<uintptr_t>(__builtin_return_address(0));
#endif
    const auto *inherited = nativePrimaryInherited(primaryInvocation, subject);
    NativePrimaryInvocation frame{subject, inherited ? inherited->value : NativePrimaryValue{},
                                  primaryInvocation, true};
    primaryInvocation = &frame;
    int result = 0;
    withNativeFinally([&] {
        if (!inherited) frame.value = primaryAdmission(subject, caller, true, frame.value);
        result = originalFireButtonPressed(subject, index);
    }, [&](bool aborted) noexcept {
        primaryInvocation = frame.previous;
        if (aborted) nativeInputFailed();
    });
    return result;
}
extern "C" int __cdecl ss2vrPrimaryPredicate(int original, void *subject, unsigned kind) noexcept {
    // Scalar only: no native dereference/getter, lock, allocation or exception.
    return int(nativePrimaryRead(uint32_t(original), subject, kind, primaryInvocation));
}
SS2VR_X86_PREDICATE_ENTRY(primaryDownPredicate,ss2vrPrimaryPredicate,0,4,0)
SS2VR_X86_PREDICATE_ENTRY(primaryPressPredicate,ss2vrPrimaryPredicate,24,4,1)
SS2VR_X86_PREDICATE_ENTRY(primaryReleasePredicate,ss2vrPrimaryPredicate,24,4,2)
SS2VR_X86_PREDICATE_ENTRY(primaryHistoryPredicate,ss2vrPrimaryPredicate,24,4,3)
SS2VR_X86_PREDICATE_ENTRY(primaryHeldPredicate,ss2vrPrimaryPredicate,28,4,4)
void nativeInputFailed() noexcept {
    if (simulationInterval && simulationInterval->failure)
        *simulationInterval->failure = true;
}
bool nativeInputHealthy() noexcept {
    return !simulationInterval || !simulationInterval->failure || !*simulationInterval->failure;
}
static void invalidateInterruptedInput() noexcept {
    // Snapshot metadata only: no IPC mutex, native getter or game callback in
    // an unwind tail. Clearing session forces ordinary update() to rebuild its
    // existing controls/neutral gates from a later input sample.
    AcquireSRWLockExclusive(&snapshotLock);
    current.interruption = {current.input.session, current.input.reference,
                            current.input.sequence, GetTickCount64(), current.inputProducer};
    current.generation = advanceEpochUnlocked();
    current.ui.trackingGeneration = current.generation;
    current.input.session = current.input.focused = 0;
    current.fire[0] = current.fire[1] = false;
    current.zoom[0] = current.zoom[1] = false;
    current.ui.gameplay = 0;
    ReleaseSRWLockExclusive(&snapshotLock);
}
static void __fastcall simulationStep(void *simulation, void *) {
    if (!hooksReady.load(std::memory_order_acquire)) {
        originalSimulationStep(simulation);
        return;
    }
    SimulationInterval interval{simulation, false, false, {}, simulationInterval};
    interval.failure = interval.previous ? interval.previous->failure : &interval.failed;
    simulationInterval = &interval;
    withNativeFinally([&] {
        remote_render::noteSimulationThread();
        originalSimulationStep(simulation);
        // Retire input only after all original entity/script/physics work.
        if (interval.networkPrepared && nativeInputHealthy())
            multiplayer::completeTick();
    }, [&](bool aborted) noexcept {
        if (aborted)
            *interval.failure = true;
        simulationInterval = interval.previous;
        if (*interval.failure && !interval.previous)
            invalidateInterruptedInput();
        if (*interval.failure && interval.networkPrepared)
            multiplayer::abortTick();
    });
}
static void __fastcall entityStep(void *manager, void *) {
    if (!hooksReady.load(std::memory_order_acquire)) {
        originalEntityStep(manager);
        return;
    }
    // Lexical nesting spans BOTH preparation and original execution. Admission
    // pointers cannot be used as nesting witnesses (A -> B -> A).
    ZoomManagerFrame frame{manager,simulationInterval ? simulationInterval->simulation:nullptr,
                           nullptr,zoomManagerFrame};
    zoomManagerFrame=&frame;
    withNativeFinally([&] {
        // Existing local ownership is recognizable before even the world/game
        // getters. Remote recognition also has the callback-free MP lookup in
        // primaryAdmission until enumeration reserves its interval slot.
        if (simulationInterval && !simulationInterval->previous && !simulationInterval->managerPrepared) {
            const auto snapshot = copySnapshot();
            if (primaryLocalRecognized(snapshot, snapshot.player, snapshot.playerHandle) &&
                !preparedPrimary(snapshot.player))
                for (auto &entry : simulationInterval->preparedPlayers)
                    if (!entry.subject) {
                        entry.subject = snapshot.player; entry.handle = snapshot.playerHandle;
                        entry.recognized = true; entry.primary = NativePrimaryValue::neutral();
                        break;
                    }
        }
        frame.world=currentWorld();
        uint32_t expected=0;
        if(frame.world) memcpy(&expected,static_cast<uint8_t *>(frame.world)+0x74,4);
        frame.current=!frame.previous && simulationInterval && !simulationInterval->previous &&
            currentSimulation()==frame.simulation && reinterpret_cast<uintptr_t>(manager)==expected &&
            nativeMainThread && nativeMainThread();
        frame.preparation=frame.current && !simulationInterval->managerPrepared;
        withNativeFinally([&] {
            auto interval = simulationInterval;
            void *world = currentWorld();
            uint32_t nativeManager = 0;
            if (world)
                std::memcpy(&nativeManager, static_cast<uint8_t *>(world) + 0x74, sizeof(nativeManager));
            if (frame.current && nativeInputHealthy() && interval && !interval->previous && !interval->managerPrepared &&
                nativeMainThread && nativeMainThread() &&
                currentSimulation() == interval->simulation && reinterpret_cast<uintptr_t>(manager) == nativeManager) {
                // CSimulation has finished world changes and advanced native time before
                // entering this entity manager. Freeze every player before ANY weapon's
                // OnStep, regardless of the native entity ordering within the loop.
                interval->managerPrepared = true;
                interval->networkPrepared = !singlePlayer() && multiplayer::server() && multiplayer::beginTick();
                const auto players = multiplayer::activePlayers();
                const auto snapshot = copySnapshot();
                // Reserve ALL subjects before any update, freeze or gameplay
                // getter. Reentered OnStep must not update/freeze them twice.
                for (auto player : players) {
                    if (!player) continue;
                    auto *entry = preparedPrimary(player);
                    if (!entry)
                        for (auto &candidate : interval->preparedPlayers)
                            if (!candidate.subject) { entry = &candidate; entry->subject = player; break; }
                    if (!entry) continue;
                    const auto handle = pointerHandle(player);
                    if (entry->handle && entry->handle != handle) entry->revoked = true;
                    else entry->handle = handle;
                    entry->recognized |= primaryLocalRecognized(snapshot, player, handle) ||
                                         multiplayer::knownVrAvatar(handle);
                    if (entry->recognized) entry->primary = NativePrimaryValue::neutral();
                }
                for (auto player : players)
                    if (player) {
                        auto *entry = preparedPrimary(player);
                        if (!entry || entry->revoked || entry->prepared) continue;
                        const auto handle = pointerHandle(player);
                        if (!handle || resolve(handle) != player)
                            continue;
                        const bool localPlayer = local(player);
                        if (!interval->networkPrepared && !localPlayer)
                            continue;
                        if (localPlayer) {
                            update(player);
                            // Read the published existing snapshot, not Input:
                            // first successful tracking can establish ownership.
                            entry->recognized |= primaryLocalRecognized(copySnapshot(), player, handle);
                            if (entry->recognized) entry->primary = NativePrimaryValue::neutral();
                        }
                        // Only a successfully opened authoritative interval may freeze
                        // network input. Local preparation never owns an ACK/tick.
                        if (interval->networkPrepared)
                            authorityStep(player);
                        entry->recognized |= multiplayer::knownVrAvatar(handle);
                        if (entry->recognized) entry->primary = NativePrimaryValue::neutral();
                        preparePrimary(*entry, interval->networkPrepared &&
                            multiplayer::authority(player).negotiated);
                    }
            }
        }, [&](bool aborted) noexcept {
            if (aborted)
                nativeInputFailed();
        });
        frame.preparation=false;
        frame.execution=frame.current && simulationInterval && simulationInterval->managerPrepared;
        originalEntityStep(manager);
    },[&](bool aborted) noexcept {
        zoomManagerFrame=frame.previous;
        if(aborted) nativeInputFailed();
    });
}
// Native zoom owns every flag, timer, damage value and sound. These bounded
// records retain only adapter provenance across copied input/authority state.
using ZoomClaim=NativeZoomClaim;
static NativeZoomClaims zoomClaims;
static SRWLOCK zoomLock = SRWLOCK_INIT;
enum class ZoomOperation { Step, Fire, Activate, Deactivate, PutDown, Delete, ExternalActivate };
struct ZoomContext {
    void *weapon;
    ZoomOperation operation;
    ZoomClaim claim;
    ZoomContext *previous;
    bool revoked = false, sourceRevoked = false;
    NativeZoomDeleteRouting deleteRouting;
};
static thread_local ZoomContext *zoomContext = nullptr;
static ZoomClaim zoomClaim(void *weapon) noexcept {
    ZoomClaim result;
    AcquireSRWLockShared(&zoomLock);
    result=zoomClaims.find(weapon);
    ReleaseSRWLockShared(&zoomLock);
    return result;
}
static bool zoomToken(const ZoomContext &context,ZoomClaim &out) noexcept {
    if(context.revoked || !context.claim.serial) return false;
    out=zoomClaim(context.weapon);
    return out.serial==context.claim.serial && out.revision==context.claim.revision;
}
static void zoomRevokeFrames(void *weapon,bool source=false) noexcept {
    for(auto *frame=zoomContext;frame;frame=frame->previous)
        if(frame->weapon==weapon) {
            frame->revoked=true;
            frame->sourceRevoked|=source;
        }
}
static void zoomChange(uint64_t serial,bool deletion,bool takeover,bool abort) noexcept {
    AcquireSRWLockExclusive(&zoomLock);
    zoomClaims.change(serial,deletion,takeover,abort);
    ReleaseSRWLockExclusive(&zoomLock);
}
static void zoomMarkDeleting(uint64_t serial) noexcept {
    // Caller captures subject before setting the deletion context.
    AcquireSRWLockExclusive(&zoomLock);
    zoomClaims.change(serial,true,false,false);
    ReleaseSRWLockExclusive(&zoomLock);
}
static void zoomRetire(uint64_t serial) noexcept {
    AcquireSRWLockExclusive(&zoomLock);
    zoomClaims.sourceDeleted(serial);
    ReleaseSRWLockExclusive(&zoomLock);
}
static void zoomOwnerDeleting(void *owner) noexcept {
    AcquireSRWLockExclusive(&zoomLock);
    zoomClaims.ownerDeleting(owner);
    for(auto *frame=zoomContext;frame;frame=frame->previous)
        if(frame->claim.owner==owner) frame->revoked=true;
    ReleaseSRWLockExclusive(&zoomLock);
}
static ZoomClaim zoomReserve(void *weapon,void *owner,uint32_t wh,uint32_t oh,unsigned hand) noexcept {
    ZoomClaim result;
    AcquireSRWLockExclusive(&zoomLock);
    result=zoomClaims.reserve(weapon,owner,wh,oh,hand);
    ReleaseSRWLockExclusive(&zoomLock);
    return result; // Exhaustion never evicts an outstanding cleanup obligation.
}
static bool zoomBorrowed(ZoomContext &context,ZoomClaim &claim) {
    // Metadata FIRST: a nested native deletion may already have freed weapon.
    if(!zoomToken(context,claim) || claim.deleting || context.revoked)
        return false;
    if(!claim.weaponHandle || resolve(claim.weaponHandle)!=context.weapon || !nativeSniper(context.weapon))
        return false;
    uint32_t deleted=0;
    memcpy(&deleted,static_cast<uint8_t *>(context.weapon)+0x10,4);
    return !(deleted&2); // Callback-specific resource admission, not an allocation pin.
}
struct ZoomIntent { bool admitted=false, desired=false; void *owner=nullptr; uint32_t wh=0,oh=0; unsigned hand=2; };
static ZoomIntent zoomIntent(ZoomContext &context,NativeZoomQuery purpose=NativeZoomQuery::Effect) {
    ZoomIntent out;
    if(context.revoked) return out;
    void *weapon=context.weapon;
    const auto *frame=zoomManagerFrame;
    if(!frame || !nativeZoomPhaseAllowed(frame->previous!=nullptr,frame->current,frame->preparation,
        frame->execution,purpose) || !simulationInterval || simulationInterval->previous ||
        currentSimulation()!=frame->simulation || currentWorld()!=frame->world || !nativeInputHealthy() ||
        !hooksReady.load(std::memory_order_acquire) || !nativeMainThread || !nativeMainThread() ||
        !nativeSniper(weapon)) return out;
    out.wh=pointerHandle(weapon);
    if(context.revoked) return {};
    memcpy(&out.oh,static_cast<uint8_t *>(weapon)+0x28,4);
    out.owner=out.oh ? resolve(out.oh):nullptr;
    if(!out.wh || resolve(out.wh)!=weapon || !out.owner) return out;
    RiderIdentity rider;
    if(!readNativeRider(out.owner,rider) || !rider.handheld() || !isAlive(out.owner) || thirdPerson(out.owner) ||
        context.revoked || !nativeOwnWeapons(out.owner) || context.revoked || resolve(out.wh)!=weapon ||
        resolve(out.oh)!=out.owner) return out;
    uint32_t actualOwner=0,ownerView=0;
    memcpy(&actualOwner,static_cast<uint8_t *>(weapon)+0x28,4);
    memcpy(&ownerView,static_cast<uint8_t *>(out.owner)+0x558,4);
    if(actualOwner!=out.oh || ownerView!=0) return {};
    const uint32_t left=nativeHandle(out.owner,0),right=nativeHandle(out.owner,1);
    if(left && left==right) return out;
    out.hand=left==out.wh ? 0u:right==out.wh ? 1u:2u;
    if(out.hand>=2) return out;
    if(!singlePlayer() && multiplayer::server()) {
        const auto authority=copyAuthority(out.owner);
        const auto live=multiplayer::authority(out.owner);
        if(live.negotiated && !(purpose==NativeZoomQuery::PreserveCrossDelete && frame->preparation &&
            frame->frozenOwner!=out.oh)) {
            if(purpose==NativeZoomQuery::PreserveCrossDelete && frame->preparation) {
                out.admitted=currentWeaponSample(live,live,out.hand);
                out.desired=out.admitted && (network::intervalZoomMask(live.pose)&(1u<<out.hand));
                return out;
            }
            out.admitted=authority.hand[out.hand]==out.wh && authority.rider==rider &&
                currentWeaponSample(authority.sample,live,out.hand);
            out.desired=out.admitted && (network::intervalZoomMask(live.pose)&(1u<<out.hand));
            return out;
        }
    }
    const auto snapshot=copySnapshot();
    out.admitted=out.owner==snapshot.player && out.oh==snapshot.playerHandle && livePlayer(snapshot) &&
        handOf(weapon,snapshot)==int(out.hand) && vrSession(snapshot) && fresh(snapshot.input) &&
        snapshot.ui.gameplay && !recenterHeld(snapshot.input) && !snapshot.selecting[out.hand] &&
        !snapshot.ui.wheel[out.hand].open && snapshot.input.handValid[out.hand] && finite(snapshot.input.hand[out.hand]);
    if(out.admitted && multiplayer::remoteClient())
        out.admitted=multiplayer::localZoomAllowed(out.owner,out.hand,snapshot.intentEpoch[out.hand],
            snapshot.input.zoomInputGeneration[out.hand],snapshot.input.sequence);
    out.desired=out.admitted && snapshot.zoom[out.hand];
    return out;
}
static bool zoomState(void *weapon,bool active) {
    uint32_t deleted=0;int state=0;
    memcpy(&deleted,static_cast<uint8_t *>(weapon)+0x10,4);
    memcpy(&state,static_cast<uint8_t *>(weapon)+0xb0,4);
    return nativeZoomStateAllowed(state,active,deleted);
}
extern "C" int __cdecl ss2vrZoomPredicate(int original,void *weapon,unsigned kind) noexcept {
    // No engine callbacks/allocations or subject dereference in a naked helper.
    auto *context=zoomContext;
    if(!context || context->weapon!=weapon) return original;
    const auto live=zoomClaim(weapon);
    if(context->operation==ZoomOperation::Step) {
        const int result=nativeZoomStepPredicate(kind,original,context->claim,live,
                                                 context->revoked,context->sourceRevoked);
        if(kind==4 && result && context->claim.serial && context->claim.serial==live.serial &&
            context->claim.revision==live.revision && live.effect && live.hand==0) {
            AcquireSRWLockExclusive(&zoomLock);
            zoomClaims.audioStarted(live.serial,live.revision);
            ReleaseSRWLockExclusive(&zoomLock);
        }
        return result;
    }
    ZoomClaim claim;
    if(!zoomToken(*context,claim)) return original;
    const bool cleanup=context->operation==ZoomOperation::Deactivate;
    if(kind==0) return context->operation==ZoomOperation::Activate && claim.effect ? 1:original;
    if(kind==1) return cleanup && claim.effect ? 0:original;
    if(kind==2) return cleanup && claim.effect && claim.hand==0 ? 1:original;
    return original;
}
SS2VR_X86_PREDICATE_ENTRY(zoomActivatePredicate,ss2vrZoomPredicate,28,4,0)
SS2VR_X86_PREDICATE_ENTRY(zoomOwnerPredicate,ss2vrZoomPredicate,28,4,1)
SS2VR_X86_PREDICATE_ENTRY(zoomFovPredicate,ss2vrZoomPredicate,28,4,2)
SS2VR_X86_PREDICATE_ENTRY(zoomInterpolatePredicate,ss2vrZoomPredicate,28,4,3)
SS2VR_X86_PREDICATE_ENTRY(zoomSoundStartPredicate,ss2vrZoomPredicate,24,4,4)
SS2VR_X86_PREDICATE_ENTRY(zoomSoundStopPredicate,ss2vrZoomPredicate,28,4,5)
static void zoomNative(ZoomContext &parent,bool activate) {
    ZoomContext operation{parent.weapon,activate ? ZoomOperation::Activate:ZoomOperation::Deactivate,
                          parent.claim,zoomContext};
    zoomContext=&operation;
    withNativeFinally([&] {
        if(activate) originalZoomActivate(parent.weapon);
        else originalZoomDeactivate(parent.weapon);
    },[&](bool aborted) noexcept {
        zoomContext=operation.previous;
        if(aborted) {
            zoomChange(operation.claim.serial,false,false,true);
            zoomRevokeFrames(operation.weapon);
            nativeInputFailed();
        }
    });
}
static void zoomReconcile(ZoomContext &context,bool activation) {
    if(context.revoked) return;
    ZoomClaim claim;
    if(context.claim.serial && !zoomBorrowed(context,claim)) return;
    const auto intent=zoomIntent(context);
    if(context.revoked) return;
    const bool active=nativeZoomFlag(context.weapon)!=0;
    const bool sameOwner=!context.claim.serial || (claim.owner==intent.owner && claim.ownerHandle==intent.oh &&
        claim.weaponHandle==intent.wh && claim.hand==intent.hand && !claim.blocked && !claim.teardown);
    const bool desired=sameOwner && intent.admitted && intent.desired && zoomState(context.weapon,active);
    if(!desired) {
        if(context.claim.serial && claim.effect) {
            nativeBaseAlternativeRelease(context.weapon);
            if(context.revoked || !zoomBorrowed(context,claim)) return;
            if(active) zoomNative(context,false);
        }
        return;
    }
    if(!activation && !context.claim.serial) return;
    if(!context.claim.serial) {
        if(active) return; // No unmanaged adoption.
        if(context.revoked || resolve(intent.wh)!=context.weapon || resolve(intent.oh)!=intent.owner) return;
        const auto live=zoomIntent(context);
        if(!live.admitted || !live.desired || live.wh!=intent.wh || live.oh!=intent.oh || live.hand!=intent.hand) return;
        context.claim=zoomReserve(context.weapon,intent.owner,intent.wh,intent.oh,intent.hand);
        if(!context.claim.serial) return;
        claim=context.claim;
    }
    if(!zoomBorrowed(context,claim) || claim.blocked || !claim.effect) return;
    nativeBaseAlternativePress(context.weapon);
    if(context.revoked || !zoomBorrowed(context,claim)) return;
    if(!active && activation) zoomNative(context,true);
}
static void __fastcall sniperStep(void *weapon,void *) {
    ZoomContext context{weapon,ZoomOperation::Step,zoomClaim(weapon),zoomContext};
    for(auto *parent=context.previous;parent;parent=parent->previous)
        if(parent->weapon==weapon && parent->revoked) {
            context.revoked=true;context.sourceRevoked|=parent->sourceRevoked;
        }
    zoomContext=&context;
    withNativeFinally([&] {
        zoomReconcile(context,true);
        originalSniperStep(weapon);
    },[&](bool aborted) noexcept {
        zoomContext=context.previous;
        if(aborted) { zoomChange(context.claim.serial,false,false,true); zoomRevokeFrames(weapon);nativeInputFailed(); }
    });
}
static void __fastcall baseWeaponStep(void *weapon,void *) {
    const auto caller=reinterpret_cast<uintptr_t>(__builtin_return_address(0));
    auto *parent=zoomContext;
    withNativeFinally([&] {
        originalBaseWeaponStep(weapon);
        if(caller==sniperBaseStepReturn && parent && parent==zoomContext &&
            parent->operation==ZoomOperation::Step && parent->weapon==weapon && !parent->revoked)
            zoomReconcile(*parent,false); // Never reserve/restart activation after native base.
    },[&](bool aborted) noexcept {
        if(aborted) { if(parent) parent->revoked=true;nativeInputFailed(); }
    });
}
static int __fastcall sniperFire(void *weapon,void *,float interval) {
    int result=0;
    ZoomContext context{weapon,ZoomOperation::Fire,zoomClaim(weapon),zoomContext};
    for(auto *parent=context.previous;parent;parent=parent->previous)
        if(parent->weapon==weapon && parent->revoked) {
            context.revoked=true;context.sourceRevoked|=parent->sourceRevoked;
        }
    zoomContext=&context;
    withNativeFinally([&] {
        zoomReconcile(context,true); // Includes frozen historical OFF, before native damage use.
        result=originalSniperFire(weapon,interval);
    },[&](bool aborted) noexcept {
        zoomContext=context.previous;
        if(aborted) { zoomChange(context.claim.serial,false,false,true);zoomRevokeFrames(weapon);nativeInputFailed(); }
    });
    return result;
}
static void __fastcall zoomActivated(void *weapon,void *) {
    const auto claim=zoomClaim(weapon);
    zoomChange(claim.serial,false,true,false);
    zoomRevokeFrames(weapon);
    ZoomContext context{weapon,ZoomOperation::ExternalActivate,zoomClaim(weapon),zoomContext};
    zoomContext=&context;
    withNativeFinally([&] { originalZoomActivate(weapon); },[&](bool aborted) noexcept {
        zoomContext=context.previous;
        if(aborted) { zoomChange(context.claim.serial,false,false,true);nativeInputFailed(); }
    });
}
static void __fastcall zoomDeactivated(void *weapon,void *) {
    const auto caller=reinterpret_cast<uintptr_t>(__builtin_return_address(0));
    const auto claim=zoomClaim(weapon);
    auto *parent=zoomContext;
    if(caller==sniperCrossDeleteReturn && parent && parent->operation==ZoomOperation::Delete &&
        nativeZoomPreserveDelete(parent->deleteRouting,claim)) {
        bool suppress=false;
        ZoomContext borrowed{weapon,ZoomOperation::Deactivate,claim,zoomContext};
        zoomContext=&borrowed;
        withNativeFinally([&] {
            const auto intent=zoomIntent(borrowed,NativeZoomQuery::PreserveCrossDelete);
            ZoomClaim currentClaim;
            suppress=intent.admitted && intent.desired && intent.owner==claim.owner &&
                intent.oh==claim.ownerHandle && intent.wh==claim.weaponHandle && intent.hand==0 &&
                zoomBorrowed(borrowed,currentClaim) && currentClaim.effect && !currentClaim.blocked &&
                !currentClaim.teardown && !currentClaim.deleting;
        },[&](bool aborted) noexcept {
            zoomContext=borrowed.previous;
            if(aborted) { suppress=false;nativeInputFailed(); }
        });
        if(suppress) return;
    }
    ZoomContext context{weapon,ZoomOperation::Deactivate,claim,zoomContext};
    zoomContext=&context;
    withNativeFinally([&] { originalZoomDeactivate(weapon); },[&](bool aborted) noexcept {
        zoomContext=context.previous;
        if(aborted) { zoomChange(context.claim.serial,false,false,true);zoomRevokeFrames(weapon);nativeInputFailed(); }
    });
}
static void __fastcall sniperPutDown(void *weapon,void *,int immediate) {
    ZoomContext context{weapon,ZoomOperation::PutDown,zoomClaim(weapon),zoomContext};
    zoomRevokeFrames(weapon);
    zoomContext=&context;
    withNativeFinally([&] { originalSniperPutDown(weapon,immediate); },[&](bool aborted) noexcept {
        zoomContext=context.previous;
        if(aborted) { zoomChange(context.claim.serial,false,false,true);nativeInputFailed(); }
    });
}
static void __fastcall step(void *p, void *) {
    if (!hooksReady.load(std::memory_order_acquire)) {
        originalStep(p);
        return;
    }
    withNativeFinally([&] {
        const uint32_t handle = pointerHandle(p);
        bool prepared = false;
        if (simulationInterval && handle && resolve(handle) == p)
            for (auto *interval = simulationInterval; interval && !prepared; interval = interval->previous) {
                if (!interval->managerPrepared) continue;
                for (const auto &candidate : interval->preparedPlayers)
                    if (candidate.handle == handle && candidate.subject == p) {
                        prepared = true;
                        break;
                    }
            }
        // A late avatar may use the existing OnStep update fallback, but cannot
        // acquire high after interval preparation. Reserve neutral before it.
        if (!prepared && simulationInterval && !preparedPrimary(p))
            for (auto &entry : simulationInterval->preparedPlayers)
                if (!entry.subject) {
                    const auto snapshot = copySnapshot();
                    entry.subject = p; entry.handle = handle;
                    entry.recognized = primaryLocalRecognized(snapshot, p, handle) ||
                                      multiplayer::knownVrAvatar(handle);
                    if (entry.recognized) entry.primary = NativePrimaryValue::neutral();
                    break;
                }
        if (!prepared) {
            update(p);
            if (auto *entry = preparedPrimary(p)) {
                entry->recognized |= primaryLocalRecognized(copySnapshot(), p, handle);
                if (entry->recognized) entry->primary = NativePrimaryValue::neutral();
            }
        }
        // A reserved manager entry owns preparation even during native reentry.
        // In particular, a failed/unopened network interval cannot freeze here.
        if (!prepared)
            authorityStep(p);
    }, [&](bool aborted) noexcept {
        if (aborted)
            nativeInputFailed();
    });
    originalStep(p);
    withNativeFinally([&] {
        authorityAfterStep(p);
        remote_render::observePlayer(p);
        // Native equip/holster transitions can finish during OnStep, after the request.
        // Refresh ownership without resampling input or advancing wheel/trigger state twice.
        auto s = copySnapshot();
        if (channel.shared && p == s.player && local(p) && livePlayer(s)) {
            refreshOwnership(s, p);
            int currentHealth = health(p);
            if (s.ui.gameplay && currentHealth < s.ui.health)
                damageFeedback.fetch_add(1, std::memory_order_relaxed);
            s.ui.health = currentHealth;
            s.ui.armor = armor(p);
            publishSnapshot(s);
        }
    }, [&](bool aborted) noexcept {
        if (aborted)
            nativeInputFailed();
    });
}
static void __fastcall deleted(void *p, void *) {
    revokePrimary(p);
    zoomOwnerDeleting(p);
    remote_render::invalidatePlayer(p);
    multiplayer::invalidatePlayer(p);
    Snapshot invalid;
    bool changed = false;
    AcquireSRWLockExclusive(&snapshotLock);
    if (current.player == p) {
        uint32_t epoch = advanceEpochUnlocked();
        const auto interruption = current.interruption;
        current = {};
        current.interruption = interruption;
        current.generation = epoch;
        current.ui.trackingGeneration = epoch;
        current.ui.tickMs = GetTickCount64();
        invalid = current;
        changed = true;
        for (auto &c : calibration)
            c = {};
        for (auto &g : gates)
            g = {};
        for (auto &w : wheels)
            w = {};
    }
    ReleaseSRWLockExclusive(&snapshotLock);
    if (changed)
        publishSnapshot(invalid);
    originalDelete(p);
}
void invalidateRenderer() {
    AcquireSRWLockExclusive(&snapshotLock);
    uint32_t epoch = advanceEpochUnlocked();
    const auto interruption = current.interruption;
    current = {};
    current.interruption = interruption;
    current.generation = epoch;
    current.ui.trackingGeneration = epoch;
    current.ui.tickMs = GetTickCount64();
    for (auto &c : calibration)
        c = {};
    Snapshot invalid = current;
    ReleaseSRWLockExclusive(&snapshotLock);
    publishSnapshot(invalid);
}
static void invalidWeapon(void *w) {
    uint32_t owner;
    memcpy(&owner, static_cast<uint8_t *>(w) + 0x28, 4);
    if (auto player = resolve(owner)) {
        remote_render::invalidatePlayer(player);
        auto value = copyAuthority(player);
        if (value.sample.negotiated) {
            uint8_t changed = 0;
            uint32_t weapon = pointerHandle(w);
            for (unsigned hand = 0; hand < 2; ++hand)
                if (value.hand[hand] == weapon)
                    changed |= uint8_t(1u << hand);
            multiplayer::invalidateWeapons(player, ++value.weaponGeneration, changed);
            network::invalidateWeaponIntents(value.sample.pose, changed);
            saveAuthority(value);
        }
    }
    uint32_t id = pointerHandle(w);
    Snapshot invalid;
    bool changed = false;
    AcquireSRWLockExclusive(&snapshotLock);
    for (int h = 0; h < 2; h++)
        if (current.handle[h] == id) {
            if (current.rider.handheld()) {
                current.generation = advanceEpochUnlocked();
                current.fire[h] = false;
                current.zoom[h] = false;
            }
            current.handle[h] = 0;
            calibration[h] = {};
            current.ui.trackingGeneration = current.generation;
            current.ui.currentWeapon[h] = -1;
            current.ui.currentAmmo[h] = 0;
            current.ui.tickMs = GetTickCount64();
            changed = true;
        }
    invalid = current;
    ReleaseSRWLockExclusive(&snapshotLock);
    if (changed)
        publishSnapshot(invalid);
}
static void __fastcall weaponDeleted(void *w, void *) {
    revokePrimary(w);
    invalidWeapon(w);
    originalWeaponDelete(w);
}
static void __fastcall sniperDeleted(void *w, void *) {
    revokePrimary(w);
    ZoomContext context{w,ZoomOperation::Delete,zoomClaim(w),zoomContext};
    zoomRevokeFrames(w,true);
    zoomMarkDeleting(context.claim.serial);
    context.claim=zoomClaim(w);
    uint32_t owner=0,hand=0;
    memcpy(&owner,static_cast<uint8_t *>(w)+0x28,4);
    memcpy(&hand,static_cast<uint8_t *>(w)+0xbc,4);
    context.deleteRouting={owner ? resolve(owner):nullptr,owner,hand ? 1u:0u};
    zoomContext=&context;
    withNativeFinally([&] {
        invalidWeapon(w);
        originalSniperDelete(w); // Own deactivation, base teardown and sound source deletion once.
    },[&](bool aborted) noexcept {
        zoomContext=context.previous;
        if(aborted) { zoomChange(context.claim.serial,false,false,true);nativeInputFailed(); }
        else zoomRetire(context.claim.serial);
    });
}
static int __fastcall sniperAlternativePressed(void *w, void *) {
#ifdef _MSC_VER
    const auto caller = reinterpret_cast<uintptr_t>(_ReturnAddress());
#else
    const auto caller = reinterpret_cast<uintptr_t>(__builtin_return_address(0));
#endif
    // In the native no-left-weapon branch, plcmdAltFire toggles the right
    // sniper's zoom. VR uses that command for LEFT primary fire. Block only
    // this demonstrated dispatch origin, before it changes native damage/time.
    if (hooksReady.load(std::memory_order_acquire) && caller == playerAlternativePressReturn &&
        nativeMainThread && nativeMainThread()) {
        uint32_t ownerHandle = 0;
        std::memcpy(&ownerHandle, static_cast<uint8_t *>(w) + 0x28, sizeof(ownerHandle));
        if (void *owner = resolve(ownerHandle)) {
            const auto s = copySnapshot();
            const bool localRoute = owner == s.player && ownerHandle == s.playerHandle &&
                                    livePlayer(s) && vrSession(s);
            const auto peer = localRoute ? multiplayer::Sample{} : multiplayer::authority(owner);
            const bool remoteRoute = multiplayer::server() && peer.negotiated &&
                                     peer.avatar == ownerHandle && peer.incarnation;
            if ((localRoute || remoteRoute) && resolve(nativeHandle(owner, 1)) == w &&
                !resolve(nativeHandle(owner, 0)))
                return 1; // Preserve sniper's native handled-event return ABI.
        }
    }
    return originalSniperAlternativePress(w);
}
thread_local void *eyePlayer = nullptr;
thread_local Request eyeRequest;
thread_local Snapshot eyeSnapshot;
thread_local int eyeIndex = -1;
thread_local Pose eyeAnchor, eyeNativeCamera;
struct PhysicalWeaponInvocation {
    void *weapon = nullptr;
    int hand = -1;
    WeaponViewPass pass;
    Matrix44 entryProjection{};
    float entryNear = 0, entryFar = 1;
    ScopePoseObservation scopePose;
    bool scopePoseAmbiguous = false;
    bool scopeGeometryRejected = false;
    bool ordinaryCommand = false;
};
thread_local PhysicalWeaponInvocation *physicalWeapon = nullptr;
thread_local void *executingView = nullptr;
thread_local WeaponWorldView preparedWeaponWorld, executedWeaponWorld;
thread_local Matrix44 executedUiProjection;
thread_local bool executedUiProjectionValid = false;
thread_local bool rootCaptureArmed = false, rootCaptureAttempted = false;
thread_local bool weaponPairFault = false;
thread_local ScopeObservationBank eyeScopePoses;
static thread_local float eyeNativeBaseFov = 0;
static thread_local bool scopePreview = false;
struct NativeScopeSource {
    ScopeSourceView view;
    ScopeCaptureCallback callback = nullptr;
    ScopeCaptureCurrent current = nullptr;
    void *context = nullptr;
    bool active = false, prepared = false, attempted = false, queued = false, executed = false, fault = false;
};
static thread_local NativeScopeSource scopeSource;
static ScopeCaptureCommands scopeCaptureCommands;
static uintptr_t scopeInjectionReturn = 0;
struct NativeProjectionObservation {
    void *player = nullptr;
    Matrix44 *output = nullptr;
    float baseFov = 0;
    bool seen = false, rejected = false;
};
static thread_local NativeProjectionObservation *nativeProjectionObservation = nullptr;
thread_local void *rootView = nullptr;
thread_local LaserAim eyeLasers[2];
static thread_local float eyeWorldVisibility = 1;
float frozenWorldVisibility() {
    return eyeWorldVisibility;
}
static void freezeLasers(const Request &request);
static void drawLasers();
static void __fastcall viewExecute(void *command, void *) {
    const bool root = command == rootView && eyePlayer && eyeIndex >= 0;
    ScopedWeaponContext<void> execution(executingView, command);
    if (root && !rootCaptureAttempted) {
        rootCaptureAttempted = true;
        rootCaptureArmed = true;
    }
    const bool sourceRoot = command == rootView && scopeSource.active && eyePlayer && eyeIndex == -1;
    originalViewExecute(command);
    if (sourceRoot) scopeSource.executed = true;
    if (root) {
        rootCaptureArmed = false;
        if (!executedWeaponWorld.valid)
            weaponPairFault = true;
        drawLasers();
    }
}
static void __fastcall viewPrepare(void *command, void *, const Matrix34 &view, const Matrix44 &projection,
                                   const Box1 &depthRange, uint32_t identifier) {
#ifdef _MSC_VER
    auto caller = reinterpret_cast<uintptr_t>(_ReturnAddress());
#else
    auto caller = reinterpret_cast<uintptr_t>(__builtin_return_address(0));
#endif
    // Only renPrepareRender's main-root call. Native nested/shadow/reflection
    // preparation keeps its own identifiers and inherits parent IDs normally.
    if (scopeSource.active && eyePlayer && eyeIndex == -1 && caller == rootPrepareReturn) {
        if (rootView) scopeSource.fault = true;
        else {
            rootView = command;
            scopeSource.prepared = finiteMatrix(view) && finiteProjection(projection) &&
                validDepthRange(depthRange.min,depthRange.max) &&
                sameWeaponView(view,matrix(inverse(scopeSource.view.camera))) &&
                sameProjectionXY(projection,ss2vr::projection(scopeSource.view.fov));
            if (!scopeSource.prepared) scopeSource.fault = true;
            identifier ^= scopeSource.view.hand == 0 ? 0xa0000000u : 0xe0000000u;
        }
    }
    if (eyeIndex >= 0 && eyePlayer && caller == rootPrepareReturn) {
        if (rootView) {
            weaponPairFault = true; // Another root cannot inherit this eye's execution capture.
            preparedWeaponWorld = {};
            executedWeaponWorld = {};
            executedUiProjectionValid = false;
            rootCaptureArmed = false;
            originalViewPrepare(command, view, projection, depthRange, identifier);
            return;
        }
        rootView = command;
        const Pose eye = worldEyeTracking(eyeAnchor, eyeSnapshot.origin, eyeSnapshot.turn,
                                         eyeRequest.input.head, eyeRequest.eye[eyeIndex]);
        const Matrix34 expectedView = matrix(inverse(eye));
        preparedWeaponWorld = {view, projection, depthRange.min, depthRange.max,
            finiteMatrix(view) && finiteProjection(projection) &&
            validDepthRange(depthRange.min, depthRange.max) && sameWeaponView(view, expectedView) &&
            sameProjectionXY(projection, ss2vr::projection(eyeRequest.fov[eyeIndex]))};
        identifier ^= scopePreview ? 0x40000000u : (eyeIndex == 0 ? 0x80000000u : 0xc0000000u);
    }
    originalViewPrepare(command, view, projection, depthRange, identifier);
}
bool beginStereo(void *p, const Request &request) {
    weaponPairFault = false;
    eyeWorldVisibility = 1;
    eyeSnapshot = copySnapshot();
    if (!nativeFrameIdentity(request, eyeSnapshot.input, eyeSnapshot.generation, eyeSnapshot.initialized,
                             eyeSnapshot.ui.gameplay, p == eyeSnapshot.player && livePlayer(eyeSnapshot)) ||
        !vrSession(eyeSnapshot))
        return false;
    originalCamera(p, &eyeNativeCamera);
    bool ready = nativeRiderCurrent(p, eyeSnapshot.rider) &&
                 nativeTrackingAnchor(p, eyeAnchor, &eyeSnapshot.rider) && finite(eyeNativeCamera);
    if (ready) {
        freezeLasers(request);
        if (settings.headFade) {
            const Pose head =
                worldHeadTracking(eyeAnchor, eyeSnapshot.origin, eyeSnapshot.turn, request.input.head);
            AcquireSRWLockShared(&laserLock);
            const auto presentation =
                freezeHeadVisibility(headObstruction, eyeAnchor, head, eyeSnapshot.playerHandle, request,
                                     GetTickCount64(), settings.headFadeDepth);
            ReleaseSRWLockShared(&laserLock);
            eyeWorldVisibility = presentation.value;
        }
        remote_render::freezePair();
    }
    return ready;
}
bool commitStereo(void *p,const Request &request,Slot &slot) {
    // Same existing snapshot lock, now explicitly retired on native unwind too.
    AcquireSRWLockShared(&snapshotLock);
    bool committed=false;
    withNativeFinally([&] {
        const bool eligible=nativeFrameIdentity(request,current.input,current.generation,current.initialized,
            current.ui.gameplay,p==current.player && livePlayer(current)) && vrSession(current) &&
            fresh(current.input) && trackingEligible(p) && nativeRiderCurrent(p,current.rider) && !weaponPairFault;
        committed=remote_render::commitPair(slot,request,eligible);
    },[&](bool aborted) noexcept {
        if(aborted) nativeUiFault();
        ReleaseSRWLockShared(&snapshotLock);
    });
    return committed;
}
void beginEye(void *p, const Request &r, int i) {
    scopePreview = false;
    scopeSource = {};
    remote_render::useFrozenPair(true);
    rootView = nullptr;
    preparedWeaponWorld = {};
    executedWeaponWorld = {};
    executedUiProjectionValid = false;
    rootCaptureArmed = rootCaptureAttempted = false;
    eyePlayer = p;
    eyeRequest = r;
    eyeSnapshot.input = r.input;
    eyeIndex = i;
    eyeScopePoses = {};
    eyeNativeBaseFov = 0;
}
void endEye() {
    scopePreview = false;
    scopeSource = {};
    remote_render::useFrozenPair(false);
    eyePlayer = nullptr;
    eyeIndex = -1;
    physicalWeapon = nullptr;
    executingView = nullptr;
    rootCaptureArmed = false;
    rootView = nullptr;
    eyeScopePoses = {};
    eyeNativeBaseFov = 0;
}
static Pose *__fastcall camera(void *p, void *, Pose *out) {
    originalCamera(p, out);
    auto s = copySnapshot();
    if (p == eyePlayer && scopeSource.active && eyeSnapshot.initialized)
        *out = scopeSource.view.camera;
    else if (p == eyePlayer && eyeIndex >= 0 && eyeSnapshot.initialized)
        *out = worldEyeTracking(eyeAnchor, eyeSnapshot.origin, eyeSnapshot.turn, eyeRequest.input.head,
                                eyeRequest.eye[eyeIndex]);
    else if (p == s.player && s.rider.handheld() && nativeRiderCurrent(p, s.rider) &&
             vrSession(s) && fresh(s.input)) {
        Pose body;
        if (trackingAnchor(p, body))
            *out = worldHeadTracking(body, s.origin, s.turn, s.input.head);
    }
    return out;
}
static int __fastcall renderThirdPerson(void *p, void *) {
#ifdef _MSC_VER
    const auto caller = reinterpret_cast<uintptr_t>(_ReturnAddress());
#else
    const auto caller = reinterpret_cast<uintptr_t>(__builtin_return_address(0));
#endif
    if (caller == scopeInjectionReturn && p == eyePlayer && scopeSource.active && eyeIndex == -1) {
        if (scopeSource.attempted) scopeSource.fault = true;
        else {
            scopeSource.attempted = true;
            if (scopeSource.prepared && !scopeSource.fault)
                scopeSource.queued = scopeCaptureCommands.queue(rootView,caller,scopeSource.callback,
                                                                scopeSource.current,scopeSource.context);
            if (!scopeSource.queued) scopeSource.fault = true;
        }
        return 1; // Only native source collection's existing no-handheld path.
    }
    if (caller == mountedAvatarReturn && p == eyePlayer && eyeIndex >= 0 &&
        eyeSnapshot.rider.seated() && eyeSnapshot.initialized &&
        nativeRiderCurrent(p, eyeSnapshot.rider) && vrSession(eyeSnapshot))
        return 0; // Native root selects this player as the first-person avatar.
    return originalThirdPerson(p);
}
static Matrix44 *__fastcall project(void *p, void *, Matrix44 *out) {
#ifdef _MSC_VER
    const auto caller = reinterpret_cast<uintptr_t>(_ReturnAddress());
#else
    const auto caller = reinterpret_cast<uintptr_t>(__builtin_return_address(0));
#endif
    NativeProjectionObservation observation{p,out};
    auto *previous = nativeProjectionObservation;
    if (previous) previous->rejected = true; // Nested projection cannot certify the outer sample.
    const bool observe = caller == rootProjectionReturn && p == eyePlayer && eyeIndex >= 0 &&
                         eyeSnapshot.initialized && !previous;
    if (observe) eyeNativeBaseFov = 0;
    nativeProjectionObservation = observe ? &observation : nullptr;
    bool completed = false;
    // The observation lives above the native finally frame. Restore its TLS
    // pointer on native unwind too; never retain a pointer into an unwound hook.
    withNativeFinally([&] {
        const auto result = originalProjection(p, out);
        completed = true;
        if (observe && result == out && observation.seen && !observation.rejected &&
            p == eyePlayer && eyeIndex >= 0 && !weaponPairFault)
            eyeNativeBaseFov = observation.baseFov;
    },[&](bool aborted) noexcept {
        nativeProjectionObservation = previous;
        if (aborted && observe) eyeNativeBaseFov = 0;
    });
    if (!completed) { weaponPairFault = true; return out; }
    if (p == eyePlayer && scopeSource.active) {
        const float nz = out->m[11]/(out->m[10]-1), fz = out->m[11]/(out->m[10]+1);
        if (!(std::isfinite(nz) && nz > 0 && std::isfinite(fz) && fz > nz)) scopeSource.fault = true;
        else *out = projection(scopeSource.view.fov,nz,fz);
        return out;
    }
    if (p == eyePlayer && eyeIndex >= 0) {
        float nz = out->m[11] / (out->m[10] - 1), fz = out->m[11] / (out->m[10] + 1);
        if (!(nz > 0 && fz > nz && std::isfinite(fz))) {
            nz = .05f;
            fz = 10000;
        }
        *out = projection(eyeRequest.fov[eyeIndex], nz, fz);
    }
    return out;
}
static Matrix44 *__cdecl weaponFrustum(Matrix44 *out, float fov, float aspect, float nz, float fz) {
#ifdef _MSC_VER
    const auto caller = reinterpret_cast<uintptr_t>(_ReturnAddress());
#else
    const auto caller = reinterpret_cast<uintptr_t>(__builtin_return_address(0));
#endif
    if (auto *sample = nativeProjectionObservation; sample && caller == nativeFrustumReturn) {
        if (sample->seen || out != sample->output || sample->player != eyePlayer || eyeIndex < 0)
            sample->rejected = true;
        sample->seen = true;
        // This exact native call follows the side-effect-free Player getter
        // with no intervening call. Verify its actual virtual slot before reading
        // the field it returned; subclasses/foreign getters are not equivalent.
        if (!sample->rejected && readableMemory(sample->player,0x85c)) {
            uintptr_t table = 0, getter = 0;
            float multiplier = 0;
            std::memcpy(&table,sample->player,4);
            if (table <= UINTPTR_MAX-0x5fc && readableMemory(reinterpret_cast<void *>(table+0x5f8),4))
                std::memcpy(&getter,reinterpret_cast<void *>(table+0x5f8),4);
            if (getter == playerFovGetter) {
                std::memcpy(&multiplier,static_cast<uint8_t *>(sample->player)+0x858,4);
                sample->baseFov = scopeNativeBaseFov(fov,multiplier);
            }
        }
    }
    if (physicalWeapon && caller == weaponFrustumReturn) {
        if (physicalWeapon->pass.projection(*out))
            return out;
        weaponPairFault = true;
    }
    return originalFrustum(out, fov, aspect, nz, fz);
}
static Matrix34 *__cdecl weaponMatrixInverse(Matrix34 *out, const Matrix34 &incomingCamera) {
#ifdef _MSC_VER
    const auto caller = reinterpret_cast<uintptr_t>(_ReturnAddress());
#else
    const auto caller = reinterpret_cast<uintptr_t>(__builtin_return_address(0));
#endif
    if (physicalWeapon && caller == weaponInverseReturn) {
        if (physicalWeapon->pass.view(*out))
            return out;
        weaponPairFault = true;
    }
    return originalMatrixInverse(out, incomingCamera);
}
static bool physicalCallbacksAvailable() {
    return nativeDepthRange && *nativeDepthRange == expectedDepthRange &&
           nativeProjectionSet && *nativeProjectionSet == expectedProjectionSet;
}
static void __cdecl weaponDepthRange(float nearDepth, float farDepth) {
#ifdef _MSC_VER
    const auto caller = reinterpret_cast<uintptr_t>(_ReturnAddress());
#else
    const auto caller = reinterpret_cast<uintptr_t>(__builtin_return_address(0));
#endif
    const bool capture = caller == rootDepthReturn && rootCaptureArmed &&
                         executingView == rootView && eyePlayer && eyeIndex >= 0;
    if (physicalWeapon && caller == weaponDepthReturn) {
        physicalWeapon->pass.graphicsSetup(); // Callback mismatch still reaches native graphics setup.
        if (!physicalCallbacksAvailable() || !physicalWeapon->pass.depth(nearDepth, farDepth)) {
            physicalWeapon->pass.failed = true;
            weaponPairFault = true;
        }
    }
    originalDepthRange(nearDepth, farDepth);
    if (physicalWeapon && caller == weaponRestoreReturn) {
        if (!physicalWeapon->pass.restored() || !physicalCallbacksAvailable() ||
            std::memcmp(nativeCurrentProjection, &physicalWeapon->entryProjection, sizeof(Matrix44)) ||
            *nativeDepthNear != physicalWeapon->entryNear || *nativeDepthFar != physicalWeapon->entryFar) {
            physicalWeapon->pass.failed = true;
            weaponPairFault = true;
        }
    }
    if (capture) {
        rootCaptureArmed = false; // Parent reactivation cannot overwrite this eye snapshot.
        if (physicalCallbacksAvailable())
            executedWeaponWorld = executedWeaponView(preparedWeaponWorld, *nativeCurrentView,
                                                      *nativeCurrentProjection, *nativeDepthNear,
                                                      *nativeDepthFar);
        executedUiProjectionValid = executedWeaponWorld.valid && nativeAdjustedProjection &&
                                    finiteProjection(*nativeAdjustedProjection);
        if (executedUiProjectionValid)
            executedUiProjection = *nativeAdjustedProjection;
        if (!executedWeaponWorld.valid)
            weaponPairFault = true;
    }
}
// Copy while this eye still owns its completed root, before marker ortho setup
// and endEye retire native context. Never read a later UI projection as world P.
bool copyExecutedUiProjection(void *player, const Request &request, int index, Matrix44 &out) {
    if (!nativeMainThread || !nativeMainThread() || player != eyePlayer || index != eyeIndex ||
        index < 0 || index > 1 || !sameRequest(request, eyeRequest) || !rootCaptureAttempted ||
        !executedWeaponWorld.valid || !executedUiProjectionValid || weaponPairFault)
        return false;
    out = executedUiProjection;
    return true;
}
static void finishPhysicalWeapon(PhysicalWeaponInvocation &invocation) noexcept {
    if (invocation.pass.complete())
        return;
    weaponPairFault = true;
    if (!invocation.pass.cleanupRequired() || !physicalCallbacksAvailable())
        return;
    // Original Render exceptions propagate. Only best-effort adapter cleanup
    // exceptions are contained; the pair is already permanently ineligible.
    ScopedWeaponContext<PhysicalWeaponInvocation> suppress(physicalWeapon, nullptr);
    try {
        (*nativeProjectionSet)(invocation.entryProjection);
        *nativeCachedMatrices &= ~6u; // Native4CA80 invalidates projection products.
        originalDepthRange(invocation.entryNear, invocation.entryFar);
    } catch (...) {
        // Failed cleanup is not permission to publish either eye.
    }
}
struct FinishPhysicalWeapon {
    PhysicalWeaponInvocation *invocation;
    ~FinishPhysicalWeapon() { if (invocation) finishPhysicalWeapon(*invocation); }
};
static thread_local unsigned nativeShotDepth = 0;
struct MuzzleContext {
    bool accepted = false, authoritative = false;
    RiderIdentity rider;
    void *player = nullptr;
    uint32_t handle = 0, owner = 0;
    unsigned hand = 2;
    multiplayer::Sample sample;
};
static MuzzleContext muzzleContext(void *weapon, const Snapshot &s, bool explicitLocal) {
    MuzzleContext context;
    if (!hooksReady.load(std::memory_order_acquire))
        return context;
    // Actual shots give negotiated native authority precedence. Laser queries
    // explicitly supply their presentation-correlated local snapshot instead.
    if (!explicitLocal && !singlePlayer() && multiplayer::server()) {
        uint32_t owner = 0;
        memcpy(&owner, static_cast<uint8_t *>(weapon) + 0x28, sizeof(owner));
        void *player = owner ? resolve(owner) : nullptr;
        const auto peer = player ? multiplayer::authority(player) : multiplayer::Sample{};
        if (peer.negotiated) {
            context.authoritative = true;
            context.player = player;
            context.owner = owner;
            context.handle = pointerHandle(weapon);
            const auto value = copyAuthority(player);
            context.sample = value.sample;
            context.rider = value.rider;
            if (!context.rider.handheld() || !nativeRiderCurrent(player, context.rider))
                return context;
            for (unsigned h = 0; h < 2; ++h) {
                MuzzleBinding binding{owner, value.sample.avatar, peer.avatar,
                                      value.sample.incarnation, peer.incarnation,
                                      context.handle, value.hand[h], nativeHandle(player, int(h)),
                                      value.hand[1 - h],
                                      *reinterpret_cast<int *>(static_cast<uint8_t *>(weapon) + 0xb4), h,
                                      peer.negotiated, peer.valid,
                                      currentWeaponSample(value.sample, peer, h),
                                      isAlive(player) != 0,
                                      GetTickCount64(), value.sample.receivedMs};
                if (eligibleAuthorityMuzzle(value.sample.pose, binding)) {
                    context.accepted = true;
                    context.hand = h;
                    break;
                }
            }
            return context; // Rejected authority cannot fall through to local tracking.
        }
    }
    const int hand = handOf(weapon, s);
    if (eligibleLocalMuzzle(s.input, hand < 0 ? 2u : unsigned(hand), hand >= 0, vrSession(s),
                            hand >= 0 && trackingEligible(s.player), fresh(s.input))) {
        context.accepted = true;
        context.player = s.player;
        context.rider = s.rider;
        context.owner = s.playerHandle;
        context.hand = unsigned(hand);
        context.handle = s.handle[hand];
        uint32_t owner = 0;
        memcpy(&owner, static_cast<uint8_t *>(weapon) + 0x28, sizeof(owner));
        context.accepted = owner == context.owner;
    }
    return context;
}
static Pose *shooting(void *w, Pose *out, PoseGet original, const Snapshot *snapshot = nullptr,
                      bool *calibrated = nullptr) {
    const bool nested = nativeShotDepth++ != 0;
    const auto s = nested ? Snapshot{} : (snapshot ? *snapshot : copySnapshot());
    const auto context = nested ? MuzzleContext{} : muzzleContext(w, s, snapshot != nullptr);
    // Zoomed native sniper placement (Sam+172A70) is the owner's view
    // origin, not its gun attachment. Keep native zoom/damage, but obtain
    // the base weapon's attachment once for positively identified VR aim.
    // This shared boundary also serves collision lasers and server shots.
    invokeNativeMuzzle(
        original == originalSniperShot, context.accepted, nested,
        [&] { original(w, out); --nativeShotDepth; },
        [&] { originalShot(w, out); --nativeShotDepth; },
        [&] {
            if (!finite(*out) || resolve(context.handle) != w || resolve(context.owner) != context.player)
                return;
            if (!context.rider.handheld() || !nativeRiderCurrent(context.player, context.rider))
                return;
            uint32_t owner = 0;
            memcpy(&owner, static_cast<uint8_t *>(w) + 0x28, sizeof(owner));
            if (owner != context.owner || nativeHandle(context.player, int(context.hand)) != context.handle)
                return;
            if (context.authoritative) {
                // Retain the same validated hand/sample. Only lifecycle is
                // rechecked; never select a weaker or newer placement context.
                const auto peer = multiplayer::authority(context.player);
                if (!currentWeaponSample(context.sample, peer, context.hand) || !isAlive(context.player))
                    return;
                Pose body, camera, modelPose;
                if (!nativeWeaponReference(context.player, w, body, camera, modelPose))
                    return;
                if (resolve(context.owner) != context.player || resolve(context.handle) != w ||
                    nativeHandle(context.player, int(context.hand)) != context.handle ||
                    !isAlive(context.player) || !nativeRiderCurrent(context.player, context.rider))
                    return;
                memcpy(&owner, static_cast<uint8_t *>(w) + 0x28, sizeof(owner));
                if (owner != context.owner ||
                    !currentWeaponSample(context.sample, multiplayer::authority(context.player), context.hand))
                    return;
                Pose grip = compose(body, context.sample.pose.grip[context.hand]);
                *out = retargetShot(camera, *out, compose(inverse(camera), modelPose).p, grip);
                return;
            }
            Pose body, nativeCamera;
            if (!vrSession(s) || !fresh(s.input) || !trackingAnchor(s.player, body))
                return;
            originalCamera(s.player, &nativeCamera);
            if (!finite(nativeCamera) || !localWeaponCurrent(w, s, context.hand))
                return;
            Pose hand = worldHandTracking(
                body, s.origin, s.turn, s.input.head,
                calibratedGrip(s.input, context.hand,
                               *reinterpret_cast<int *>(static_cast<uint8_t *>(w) + 0xb4), settings));
            AcquireSRWLockShared(&snapshotLock);
            auto c = calibration[context.hand];
            ReleaseSRWLockShared(&snapshotLock);
            Pose target;
            if (c.valid && c.handle == context.handle)
                target = retargetShot(nativeCamera, *out, c.nativeModelLocal.p, hand);
            else
                target = {normalize(multiply(hand.q, multiply(inverse(nativeCamera.q), out->q))), hand.p};
            if (!finite(target) || !localWeaponCurrent(w, s, context.hand))
                return;
            if (calibrated)
                *calibrated = c.valid && c.handle == context.handle && GetTickCount64() >= c.tickMs &&
                              GetTickCount64() - c.tickMs <= 100;
            *out = target;
        }, calibrated);
    return out;
}
static Pose *__fastcall shot(void *w, void *, Pose *out) {
    return shooting(w, out, originalShot);
}
static Pose *__fastcall sniperShot(void *w, void *, Pose *out) {
    return shooting(w, out, originalSniperShot);
}
// These are the exact shared counts/reset pointer cleared by Engine DAD90.
// The child-traversal flag 2C7F9C is not an idle indicator or a lock.
static bool laserModelScratchIdle() {
    if (!laserEngineBase || !nativeMainThread || !nativeMainThread())
        return false;
    NativeModelScratch scratch;
    constexpr uint32_t counts[] = {0x2eac28, 0x2eac38, 0x2eac48, 0x2eac58, 0x2eac68,
                                   0x2eac78, 0x2eac98, 0x2eac88, 0x2eab40};
    for (unsigned i = 0; i != scratch.counts.size(); ++i)
        memcpy(&scratch.counts[i], reinterpret_cast<void *>(laserEngineBase + counts[i]), 4);
    memcpy(&scratch.evaluated, reinterpret_cast<void *>(laserEngineBase + 0x2eab68), 4);
    return scratch.idle();
}
static bool laserRead32(const void *object, size_t offset, uint32_t &out) {
    if (!readableMemory(object, offset + 4))
        return false;
    memcpy(&out, static_cast<const uint8_t *>(object) + offset, 4);
    return true;
}
// Presence validation for the original selector's unchecked pointer lookups.
// Selection remains native; no resource/action/idle policy is retained here.
static uint32_t laserFindIdent(const void *object, size_t pointerOffset, uint32_t ident) {
    uint32_t backing = 0, count = 0, capacity = 0;
    if (!laserRead32(object, pointerOffset - 4, capacity) || !laserRead32(object, pointerOffset, backing) ||
        !laserRead32(object, pointerOffset + 4, count) || !count || count > 4096 || count > capacity ||
        !readableMemory(reinterpret_cast<void *>(backing), size_t(count) * 4))
        return 0;
    for (uint32_t i = 0; i != count; ++i) {
        uint32_t pointer = 0, name = 0;
        memcpy(&pointer, reinterpret_cast<void *>(backing + i * 4), 4);
        if (!laserRead32(reinterpret_cast<void *>(pointer), 0, name))
            return 0;
        if (name == ident)
            return pointer;
    }
    return 0;
}
static bool readVehicleLaserSource(const Snapshot &s, VehicleLaserSource &out) {
    if (!s.rider.seated() || !nativeMainThread() || !nativeRiderCurrent(s.player, s.rider))
        return false;
    void *ride = resolve(s.rider.ride);
    uint32_t table = 0;
    VehicleLaserSource source;
    source.rider = s.rider;
    // Gate the actual implementation audited, not every entity that is rideable.
    if (!laserRead32(ride, 0, table) || table != laserTurretVtable ||
        !laserRead32(ride, 0x120, source.model) || !laserRead32(ride, 0x47c, source.resource) ||
        !laserRead32(ride, 0x1bc, source.action) || !laserRead32(ride, 0x1c4, source.blast) ||
        !laserRead32(ride, 0x114, source.mechanism) || !source.mechanism || !resolve(source.mechanism))
        return false;
    void *renderable = source.model ? resolve(source.model) : nullptr;
    if (!renderable || !readableMemory(renderable, 0x60))
        return false;
    source.instance = reinterpret_cast<uintptr_t>(laserModelInstance(renderable));
    if (!laserRead32(reinterpret_cast<void *>(source.instance), 0x18, source.config))
        return false;
    source.attachment = laserIdleAttachment;
    if (source.action != *invalidSeatIdent && source.blast != *invalidSeatIdent) {
        const auto process = laserFindIdent(reinterpret_cast<void *>(source.resource), 0x298, source.action);
        void *processPointer = reinterpret_cast<void *>(process);
        if (!process || resolve(pointerHandle(processPointer)) != processPointer)
            return false;
        const auto blast = laserFindIdent(processPointer, 0x64, source.blast);
        if (!blast || !laserRead32(reinterpret_cast<void *>(blast), 4, source.attachment))
            return false;
    }
    if (!source.usable() || source.attachment == *invalidSeatIdent ||
        !nativeRiderCurrent(s.player, s.rider) || resolve(s.rider.ride) != ride ||
        resolve(source.model) != renderable)
        return false;
    out = source;
    return true;
}
static bool laserSampleCurrent(const Snapshot &s, const Request &request) {
    const auto live = copySnapshot();
    if (!vrSession(live) || !livePlayer(live) || live.player != s.player ||
        !compatibleRiderInput(s.rider, live.rider, s.input, live.input, s.generation, live.generation,
                              GetTickCount64()) || !nativeRiderCurrent(s.player, s.rider) ||
        !nativeFrameIdentity(request, live.input, live.generation, live.initialized, live.ui.gameplay, true))
        return false;
    Lock lock(channel, 1);
    if (lock)
        for (const auto &slot : channel.shared->slot)
            if (slot.state == SlotState::Requested && !slot.cancelled && sameRequest(slot.request, request))
                return true;
    return false;
}
static bool vehicleLaserCurrent(const Snapshot &s, const Request &request,
                                const VehicleLaserSource &expected) {
    VehicleLaserSource live;
    return laserSampleCurrent(s, request) && laserModelScratchIdle() &&
           readVehicleLaserSource(s, live) && live == expected;
}
static bool vehicleLaserMuzzle(const Snapshot &s, const Request &request, VehicleLaserSource &source,
                               Pose &muzzle, Vec3 &rayDirection) {
    if (!laserModelScratchIdle() || !readVehicleLaserSource(s, source) ||
        !vehicleLaserCurrent(s, request, source))
        return false;
    void *ride = resolve(source.rider.ride);
    auto *table = *reinterpret_cast<uintptr_t **>(ride);
    uint32_t attachment = *invalidSeatIdent;
    reinterpret_cast<AttachmentIdent>(table[0x550 / 4])(ride, &attachment);
    if (attachment != source.attachment || !vehicleLaserCurrent(s, request, source))
        return false;
    Matrix34 placement;
    const bool found = laserAttachment(reinterpret_cast<void *>(source.instance), attachment, placement) != 0;
    if (!found || !vehicleLaserCurrent(s, request, source))
        return false; // Missing native attachment must not admit the identity fallback.
    Pose origin;
    reinterpret_cast<ViewOrigin>(table[0xac / 4])(ride, &origin, 1);
    if (!vehicleLaserCurrent(s, request, source))
        return false;
    Vec3 direction;
    reinterpret_cast<ShootDirection>(table[0x5a4 / 4])(ride, &direction);
    return vehicleLaserCurrent(s, request, source) && nativeLaserPose(origin, direction, muzzle, &rayDirection);
}
static void __cdecl trackedRayInit() {
    // The native caller is intentionally beginning a new query. Retire its old
    // query first, use the same native lifecycle for our rays, then leave a fresh
    // baseline for that caller. Never restore dangling native hit/cleanup state.
    originalRayInit();
#ifdef _MSC_VER
    auto caller = reinterpret_cast<uintptr_t>(_ReturnAddress());
#else
    auto caller = reinterpret_cast<uintptr_t>(__builtin_return_address(0));
#endif
    static thread_local bool querying = false;
    if (querying || eyeIndex >= 0 || scopeSource.active || caller != allowedRayReturn ||
        simulationThread.load(std::memory_order_relaxed) != GetCurrentThreadId() || !nativeMainThread())
        return;
    auto s = copySnapshot();
    if ((!settings.lasers && !settings.headFade && !s.zoom[0] && !s.zoom[1]) ||
        !vrSession(s) || !fresh(s.input) || !s.ui.gameplay || !livePlayer(s)) return;
    bool requested = false;
    Request queryRequest;
    {
        Lock lock(channel, 1);
        if (lock)
            for (const auto &slot : channel.shared->slot)
                if (slot.state == SlotState::Requested && !slot.cancelled &&
                    nativeFrameIdentity(slot.request, s.input, s.generation, s.initialized, s.ui.gameplay,
                                        true) &&
                    fresh(slot.request.input)) {
                    s.input = slot.request.input;
                    queryRequest = slot.request;
                    requested = true;
                    break;
                }
    }
    if (!requested)
        return;
    Pose body;
    if (!trackingAnchor(s.player, body))
        return;
    LaserAim existing[2];
    AcquireSRWLockShared(&laserLock);
    memcpy(existing, laserAim, sizeof(existing));
    ReleaseSRWLockShared(&laserLock);
    querying = true;
    HeadObstruction sampledHead;
    if (settings.headFade) {
        const Pose head = worldHeadTracking(body, s.origin, s.turn, s.input.head);
        const auto probe = makeHeadProbe(body, head);
        if (probe.valid) {
            originalRayInit();
            const NativeRay ray{probe.origin, probe.direction};
            setRay(ray);
            rayMaximum(probe.length);
            rayMinimum(0);
            rayRadius(settings.headRadius);
            rayCategory(bulletCategory);
            thickCategory(bulletCategory);
            rayAvatar(s.player);
            rayMechanism(getMechanism(s.player));
            rayFluids(0);
            checkRay();
            const bool hit = rayHit() != 0;
            const float rawDistance = hitDistance();
            originalRayInit(); // Retire this query even when its result is invalid.
            sampledHead = recordHeadObstruction(body, head, s.playerHandle, queryRequest, GetTickCount64(),
                                                probe, hit, rawDistance);
        }
    }
    LaserAim sampled[2];
    for (unsigned h = 0; h != 2; ++h) {
        if ((!settings.lasers && !(s.rider.handheld() && s.zoom[h])) || !s.input.handValid[h])
            continue;
        Pose muzzle;
        Vec3 direction;
        VehicleLaserSource vehicle;
        const bool mounted = s.rider.seated();
        uint32_t sourceHandle = s.handle[h];
        void *mechanism = nullptr;
        if (mounted) {
            if (h != 1 || !vehicleLaserMuzzle(s, queryRequest, vehicle, muzzle, direction))
                continue;
            sourceHandle = s.rider.ride;
            mechanism = resolve(vehicle.mechanism);
        } else {
            if (!s.rider.handheld() || aliased(s) || s.ui.wheel[h].open || !s.handle[h] || s.selecting[h])
                continue;
            void *w = resolve(s.handle[h]);
            if (!w || nativeHandle(s.player, h) != s.handle[h] || !laserModelScratchIdle())
                continue;
            const int weapon = *reinterpret_cast<int *>(static_cast<uint8_t *>(w) + 0xb4);
            bool calibrated = false;
            shooting(w, &muzzle, weapon == 13 ? originalSniperShot : originalShot, &s, &calibrated);
            if (!finite(muzzle) || !calibrated || !localWeaponCurrent(w, s, h) || !laserModelScratchIdle())
                continue;
            mechanism = getMechanism(s.player);
            direction = rotate(normalize(muzzle.q), {0, 0, -1});
        }
        if (!laserSampleCurrent(s, queryRequest) || !trackedHandCurrent(s.input, copySnapshot().input, h))
            continue;
        const auto &old = existing[h];
        const uint64_t now = GetTickCount64();
        // Moving native aim may change without an origin change. Mounted rays
        // deliberately do not borrow handheld cache reuse.
        if (!mounted && old.valid && old.kind == LaserSourceKind::Handheld &&
            old.requestSequence == queryRequest.sequence && old.sequence == s.input.sequence &&
            old.generation == s.generation && old.owner == s.playerHandle && old.weapon == sourceHandle &&
            now >= old.tickMs && now - old.tickMs < 80 && std::memcmp(&old.body, &body, sizeof(body)) == 0 &&
            std::memcmp(&old.muzzle, &muzzle, sizeof(muzzle)) == 0) {
            sampled[h] = old;
            continue;
        }
        originalRayInit();
        NativeRay ray{muzzle.p, direction};
        setRay(ray);
        rayMaximum(settings.laserDistance);
        rayMinimum(.01f);
        rayRadius(0);
        rayCategory(bulletCategory);
        thickCategory(bulletCategory);
        rayAvatar(s.player);
        rayMechanism(mechanism);
        rayFluids(0);
        checkRay();
        const bool hit = rayHit() != 0;
        const float distance = hit ? hitDistance() : settings.laserDistance;
        originalRayInit(); // Retire the native query on success and every rejection below.
        if (!std::isfinite(distance) || distance < 0 || distance > settings.laserDistance ||
            !laserSampleCurrent(s, queryRequest) || !trackedHandCurrent(s.input, copySnapshot().input, h) ||
            (mounted && !vehicleLaserCurrent(s, queryRequest, vehicle)))
            continue;
        sampled[h] = {muzzle, body, ray.origin + ray.direction * distance, s.playerHandle, sourceHandle,
                      s.generation, s.input.sequence, GetTickCount64(), true, hit};
        sampled[h].kind = mounted ? LaserSourceKind::Vehicle : LaserSourceKind::Handheld;
        sampled[h].vehicle = vehicle;
        sampled[h].requestSequence = queryRequest.sequence;
    }
    originalRayInit(); // Runs native collision cleanup and resets all native ray state.
    querying = false;
    const auto live = copySnapshot();
    if (!laserSampleCurrent(s, queryRequest)) {
        sampledHead = {};
        sampled[0] = sampled[1] = {};
    }
    for (unsigned h = 0; h != 2; ++h)
        if (sampled[h].valid && (!trackedHandCurrent(s.input, live.input, h) ||
            (sampled[h].kind == LaserSourceKind::Vehicle &&
             !vehicleLaserCurrent(s, queryRequest, sampled[h].vehicle))))
            sampled[h] = {};
    AcquireSRWLockExclusive(&laserLock);
    headObstruction = sampledHead;
    for (unsigned h = 0; h != 2; ++h)
        laserAim[h] = sampled[h];
    ReleaseSRWLockExclusive(&laserLock);
}
static void freezeLasers(const Request &request) {
    LaserFrame frame;
    frame.body = eyeAnchor;
    frame.owner = eyeSnapshot.playerHandle;
    frame.generation = eyeSnapshot.generation;
    frame.sequence = request.input.sequence;
    frame.now = GetTickCount64();
    frame.requestSequence = request.sequence;
    frame.kind = eyeSnapshot.rider.seated() ? LaserSourceKind::Vehicle : LaserSourceKind::Handheld;
    if (frame.kind == LaserSourceKind::Vehicle && !readVehicleLaserSource(eyeSnapshot, frame.vehicle)) {
        eyeLasers[0] = eyeLasers[1] = {};
        return;
    }
    const auto live = copySnapshot();
    for (unsigned h = 0; h != 2; ++h) {
        frame.weapon[h] = frame.kind == LaserSourceKind::Vehicle ? eyeSnapshot.rider.ride : eyeSnapshot.handle[h];
        frame.handValid[h] = trackedHandCurrent(request.input, live.input, h);
        frame.wheel[h] = eyeSnapshot.ui.wheel[h].open != 0;
        frame.selecting[h] = eyeSnapshot.selecting[h];
    }
    AcquireSRWLockShared(&laserLock);
    auto pair = freezeLaserPair(laserAim, frame);
    ReleaseSRWLockShared(&laserLock);
    std::copy(pair.begin(), pair.end(), eyeLasers);
}
static void drawLasers() {
    if (!settings.lasers || !eyeSnapshot.ui.gameplay || !currentCanvas || !*currentCanvas)
        return;
    // Establish identity model and this root's view/projection through native
    // helpers. Root Execute has no parent to restore its matrices.
    ortho();
    activateView(rootView);
    const bool wasDepth = *depthEnabled != 0, wasWrite = *depthWriting != 0;
    const bool wasBlend = *blending != 0, wasAlpha = *alphaTesting != 0;
    const int priorComparison = *depthComparison;
    (*enableDepth)();
    (*disableDepthWrite)();
    (*disableBlend)();
    (*disableAlpha)();
    (*setDepthComparison)(42); // GfxD3D table at RVA11274 maps native index42 to D3DCMP_LESSEQUAL.
    for (unsigned h = 0; h != 2; ++h) {
        const auto &a = eyeLasers[h];
        if (!a.valid)
            continue;
        const uint32_t color = h ? 0x40dfffffu : 0x60ff40ffu; // Native RGBA convention.
        const Vec3 side = rotate(a.muzzle.q, {.001f, 0, 0});
        drawLine(a.muzzle.p, a.end, color, UINT32_MAX);
        drawLine(a.muzzle.p + side, a.end + side, color, UINT32_MAX);
        drawLine(a.muzzle.p - side, a.end - side, color, UINT32_MAX);
        if (a.hit) {
            const float distance = std::sqrt(dot(a.end - a.muzzle.p, a.end - a.muzzle.p));
            const float radius = std::clamp(distance * .002f, .008f, .08f);
            const Vec3 center = a.end + rotate(a.muzzle.q, {0, 0, .005f});
            const Vec3 right = rotate(a.muzzle.q, {radius, 0, 0});
            const Vec3 up = rotate(a.muzzle.q, {0, radius, 0});
            drawLine(center - right, center + right, color, UINT32_MAX);
            drawLine(center - up, center + up, color, UINT32_MAX);
        }
    }
    if (!wasDepth)
        (*disableDepth)();
    if (wasWrite)
        (*enableDepthWrite)();
    if (wasBlend)
        (*enableBlend)();
    if (wasAlpha)
        (*enableAlpha)();
    (*setDepthComparison)(priorComparison);
}
static int __fastcall nativeFire(void *w, void *, float amount) {
    auto s = copySnapshot();
    int hand = handOf(w, s);
    int result = originalNativeFire(w, amount);
    if (result && hand >= 0 && vrSession(s) && fresh(s.input) && s.ui.gameplay)
        fireFeedback[hand].fetch_add(1, std::memory_order_relaxed);
    return result; // Feedback observes the native event; it never initiates damage/fire.
}
static bool trackedWeaponSession(const Snapshot &s) {
    if (!s.rider.handheld() || !nativeRiderCurrent(s.player, s.rider))
        return false;
    if (eyeIndex < 0)
        return vrSession(s);
    // Pair age is checked at admission/commit. Presentation never switches
    // back to a native camera between its frozen left and right eyes.
    return hooksReady.load(std::memory_order_acquire) && s.initialized && eyePlayer == s.player &&
           validTrackingEpoch(s.generation) && channel.shared &&
           trackingEpoch(*channel.shared) == s.generation && eyeRequest.input.session;
}
bool currentScopeDraw(ScopeDrawBinding &out) {
    out = {};
    if (!remote_render::ownsNativeThread() || weaponPairFault ||
        !physicalWeapon || physicalWeapon->pass.failed || physicalWeapon->pass.stage != 4 ||
        eyeIndex < 0 || !trackedWeaponSession(eyeSnapshot) ||
        executingView != rootView || !rootView || !livePlayer(eyeSnapshot))
        return false;
    const unsigned hand = unsigned(physicalWeapon->hand);
    if (hand >= 2 || !eyeRequest.input.handValid[hand] ||
        !finite(weaponTracking(eyeRequest.input, hand)))
        return false;
    const uint32_t handle = eyeSnapshot.handle[hand];
    void *weapon = handle ? resolve(handle) : nullptr;
    if (!weapon || weapon != physicalWeapon->weapon ||
        nativeHandle(eyeSnapshot.player, int(hand)) != handle ||
        nativeHandle(eyeSnapshot.player, int(1 - hand)) == handle)
        return false;
    uint32_t ownerHandle = 0, modelHandle = 0;
    int nativeId = -1;
    std::memcpy(&ownerHandle, static_cast<uint8_t *>(weapon) + 0x28, 4);
    std::memcpy(&modelHandle, static_cast<uint8_t *>(weapon) + 0x24, 4);
    std::memcpy(&nativeId, static_cast<uint8_t *>(weapon) + 0xb4, 4);
    if (ownerHandle != eyeSnapshot.playerHandle || nativeId != 13)
        return false;
    void *instance = modelHandle ? resolve(modelHandle) : nullptr;
    if (!instance)
        return false;
    out = {instance, eyeRequest.sequence, eyeRequest.input.sequence,
           eyeSnapshot.playerHandle, handle, modelHandle, eyeSnapshot.generation, hand};
    return true;
}
void recordScopeObservation(const ScopeDrawBinding &binding, const Matrix34 &affine,
                            const ScopeSurfaceLayout &layout) {
    ScopeDrawBinding currentBinding;
    if (!finiteMatrix(affine) || !currentScopeDraw(currentBinding) ||
        currentBinding.modelInstance != binding.modelInstance ||
        currentBinding.requestSequence != binding.requestSequence ||
        currentBinding.inputSequence != binding.inputSequence ||
        currentBinding.ownerHandle != binding.ownerHandle ||
        currentBinding.weaponHandle != binding.weaponHandle ||
        currentBinding.modelHandle != binding.modelHandle ||
        currentBinding.generation != binding.generation || currentBinding.hand != binding.hand)
        return;
    if (physicalWeapon->scopePose.valid || physicalWeapon->scopePoseAmbiguous) {
        physicalWeapon->scopePose = {};
        physicalWeapon->scopePoseAmbiguous = true;
        return;
    }
    physicalWeapon->scopePose = {affine, binding.requestSequence, binding.inputSequence,
                                binding.ownerHandle, binding.weaponHandle, binding.modelHandle, binding.generation,
                                true, false, layout};
    // CurrentScopeDraw has just revalidated the exact live native weapon and
    // owner. Copy only this sniper's existing interpolation fields; do not use
    // the player's shared FOV multiplier, advance time, or invoke zoom callbacks.
    // The native Step formula is F0 + (EC - F0) * F4 (1728B3..1728D9).
    physicalWeapon->scopePose.nativeBaseFovRadians = eyeNativeBaseFov;
    void *weapon = physicalWeapon->weapon;
    if (nativeSniper(weapon)) {
        uint32_t active = 0, held = 0;
        float start = 0, end = 0, progress = 0;
        const auto *bytes = static_cast<const uint8_t *>(weapon);
        std::memcpy(&active, bytes + 0xd4, 4);
        std::memcpy(&held, bytes + 0x2c, 4);
        std::memcpy(&start, bytes + 0xf0, 4);
        std::memcpy(&end, bytes + 0xec, 4);
        std::memcpy(&progress, bytes + 0xf4, 4);
        physicalWeapon->scopePose.zoom = scopeNativeZoom(active, held, start, end, progress);
    }
}
bool currentScopeQueryBoundary() {
    ScopeDrawBinding binding;
    if (!currentScopeDraw(binding) || !physicalWeapon->ordinaryCommand || !laserEngineBase)
        return false;
    int32_t query=0;
    std::memcpy(&query,reinterpret_cast<void *>(laserEngineBase+0x2c3f34),4);
    return query==-1;
}
bool currentScopeRaster(ScopeRasterObservation &out) {
    out = {};
    ScopeDrawBinding binding;
    if (!currentScopeDraw(binding) || physicalWeapon->scopePoseAmbiguous ||
        physicalWeapon->scopeGeometryRejected) return false;
    const auto pose = physicalWeapon->scopePose;
    if (!pose.valid || pose.requestSequence != binding.requestSequence || pose.inputSequence != binding.inputSequence ||
        pose.ownerHandle != binding.ownerHandle || pose.weaponHandle != binding.weaponHandle ||
        pose.modelHandle != binding.modelHandle || pose.generation != binding.generation) return false;
    Matrix34 affine;
    ScopeSurfaceLayout layout;
    const auto status=remote_render::copyScopeRaster(binding.modelInstance,affine,layout);
    applyScopeRasterStatus(status,physicalWeapon->scopePose.geometry,physicalWeapon->scopeGeometryRejected);
    if (status == ScopeRasterStatus::Rejected) {
        physicalWeapon->scopePose.opticalFrame = {};
        physicalWeapon->scopePose.imageCoordinates = {};
        physicalWeapon->scopePose.opaqueColorCandidate = false;
    }
    if (status != ScopeRasterStatus::Observed) return false;
    if (layout != pose.layout || std::memcmp(&affine,&pose.affine,sizeof(affine))) {
        retireScopeGeometry(); return false;
    }
    out = {pose,binding.hand};
    out.nearDepth=physicalWeapon->pass.world.nearDepth;
    out.farDepth=physicalWeapon->pass.world.farDepth;
    out.capClipValid = executedUiProjectionValid && scopeCapClip(executedUiProjection,
        physicalWeapon->pass.world.view,pose.affine,out.capClip);
    if (laserEngineBase) {
        uint32_t forced=1,mode=1;
        int32_t special=0;
        std::memcpy(&forced,reinterpret_cast<void *>(laserEngineBase+0x2e49f0),4);
        std::memcpy(&special,reinterpret_cast<void *>(laserEngineBase+0x2c36ec),4);
        std::memcpy(&mode,reinterpret_cast<void *>(laserEngineBase+0x2e494c),4);
        out.opaqueMode=!forced && special<0 && !mode;
    }
    return true;
}
void retireScopeGeometry(bool fatal) noexcept {
    if (physicalWeapon) {
        rejectScopeGeometry(physicalWeapon->scopePose.geometry,physicalWeapon->scopeGeometryRejected);
        physicalWeapon->scopePose.opticalFrame = {};
        physicalWeapon->scopePose.imageCoordinates = {};
        physicalWeapon->scopePose.opaqueColorCandidate = false;
    }
    if (fatal) weaponPairFault = true;
}
bool recordScopeGeometry(const ScopeRasterObservation &sample, const ScopeCapGeometry &cap,
                         const ScopeUvTransform &nativeUv, bool opaqueColorCandidate) {
    ScopeRasterObservation now;
    if (!cap.valid || !currentScopeRaster(now) || sample.hand != now.hand ||
        sample.pose.requestSequence != now.pose.requestSequence || sample.pose.inputSequence != now.pose.inputSequence ||
        sample.pose.ownerHandle != now.pose.ownerHandle || sample.pose.weaponHandle != now.pose.weaponHandle ||
        sample.pose.modelHandle != now.pose.modelHandle || sample.pose.generation != now.pose.generation ||
        sample.pose.layout != now.pose.layout || std::memcmp(&sample.pose.affine,&now.pose.affine,sizeof(Matrix34))) {
        retireScopeGeometry(); return false;
    }
    // contentVerified remains false: no material/skeleton proof here.
    ScopeOpticalFrame optical;
    ScopeImageCoordinates coordinates;
    if (!scopeOpticalFrame(cap,now.pose.affine,optical) ||
        !scopeImageCoordinates(cap,now.pose.affine,nativeUv,coordinates) ||
        (physicalWeapon->scopePose.imageCoordinates.valid &&
         physicalWeapon->scopePose.imageCoordinates.rows != coordinates.rows) ||
        !acceptScopeGeometry(physicalWeapon->scopePose.geometry,physicalWeapon->scopeGeometryRejected,cap)) {
        retireScopeGeometry(); return false;
    }
    physicalWeapon->scopePose.opticalFrame = optical;
    physicalWeapon->scopePose.imageCoordinates = coordinates;
    // Color failure never retires independently admitted geometry. This value
    // describes matching pre-draw color observations, not uninterrupted device
    // state through this DIP, image permission, source resource or transfer.
    physicalWeapon->scopePose.opaqueColorCandidate=opaqueColorCandidate && sample.opaqueMode && now.opaqueMode &&
        sample.nearDepth==now.nearDepth && sample.farDepth==now.farDepth;
    return true;
}
bool scopeImageTarget(unsigned hand,Vec3 &out) noexcept {
    out = {};
    if (hand >= 2 || !eyePlayer || eyeIndex < 0 || eyeIndex > 1 || scopePreview || weaponPairFault) return false;
    const auto &aim = eyeLasers[hand];
    if (!aim.valid || aim.kind != LaserSourceKind::Handheld || aim.owner != eyeSnapshot.playerHandle ||
        aim.weapon != eyeSnapshot.handle[hand] || aim.generation != eyeSnapshot.generation ||
        aim.requestSequence != eyeRequest.sequence || aim.sequence != eyeRequest.input.sequence) return false;
    out = aim.end;
    return std::isfinite(out.x) && std::isfinite(out.y) && std::isfinite(out.z);
}
bool scopeImageEye(Vec3 &out) noexcept {
    if (!eyePlayer || eyeIndex < 0 || eyeIndex > 1 || scopePreview || weaponPairFault) return false;
    out = worldEyeTracking(eyeAnchor,eyeSnapshot.origin,eyeSnapshot.turn,eyeRequest.input.head,
                           eyeRequest.eye[eyeIndex]).p;
    return std::isfinite(out.x) && std::isfinite(out.y) && std::isfinite(out.z);
}
bool scopePreviewNeeded(void *player, const Request &request) {
    if (!scopeInjectionReturn || player != eyeSnapshot.player || !eyeSnapshot.rider.handheld() ||
        !nativeUiFrameCurrent(player,request)) return false;
    for (unsigned hand = 0; hand < 2; ++hand)
        if (eyeSnapshot.zoom[hand] && eyeSnapshot.handle[hand] &&
            nativeSniper(resolve(eyeSnapshot.handle[hand]))) return true;
    return false;
}
void beginScopePreview(void *player, const Request &request) {
    beginEye(player,request,0);
    scopePreview = true;
}
bool copyScopeSourceView(unsigned hand, ScopeSourceView &out) {
    out = {};
    if (!scopePreview || eyeIndex != 0 || !observedScopePose(hand,out.pose) ||
        !out.pose.opaqueColorCandidate || !out.pose.imageCoordinates.valid ||
        !scopeOpticalFrameValid(out.pose.opticalFrame)) return false;
    float magnification = 0;
    if (!scopeNativeMagnification(out.pose.nativeBaseFovRadians,out.pose.zoom,magnification) ||
        !scopeOpticalProjection(out.pose.opticalFrame,settings.scopeEyeRelief,magnification,out.opticalProjection) ||
        !scopeOpticalFov(out.opticalProjection,out.fov)) return false;
    matrixPose(&out.camera,out.pose.opticalFrame.camera);
    if (!finite(out.camera) || !sameWeaponView(matrix(out.camera),out.pose.opticalFrame.camera)) return false;
    out.hand = hand;
    return true;
}
bool beginScopeSource(void *player, const Request &request, const ScopeSourceView &view,
                      ScopeCaptureCallback callback, ScopeCaptureCurrent currentSource, void *context) {
    if (!callback || !currentSource || !context || view.hand >= 2 || !view.pose.valid ||
        !finite(view.camera) || !scopeOpticalProjectionValid(view.opticalProjection) ||
        player != eyeSnapshot.player || !eyeSnapshot.rider.handheld() ||
        view.pose.requestSequence != request.sequence || view.pose.inputSequence != request.input.sequence ||
        view.pose.generation != eyeSnapshot.generation || view.pose.ownerHandle != eyeSnapshot.playerHandle ||
        view.pose.weaponHandle != eyeSnapshot.handle[view.hand] || !nativeUiFrameCurrent(player,request)) return false;
    beginEye(player,request,-1); // Explicit source purpose, never a synthetic eye index.
    scopeSource.view = view;
    scopeSource.callback = callback; scopeSource.current = currentSource; scopeSource.context = context;
    scopeSource.active = true;
    return true;
}
bool scopeSourceExecuting() noexcept {
    return scopeSource.active && scopeSource.prepared && scopeSource.queued && !scopeSource.fault &&
        !scopeSource.executed && eyePlayer && eyeIndex == -1 && rootView && executingView == rootView &&
        !weaponPairFault && nativeCurrentView && nativeCurrentProjection &&
        sameWeaponView(*nativeCurrentView,matrix(inverse(scopeSource.view.camera))) &&
        sameProjectionXY(*nativeCurrentProjection,ss2vr::projection(scopeSource.view.fov));
}
bool endScopeSource(bool normalCompletion) noexcept {
    const bool completed = normalCompletion && scopeSource.active && scopeSource.prepared &&
        scopeSource.queued && scopeSource.executed && !scopeSource.fault && !weaponPairFault;
    endEye();
    return completed;
}
bool observedScopePose(unsigned hand, ScopePoseObservation &out) {
    out = {};
    if (!remote_render::ownsNativeThread() || weaponPairFault || hand >= 2 || eyeIndex < 0 ||
        !trackedWeaponSession(eyeSnapshot) || !livePlayer(eyeSnapshot))
        return false;
    const uint32_t weaponHandle = eyeSnapshot.handle[hand];
    if (!weaponHandle || nativeHandle(eyeSnapshot.player, int(hand)) != weaponHandle ||
        nativeHandle(eyeSnapshot.player, int(1 - hand)) == weaponHandle)
        return false;
    void *weapon = resolve(weaponHandle);
    uint32_t modelHandle = 0, ownerHandle = 0;
    if (!weapon)
        return false;
    std::memcpy(&modelHandle, static_cast<uint8_t *>(weapon) + 0x24, 4);
    std::memcpy(&ownerHandle, static_cast<uint8_t *>(weapon) + 0x28, 4);
    if (!modelHandle || !resolve(modelHandle) || ownerHandle != eyeSnapshot.playerHandle)
        return false;
    return eyeScopePoses.copy(hand, eyeRequest.sequence, eyeRequest.input.sequence,
                              ownerHandle, weaponHandle, modelHandle, eyeSnapshot.generation,
                              weaponPairFault, out);
}
static int __fastcall weaponAbs(void *w, void *, const Matrix34 &view, Matrix34 &out) {
#ifdef _MSC_VER
    const auto caller = reinterpret_cast<uintptr_t>(_ReturnAddress());
#else
    const auto caller = reinterpret_cast<uintptr_t>(__builtin_return_address(0));
#endif
    const bool physical = physicalWeapon && physicalWeapon->weapon == w &&
                          caller == weaponPlacementReturn;
    const int result = originalWeaponAbs(w, view, out);
    auto reject = [&]() {
        if (physical) {
            physicalWeapon->pass.placed(false);
            weaponPairFault = true;
            return 0; // Native Render skips model draw, then adapter repairs missing restore.
        }
        return result;
    };
    if (!result)
        return reject();
    const auto s = eyeIndex >= 0 ? eyeSnapshot : copySnapshot();
    const int h = handOf(w, s);
    const Input &tracking = eyeIndex >= 0 ? eyeRequest.input : s.input;
    if (h < 0 || !trackedWeaponSession(s) || (!physical && !fresh(s.input)) || !tracking.handValid[h] ||
        !finite(weaponTracking(tracking, unsigned(h))) || !finiteMatrix(out) ||
        (physical && (physicalWeapon->hand != h || physicalWeapon->pass.stage != 3)))
        return reject();
    Pose native, suppliedCamera;
    matrixPose(&native, out);
    matrixPose(&suppliedCamera, view);
    if (!localWeaponCurrent(w, s, unsigned(h)))
        return reject();
    Pose body, nativeCamera;
    if (eyeIndex >= 0) {
        body = eyeAnchor;
        nativeCamera = eyeNativeCamera;
    } else {
        if (!trackingAnchor(s.player, body))
            return reject();
        originalCamera(s.player, &nativeCamera);
    }
    if (!finite(nativeCamera) || !finite(native) || !finite(suppliedCamera) ||
        !localWeaponCurrent(w, s, unsigned(h)))
        return reject();
    const Pose hand = worldHandTracking(
        body, s.origin, s.turn, tracking.head,
        calibratedGrip(tracking, h, *reinterpret_cast<int *>(static_cast<uint8_t *>(w) + 0xb4), settings));
    const Quat offset = multiply(inverse(suppliedCamera.q), native.q);
    const Pose target{normalize(multiply(hand.q, offset)), hand.p};
    if (!finite(target))
        return reject();
    const Matrix34 staged = matrix(target);
    Matrix34 flatModel;
    const auto flatCamera = matrix(nativeCamera);
    Pose flatPose;
    const bool calibrated = originalWeaponAbs(w, flatCamera, flatModel) != 0;
    if (!localWeaponCurrent(w, s, unsigned(h)))
        return reject();
    if (calibrated)
        matrixPose(&flatPose, flatModel);
    if (!localWeaponCurrent(w, s, unsigned(h)))
        return reject();
    AcquireSRWLockExclusive(&snapshotLock);
    const bool compatible = sameHandheldRig(s.rider, current.rider, s.generation, current.generation) &&
        current.playerHandle == s.playerHandle && current.handle[h] == s.handle[h] &&
        channel.shared && trackingEpoch(*channel.shared) == s.generation;
    const bool placed = compatible && (!physical || physicalWeapon->pass.placed(finiteMatrix(staged)));
    if (placed) {
        calibration[h] = {calibrated && finite(flatPose), s.handle[h],
                          calibrated ? compose(inverse(nativeCamera), flatPose) : Pose{}, GetTickCount64()};
        out = staged;
    }
    ReleaseSRWLockExclusive(&snapshotLock);
    if (!placed)
        return reject();
    return result;
}
static void renderTrackedWeapon(void *w, Matrix34 m, bool sniper, uintptr_t caller) {
    if (physicalWeapon) {
        // Native nested rendering may leave its view changed. Do not resume
        // an outer physical draw after an unproven nested state transaction.
        physicalWeapon->pass.failed = true;
        weaponPairFault = true;
    }
    ScopedWeaponContext<PhysicalWeaponInvocation> entryBarrier(physicalWeapon, nullptr);
    const auto s = eyeIndex >= 0 ? eyeSnapshot : copySnapshot();
    const int hand = handOf(w, s);
    if (eyeIndex >= 0 && hand >= 0)
        eyeScopePoses.beginDraw(unsigned(hand)); // Supersedes earlier evidence even on early return/unwind.
    if (eyeIndex >= 0 && hand >= 0 &&
        (!eyeRequest.input.handValid[hand] || !finite(weaponTracking(eyeRequest.input, unsigned(hand)))))
        return;
    const bool physical = eyeIndex >= 0 && eyePlayer == s.player && hand >= 0 &&
                          executingView == rootView && rootView;
    PhysicalWeaponInvocation invocation;
    if (physical) {
        if (!trackedWeaponSession(s) || !executedWeaponWorld.valid || !physicalCallbacksAvailable() ||
            !finiteProjection(*nativeCurrentProjection) ||
            !validDepthRange(*nativeDepthNear, *nativeDepthFar)) {
            weaponPairFault = true;
            return;
        }
        invocation = {w, hand, {executedWeaponWorld}, *nativeCurrentProjection,
                      *nativeDepthNear, *nativeDepthFar, {}, false};
        invocation.ordinaryCommand=ordinaryWeaponRenderReturn && caller==ordinaryWeaponRenderReturn;
    }
    // Establish a suppression barrier even for unowned/nested/desktop calls.
    ScopedWeaponContext<PhysicalWeaponInvocation> context(physicalWeapon, physical ? &invocation : nullptr);
    FinishPhysicalWeapon finish{physical ? &invocation : nullptr};
    if (sniper && !(eyeIndex >= 0 && hand >= 0))
        originalSniperRender(w, m);
    else
        originalWeaponRender(w, m);
    if (physical)
        eyeScopePoses.finishDraw(unsigned(hand), invocation.scopePose,
                                 invocation.pass.complete() && !invocation.scopePoseAmbiguous);
}
static void __fastcall weaponRender(void *w, void *, Matrix34 m) {
    renderTrackedWeapon(w, m, false,reinterpret_cast<uintptr_t>(__builtin_return_address(0)));
}
static void __fastcall sniperRender(void *w, void *, Matrix34 m) {
    // Native zoom overlay stays on desktop; XR retains the native gun/hand mesh.
    renderTrackedWeapon(w, m, true,reinterpret_cast<uintptr_t>(__builtin_return_address(0)));
}
static bool markerFrameCurrent(void *player, const Request &request, int index) {
    if (index < 0 || index > 1 || eyeIndex != index || player != eyePlayer || weaponPairFault ||
        !rootCaptureAttempted || !executedWeaponWorld.valid || !nativeMainThread || !nativeMainThread() ||
        request.sequence != eyeRequest.sequence || request.session != eyeRequest.session ||
        request.reference != eyeRequest.reference || request.input.sequence != eyeRequest.input.sequence)
        return false;
    const auto live = copySnapshot();
    return live.rider == eyeSnapshot.rider && nativeRiderCurrent(player, eyeSnapshot.rider) &&
           nativeFrameIdentity(request, live.input, live.generation, live.initialized, live.ui.gameplay,
                               live.player == player && livePlayer(live)) && vrSession(live) && fresh(live.input);
}
static bool markerDrawPortCurrent(void *drawPort, const Request &request, WorldMarkerDimensions &out) {
    if (!drawPort || nativeDrawPort() != drawPort)
        return false;
    int32_t rectangle[4]{};
    memcpy(rectangle, static_cast<uint8_t *>(drawPort) + 4, sizeof(rectangle));
    return worldMarkerDimensions(rectangle, request.width, request.height, out);
}
static bool nativeMarkerPostlude(void *player, const Request &request, int index) {
    const auto reject = [] { weaponPairFault = true; return false; };
    if (!markerFrameCurrent(player, request, index))
        return reject();
    // Native renFinishRender has destroyed rootView. These are value copies;
    // no command activation, pointer dereference or second world pass belongs here.
    const auto world = executedWeaponWorld;
    void *drawPort = nativeDrawPort(), *info = nativeWorldInfo();
    const uint32_t infoHandle = info ? pointerHandle(info) : 0;
    WorldMarkerDimensions dimensions;
    const Pose eye = worldEyeTracking(eyeAnchor, eyeSnapshot.origin, eyeSnapshot.turn,
                                     request.input.head, request.eye[index]);
    if (!infoHandle || resolve(infoHandle) != info ||
        !markerDrawPortCurrent(drawPort, request, dimensions) ||
        !worldMarkerViewValid(world, eye, request.fov[index], dimensions, request.width, request.height))
        return reject();
    const auto compatible = [&] {
        WorldMarkerDimensions currentDimensions;
        return markerFrameCurrent(player, request, index) && resolve(infoHandle) == info &&
               nativeWorldInfo() == info && markerDrawPortCurrent(drawPort, request, currentDimensions) &&
               markerFrameCurrent(player, request, index);
    };
    if (!compatible())
        return reject();
    auto *vtable = *static_cast<uintptr_t **>(player);
    const float fade = reinterpret_cast<MarkerFade>(vtable[0x60c / 4])(player);
    if (!std::isfinite(fade) || !compatible())
        return reject();
    ortho();
    nativeBlendType(501);
    (*disableDepthWrite)();
    (*disableDepth)();
    (*disableAlpha)();
    if (!compatible())
        return reject();
    if (fade > 0) {
        nativeNavigation(player, world.view, world.projection, dimensions, fade);
        if (!compatible())
            return reject();
        nativeObjectives(player, world.view, world.projection, dimensions, fade);
    }
    return compatible() ? true : reject();
}
// Bound by the installed build fingerprint. Native simple programs are
// assembled from the verified c1..c4 position operation; both device objects
// occupy offset0 in their12-byte records. Borrow nothing past this draw.
bool nativeUiProgramsCurrent(IDirect3DVertexShader9 *vertex, IDirect3DPixelShader9 *pixel) {
    if (!nativeMainThread || !nativeMainThread() || !vertex || !pixel ||
        !readableMemory(uiPrograms,12) || !readableMemory(uiSimplePrograms,48) ||
        !readableMemory(uiVertexHandle,4) || !readableMemory(uiPixelHandle,4)) return false;
    const uint32_t data = uiPrograms[1], count = uiPrograms[2];
    const int32_t vh = *uiVertexHandle, ph = *uiPixelHandle;
    if (!count || count > 65536 || vh <= 0 || ph <= 0 || uint32_t(vh) > count || uint32_t(ph) > count ||
        !readableMemory(reinterpret_cast<void *>(data),size_t(count)*12)) return false;
    bool builtin = false;
    for (unsigned i=0;i<6;++i)
        builtin |= uiSimplePrograms[2*i] == vh && uiSimplePrograms[2*i+1] == ph;
    if (!builtin) return false;
    const auto *records = reinterpret_cast<const uint32_t *>(data);
    return records[3*(vh-1)] == reinterpret_cast<uintptr_t>(vertex) &&
           records[3*(ph-1)] == reinterpret_cast<uintptr_t>(pixel);
}
using OverlayRender = void(__thiscall *)(void *,int);
using NativeFill = void(__cdecl *)(uint32_t);
static VoidThis originalBrainRender = nullptr;
static OverlayRender originalOverlayRender = nullptr;
static NativeFill originalNativeFill = nullptr;
static uintptr_t overlayParentReturn = 0, uiFadeReturn[2]{};
struct NativeUiOwner {
    void *brain = nullptr, *player = nullptr;
    uint32_t brainHandle = 0, playerHandle = 0;
};
static thread_local NativeUiOwner uiOwner;
static thread_local unsigned uiOwnerDepth = 0, uiOverlayDepth = 0;
bool nativeUiOwnerCurrent(void *player) {
    if(uiOwnerDepth!=1 || !player || player!=uiOwner.player || !uiOwner.brain ||
        !nativeMainThread || !nativeMainThread() || !uiOwner.brainHandle || !uiOwner.playerHandle ||
        resolve(uiOwner.brainHandle)!=uiOwner.brain || resolve(uiOwner.playerHandle)!=player ||
        !readableMemory(static_cast<uint8_t *>(uiOwner.brain)+0x28,4)) return false;
    uint32_t handle=0;
    memcpy(&handle,static_cast<uint8_t *>(uiOwner.brain)+0x28,4);
    return handle==uiOwner.playerHandle && resolve(handle)==player;
}
bool nativeUiFrameCurrent(void *player,const Request &request) {
    if(!nativeUiOwnerCurrent(player)) return false;
    const auto live=copySnapshot();
    return live.rider==eyeSnapshot.rider && nativeRiderCurrent(player,eyeSnapshot.rider) &&
        nativeFrameIdentity(request,live.input,live.generation,live.initialized,live.ui.gameplay,
                            live.player==player && livePlayer(live)) &&
        vrSession(live) && fresh(live.input) && trackingEligible(player) && !weaponPairFault;
}
static void __fastcall brainRender(void *brain,void *) {
    if(uiOwnerDepth) {
        nativeUiFault();
        ++uiOwnerDepth;
        withNativeFinally([&]{ originalBrainRender(brain); },[&](bool aborted) noexcept {
            if(aborted) nativeUiFault();
            --uiOwnerDepth;
        });
        return;
    }
    uiOwnerDepth=1;
    withNativeFinally([&] {
        if(nativeMainThread && nativeMainThread() && readableMemory(static_cast<uint8_t *>(brain)+0x28,4)) {
            uint32_t handle=0;
            memcpy(&handle,static_cast<uint8_t *>(brain)+0x28,4);
            void *player=handle ? resolve(handle) : nullptr;
            const auto live=copySnapshot();
            if(player && player==live.player && handle==live.playerHandle && livePlayer(live) &&
                trackingEligible(player) && vrSession(live) && fresh(live.input) && live.ui.gameplay)
                uiOwner={brain,player,pointerHandle(brain),handle};
        }
        originalBrainRender(brain); // Native player index/listener/HUD timing owner once.
        nativeUiFinishOwner();
    },[&](bool aborted) noexcept {
        nativeUiEndOwner(aborted);
        uiOwner={}; uiOwnerDepth=0; uiOverlayDepth=0;
    });
}
static void __fastcall overlayRender(void *player,void *,int flag) {
    const auto caller=reinterpret_cast<uintptr_t>(__builtin_return_address(0));
    const bool outer=uiOverlayDepth==0;
    ++uiOverlayDepth;
    bool active=false;
    withNativeFinally([&] {
        const bool admitted=caller==overlayParentReturn && flag==0 && outer && nativeUiOwnerCurrent(player);
        active=nativeUiBeginOverlay(player,admitted);
        originalOverlayRender(player,flag);
    },[&](bool aborted) noexcept {
        if(active) nativeUiEndOverlay(!aborted);
        else if(aborted) nativeUiFault();
        --uiOverlayDepth;
    });
}
static void __cdecl nativeFill(uint32_t color) {
    const auto caller=reinterpret_cast<uintptr_t>(__builtin_return_address(0));
    bool tagged=false;
    withNativeFinally([&] {
        if(caller==uiFadeReturn[0] || caller==uiFadeReturn[1]) tagged=nativeUiBeginFade();
        originalNativeFill(color); // Native gradient/color/blend and timing unchanged.
    },[&](bool aborted) noexcept {
        if(tagged) nativeUiEndFade(!aborted);
        else if(aborted) nativeUiFault();
    });
}
static void __fastcall render(void *p,void *) {
    const auto caller=reinterpret_cast<uintptr_t>(__builtin_return_address(0));
    withNativeFinally([&] {
        auto s=copySnapshot();
        if(p==s.player && trackingEligible(p) && vrSession(s) && fresh(s.input) && s.ui.gameplay)
            stereo(p,originalRender,caller==markerParentReturn ? nativeMarkerPostlude : nullptr);
        else originalRender(p);
    },[&](bool aborted) noexcept { if(aborted) nativeUiEndOwner(true); });
}
// Native command query adapter: no fabricated CInputDevice object or command-data writes.
using CommandFloat = float(__thiscall *)(void *, uint32_t);
using CommandInt = int(__thiscall *)(void *, uint32_t);
using DataGet = void *(__thiscall *)(void *);
static CommandFloat originalValue = nullptr;
static CommandInt originalDown = nullptr, originalPressed = nullptr, originalReleased = nullptr,
                  originalRepeated = nullptr;
static VoidThis originalPoll = nullptr;
static DataGet getData = nullptr;
static uint32_t *(__cdecl *toId)(uint32_t *, const char *) = nullptr;
static uint32_t ids[14]{};
static const char *commands[] = {"plcmdX+",   "plcmdX-",        "plcmdZ+",  "plcmdZ-",     "plcmdY+",
                                 "plcmdFire", "plcmdAltFire",   "plcmdUse", "plcmdSprint", "plcmdH+",
                                 "plcmdH-",   "plcmdMouseLook", "plcmdP+",  "plcmdP-"};
static void *playerBindings = nullptr;
static float values[14]{}, priorValues[14]{};
// Provenance of the existing cached command values, not an admission policy.
struct ControlSample {
    RiderIdentity rider;
    uint32_t generation = 0, intentEpoch[2]{};
    Input input;
    Pose origin;
    float turn = 0;
    bool client = false, enabled = false;
};
static ControlSample sampledControls;
static SRWLOCK controlsLock = SRWLOCK_INIT;
using LookClamp = void(__thiscall *)(void *, Vec3 &);
using QuaternionEuler = Vec3 *(__cdecl *)(Vec3 *, const Quat &);
static LookClamp originalLookClamp = nullptr;
static QuaternionEuler quaternionEuler = nullptr;
static bool currentControls(int index, const Snapshot &snapshot, const ControlSample &captured,
                            bool active = true) {
    if (index < 0)
        return true;
    if (!captured.enabled || !compatibleRiderInput(captured.rider, snapshot.rider,
        captured.input, snapshot.input, captured.generation, snapshot.generation, GetTickCount64()) ||
        recenterHeld(captured.input) || recenterHeld(snapshot.input) ||
        !nativeRiderCurrent(snapshot.player, captured.rider))
        return false;
    if (index != 5 && index != 6)
        return true;
    const unsigned hand = index == 5 ? 1u : 0u;
    if ((active && !snapshot.fire[hand]) || !primaryActionEligible(captured.input, hand) ||
        !primaryActionEligible(snapshot.input, hand) || !trackedHandCurrent(captured.input, snapshot.input, hand) ||
        captured.input.primaryInputGeneration[hand] != snapshot.input.primaryInputGeneration[hand])
        return false;
    if (captured.rider.seated())
        return true; // Native vehicle operator fire is outside handheld ACK admission.
    if (!captured.rider.handheld())
        return false;
    if (!captured.client && !multiplayer::remoteClient())
        return true;
    return captured.input.sequence == snapshot.input.sequence &&
        multiplayer::localPrimaryAllowed(snapshot.player, hand, captured.intentEpoch[hand],
            captured.input.primaryInputGeneration[hand], captured.input.sequence, active);
}
static void __fastcall mountedLookClamp(void *brain, void *, Vec3 &look) {
#ifdef _MSC_VER
    const auto caller = reinterpret_cast<uintptr_t>(_ReturnAddress());
#else
    const auto caller = reinterpret_cast<uintptr_t>(__builtin_return_address(0));
#endif
    ControlSample captured;
    AcquireSRWLockShared(&controlsLock);
    captured = sampledControls;
    ReleaseSRWLockShared(&controlsLock);
    const auto snapshot = copySnapshot();
    uint32_t brainHandle = 0;
    if (hooksReady.load(std::memory_order_acquire) && caller == mountedClampReturn &&
        nativeMainThread && nativeMainThread() && captured.rider.seated() && quaternionEuler &&
        currentControls(0, snapshot, captured) && trackedHandCurrent(captured.input, snapshot.input, 1) &&
        finite(captured.origin)) {
        brainHandle = pointerHandle(brain);
        uint32_t playerHandle = 0, ownedBrain = 0;
        if (brainHandle && resolve(brainHandle) == brain) {
            memcpy(&playerHandle, static_cast<uint8_t *>(brain) + 0x28, 4);
            memcpy(&ownedBrain, static_cast<uint8_t *>(snapshot.player) + 0x38c, 4);
        }
        Pose anchor;
        if (playerHandle == captured.rider.player && ownedBrain == brainHandle &&
            nativeTrackingAnchor(snapshot.player, anchor, &captured.rider)) {
            const Pose aim = worldHandTracking(anchor, captured.origin, captured.turn,
                captured.input.head, captured.input.hand[1]);
            Vec3 desired;
            if (validNativeBodyPose(aim)) {
                quaternionEuler(&desired, aim.q);
                desired.z = 0; // Native vehicle control look has heading/pitch, zero bank.
                if (std::isfinite(desired.x) && std::isfinite(desired.y) &&
                    nativeRiderCurrent(snapshot.player, captured.rider) && resolve(brainHandle) == brain) {
                    look = desired;
                }
            }
        }
    }
    originalLookClamp(brain, look); // Preserve native ride/camera pitch limits and downstream RPC.
}
static void __fastcall poll(void *b, void *) {
    originalPoll(b);
    if (!ids[0])
        for (int i = 0; i < 14; i++)
            toId(&ids[i], commands[i]);
    void *data = getData(b);
    if (!data)
        return;
    auto entries = *reinterpret_cast<uint8_t **>(static_cast<uint8_t *>(data) + 4);
    int count = *reinterpret_cast<int *>(static_cast<uint8_t *>(data) + 8);
    if (!entries || count < 1 || count > 1024)
        return;
    bool isPlayer = false;
    for (int i = 0; i < count; i++)
        if (*reinterpret_cast<uint32_t *>(entries + i * 0x40) == ids[5]) {
            isPlayer = true;
            break;
        }
    if (!isPlayer)
        return;
    auto s = copySnapshot();
    bool enabled = livePlayer(s) && s.initialized && fresh(s.input) && !recenterHeld(s.input) &&
                   s.ui.gameplay && GetTickCount64() - s.ui.tickMs < 200 && nativeRiderCurrent(s.player, s.rider);
    bool fire[2]{s.fire[0], s.fire[1]};
    const bool client = multiplayer::remoteClient();
    if (enabled && client && s.rider.handheld())
        for (unsigned h = 0; h < 2; ++h)
            fire[h] = fire[h] && multiplayer::localPrimaryAllowed(s.player, h, s.intentEpoch[h],
                s.input.primaryInputGeneration[h], s.input.sequence);
    AcquireSRWLockExclusive(&controlsLock);
    playerBindings = b;
    const bool sameContext = sampledControls.rider == s.rider && sampledControls.generation == s.generation &&
        sampledControls.input.session == s.input.session && sampledControls.input.reference == s.input.reference;
    if (sameContext)
        memcpy(priorValues, values, sizeof(values));
    else
        std::fill(std::begin(priorValues), std::end(priorValues), 0);
    for (unsigned hand = 0; hand < 2; ++hand)
        if (!primaryCommandHistoryCompatible(sampledControls.input, s.input, hand,
                sampledControls.intentEpoch[hand], s.intentEpoch[hand], s.rider.handheld()))
            priorValues[hand == 1 ? 5 : 6] = 0;
    sampledControls = {s.rider, s.generation, {s.intentEpoch[0], s.intentEpoch[1]},
        s.input, s.origin, s.turn, client, enabled};
    std::fill(std::begin(values), std::end(values), 0);
    if (enabled) {
        float x = s.ui.wheel[0].open ? 0 : s.input.axis[0][0],
              y = s.ui.wheel[0].open ? 0 : s.input.axis[0][1];
        auto direction = rotate(yaw(s.rider.seated() ? 0.f : s.turn), Vec3{x, 0, -y});
        // Verified native labels: X+ right, X- left, Z- forward, Z+ backward.
        values[0] = std::max(0.f, direction.x);
        values[1] = std::max(0.f, -direction.x);
        values[2] = std::max(0.f, direction.z);
        values[3] = std::max(0.f, -direction.z);
        values[4] = s.jump ? 1 : 0;
        values[5] = fire[1] ? 1 : 0;
        values[6] = fire[0] ? 1 : 0;
        values[7] = s.use ? 1 : 0;
        values[8] = s.sprint ? 1 : 0;
    }
    ReleaseSRWLockExclusive(&controlsLock);
}
static int commandIndex(void *b, uint32_t id) {
    if (b != playerBindings)
        return -1;
    for (int i = 0; i < 14; i++)
        if (ids[i] == id)
            return i;
    return -1;
}
static float __fastcall commandValue(void *b, void *, uint32_t id) {
    auto snapshot = copySnapshot();
    if (!vrSession(snapshot))
        return originalValue(b, id);
    bool enabled = fresh(snapshot.input) && !recenterHeld(snapshot.input) && snapshot.ui.gameplay &&
                   GetTickCount64() - snapshot.ui.tickMs < 200 && livePlayer(snapshot);
    AcquireSRWLockShared(&controlsLock);
    int i = commandIndex(b, id);
    float v = i < 0 || !enabled ? 0 : values[i];
    const auto captured = sampledControls;
    ReleaseSRWLockShared(&controlsLock);
    if (!currentControls(i, snapshot, captured, v > 0))
        v = 0;
    return i < 0 ? originalValue(b, id) : v;
}
static int query(void *b, uint32_t id, int mode, CommandInt original) {
    auto snapshot = copySnapshot();
    if (!vrSession(snapshot))
        return original(b, id);
    bool enabled = fresh(snapshot.input) && !recenterHeld(snapshot.input) && snapshot.ui.gameplay &&
                   GetTickCount64() - snapshot.ui.tickMs < 200 && livePlayer(snapshot);
    AcquireSRWLockShared(&controlsLock);
    int i = commandIndex(b, id);
    float v = i < 0 || !enabled ? 0 : values[i], prev = i < 0 ? 0 : priorValues[i];
    const auto captured = sampledControls;
    ReleaseSRWLockShared(&controlsLock);
    if (i < 0)
        return original(b, id);
    return nativeCommandQuery(v, prev, mode, currentControls(i, snapshot, captured, v > 0));
}
static int __fastcall commandDown(void *b, void *, uint32_t id) {
    return query(b, id, 0, originalDown);
}
static int __fastcall commandPressed(void *b, void *, uint32_t id) {
    return query(b, id, 1, originalPressed);
}
static int __fastcall commandReleased(void *b, void *, uint32_t id) {
    return query(b, id, 2, originalReleased);
}
static int __fastcall commandRepeated(void *b, void *, uint32_t id) {
    return query(b, id, 1, originalRepeated);
}
static std::vector<void *> ownedHooks;
static bool attachPoisoned = false;
static bool rollbackNativeHooks() {
    hooksReady = false;
    bool result = rollbackHooks(
        ownedHooks,
        [](void *address) {
            // Cancel any queued enable before directly disabling. MinHook's
            // ApplyQueued can stop midway; a second queued apply is not rollback.
            auto queued = MH_QueueDisableHook(address);
            auto status = MH_DisableHook(address);
            bool ok = queued == MH_OK && (status == MH_OK || status == MH_ERROR_DISABLED);
            if (!ok)
                log("Hook disable rollback failed: queued=%d disable=%d", int(queued), int(status));
            return ok;
        },
        [](void *address) {
            auto status = MH_RemoveHook(address);
            if (status != MH_OK)
                log("Hook removal rollback failed: %d", int(status));
            return status == MH_OK;
        });
    attachPoisoned = !result;
    return result;
}
static bool hook(HMODULE m, const char *name, void *target, void **original) {
    void *address = reinterpret_cast<void *>(GetProcAddress(m, name));
    if (!address || MH_CreateHook(address, target, original) != MH_OK) {
        log("Required hook missing: %s", name);
        return false;
    }
    ownedHooks.push_back(address);
    return MH_QueueEnableHook(address) == MH_OK;
}
static bool internalHook(HMODULE m, uint32_t rva, void *target, void **original) {
    void *address = reinterpret_cast<uint8_t *>(m) + rva;
    if (MH_CreateHook(address, target, original) != MH_OK) {
        log("Required fingerprinted internal hook missing: %x", unsigned(rva));
        return false;
    }
    ownedHooks.push_back(address);
    return MH_QueueEnableHook(address) == MH_OK;
}
template <class T> static bool symbol(HMODULE m, const char *name, T &out) {
    out = loadProc<T>(m, name);
    if (!out)
        log("Required symbol missing: %s", name);
    return out != nullptr;
}
bool attach(bool headless) {
    if (hooksReady)
        return true;
    if (attachPoisoned) {
        log("Native VR attach refused after incomplete rollback; restart required");
        return false;
    }
    if (!supported(headless))
        return false;
    std::wstring settingsDirectory;
    if (moduleDirectory(nullptr,settingsDirectory))
        settings=loadSettings(settingsDirectory+L"SS2VR\\SS2VR.ini");
    auto g = GetModuleHandleW(L"Sam2Game.dll"), e = GetModuleHandleW(L"Engine.dll"),
         c = GetModuleHandleW(L"Core.dll");
    const auto graphics = headless ? nullptr : GetModuleHandleW(L"GfxD3D.dll");
    // Cached function addresses and trampolines belong to these exact modules.
    // Keep their mappings resident rather than accepting a stale reload target.
    for (auto module : {g, e, c, graphics}) {
        if (!module) continue;
        HMODULE pinned = nullptr;
        if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN,
                                reinterpret_cast<LPCWSTR>(module), &pinned)) {
            log("Native dependency pin failed: %lu", GetLastError());
            return false;
        }
    }
    // Fingerprinted Engine renPrepareRender invokes root Prepare at RVA 0x14c512 (return 0x14c517).
    rootPrepareReturn = reinterpret_cast<uintptr_t>(e) + 0x14c517;
    rootProjectionReturn = reinterpret_cast<uintptr_t>(g) + 0x94398;
    nativeFrustumReturn = reinterpret_cast<uintptr_t>(g) + 0x9436f;
    playerFovGetter = reinterpret_cast<uintptr_t>(g) + 0xf8050;
    scopeInjectionReturn = reinterpret_cast<uintptr_t>(g) + 0xfda0b;
    if (!headless && !scopeCaptureCommands.configure(e,g)) {
        log("Scope native capture command ABI unavailable");
        scopeInjectionReturn = 0; // Optional source cannot queue or become publishable.
    }
    markerParentReturn = reinterpret_cast<uintptr_t>(g) + 0xfea8a;
    overlayParentReturn = reinterpret_cast<uintptr_t>(g) + 0xeb85d;
    uiFadeReturn[0] = reinterpret_cast<uintptr_t>(g) + 0xfe90a;
    uiFadeReturn[1] = reinterpret_cast<uintptr_t>(g) + 0xfea07;
    // Audited CLOSRequest setup: every collision setter follows this rayInit call.
    allowedRayReturn = reinterpret_cast<uintptr_t>(g) + 0x21ae69;
    laserEngineBase = reinterpret_cast<uintptr_t>(e);
    laserTurretVtable = reinterpret_cast<uintptr_t>(g) + 0x2b39e8;
    weaponFrustumReturn = reinterpret_cast<uintptr_t>(g) + 0x4c8e9;
    weaponInverseReturn = reinterpret_cast<uintptr_t>(g) + 0x4c915;
    weaponDepthReturn = reinterpret_cast<uintptr_t>(g) + 0x4c9a6;
    weaponPlacementReturn = reinterpret_cast<uintptr_t>(g) + 0x4c9bb;
    ordinaryWeaponRenderReturn = reinterpret_cast<uintptr_t>(g) + 0x4bf3a;
    weaponRestoreReturn = reinterpret_cast<uintptr_t>(g) + 0x4ca93;
    rootDepthReturn = reinterpret_cast<uintptr_t>(e) + 0x155dd1;
    playerAlternativePressReturn = reinterpret_cast<uintptr_t>(g) + 0x101f8b;
    primaryOperatorReturn = reinterpret_cast<uintptr_t>(g) + 0xa6e09;
    primaryHeldReturn = reinterpret_cast<uintptr_t>(g) + 0xf6f1f;
    sniperBaseStepReturn = reinterpret_cast<uintptr_t>(g)+0x172830;
    sniperCrossDeleteReturn = reinterpret_cast<uintptr_t>(g)+0x171a54;
    mountedAvatarReturn = reinterpret_cast<uintptr_t>(g) + 0x943b6;
    mountedClampReturn = reinterpret_cast<uintptr_t>(g) + 0xf3255;
    if (!headless) {
        expectedDepthRange = reinterpret_cast<DepthRange>(reinterpret_cast<uint8_t *>(graphics) + 0x56a0);
        expectedProjectionSet = reinterpret_cast<ProjectionSet>(reinterpret_cast<uint8_t *>(graphics) + 0x69a0);
    }
    bool ok = true;
#define S(module, name, out) ok = symbol(module, name, out) && ok
    S(c, "?hvHandleToPointer@SeriousEngine@@YAPAXK@Z", resolve);
    S(c, "?hvPointerToHandle@SeriousEngine@@YAKPAX@Z", pointerHandle);
    S(c, "?thrIsThisMainThread@SeriousEngine@@YAHXZ", nativeMainThread);
    S(c, "?_st_idInvalid@SeriousEngine@@3UInvalidIdent@1@B", invalidSeatIdent);
    if (!headless)
        S(c, "?mthQuaternionToEuler@SeriousEngine@@YA?AVVector3f@1@ABVQuaternion4f@1@@Z", quaternionEuler);
    S(c, "?mthMatrixToQuatVect@SeriousEngine@@YA?AVQuatVect@1@ABVMatrix34f@1@@Z", matrixPose);
    S(c, "?strConvertStringToID@SeriousEngine@@YA?AVIDENT@1@PBD@Z", toId);
    S(e, "?simGetCurrent@SeriousEngine@@YAPAVCSimulation@1@XZ", currentSimulation);
    S(e, "?wldGetCurrent@SeriousEngine@@YAPAVCWorld@1@XZ", currentWorld);
    S(g, "?samIsSinglePlayer@SeriousEngine@@YAHXZ", singlePlayer);
    S(g, "?samGetGameInfoEntity@SeriousEngine@@YA?AV?$Handle@VCGameInfoEntity@SeriousEngine@@@1@XZ", nativeGameInfo);
    S(g, "?IsComboWeaponsEnabled@CGameInfoEntity@SeriousEngine@@QAEHXZ", nativeComboWeapons);
    S(g, "?IsFlippingFireButtons@CPlayerPuppetEntity@SeriousEngine@@UAEHXZ", nativeFlipButtons);
    S(g, "?GetWeaponFiringButton@CPlayerPuppetEntity@SeriousEngine@@UAEJV?$Handle@VCBaseWeaponEntity@SeriousEngine@@@2@@Z", nativeWeaponButton);
    S(g, "??_7CSniperWeaponEntity@SeriousEngine@@6B@", sniperVtable);
    S(g, "?IsZooming@CSniperWeaponEntity@SeriousEngine@@UAEHXZ", nativeZoomFlag);
    S(g, "?OnAlternativeFirePressed@CBaseWeaponEntity@SeriousEngine@@UAEHXZ", nativeBaseAlternativePress);
    S(g, "?OnAlternativeFireReleased@CBaseWeaponEntity@SeriousEngine@@UAEXXZ", nativeBaseAlternativeRelease);
    S(g, "?CanFireFromOwnWeapons@CPuppetEntity@SeriousEngine@@QAEHXZ", nativeOwnWeapons);
    S(g, "?IsLocal@CPuppetEntity@SeriousEngine@@QAEHXZ", isLocal);
    S(g, "?IsAlive@CPuppetEntity@SeriousEngine@@UAEHXZ", isAlive);
    S(g, "?RendersIn3rdPerson@CPuppetEntity@SeriousEngine@@UAEHXZ", thirdPerson);
    S(g, "?GetAbsPlacement@CPuppetEntity@SeriousEngine@@UAE?AVQuatVect@2@XZ", nativeBodyPlacement);
    S(g, "?GetViewOrigin@CPuppetEntity@SeriousEngine@@UAE?AVQuatVect@2@W4LOSRequestPurpose@2@@Z",
      baseViewOrigin);
    S(g, "?GetHealth@CPuppetEntity@SeriousEngine@@UAEJXZ", health);
    S(g, "?GetArmor@CPuppetEntity@SeriousEngine@@UAEJXZ", armor);
    S(g, "?IsDualWielding@CPlayerPuppetEntity@SeriousEngine@@QAEHXZ", isDual);
    S(g, "?IsNetricsaScreenActive@CPlayerPuppetEntity@SeriousEngine@@QAEHXZ", isNetricsa);
    S(g, "?SetCurrentWeapon@CPlayerPuppetEntity@SeriousEngine@@UAEXW4WeaponIndex@2@W4PlayerHand@2@H@Z",
      setWeapon);
    S(g, "?ToggleDualWielding@CPlayerPuppetEntity@SeriousEngine@@QAEXH@Z", toggleDual);
    S(g, "?CanChangeWeapon@CPlayerPuppetEntity@SeriousEngine@@QAEHW4PlayerHand@2@@Z", canChange);
    S(g, "?IsWeaponInInventory@CPlayerInventory@SeriousEngine@@QAEHW4WeaponIndex@2@@Z", inInventory);
    S(g, "?GetAmmoQuantityForWeapon@CPlayerInventory@SeriousEngine@@QAEJW4WeaponIndex@2@@Z", ammo);
    if (!headless) {
        S(e, "?GetModelInstance@CModelRenderable@SeriousEngine@@QAEPAVCModelInstance@2@XZ", laserModelInstance);
        S(e, "?mdlGetAttachmentAbsolutePlacement@SeriousEngine@@YAHPAVCModelInstance@1@VIDENT@1@AAVMatrix34f@1@@Z", laserAttachment);
        S(g, "?RenderNavigationalBeacons@CPuppetEntity@SeriousEngine@@QAEXABVMatrix34f@2@ABVMatrix44f@2@ABVVector2l@2@M@Z", nativeNavigation);
        S(g, "?RenderMissionObjectives@CPuppetEntity@SeriousEngine@@QAEXABVMatrix34f@2@ABVMatrix44f@2@ABVVector2l@2@M@Z", nativeObjectives);
        S(g, "?samGetWorldInfo@SeriousEngine@@YAPAVCWorldInfoEntity@1@XZ", nativeWorldInfo);
        S(e, "?gfxGetCurrentDrawPort@SeriousEngine@@YAPAVCDrawPort@1@XZ", nativeDrawPort);
        S(e, "?gfuBlendType@SeriousEngine@@YAXW4GfuBlendType@1@@Z", nativeBlendType);
        S(e, "?GetData@CInputBindings@SeriousEngine@@QAEPAVCInputBindingsData@2@XZ", getData);
        S(g, "?GetMechanism@CPuppetEntity@SeriousEngine@@UAEPAVCMechanism@2@XZ", getMechanism);
        S(e, "?raySetRay@SeriousEngine@@YAXABVRay3f@1@@Z", setRay);
        S(e, "?raySetMaxDistance@SeriousEngine@@YAXM@Z", rayMaximum);
        S(e, "?raySetMinDistance@SeriousEngine@@YAXM@Z", rayMinimum);
        S(e, "?raySetRayRadius@SeriousEngine@@YAXM@Z", rayRadius);
        S(e, "?cldCheckRay@SeriousEngine@@YAHXZ", checkRay);
        S(e, "?rayIsHit@SeriousEngine@@YAHXZ", rayHit);
        S(e, "?rayGetHitDistance@SeriousEngine@@YAMXZ", hitDistance);
        S(e, "?cldSetRayCategory@SeriousEngine@@YAXVIDENT@1@@Z", rayCategory);
        S(e, "?cldSetThickRayCategory@SeriousEngine@@YAXVIDENT@1@@Z", thickCategory);
        S(e, "?cldSetAvatar@SeriousEngine@@YAXPAVCEntity@1@@Z", rayAvatar);
        S(e, "?cldSetAvatarMechanism@SeriousEngine@@YAXPAVCMechanism@1@@Z", rayMechanism);
        S(e, "?cldSetRayTestsFluids@SeriousEngine@@YAXH@Z", rayFluids);
        S(e, "?gfuDrawLine3f@SeriousEngine@@YAXABVVector3f@1@0KK@Z", drawLine);
        S(e, "?_gfxEnableDepthBuffer@SeriousEngine@@3P6AXXZA", enableDepth);
        S(e, "?_gfxDisableDepthBuffer@SeriousEngine@@3P6AXXZA", disableDepth);
        S(e, "?_gfxEnableDepthWrite@SeriousEngine@@3P6AXXZA", enableDepthWrite);
        S(e, "?_gfxDisableDepthWrite@SeriousEngine@@3P6AXXZA", disableDepthWrite);
        S(e, "?_gfx_bDepthBuffer@SeriousEngine@@3HA", depthEnabled);
        S(e, "?_gfx_bDepthWrite@SeriousEngine@@3HA", depthWriting);
        S(e, "?_gfxEnableBlend@SeriousEngine@@3P6AXXZA", enableBlend);
        S(e, "?_gfxDisableBlend@SeriousEngine@@3P6AXXZA", disableBlend);
        S(e, "?_gfxEnableAlphaTest@SeriousEngine@@3P6AXXZA", enableAlpha);
        S(e, "?_gfxDisableAlphaTest@SeriousEngine@@3P6AXXZA", disableAlpha);
        S(e, "?_gfx_bBlending@SeriousEngine@@3HA", blending);
        S(e, "?_gfx_bAlphaTest@SeriousEngine@@3HA", alphaTesting);
        S(e, "?_gfx_eDepthFunc@SeriousEngine@@3W4GfxComp@1@A", depthComparison);
        S(e, "?_gfxDepthFunc@SeriousEngine@@3P6AXW4GfxComp@1@@ZA", setDepthComparison);
        S(e, "?gfuOrtho@SeriousEngine@@YAXXZ", ortho);
        S(e, "?ActivateExecution@CViewRenCmd@SeriousEngine@@QAEXXZ", activateView);
        S(e, "?_gfx_pcvCurrent@SeriousEngine@@3PAVCCanvas@1@A", currentCanvas);
        S(e, "?_gfx_mCurrentProjection@SeriousEngine@@3VMatrix44f@1@A", nativeCurrentProjection);
        S(e, "?_gfx_mAdjustedProjection@SeriousEngine@@3VMatrix44f@1@A", nativeAdjustedProjection);
        S(e, "?_gfx_hCurrentVertexProgram@SeriousEngine@@3JA", uiVertexHandle);
        S(e, "?_gfx_hCurrentPixelProgram@SeriousEngine@@3JA", uiPixelHandle);
        S(e, "?_gfx_avppPrograms@SeriousEngine@@3V?$CStaticStackArray@UVexPixProgram@SeriousEngine@@@1@A", uiPrograms);
        uiSimplePrograms = reinterpret_cast<int32_t *>(reinterpret_cast<uint8_t *>(e)+0x2e6f2c);
        S(e, "?_gfx_mCurrentView@SeriousEngine@@3VMatrix34f@1@A", nativeCurrentView);
        S(e, "?_gfx_fDepthNear@SeriousEngine@@3MA", nativeDepthNear);
        S(e, "?_gfx_fDepthFar@SeriousEngine@@3MA", nativeDepthFar);
        S(e, "?_gfxDepthRange@SeriousEngine@@3P6AXMM@ZA", nativeDepthRange);
        S(e, "?_gfxProjectionMatrix@SeriousEngine@@3P6AXABVMatrix44f@1@@ZA", nativeProjectionSet);
        S(e, "?_gfx_ulCachedMatrices@SeriousEngine@@3KA", nativeCachedMatrices);
    }
#undef S
    if (!ok)
        return false;
    // Exact installed-build instruction boundaries, checked before any hook is
    // created. The five trampolines relocate these original instructions; in
    // particular 8E75A remains the ONLY writer of native operator history.
    struct PrimaryBytes { uint32_t rva; uint8_t size; uint8_t bytes[6]; };
    constexpr PrimaryBytes primaryBytes[] = {
        {0x8e580,6,{0x55,0x8b,0xec,0x83,0xec,0x18}},
        {0x8e480,6,{0x55,0x8b,0xec,0x56,0x8b,0xf1}},
        {0x8e5bb,6,{0x8a,0x86,0x48,0x03,0x00,0x00}},
        {0x8e5e0,5,{0x83,0xc4,0x04,0x84,0xcb}},
        {0x8e610,5,{0x83,0xc4,0x04,0x84,0xcb}},
        {0x8e75a,6,{0x88,0x8e,0x48,0x03,0x00,0x00}},
        {0x8e4dd,5,{0x83,0xc4,0x04,0x23,0xc3}},
    };
    for (const auto &entry : primaryBytes)
        if (memcmp(reinterpret_cast<const uint8_t *>(g) + entry.rva, entry.bytes, entry.size)) {
            log("Native primary instruction mismatch: %x", unsigned(entry.rva));
            return false;
        }
    if (!headless) {
        toId(&bulletCategory, "bullet");
        toId(&laserIdleAttachment, "Barrel01");
    }
#define H(module, name, fn, orig)                                                                            \
    ok = hook(module, name, reinterpret_cast<void *>(fn), reinterpret_cast<void **>(&orig)) && ok
    if (headless) {
        ok = symbol(g, "?GetCameraPlacement@CPuppetEntity@SeriousEngine@@UAE?AVQuatVect@2@XZ",
                    originalCamera) &&
             ok;
        ok = symbol(g, "?GetWeaponAbsPlacement@CBaseWeaponEntity@SeriousEngine@@QAEHABVMatrix34f@2@AAV32@@Z",
                    originalWeaponAbs) &&
             ok;
    } else {
        H(g, "?GetCameraPlacement@CPuppetEntity@SeriousEngine@@UAE?AVQuatVect@2@XZ", camera, originalCamera);
        H(g, "?GetProjectionMatrix@CPuppetEntity@SeriousEngine@@QAE?AVMatrix44f@2@XZ", project,
          originalProjection);
        H(g, "?Render3D@CPuppetEntity@SeriousEngine@@UAEXXZ", render, originalRender);
        H(g, "?RenderView@CPlayerBrainEntity@SeriousEngine@@UAEXXZ", brainRender, originalBrainRender);
        H(g, "?RenderOverlay@CPlayerPuppetEntity@SeriousEngine@@UAEXH@Z", overlayRender, originalOverlayRender);
        H(e, "?gfuFill@SeriousEngine@@YAXK@Z", nativeFill, originalNativeFill);
        H(g, "?RendersIn3rdPerson@CPuppetEntity@SeriousEngine@@UAEHXZ", renderThirdPerson,
          originalThirdPerson);
        H(g, "?ClampLookDirEul@CPlayerBrainEntity@SeriousEngine@@QAEXAAVVector3f@2@@Z", mountedLookClamp,
          originalLookClamp);
        H(e, "?Prepare@CViewRenCmd@SeriousEngine@@QAEXABVMatrix34f@2@ABVMatrix44f@2@ABVBox1f@2@K@Z",
          viewPrepare, originalViewPrepare);
        H(e, "?Execute@CViewRenCmd@SeriousEngine@@UAEXXZ", viewExecute, originalViewExecute);
        H(e, "?rayInit@SeriousEngine@@YAXXZ", trackedRayInit, originalRayInit);
    }
    H(g, "?OnStep@CPlayerPuppetEntity@SeriousEngine@@UAEXXZ", step, originalStep);
    H(g, "?OnDelete@CPlayerPuppetEntity@SeriousEngine@@UAEXXZ", deleted, originalDelete);
    H(g, "?OnDelete@CBaseWeaponEntity@SeriousEngine@@UAEXXZ", weaponDeleted, originalWeaponDelete);
    H(g, "?OnStep@CSniperWeaponEntity@SeriousEngine@@UAEXXZ", sniperStep, originalSniperStep);
    H(g, "?OnStep@CBaseWeaponEntity@SeriousEngine@@UAEXXZ", baseWeaponStep, originalBaseWeaponStep);
    H(g, "?OnFire@CSniperWeaponEntity@SeriousEngine@@UAEHM@Z", sniperFire, originalSniperFire);
    H(g, "?ActivateZoomMode@CSniperWeaponEntity@SeriousEngine@@UAEXXZ", zoomActivated, originalZoomActivate);
    H(g, "?DeactivateZoomMode@CSniperWeaponEntity@SeriousEngine@@UAEXXZ", zoomDeactivated, originalZoomDeactivate);
    H(g, "?PutDown@CSniperWeaponEntity@SeriousEngine@@UAEXH@Z", sniperPutDown, originalSniperPutDown);
    H(g, "?OnDelete@CSniperWeaponEntity@SeriousEngine@@UAEXXZ", sniperDeleted, originalSniperDelete);
    H(g, "?OnAlternativeFirePressed@CSniperWeaponEntity@SeriousEngine@@UAEHXZ",
      sniperAlternativePressed, originalSniperAlternativePress);
    H(g, "?ExecuteOperatorFiring@CPuppetEntity@SeriousEngine@@UAEXXZ", operatorFiring, originalOperatorFiring);
    H(g, "?IsFireButtonPressed@CPuppetEntity@SeriousEngine@@UAEHJ@Z", fireButtonPressed, originalFireButtonPressed);
    H(g, "?GetShootingPlacement@CBaseWeaponEntity@SeriousEngine@@UAE?AVQuatVect@2@XZ", shot, originalShot);
    H(g, "?GetShootingPlacement@CSniperWeaponEntity@SeriousEngine@@UAE?AVQuatVect@2@XZ", sniperShot,
      originalSniperShot);
    if (!headless) {
        H(g, "?DoTheFiring@CBaseWeaponEntity@SeriousEngine@@UAEHM@Z", nativeFire, originalNativeFire);
        H(g, "?GetWeaponAbsPlacement@CBaseWeaponEntity@SeriousEngine@@QAEHABVMatrix34f@2@AAV32@@Z", weaponAbs,
          originalWeaponAbs);
        H(g, "?Render@CBaseWeaponEntity@SeriousEngine@@UAEXVMatrix34f@2@@Z", weaponRender,
          originalWeaponRender);
        H(g, "?Render@CSniperWeaponEntity@SeriousEngine@@UAEXVMatrix34f@2@@Z", sniperRender,
          originalSniperRender);
        H(c, "?mthFrustumFOVX@SeriousEngine@@YA?AVMatrix44f@1@MMMM@Z", weaponFrustum, originalFrustum);
        H(c, "?mthInvertM34f@SeriousEngine@@YA?AVMatrix34f@1@ABV21@@Z", weaponMatrixInverse,
          originalMatrixInverse);
        ok = internalHook(graphics, 0x56a0, reinterpret_cast<void *>(weaponDepthRange),
                          reinterpret_cast<void **>(&originalDepthRange)) && ok;
        H(e, "?PollValues@CInputBindings@SeriousEngine@@QAEXXZ", poll, originalPoll);
        H(e, "?GetCommandValue@CInputBindings@SeriousEngine@@QAEMVIDENT@2@@Z", commandValue, originalValue);
        H(e, "?IsCommandDown@CInputBindings@SeriousEngine@@QAEHVIDENT@2@@Z", commandDown, originalDown);
        H(e, "?IsCommandPressed@CInputBindings@SeriousEngine@@QAEHVIDENT@2@@Z", commandPressed,
          originalPressed);
        H(e, "?IsCommandReleased@CInputBindings@SeriousEngine@@QAEHVIDENT@2@@Z", commandReleased,
          originalReleased);
        H(e, "?IsCommandPressedOrRepeated@CInputBindings@SeriousEngine@@QAEHVIDENT@2@@Z", commandRepeated,
          originalRepeated);
    }
    H(e, "?Step@CSimulation@SeriousEngine@@QAEXXZ", simulationStep, originalSimulationStep);
#undef H
    ok = internalHook(g, 0x8e5bb, reinterpret_cast<void *>(primaryDownPredicate), &primaryDownPredicate_original) && ok;
    ok = internalHook(g, 0x8e5e0, reinterpret_cast<void *>(primaryPressPredicate), &primaryPressPredicate_original) && ok;
    ok = internalHook(g, 0x8e610, reinterpret_cast<void *>(primaryReleasePredicate), &primaryReleasePredicate_original) && ok;
    ok = internalHook(g, 0x8e75a, reinterpret_cast<void *>(primaryHistoryPredicate), &primaryHistoryPredicate_original) && ok;
    ok = internalHook(g, 0x8e4dd, reinterpret_cast<void *>(primaryHeldPredicate), &primaryHeldPredicate_original) && ok;
    ok = internalHook(g,0x171d00,reinterpret_cast<void *>(zoomActivatePredicate),&zoomActivatePredicate_original) && ok;
    ok = internalHook(g,0x171e05,reinterpret_cast<void *>(zoomOwnerPredicate),&zoomOwnerPredicate_original) && ok;
    ok = internalHook(g,0x171e7b,reinterpret_cast<void *>(zoomFovPredicate),&zoomFovPredicate_original) && ok;
    ok = internalHook(g,0x172854,reinterpret_cast<void *>(zoomInterpolatePredicate),&zoomInterpolatePredicate_original) && ok;
    ok = internalHook(g,0x17297f,reinterpret_cast<void *>(zoomSoundStartPredicate),&zoomSoundStartPredicate_original) && ok;
    ok = internalHook(g,0x172a3c,reinterpret_cast<void *>(zoomSoundStopPredicate),&zoomSoundStopPredicate_original) && ok;
    ok = multiplayer::initialize(e, c, g, hook) && ok;
    ok = internalHook(e, 0x1b3b60, reinterpret_cast<void *>(entityStep),
                      reinterpret_cast<void **>(&originalEntityStep)) &&
         ok;
    if (!headless) {
        ok = menus::initialize(g, internalHook) && ok;
        ok = remote_render::initialize(e, c, g, internalHook, settings.remoteHeadTracking) && ok;
    }
    if (!ok) {
        rollbackNativeHooks();
        return false;
    }
    ok = MH_ApplyQueued() == MH_OK;
    if (!ok)
        rollbackNativeHooks();
    hooksReady = ok;
    log("Native VR hooks %s; no game/headset verification", ok ? "attached" : "failed");
    return ok;
}
} // namespace ss2vr::game
