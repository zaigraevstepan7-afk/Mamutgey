#include "game.h"
#include <cstdio>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <dlfcn.h>
#include <atomic>

using namespace Offsets;

static std::atomic<bool> g_Il2CppThreadReady{false};
static std::atomic<int> g_WarmupTicks{0};

uintptr_t FindLibBase(const char* name) {
    std::ifstream maps("/proc/self/maps");
    std::string line;
    uintptr_t best = 0;
    while (std::getline(maps, line)) {
        if (line.find(name) == std::string::npos) continue;
        if (line.find("libil2cpp.so") == std::string::npos) continue;
        uintptr_t start = 0;
        std::stringstream ss(line);
        ss >> std::hex >> start;
        if (!start) continue;
        bool isExec = line.find("r-xp") != std::string::npos;
        if (isExec) {
            if (!best || start < best) best = start;
        } else if (!best) {
            best = start;
        }
    }
    return best;
}

bool Il2CppReady() {
    if (!UBase) {
        UBase = FindLibBase("libil2cpp.so");
        if (UBase) LOGI("libil2cpp.so @ %p", (void*)UBase);
    }
    return UBase != 0;
}

// Must call once on the tick thread before any managed IL2CPP invokes.
bool Il2CppAttachThread() {
    if (g_Il2CppThreadReady.load()) return true;
    if (!Il2CppReady()) return false;

    void* mod = dlopen("libil2cpp.so", RTLD_NOW);
    if (!mod) mod = dlopen("libil2cpp.so", RTLD_NOLOAD);
    if (!mod) {
        LOGE("dlopen libil2cpp failed");
        return false;
    }

    using DomainFn = void* (*)();
    using AttachFn = void* (*)(void*);
    auto domain_get = reinterpret_cast<DomainFn>(dlsym(mod, "il2cpp_domain_get"));
    auto thread_attach = reinterpret_cast<AttachFn>(dlsym(mod, "il2cpp_thread_attach"));
    if (!domain_get || !thread_attach) {
        LOGE("il2cpp_domain_get/thread_attach missing");
        return false;
    }
    void* domain = domain_get();
    if (!domain) return false;
    thread_attach(domain);
    g_Il2CppThreadReady.store(true);
    LOGI("il2cpp_thread_attach OK");
    return true;
}

static void* Call_get_transform(void* component) {
    using Fn = void* (*)(void*, const void*);
    return AsPtr<Fn>(Component_get_transform)(component, nullptr);
}

static Vector3 Call_get_position(void* transform) {
    Vector3 out{};
    using Fn = void (*)(void*, Vector3*, const void*);
    AsPtr<Fn>(Transform_get_position_Injected)(transform, &out, nullptr);
    return out;
}

static void* Call_Camera_main() {
    using Fn = void* (*)(const void*);
    return AsPtr<Fn>(Camera_get_main)(nullptr);
}

static Vector3 Call_WorldToScreen(void* cam, Vector3 world) {
    Vector3 out{};
    using Fn = void (*)(void*, Vector3*, int32_t, Vector3*, const void*);
    AsPtr<Fn>(Camera_WorldToScreenPoint_Injected)(cam, &world, 2, &out, nullptr);
    return out;
}

static void* GetLocalPlayer() {
    void* ti = GetTypeInfo(PlayerControl_TypeInfo);
    if (!ti) return nullptr;
    void* sf = GetStaticFields(ti);
    if (!sf) return nullptr;
    return *reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(sf) + PC_LocalPlayer);
}

void* Game_GetLocalPlayer() { return GetLocalPlayer(); }

static void SetBehaviourEnabled(void* behaviour, bool enabled) {
    if (!behaviour || !IsUnityAlive(behaviour)) return;
    using Fn = void (*)(void*, bool, const void*);
    AsPtr<Fn>(Behaviour_set_enabled)(behaviour, enabled, nullptr);
}

