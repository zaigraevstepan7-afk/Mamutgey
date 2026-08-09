#include "game.h"
#include <cstdio>
#include <fstream>
#include <sstream>
#include <unistd.h>

using namespace Offsets;

uintptr_t FindLibBase(const char* name) {
    std::ifstream maps("/proc/self/maps");
    std::string line;
    while (std::getline(maps, line)) {
        if (line.find(name) == std::string::npos) continue;
        // first mapping for the lib
        uintptr_t start = 0;
        std::stringstream ss(line);
        ss >> std::hex >> start;
        return start;
    }
    return 0;
}

bool Il2CppReady() {
    if (!UBase) {
        UBase = FindLibBase("libil2cpp.so");
        if (UBase) LOGI("libil2cpp.so @ %p", (void*)UBase);
    }
    return UBase != 0;
}

static void* Call_get_Data(void* player) {
    using Fn = void* (*)(void*, const void*);
    return AsPtr<Fn>(PlayerControl_get_Data)(player, nullptr);
}

static Il2CppString* Call_get_PlayerName(void* npi) {
    using Fn = Il2CppString* (*)(void*, const void*);
    return AsPtr<Fn>(NetworkedPlayerInfo_get_PlayerName)(npi, nullptr);
}

static void* Call_get_DefaultOutfit(void* npi) {
    using Fn = void* (*)(void*, const void*);
    return AsPtr<Fn>(NetworkedPlayerInfo_get_DefaultOutfit)(npi, nullptr);
}

static void* Call_get_transform(void* component) {
    using Fn = void* (*)(void*, const void*);
    return AsPtr<Fn>(Component_get_transform)(component, nullptr);
}

static Vector3 Call_get_position(void* transform) {
    Vector3 out{};
    using Fn = void (*)(void*, Vector3*, const void*);
    AsPtr<Fn>(0x44565FC)(transform, &out, nullptr); // get_position_Injected
    return out;
}

static void* Call_Camera_main() {
    using Fn = void* (*)(const void*);
    return AsPtr<Fn>(Camera_get_main)(nullptr);
}

static Vector3 Call_WorldToScreen(void* cam, Vector3 world) {
    Vector3 out{};
    // WorldToScreenPoint_Injected(this, &pos, eye, &ret, method) eye: Mono=2
    using Fn = void (*)(void*, Vector3*, int32_t, Vector3*, const void*);
    AsPtr<Fn>(0x4411038)(cam, &world, 2, &out, nullptr);
    return out;
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
    // Joined lobby still useful for testing names; Started = in round
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

    void* cam = Call_Camera_main();
    Vector3 localPos{};
    bool haveLocalPos = false;
    if (local && IsUnityAlive(local)) {
        void* tr = Call_get_transform(local);
        if (tr) {
            localPos = Call_get_position(tr);
            haveLocalPos = true;
        }
    }

    for (int i = 0; i < list->size; ++i) {
        void* player = list->items->vector[i];
        if (!player || !IsUnityAlive(player)) continue;
        if (Read<bool>(player, PC_isDummy) || Read<bool>(player, PC_notRealPlayer)) continue;

        EspPlayer ep;
        ep.player = player;
        ep.playerId = Read<uint8_t>(player, PC_PlayerId);
        ep.isLocal = (player == local);

        void* data = Read<void*>(player, PC_CachedPlayerData);
        if (!data) data = Call_get_Data(player);
        ep.data = data;
        if (data) {
            ep.isDead = Read<bool>(data, NPI_IsDead);
            ep.disconnected = Read<bool>(data, NPI_Disconnected);
            if (ep.disconnected) continue;
            if (g_Cheat.hideDead && ep.isDead) continue;

            ep.role = static_cast<RoleTypes>(Read<uint16_t>(data, NPI_RoleType));
            void* roleBeh = Read<void*>(data, NPI_Role);
            if (roleBeh) {
                int team = Read<int32_t>(roleBeh, RB_TeamType);
                auto roleFromBeh = static_cast<RoleTypes>(Read<uint16_t>(roleBeh, RB_Role));
                ep.role = roleFromBeh;
                ep.isMurder = (team == 1) || IsMurderRole(roleFromBeh);
            } else {
                ep.isMurder = IsMurderRole(ep.role);
            }

            Il2CppString* nameStr = Call_get_PlayerName(data);
            ep.name = Il2CppStringToUtf8(nameStr);
            void* outfit = Call_get_DefaultOutfit(data);
            if (outfit) {
                ep.colorId = Read<int32_t>(outfit, Outfit_ColorId);
                if (ep.name.empty()) {
                    ep.name = Il2CppStringToUtf8(Read<Il2CppString*>(outfit, Outfit_PlayerName));
                }
            }
            if (ep.name.empty()) {
                char tmp[32];
                snprintf(tmp, sizeof(tmp), "P%u", ep.playerId);
                ep.name = tmp;
            }
        }

        void* tr = Call_get_transform(player);
        if (!tr) continue;
        ep.world = Call_get_position(tr);
        if (haveLocalPos) ep.distance = Dist2D(localPos, ep.world);

        if (cam && IsUnityAlive(cam)) {
            Vector3 sp = Call_WorldToScreen(cam, ep.world);
            ep.onScreen = (sp.z > 0.f);
            ep.screen = {sp.x, sp.y};
        }

        next.push_back(std::move(ep));
    }

    std::lock_guard<std::mutex> lk(g_EspMutex);
    g_EspSnapshot.swap(next);
}
