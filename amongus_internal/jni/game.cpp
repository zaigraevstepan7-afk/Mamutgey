#include "game.h"
#include <cstdio>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <dlfcn.h>
#include <atomic>
#include <cstring>

using namespace Offsets;

static std::atomic<bool> g_Il2CppThreadReady{false};
static std::atomic<int> g_WarmupTicks{0};
static char g_Status[192] = "init";

static void SetStatus(const char* s) {
    if (!s) return;
    strncpy(g_Status, s, sizeof(g_Status) - 1);
    g_Status[sizeof(g_Status) - 1] = 0;
    LOGI("status: %s", g_Status);
}

const char* Game_Status() { return g_Status; }

uintptr_t FindLibBase(const char* name) {
    std::ifstream maps("/proc/self/maps");
    std::string line;
    uintptr_t best = 0;
    while (std::getline(maps, line)) {
        if (line.find("libil2cpp.so") == std::string::npos) continue;
        if (name && line.find(name) == std::string::npos) continue;
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

static std::string FindLibPath() {
    std::ifstream maps("/proc/self/maps");
    std::string line;
    while (std::getline(maps, line)) {
        if (line.find("libil2cpp.so") == std::string::npos) continue;
        auto p = line.find('/');
        if (p != std::string::npos) return line.substr(p);
    }
    return {};
}

bool Il2CppReady() {
    if (!UBase) {
        UBase = FindLibBase("libil2cpp.so");
        if (UBase) LOGI("libil2cpp.so @ %p", (void*)UBase);
    }
    return UBase != 0;
}

bool Il2CppAttachThread() {
    if (g_Il2CppThreadReady.load()) return true;
    if (!Il2CppReady()) {
        SetStatus("no il2cpp base");
        return false;
    }

    using DomainFn = void* (*)();
    using AttachFn = void* (*)(void*);

    DomainFn domain_get = reinterpret_cast<DomainFn>(dlsym(RTLD_DEFAULT, "il2cpp_domain_get"));
    AttachFn thread_attach = reinterpret_cast<AttachFn>(dlsym(RTLD_DEFAULT, "il2cpp_thread_attach"));

    if (!domain_get || !thread_attach) {
        void* mod = nullptr;
        std::string path = FindLibPath();
        if (!path.empty()) {
            // strip trailing spaces
            while (!path.empty() && (path.back() == ' ' || path.back() == '\r')) path.pop_back();
            mod = dlopen(path.c_str(), RTLD_NOW);
        }
        if (!mod) mod = dlopen("libil2cpp.so", RTLD_NOW | RTLD_NOLOAD);
        if (!mod) mod = dlopen("libil2cpp.so", RTLD_NOW);
        if (!mod) {
            SetStatus("dlopen il2cpp fail");
            return false;
        }
        domain_get = reinterpret_cast<DomainFn>(dlsym(mod, "il2cpp_domain_get"));
        thread_attach = reinterpret_cast<AttachFn>(dlsym(mod, "il2cpp_thread_attach"));
    }

    if (!domain_get || !thread_attach) {
        SetStatus("il2cpp symbols missing");
        return false;
    }
    void* domain = domain_get();
    if (!domain) {
        SetStatus("il2cpp domain null");
        return false;
    }
    thread_attach(domain);
    g_Il2CppThreadReady.store(true);
    SetStatus("il2cpp attached");
    return true;
}

// Validate Il2CppClass* by reading name pointer at +0x10 (Il2CppClass_1.name)
static bool ClassNameIs(void* klass, const char* expect) {
    if (!klass || !expect) return false;
    const char* name = *reinterpret_cast<const char**>(reinterpret_cast<uintptr_t>(klass) + 0x10);
    if (!name) return false;
    // quick sanity: first char printable
    if (name[0] < 0x20 || name[0] > 0x7e) return false;
    return std::strcmp(name, expect) == 0;
}

static void* GetStaticFieldsVerified(void* klass) {
    if (!klass) return nullptr;
    // Primary: Il2CppClass.static_fields @ 0xB8 (confirmed from this dump's il2cpp.h)
    void* sf = *reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(klass) + 0xB8);
    return sf;
}

static void* GetPlayerControlClass() {
    void* ti = GetTypeInfo(PlayerControl_TypeInfo);
    if (!ti) {
        SetStatus("PC TypeInfo null");
        return nullptr;
    }
    if (!ClassNameIs(ti, "PlayerControl")) {
        // Still try — name layout might differ, but log it
        const char* name = *reinterpret_cast<const char**>(reinterpret_cast<uintptr_t>(ti) + 0x10);
        LOGI("PC klass name@+0x10 = %s (expected PlayerControl)", name ? name : "(null)");
    }
    return ti;
}

static void* GetLocalPlayer() {
    void* ti = GetPlayerControlClass();
    if (!ti) return nullptr;
    void* sf = GetStaticFieldsVerified(ti);
    if (!sf) {
        SetStatus("PC static_fields null");
        return nullptr;
    }
    void* local = *reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(sf) + PC_LocalPlayer);
    return local;
}

