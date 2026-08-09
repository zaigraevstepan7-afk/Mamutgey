#pragma once
// Among Us Android 2026.6.5 (versionCode 7045) arm64-v8a
// Package: com.innersloth.spacemafia
// Source: APKPure XAPK (libil2cpp.so)
// Unity 2022.3.62f3 / IL2CPP metadata v31

#include <cstdint>

namespace Offsets {

// TypeInfo globals (RVA in libil2cpp.so) — dereference once to get Il2CppClass*
constexpr uintptr_t PlayerControl_TypeInfo = 0x4AA4918;
constexpr uintptr_t GameData_TypeInfo      = 0x4A9FD58;
constexpr uintptr_t AmongUsClient_TypeInfo = 0x4A9CFE0;
constexpr uintptr_t ShipStatus_TypeInfo    = 0x4AA6348;
constexpr uintptr_t RoleManager_TypeInfo   = 0x4AA57E0;

// PlayerControl static field offsets inside static_fields blob
constexpr uintptr_t PC_LocalPlayer        = 0x0;
constexpr uintptr_t PC_AllPlayerControls  = 0x8;

// PlayerControl instance (dump.cs)
constexpr uintptr_t PC_PlayerId           = 0x35;
constexpr uintptr_t PC_CachedPlayerData   = 0x70;
constexpr uintptr_t PC_inVent             = 0x60;
constexpr uintptr_t PC_isDummy            = 0x110;
constexpr uintptr_t PC_notRealPlayer      = 0x111;

// NetworkedPlayerInfo instance
constexpr uintptr_t NPI_PlayerId          = 0x35;
constexpr uintptr_t NPI_RoleType          = 0x50; // RoleTypes ushort
constexpr uintptr_t NPI_Disconnected      = 0x64;
constexpr uintptr_t NPI_Role              = 0x68; // RoleBehaviour*
constexpr uintptr_t NPI_IsDead            = 0x78;
constexpr uintptr_t NPI_Object            = 0x80; // PlayerControl*

// RoleBehaviour
constexpr uintptr_t RB_Role               = 0x20; // RoleTypes
constexpr uintptr_t RB_TeamType           = 0x6C; // RoleTeamTypes (0 crew, 1 impostor)
constexpr uintptr_t RB_CanUseKillButton   = 0x61;

// PlayerOutfit
constexpr uintptr_t Outfit_ColorId        = 0x10;
constexpr uintptr_t Outfit_PlayerName     = 0x40;

// AmongUsClient / InnerNetClient
constexpr uintptr_t AUC_GameState         = 0xAC; // InnerNetClient.GameStates

// Method RVAs (callables)
constexpr uintptr_t PlayerControl_get_Data              = 0x21B47F8;
constexpr uintptr_t NetworkedPlayerInfo_get_PlayerName  = 0x2415BDC;
constexpr uintptr_t NetworkedPlayerInfo_get_Object      = 0x2413F74;
constexpr uintptr_t NetworkedPlayerInfo_get_DefaultOutfit = 0x241652C;
constexpr uintptr_t RoleBehaviour_get_IsImpostor        = 0x21F6F00;
constexpr uintptr_t RoleManager_IsImpostorRole          = 0x21FEB14;
constexpr uintptr_t Camera_get_main                     = 0x4411BAC;
constexpr uintptr_t Camera_WorldToScreenPoint           = 0x44112F8;
constexpr uintptr_t Component_get_transform             = 0x44490BC;
constexpr uintptr_t Transform_get_position              = 0x44565A0;
constexpr uintptr_t Transform_get_position_Injected     = 0x44565A0 + 0; // prefer Injected if available
constexpr uintptr_t Object_get_Name                     = 0; // unused

enum class RoleTypes : uint16_t {
    Crewmate = 0,
    Impostor = 1,
    Scientist = 2,
    Engineer = 3,
    GuardianAngel = 4,
    Shapeshifter = 5,
    CrewmateGhost = 6,
    ImpostorGhost = 7,
    Noisemaker = 8,
    Phantom = 9,
    Tracker = 10,
    Detective = 12,
    Viper = 18,
};

enum class GameStates : int32_t {
    NotJoined = 0,
    Joined = 1,
    Started = 2,
    Ended = 3,
};

inline bool IsMurderRole(RoleTypes r) {
    switch (r) {
        case RoleTypes::Impostor:
        case RoleTypes::Shapeshifter:
        case RoleTypes::ImpostorGhost:
        case RoleTypes::Phantom:
        case RoleTypes::Viper:
            return true;
        default:
            return false;
    }
}

inline const char* RoleName(RoleTypes r) {
    switch (r) {
        case RoleTypes::Crewmate: return "Crewmate";
        case RoleTypes::Impostor: return "Impostor";
        case RoleTypes::Scientist: return "Scientist";
        case RoleTypes::Engineer: return "Engineer";
        case RoleTypes::GuardianAngel: return "GuardianAngel";
        case RoleTypes::Shapeshifter: return "Shapeshifter";
        case RoleTypes::CrewmateGhost: return "CrewGhost";
        case RoleTypes::ImpostorGhost: return "ImpGhost";
        case RoleTypes::Noisemaker: return "Noisemaker";
        case RoleTypes::Phantom: return "Phantom";
        case RoleTypes::Tracker: return "Tracker";
        case RoleTypes::Detective: return "Detective";
        case RoleTypes::Viper: return "Viper";
        default: return "Unknown";
    }
}

} // namespace Offsets
