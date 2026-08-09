#include "game.h"
#include <cstdio>
#include <fstream>
#include <sstream>
#include <unistd.h>

using namespace Offsets;

uintptr_t FindLibBase(const char* name) {
    std::ifstream maps("/proc/self/maps");
    std::string line;
    uintptr_t best = 0;
    while (std::getline(maps, line)) {
        if (line.find(name) == std::string::npos) continue;
        // Require path ends with libil2cpp.so (avoid false matches)
        if (line.find("libil2cpp.so") == std::string::npos) continue;
        uintptr_t start = 0;
        std::stringstream ss(line);
        ss >> std::hex >> start;
        if (!start) continue;
        bool isExec = line.find("r-xp") != std::string::npos;
        if (isExec) {
            if (!best || start < best) best = start;
        } else if (!best) {
            best = start; // fallback first mapping
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

// Read Default outfit (PlayerOutfitType=0) from NPI.Outfits without managed calls.
// Dictionary`2 arm64: buckets@0x10, entries@0x18, count@0x20
// Entry<int,object>: hash@0, next@4, key@8, value@16 (size 24)
static void* TryGetDefaultOutfit(void* npi) {
    void* dict = Read<void*>(npi, NPI_Outfits);
    if (!dict) return nullptr;
    auto* entries = *reinterpret_cast<Il2CppArray**>(reinterpret_cast<uintptr_t>(dict) + 0x18);
    int32_t count = *reinterpret_cast<int32_t*>(reinterpret_cast<uintptr_t>(dict) + 0x20);
    if (!entries || count <= 0 || entries->max_length == 0 || entries->max_length > 64)
        return nullptr;
    constexpr size_t kEntrySize = 24;
    auto* base = reinterpret_cast<uint8_t*>(&entries->vector[0]);
    const uintptr_t n = entries->max_length;
    for (uintptr_t i = 0; i < n; ++i) {
        uint8_t* e = base + i * kEntrySize;
        int32_t hash = *reinterpret_cast<int32_t*>(e);
        if (hash < 0) continue;
        int32_t key = *reinterpret_cast<int32_t*>(e + 8);
        if (key != 0) continue; // PlayerOutfitType.Default
        void* val = *reinterpret_cast<void**>(e + 16);
        // PlayerOutfit is a plain managed object (not UnityEngine.Object)
        if (val) return val;
    }
    return nullptr;
}

static void* GetLocalPlayer() {
    void* ti = GetTypeInfo(PlayerControl_TypeInfo);
    void* sf = GetStaticFields(ti);
    if (!sf) return nullptr;
    return *reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(sf) + PC_LocalPlayer);
}

static void* GetAllPlayersList() {
    void* ti = GetTypeInfo(PlayerControl_TypeInfo);
    void* sf = GetStaticFields(ti);
    if (!sf) return nullptr;
    return *reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(sf) + PC_AllPlayerControls);
}

static int GetGameState() {
    void* ti = GetTypeInfo(AmongUsClient_TypeInfo);
    void* sf = GetStaticFields(ti);
    if (!sf) return -1;
    void* client = *reinterpret_cast<void**>(sf);
    if (!client || !IsUnityAlive(client)) return -1;
    return Read<int32_t>(client, AUC_GameState);
}

void Game_TickCollect() {
    if (!Il2CppReady()) return;

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

    void* cam = Call_Camera_main();
    if (cam && !IsUnityAlive(cam)) cam = nullptr;

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

        // Prefer field only — avoid managed get_Data on worker thread when possible
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

            // Name / color from outfit fields — no managed getters (worker thread unsafe)
            void* outfit = TryGetDefaultOutfit(data);
            if (outfit) {
                ep.colorId = Read<int32_t>(outfit, Outfit_ColorId);
                ep.name = Il2CppStringToUtf8(Read<Il2CppString*>(outfit, Outfit_PlayerName));
            }
            if (ep.name.empty()) {
                char tmp[32];
                snprintf(tmp, sizeof(tmp), "P%u", ep.playerId);
                ep.name = tmp;
            }
        } else {
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
            // Orthographic: z is depth; behind camera typically z < 0
            ep.onScreen = (sp.z > 0.f);
            ep.screen = {sp.x, sp.y};
        }

        next.push_back(std::move(ep));
    }

    std::lock_guard<std::mutex> lk(g_EspMutex);
    g_EspSnapshot.swap(next);
}
