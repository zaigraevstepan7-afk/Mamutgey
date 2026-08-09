#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <cmath>
#include <android/log.h>

#define LOG_TAG "AUInternal"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

struct Vector2 { float x, y; };
struct Vector3 { float x, y, z; };

struct Il2CppObject {
    void* klass;
    void* monitor;
};

struct Il2CppString {
    void* klass;
    void* monitor;
    int32_t length;
    char16_t chars[1];
};

struct Il2CppArray {
    void* klass;
    void* monitor;
    void* bounds;
    uintptr_t max_length;
    void* vector[1];
};

struct Il2CppList {
    void* klass;
    void* monitor;
    Il2CppArray* items;
    int32_t size;
    int32_t version;
};

struct Il2CppClass_Minimal {
    // Enough to reach static_fields on Unity 2022 / IL2CPP
    // Layout from dump: Il2CppClass { Il2CppClass_1 _1; void* static_fields; ... }
    char _pad[0xB8]; // tuned below via runtime probe if needed
    void* static_fields;
};

// Unity 2022.3 Il2CppClass: static_fields is typically at offset 0xB8 on arm64
constexpr size_t kStaticFieldsOffset = 0xB8;

inline uintptr_t UBase = 0; // libil2cpp.so base

template <typename T>
inline T& Field(void* obj, uintptr_t offset) {
    return *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(obj) + offset);
}

template <typename T>
inline T Read(void* obj, uintptr_t offset) {
    if (!obj) return T{};
    return *reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(obj) + offset);
}

template <typename T>
inline T AsPtr(uintptr_t rva) {
    return reinterpret_cast<T>(UBase + rva);
}

inline void* GetTypeInfo(uintptr_t typeInfoRva) {
    if (!UBase) return nullptr;
    void** slot = reinterpret_cast<void**>(UBase + typeInfoRva);
    if (!slot) return nullptr;
    return *slot;
}

inline void* GetStaticFields(void* typeInfo) {
    if (!typeInfo) return nullptr;
    return *reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(typeInfo) + kStaticFieldsOffset);
}

inline bool IsUnityAlive(void* unityObj) {
    if (!unityObj) return false;
    // UnityEngine.Object.m_CachedPtr at +0x10
    auto cached = *reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(unityObj) + 0x10);
    return cached != 0;
}

inline std::string Il2CppStringToUtf8(Il2CppString* s) {
    if (!s || s->length <= 0 || s->length > 256) return {};
    std::string out;
    out.reserve(static_cast<size_t>(s->length));
    for (int i = 0; i < s->length; ++i) {
        char16_t c = s->chars[i];
        if (c < 0x80) out.push_back(static_cast<char>(c));
        else if (c < 0x800) {
            out.push_back(static_cast<char>(0xC0 | (c >> 6)));
            out.push_back(static_cast<char>(0x80 | (c & 0x3F)));
        } else {
            out.push_back(static_cast<char>(0xE0 | (c >> 12)));
            out.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (c & 0x3F)));
        }
    }
    return out;
}

inline float Dist2D(const Vector3& a, const Vector3& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}
