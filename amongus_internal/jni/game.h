#pragma once
#include "il2cpp_api.h"
#include "offsets.h"
#include <mutex>
#include <vector>

struct EspPlayer {
    void* player = nullptr;
    void* data = nullptr;
    uint8_t playerId = 0;
    std::string name;
    Offsets::RoleTypes role = Offsets::RoleTypes::Crewmate;
    bool isMurder = false;
    bool isDead = false;
    bool disconnected = false;
    bool isLocal = false;
    Vector3 world{};
    Vector2 screen{};
    bool onScreen = false;
    float distance = 0.f;
    int colorId = 0;
};

struct CheatState {
    bool menuOpen = false;
    bool espEnabled = true;
    bool murderEspEnabled = true;
    bool espBox = true;
    bool espLine = true;
    bool espName = true;
    bool espDistance = true;
    bool espRole = true;
    bool hideDead = true;
    float boxThickness = 2.0f;
};

inline CheatState g_Cheat;
inline std::mutex g_EspMutex;
inline std::vector<EspPlayer> g_EspSnapshot;

bool Il2CppReady();
void Game_TickCollect(); // gather players for ESP (safe-ish to call from render)
uintptr_t FindLibBase(const char* name);
