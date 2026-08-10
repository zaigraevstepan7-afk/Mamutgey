#include "Functions.h"

static uintptr_t GA = 0;
// Prefer *Injected variants when present (ARM64 ABI-safe)
static uintptr_t RVA_W2S = 0;
static uintptr_t RVA_GetPos = 0;
static bool W2S_Injected = false;
static bool Pos_Injected = false;

void Functions::init(uintptr_t il2cpp_base) {
    GA = il2cpp_base;
    g_Il2Cpp = il2cpp_base;
    RVA_W2S = Offsets::WorldToScreenPoint;
    RVA_GetPos = Offsets::TransformGetPosition;
    // Heuristic: Injected RVAs are set by Resolve with high bit flag unused —
    // Resolve sets Offsets directly; we detect by trying known naming at resolve time.
}

void Functions::SetInjectedFlags(bool w2s_inj, bool pos_inj) {
    W2S_Injected = w2s_inj;
    Pos_Injected = pos_inj;
}

void* Functions::CameraGetMain() {
    if (!GA || !Offsets::CameraGetMain) return nullptr;
    using Fn = void* (*)(const void*);
    return reinterpret_cast<Fn>(GA + Offsets::CameraGetMain)(nullptr);
}

void Functions::CameraWorldToScreen(void* cam, Vector3* world, Vector3* screen) {
    if (!cam || !world || !screen || !GA || !Offsets::WorldToScreenPoint) return;
    if (W2S_Injected) {
        using Fn = void (*)(void*, Vector3*, int, Vector3*, const void*);
        reinterpret_cast<Fn>(GA + Offsets::WorldToScreenPoint)(cam, world, 2, screen, nullptr);
    } else {
        // stanuwu PC style
        using Fn = void (*)(void*, void*, int, void*);
        reinterpret_cast<Fn>(GA + Offsets::WorldToScreenPoint)(cam, world, 2, screen);
    }
}

void* Functions::ComponentGetTransform(void* component) {
    if (!component || !GA || !Offsets::ComponentGetTransform) return nullptr;
    using Fn = void* (*)(void*, const void*);
    return reinterpret_cast<Fn>(GA + Offsets::ComponentGetTransform)(component, nullptr);
}

void Functions::TransformGetPosition(void* transform, Vector3* out) {
    if (!transform || !out || !GA || !Offsets::TransformGetPosition) return;
    if (Pos_Injected) {
        using Fn = void (*)(void*, Vector3*, const void*);
        reinterpret_cast<Fn>(GA + Offsets::TransformGetPosition)(transform, out, nullptr);
    } else {
        using Fn = void (*)(void*, void*);
        reinterpret_cast<Fn>(GA + Offsets::TransformGetPosition)(transform, out);
    }
}

void* Functions::TextMeshGetText(void* textMesh) {
    if (!textMesh || !GA || !Offsets::TextMeshGetText) return nullptr;
    using Fn = void* (*)(void*, const void*);
    return reinterpret_cast<Fn>(GA + Offsets::TextMeshGetText)(textMesh, nullptr);
}

void Functions::TextMeshGetColor(void* textMesh, Color* out) {
    if (!textMesh || !out || !GA || !Offsets::TextMeshGetColor) return;
    using Fn = void (*)(void*, Color*, const void*);
    reinterpret_cast<Fn>(GA + Offsets::TextMeshGetColor)(textMesh, out, nullptr);
}
