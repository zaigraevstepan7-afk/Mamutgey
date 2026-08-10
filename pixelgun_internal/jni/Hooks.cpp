#include "Hooks.h"
#include "Functions.h"
#include "Offsets.h"
#include "And64InlineHook.hpp"

#include <cstring>

// ---- helpers copied from stanuwu Hooks.cpp ----
static std::string get_player_name(void* player_move_c) {
    if (!player_move_c) return {};
    void* nick = *reinterpret_cast<void**>((uintptr_t)player_move_c + Offsets::nickLabel);
    if (!nick) return {};
    auto* s = (Il2CppString*)Functions::TextMeshGetText(nick);
    return Il2CppToUtf8(s);
}

static void* get_player_transform(void* player) {
    if (!player) return nullptr;
    return *reinterpret_cast<void**>((uintptr_t)player + Offsets::myPlayerTransform);
}

static bool is_player_enemy(void* player) {
    if (!player) return false;
    void* nick = *reinterpret_cast<void**>((uintptr_t)player + Offsets::nickLabel);
    if (!nick) return false;
    Color c{};
    Functions::TextMeshGetColor(nick, &c);
    return c.r == 1.f && c.g == 0.f && c.b == 0.f; // stanuwu: red = enemy
}

static bool is_my_player(void* player_move_c) {
    return get_player_name(player_move_c) == "#Player Nickname";
}

// ---- ModuleESP::add_esp logic (stanuwu) ----
static void add_esp(void* player) {
    if (!player || !g_OurPlayer || !g_MainCamera) return;
    if (!g_Cheat.espEnabled) return;

    void* transform = get_player_transform(player);
    if (!transform) return;

    Vector3 position{};
    Functions::TransformGetPosition(transform, &position);
    Vector3 top_world{position.x, position.y + 2.f, position.z};
    Vector3 screen_pos{}, screen_top{};
    Functions::CameraWorldToScreen(g_MainCamera, &position, &screen_pos);
    Functions::CameraWorldToScreen(g_MainCamera, &top_world, &screen_top);

    if (screen_pos.z < 0.01f) return;

    float scaled = screen_pos.y - screen_top.y;
    float width2 = scaled / 2.f;
    float height2 = scaled * 1.5f / 2.f;

    bool enemy = is_player_enemy(player);
    if (!enemy && !g_Cheat.showTeammates) return;

    void* cam_tr = Functions::ComponentGetTransform(g_MainCamera);
    Vector3 cam_pos{};
    if (cam_tr) Functions::TransformGetPosition(cam_tr, &cam_pos);
    float dist = Dist3(position, cam_pos);

    EspPlayer ep;
    ep.sx = screen_pos.x;
    ep.sy = screen_pos.y; // overlay flips Y later using view height
    ep.sz = screen_pos.z;
    ep.width2 = width2;
    ep.height2 = height2;
    ep.distance = dist;
    ep.enemy = enemy;
    ep.onScreen = true;
    ep.name = get_player_name(player);

    std::lock_guard<std::mutex> lk(g_EspMu);
    g_EspList.push_back(std::move(ep));
}

using UpdateFn = void (*)(void* thiz, const void* method);
static UpdateFn s_OriginalUpdate = nullptr;

static void hooked_PlayerMoveC_Update(void* thiz, const void* method) {
    // Clear list once per frame from local player path
    bool mine = is_my_player(thiz);
    if (mine) {
        {
            std::lock_guard<std::mutex> lk(g_EspMu);
            g_EspList.clear();
        }
        g_OurPlayer = thiz;
        g_MainCamera = Functions::CameraGetMain();
    } else if (g_OurPlayer && g_MainCamera) {
        add_esp(thiz);
    }
    if (s_OriginalUpdate) s_OriginalUpdate(thiz, method);
}

void Hooks_Install() {
    if (!g_Il2Cpp || !Offsets::PlayerMoveCUpdate) {
        LOGE("Hooks_Install: missing Update RVA");
        return;
    }
    void* target = (void*)(g_Il2Cpp + Offsets::PlayerMoveCUpdate);
    A64HookFunction(target, (void*)hooked_PlayerMoveC_Update, (void**)&s_OriginalUpdate);
    LOGI("hooked PlayerMoveC.Update @ %p (RVA 0x%lx)", target,
         (unsigned long)Offsets::PlayerMoveCUpdate);
}

void Hooks_DrawTick() {}