static void* GetRoleManager() {
    void* ti = GetTypeInfo(RoleManager_TypeInfo);
    if (!ti) return nullptr;
    void* sf = GetStaticFields(ti);
    if (!sf) return nullptr;
    // DestroyableSingleton<T>._instance @ static 0x0
    return *reinterpret_cast<void**>(sf);
}

static void DoBecomeMurderer(void* local) {
    if (!local || !IsUnityAlive(local)) return;
    const auto role = static_cast<uint16_t>(RoleTypes::Impostor);

    // Prefer networked RPC (works as host; may soft-apply as client)
    using RpcFn = void (*)(void*, uint16_t, bool, const void*);
    AsPtr<RpcFn>(PlayerControl_RpcSetRole)(local, role, true, nullptr);

    // Also try RoleManager.SetRole for local assignment
    void* rm = GetRoleManager();
    if (rm && IsUnityAlive(rm)) {
        using SetFn = void (*)(void*, void*, uint16_t, const void*);
        AsPtr<SetFn>(RoleManager_SetRole)(rm, local, role, nullptr);
    }

    // Reset kill cooldown locally
    *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(local) + PC_killTimer) = 0.f;

    // Force CanUseKillButton on current role behaviour if present
    void* data = Read<void*>(local, PC_CachedPlayerData);
    if (data && IsUnityAlive(data)) {
        void* roleBeh = Read<void*>(data, NPI_Role);
        if (roleBeh && IsUnityAlive(roleBeh)) {
            *reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(roleBeh) + RB_CanUseKillButton) = true;
            *reinterpret_cast<int32_t*>(reinterpret_cast<uintptr_t>(roleBeh) + RB_TeamType) = 1;
            *reinterpret_cast<uint16_t*>(reinterpret_cast<uintptr_t>(roleBeh) + RB_Role) =
                static_cast<uint16_t>(RoleTypes::Impostor);
        }
        *reinterpret_cast<uint16_t*>(reinterpret_cast<uintptr_t>(data) + NPI_RoleType) =
            static_cast<uint16_t>(RoleTypes::Impostor);
    }
    LOGI("become murderer applied");
}

static void ApplyNoclip(void* local, bool on) {
    if (!local || !IsUnityAlive(local)) return;

    // Disable player wall collider — classic Among Us noclip
    void* col = Read<void*>(local, PC_Collider);
    SetBehaviourEnabled(col, !on);

    // Keep rigidbody simulated so movement still works; only collider off
    // Optional: also toggle physics body collider path via MyPhysics.body — leave simulated on
}

void Game_ApplyCheats() {
    if (!Il2CppReady() || !Il2CppAttachThread()) return;
    int warm = g_WarmupTicks.load();
    if (warm < 180) return;

    void* local = GetLocalPlayer();
    if (!local || !IsUnityAlive(local)) return;

    // Noclip every tick so game scripts can't re-enable collider
    ApplyNoclip(local, g_Cheat.noclip.load());

    if (g_Cheat.becomeMurderPending.exchange(false)) {
        DoBecomeMurderer(local);
    }
}

static void* GetAllPlayersList() {
    void* ti = GetTypeInfo(PlayerControl_TypeInfo);
    if (!ti) return nullptr;
    void* sf = GetStaticFields(ti);
    if (!sf) return nullptr;
    return *reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(sf) + PC_AllPlayerControls);
}

static int GetGameState() {
    void* ti = GetTypeInfo(AmongUsClient_TypeInfo);
    if (!ti) return -1;
    void* sf = GetStaticFields(ti);
    if (!sf) return -1;
    void* client = *reinterpret_cast<void**>(sf);
    if (!client || !IsUnityAlive(client)) return -1;
    return Read<int32_t>(client, AUC_GameState);
}

