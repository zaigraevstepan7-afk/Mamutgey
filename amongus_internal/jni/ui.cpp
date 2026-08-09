#include "ui.h"
#include "imgui/imgui.h"
#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <vector>

struct TouchEv {
    float x, y;
    bool down;
};

static std::mutex g_TouchMu;
static std::vector<TouchEv> g_TouchQ;
std::atomic<bool> g_ImguiWantMouse{false};

void Menu_PushTouch(float x, float y, bool down) {
    std::lock_guard<std::mutex> lk(g_TouchMu);
    if (g_TouchQ.size() < 64) g_TouchQ.push_back({x, y, down});
}

void Menu_ApplyTouches() {
    std::vector<TouchEv> q;
    {
        std::lock_guard<std::mutex> lk(g_TouchMu);
        q.swap(g_TouchQ);
    }
    ImGuiIO& io = ImGui::GetIO();
    for (auto& e : q) {
        io.AddMousePosEvent(e.x, e.y);
        io.AddMouseButtonEvent(0, e.down);
    }
}

static ImU32 ColorForPlayer(const EspPlayer& p) {
    if (p.isMurder && g_Cheat.murderEspEnabled)
        return IM_COL32(255, 60, 60, 255);
    if (p.isDead)
        return IM_COL32(160, 160, 160, 220);
    // soft crew palette by color id
    static const ImU32 kColors[] = {
        IM_COL32(198, 17, 17, 255),   // red
        IM_COL32(19, 46, 210, 255),   // blue
        IM_COL32(17, 128, 45, 255),   // green
        IM_COL32(238, 84, 187, 255),  // pink
        IM_COL32(240, 125, 13, 255),  // orange
        IM_COL32(246, 246, 87, 255),  // yellow
        IM_COL32(63, 71, 78, 255),    // black
        IM_COL32(215, 225, 241, 255), // white
        IM_COL32(107, 47, 188, 255),  // purple
        IM_COL32(113, 73, 30, 255),   // brown
        IM_COL32(56, 255, 221, 255),  // cyan
        IM_COL32(80, 240, 57, 255),   // lime
    };
    int id = p.colorId;
    if (id < 0 || id >= (int)(sizeof(kColors) / sizeof(kColors[0])))
        return IM_COL32(80, 200, 255, 255);
    return kColors[id];
}

void Esp_Draw(float displayW, float displayH) {
    if (!g_Cheat.espEnabled && !g_Cheat.murderEspEnabled) return;

    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    std::vector<EspPlayer> snap;
    {
        std::lock_guard<std::mutex> lk(g_EspMutex);
        snap = g_EspSnapshot;
    }

    ImVec2 localScreen(displayW * 0.5f, displayH * 0.85f);

    for (const auto& p : snap) {
        if (p.isLocal) continue;
        const bool showMurder = g_Cheat.murderEspEnabled && p.isMurder;
        const bool showNormal = g_Cheat.espEnabled;
        if (!showMurder && !showNormal) continue;
        if (!p.onScreen) continue;

        // Unity bottom-left origin → ImGui top-left
        float sx = p.screen.x;
        float sy = displayH - p.screen.y;
        if (sx < -50 || sy < -50 || sx > displayW + 50 || sy > displayH + 50) continue;

        ImU32 col = ColorForPlayer(p);
        if (showMurder && p.isMurder) {
            col = IM_COL32(255, 40, 40, 255);
        }

        // Approximate player box from distance (Among Us characters ~1 unit tall)
        float scale = std::clamp(220.0f / std::max(p.distance, 0.35f), 28.0f, 140.0f);
        float boxW = scale * 0.55f;
        float boxH = scale;
        ImVec2 mn(sx - boxW * 0.5f, sy - boxH);
        ImVec2 mx(sx + boxW * 0.5f, sy);

        if (g_Cheat.espBox) {
            float t = showMurder && p.isMurder ? g_Cheat.boxThickness + 1.0f : g_Cheat.boxThickness;
            dl->AddRect(mn, mx, col, 0.0f, 0, t);
            if (showMurder && p.isMurder) {
                dl->AddRect(ImVec2(mn.x - 2, mn.y - 2), ImVec2(mx.x + 2, mx.y + 2),
                            IM_COL32(255, 0, 0, 120), 0.0f, 0, 1.5f);
            }
        }

        if (g_Cheat.espLine) {
            dl->AddLine(localScreen, ImVec2(sx, sy), col, 1.5f);
        }

        if (g_Cheat.espName || g_Cheat.espRole || g_Cheat.espDistance) {
            char buf[192];
            buf[0] = 0;
            auto append = [&](const char* piece) {
                size_t used = strlen(buf);
                if (used + 1 >= sizeof(buf)) return;
                strncat(buf, piece, sizeof(buf) - used - 1);
            };
            if (g_Cheat.espName) {
                snprintf(buf, sizeof(buf), "%s", p.name.empty() ? "Player" : p.name.c_str());
            }
            if (g_Cheat.espRole) {
                char tmp[96];
                snprintf(tmp, sizeof(tmp), "%s[%s]", buf[0] ? " " : "", Offsets::RoleName(p.role));
                append(tmp);
            }
            if (g_Cheat.espDistance) {
                char tmp[32];
                snprintf(tmp, sizeof(tmp), " %.1fm", p.distance);
                append(tmp);
            }
            if (showMurder && p.isMurder) {
                append(" *MURDER*");
            }
            ImVec2 ts = ImGui::CalcTextSize(buf);
            ImVec2 tp(sx - ts.x * 0.5f, mn.y - ts.y - 2.0f);
            dl->AddRectFilled(ImVec2(tp.x - 3, tp.y - 1), ImVec2(tp.x + ts.x + 3, tp.y + ts.y + 1),
                              IM_COL32(0, 0, 0, 160), 3.0f);
            dl->AddText(tp, col, buf);
        }
    }
}

