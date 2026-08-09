#pragma once
#include "il2cpp_api.h"
#include "offsets.h"
#include <mutex>
#include <vector>
#include <atomic>

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
    std::atomic<bool> espEnabled{false};
    std::atomic<bool> murderEspEnabled{false};
    std::atomic<bool> espBox{false};
    std::atomic<bool> espLine{false};
    std::atomic<bool> espName{false};
    std::atomic<bool> espDistance{false};
    std::atomic<bool> espRole{false};
    std::atomic<bool> hideDead{true};

    std::atomic<bool> noclip{false};
    std::atomic<bool> becomeMurderPending{false};
};

inline CheatState g_Cheat;
inline std::mutex g_EspMutex;
inline std::vector<EspPlayer> g_EspSnapshot;

bool Il2CppReady();
bool Il2CppAttachThread();
void Game_TickCollect();
void Game_ApplyCheats();
uintptr_t FindLibBase(const char* name);
void* Game_GetLocalPlayer();