void* Game_GetLocalPlayer() { return GetLocalPlayer(); }

static void* GetAllPlayersList() {
    void* ti = GetPlayerControlClass();
    if (!ti) return nullptr;
    void* sf = GetStaticFieldsVerified(ti);
    if (!sf) return nullptr;
    return *reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(sf) + PC_AllPlayerControls);
}

static int GetGameState() {
    void* ti = GetTypeInfo(AmongUsClient_TypeInfo);
    if (!ti) return -1;
    void* sf = GetStaticFieldsVerified(ti);
    if (!sf) return -1;
    // AmongUsClient inherits InnerNetClient; Instance is typically static on AmongUsClient
    // Dump: AmongUsClient has static Instance via DestroyableSingleton pattern OR
    // InnerNetClient fields on instance. ScriptMetadata AmongUsClient_TypeInfo statics:
    // Check il2cpp — AmongUsClient_StaticFields
    void* client = *reinterpret_cast<void**>(sf);
    if (!client) return -1;
    if (!IsUnityAlive(client)) return -1;
    return Read<int32_t>(client, AUC_GameState);
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

static void SetBehaviourEnabled(void* behaviour, bool enabled) {
    if (!behaviour) return;
    // Collider2D is a UnityEngine.Object — check native ptr
    if (!IsUnityAlive(behaviour)) return;
    using Fn = void (*)(void*, bool, const void*);
    AsPtr<Fn>(Behaviour_set_enabled)(behaviour, enabled, nullptr);
}

static bool GetBehaviourEnabled(void* behaviour) {
    if (!behaviour || !IsUnityAlive(behaviour)) return false;
    using Fn = bool (*)(void*, const void*);
    return AsPtr<Fn>(0x444858C)(behaviour, nullptr); // Behaviour.get_enabled
}

// MethodInfo* slot for DestroyableSingleton<RoleManager>.get_Instance
constexpr uintptr_t Method_DestroyableSingleton_RoleManager_get_Instance = 0x4AB2F98;
constexpr uintptr_t DestroyableSingleton_object_get_Instance = 0x2E6C1A8;
constexpr uintptr_t PlayerControl_SetKillTimer = 0x21B5500;

static void* GetRoleManagerInstance() {
    // Correct path: generic DestroyableSingleton<RoleManager>.get_Instance(MethodInfo*)
    void** slot = reinterpret_cast<void**>(UBase + Method_DestroyableSingleton_RoleManager_get_Instance);
    void* methodInfo = *slot;
    if (!methodInfo) methodInfo = reinterpret_cast<void*>(slot); // some builds store MI inline
    using Fn = void* (*)(const void*);
    void* rm = AsPtr<Fn>(DestroyableSingleton_object_get_Instance)(methodInfo);
    if (!rm || !IsUnityAlive(rm)) {
        SetStatus("RM Instance null");
        return nullptr;
    }
    return rm;
}

static void DoBecomeMurderer(void* local) {
    if (!local || !IsUnityAlive(local)) {
        SetStatus("murder: no local");
        return;
    }

    const uint16_t role = static_cast<uint16_t>(RoleTypes::Impostor);

    // 1) Networked RPC
    using RpcFn = void (*)(void*, uint16_t, bool, const void*);
    AsPtr<RpcFn>(PlayerControl_RpcSetRole)(local, role, true, nullptr);

    // 2) RoleManager.SetRole (needs real Instance)
    void* rm = GetRoleManagerInstance();
    if (rm) {
        using SetFn = void (*)(void*, void*, uint16_t, const void*);
        AsPtr<SetFn>(RoleManager_SetRole)(rm, local, role, nullptr);
    }

    // 3) Kill timer via proper setter
    using KillFn = void (*)(void*, float, const void*);
    AsPtr<KillFn>(PlayerControl_SetKillTimer)(local, 0.f, nullptr);

    // 4) Direct memory writes as fallback (verified dump offsets)
    void* data = Read<void*>(local, PC_CachedPlayerData);
    if (data) {
        *reinterpret_cast<uint16_t*>(reinterpret_cast<uintptr_t>(data) + NPI_RoleType) = role;
        void* roleBeh = Read<void*>(data, NPI_Role);
        if (roleBeh) {
            *reinterpret_cast<uint16_t*>(reinterpret_cast<uintptr_t>(roleBeh) + RB_Role) = role;
            *reinterpret_cast<int32_t*>(reinterpret_cast<uintptr_t>(roleBeh) + RB_TeamType) = 1; // Impostor
            *reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(roleBeh) + RB_CanUseKillButton) = true;
        }
    }

    SetStatus("murder applied");
}

