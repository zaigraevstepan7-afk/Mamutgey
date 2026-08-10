#pragma once
// Thin wrappers — same API surface as stanuwu Functions.cpp (ESP subset)

#include "log.h"
#include "Offsets.h"

namespace Functions {
    void init(uintptr_t il2cpp_base);
    void SetInjectedFlags(bool w2s_inj, bool pos_inj);

    void* CameraGetMain();
    void CameraWorldToScreen(void* cam, Vector3* world, Vector3* screen);
    void* ComponentGetTransform(void* component);
    void TransformGetPosition(void* transform, Vector3* out);
    void* TextMeshGetText(void* textMesh);
    void TextMeshGetColor(void* textMesh, Color* out);
}