void Menu_Draw(float displayW, float displayH) {
    (void)displayW;
    // DisplaySize / DeltaTime must already be set before NewFrame() in main.cpp

    // Floating open / close button (always visible)
    ImGui::SetNextWindowPos(ImVec2(24.0f, displayH * 0.35f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(86.0f, 86.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 18.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.10f, 0.14f, 0.92f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.18f, 0.22f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.95f, 0.28f, 0.28f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.70f, 0.10f, 0.14f, 1.0f));

    ImGui::Begin("##fab", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoSavedSettings);
    if (ImGui::Button(g_Cheat.menuOpen ? "CLOSE" : "MENU", ImVec2(70, 70))) {
        g_Cheat.menuOpen = !g_Cheat.menuOpen;
    }
    ImGui::End();
    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar(2);

    if (!g_Cheat.menuOpen) return;

    ImGui::SetNextWindowSize(ImVec2(360.0f, 420.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(displayW * 0.5f - 180.0f, displayH * 0.2f), ImGuiCond_FirstUseEver);
    ImGui::Begin("Among Us Internal 2026.6.5", &g_Cheat.menuOpen,
                 ImGuiWindowFlags_NoCollapse);

    ImGui::TextUnformatted("Kitty inject | arm64 | dump-synced");
    ImGui::Separator();

    ImGui::TextUnformatted("ESP");
    ImGui::Checkbox("Player ESP", &g_Cheat.espEnabled);
    ImGui::Checkbox("Murder ESP (Impostors)", &g_Cheat.murderEspEnabled);
    ImGui::Checkbox("Boxes", &g_Cheat.espBox);
    ImGui::Checkbox("Snaplines", &g_Cheat.espLine);
    ImGui::Checkbox("Names", &g_Cheat.espName);
    ImGui::Checkbox("Roles", &g_Cheat.espRole);
    ImGui::Checkbox("Distance", &g_Cheat.espDistance);
    ImGui::Checkbox("Hide dead", &g_Cheat.hideDead);
    ImGui::SliderFloat("Box thickness", &g_Cheat.boxThickness, 1.0f, 5.0f);

    ImGui::Separator();
    {
        std::lock_guard<std::mutex> lk(g_EspMutex);
        int murder = 0;
        for (auto& p : g_EspSnapshot) if (p.isMurder && !p.isLocal) ++murder;
        ImGui::Text("Players: %d | Murder: %d", (int)g_EspSnapshot.size(), murder);
    }
    ImGui::Text("libil2cpp: %p", (void*)UBase);
    ImGui::End();
}