static void ApplyNoclip(void* local, bool on) {
    if (!local || !IsUnityAlive(local)) return;

    // Primary: PlayerControl.Collider (dump 0xC8) — wall collision
    void* col = Read<void*>(local, PC_Collider);
    if (col) {
        SetBehaviourEnabled(col, !on);
        bool en = GetBehaviourEnabled(col);
        // If managed call didn't stick, we still report
        if (on && en) {
            // retry once
            SetBehaviourEnabled(col, false);
        }
    }

    // Also toggle physics body simulated? NO — that freezes movement.
    // Instead ensure moveable flag is true when noclip on
    if (on) {
        *reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(local) + 0x4C) = true; // moveable
    }

    static int logThrottle = 0;
    if ((++logThrottle % 60) == 0) {
        char buf[128];
        snprintf(buf, sizeof(buf), "noclip=%d col=%p en=%d",
                 on ? 1 : 0, col, col ? (GetBehaviourEnabled(col) ? 1 : 0) : -1);
        SetStatus(buf);
    }
}

void Game_ApplyCheats() {
    if (!Il2CppReady()) return;
    if (!Il2CppAttachThread()) return;

    void* local = GetLocalPlayer();
    if (!local) {
        static int t = 0;
        if ((++t % 90) == 0) SetStatus("wait LocalPlayer");
        return;
    }
    if (!IsUnityAlive(local)) {
        SetStatus("LocalPlayer dead ptr");
        return;
    }

    // Always apply noclip state (re-assert each tick)
    ApplyNoclip(local, g_Cheat.noclip.load());

    if (g_Cheat.becomeMurderPending.exchange(false)) {
        DoBecomeMurderer(local);
    }
}

void Game_TickCollect() {
    if (!Il2CppReady()) return;
    if (!Il2CppAttachThread()) return;

    g_WarmupTicks.fetch_add(1);

    // Skip heavy ESP work unless enabled
    if (!g_Cheat.espEnabled.load() && !g_Cheat.murderEspEnabled.load()) {
        std::lock_guard<std::mutex> lk(g_EspMutex);
        g_EspSnapshot.clear();
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

        char tmp[32];
        snprintf(tmp, sizeof(tmp), "P%u", ep.playerId);
        ep.name = tmp;

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
