#pragma once
#include <mutex>
#include <vector>
#include <string>
#include "log.h"

// ESP list — same idea as stanuwu ModuleESP::to_draw
struct EspPlayer {
    float sx, sy, sz;
    float width2, height2;
    float distance;
    bool enemy;
    bool onScreen;
    std::string name;
};

struct CheatState {
    bool espEnabled = true;
    bool showTeammates = true;
    bool showDistance = true;
    bool showTracers = true;
};

inline CheatState g_Cheat;
inline std::mutex g_EspMu;
inline std::vector<EspPlayer> g_EspList;
inline void* g_OurPlayer = nullptr;
inline void* g_MainCamera = nullptr;

void Hooks_Install();
void Hooks_DrawTick(); // no-op; snapshot filled from Update hook
