#pragma once
// Pixel Gun 3D 26.10.2 (Android arm64) — offset layout mirrored from
// https://github.com/stanuwu/PixelGunCheatInternal (Offsets.h)
// Base = libil2cpp.so (not GameAssembly.dll)
//
// RVAs below are PLACEHOLDERS until runtime dump fills them.
// Inject libpg_internal.so once in a match; it writes
// /sdcard/Download/pg3d_26.10.2_offsets.h with resolved values.

#include <cstdint>

namespace Offsets {
    // --- Methods (RVA in libil2cpp.so) — same roles as stanuwu cheat ---
    inline uintptr_t PlayerMoveCUpdate        = 0; // was 0x1C4A710 on old PC build
    inline uintptr_t PlayerMoveCFixedUpdate   = 0;
    inline uintptr_t OnPreRender              = 0;
    inline uintptr_t OnSceneUnload            = 0;
    inline uintptr_t WorldToScreenPoint       = 0; // Camera.WorldToScreenPoint
    inline uintptr_t CameraGetMain            = 0;
    inline uintptr_t ComponentGetTransform    = 0;
    inline uintptr_t TransformGetPosition     = 0;
    inline uintptr_t TransformGetRotation     = 0;
    inline uintptr_t TextMeshGetText          = 0;
    inline uintptr_t TextMeshGetColor         = 0;
    inline uintptr_t BehaviourGetEnabled      = 0;
    inline uintptr_t ObjectGetInstanceID      = 0;

    // --- Fields on PlayerMoveC / WeaponSounds (same names as stanuwu) ---
    inline uintptr_t myPlayerTransform        = 0x3C0; // Transform*
    inline uintptr_t nickLabel                = 0x3D8; // TextMesh*
    inline uintptr_t headCollider             = 0x130;
    inline uintptr_t playerMoveCPlayerDamageable = 0x690;
    inline uintptr_t weaponSoundsPlayerMoveC  = 0x550;
}