void Game_TickCollect() {
    if (!Il2CppReady()) return;
    if (!Il2CppAttachThread()) return;

    // Warm up a few seconds after attach before touching gameplay objects.
    int warm = g_WarmupTicks.fetch_add(1);
    if (warm < 180) { // ~3s at 16ms
        return;
    }

    std::vector<EspPlayer> next;
    int state = GetGameState();
    if (state != static_cast<int>(GameStates::Started) &&
        state != static_cast<int>(GameStates::Joined)) {
        std::lock_guard<std::mutex> lk(g_EspMutex);
        g_EspSnapshot.swap(next);
        return;
    }

    void* local = GetLocalPlayer();
    void* listObj = GetAllPlayersList();
    if (!listObj) {
        std::lock_guard<std::mutex> lk(g_EspMutex);
        g_EspSnapshot.swap(next);
        return;
    }

    auto* list = reinterpret_cast<Il2CppList*>(listObj);
    if (!list->items || list->size <= 0 || list->size > 32) {
        std::lock_guard<std::mutex> lk(g_EspMutex);
        g_EspSnapshot.swap(next);
        return;
    }
    if (list->items->max_length > 0 &&
        static_cast<uintptr_t>(list->size) > list->items->max_length) {
        std::lock_guard<std::mutex> lk(g_EspMutex);
        g_EspSnapshot.swap(next);
        return;
    }

    void* cam = nullptr;
    // Camera only once match is running — safer than lobby.
    if (state == static_cast<int>(GameStates::Started)) {
        cam = Call_Camera_main();
        if (cam && !IsUnityAlive(cam)) cam = nullptr;
    }

    Vector3 localPos{};
    bool haveLocalPos = false;
    if (local && IsUnityAlive(local)) {
        void* tr = Call_get_transform(local);
        if (tr && IsUnityAlive(tr)) {
            localPos = Call_get_position(tr);
            haveLocalPos = true;
        }
    }

    const bool hideDead = g_Cheat.hideDead.load();

    for (int i = 0; i < list->size; ++i) {
        void* player = list->items->vector[i];
        if (!player || !IsUnityAlive(player)) continue;
        if (Read<bool>(player, PC_isDummy) || Read<bool>(player, PC_notRealPlayer)) continue;

        EspPlayer ep;
        ep.player = player;
        ep.playerId = Read<uint8_t>(player, PC_PlayerId);
        ep.isLocal = (player == local);

        void* data = Read<void*>(player, PC_CachedPlayerData);
        if (data && !IsUnityAlive(data)) data = nullptr;
        ep.data = data;

        if (data) {
            ep.isDead = Read<bool>(data, NPI_IsDead);
            ep.disconnected = Read<bool>(data, NPI_Disconnected);
            if (ep.disconnected) continue;
            if (hideDead && ep.isDead) continue;

            ep.role = static_cast<RoleTypes>(Read<uint16_t>(data, NPI_RoleType));
            void* roleBeh = Read<void*>(data, NPI_Role);
            if (roleBeh && IsUnityAlive(roleBeh)) {
                int team = Read<int32_t>(roleBeh, RB_TeamType);
                auto roleFromBeh = static_cast<RoleTypes>(Read<uint16_t>(roleBeh, RB_Role));
                ep.role = roleFromBeh;
                ep.isMurder = (team == 1) || IsMurderRole(roleFromBeh);
            } else {
                ep.isMurder = IsMurderRole(ep.role);
            }
        }

        // Safe fallback name — no outfit dictionary walks (crashy on worker thread)
        {
            char tmp[32];
            snprintf(tmp, sizeof(tmp), "P%u", ep.playerId);
            ep.name = tmp;
        }

        void* tr = Call_get_transform(player);
        if (!tr || !IsUnityAlive(tr)) continue;
        ep.world = Call_get_position(tr);
        if (haveLocalPos) ep.distance = Dist2D(localPos, ep.world);

        if (cam) {
            Vector3 sp = Call_WorldToScreen(cam, ep.world);
            ep.onScreen = (sp.z > 0.f);
            ep.screen = {sp.x, sp.y};
        }

        next.push_back(std::move(ep));
    }

    std::lock_guard<std::mutex> lk(g_EspMutex);
    g_EspSnapshot.swap(next);
}
