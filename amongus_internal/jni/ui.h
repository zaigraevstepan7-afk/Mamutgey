#pragma once
#include "game.h"
#include <atomic>

void Esp_Draw(float displayW, float displayH);
void Menu_Draw(float displayW, float displayH);
void Menu_PushTouch(float x, float y, bool down);
void Menu_ApplyTouches(); // call from GL/render thread before NewFrame
extern std::atomic<bool> g_ImguiWantMouse;
