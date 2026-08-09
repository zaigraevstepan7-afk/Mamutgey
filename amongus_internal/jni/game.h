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
    std::atomic<bool> espEnabled{true};
    std::atomic<bool> murderEspEnabled{true};
    std::atomic<bool> espBox{true};
    std::atomic<bool> espLine{true};
    std::atomic<bool> espName{true};
    std::atomic<bool> espDistance{true};
    std::atomic<bool> espRole{true};
    std::atomic<bool> hideDead{true};
};

inline CheatState g_Cheat;
inline std::mutex g_EspMutex;
inline std::vector<EspPlayer> g_EspSnapshot;

bool Il2CppReady();
bool Il2CppAttachThread();
void Game_TickCollect();
uintptr_t FindLibBase(const char* name);
