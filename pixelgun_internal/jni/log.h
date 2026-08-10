#pragma once
#include <android/log.h>
#include <cstdint>
#include <string>
#include <cmath>

#define LOG_TAG "PGInternal"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)

struct Vector3 { float x, y, z; };
struct Color { float r, g, b, a; };

struct Il2CppString {
    void* klass;
    void* monitor;
    int32_t length;
    char16_t chars[1];
};

inline uintptr_t g_Il2Cpp = 0;

inline float Dist3(const Vector3& a, const Vector3& b) {
    float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

inline std::string Il2CppToUtf8(Il2CppString* s) {
    if (!s || s->length <= 0 || s->length > 128) return {};
    std::string out;
    out.reserve((size_t)s->length);
    for (int i = 0; i < s->length; ++i) {
        char16_t c = s->chars[i];
        if (c < 0x80) out.push_back((char)c);
        else if (c < 0x800) {
            out.push_back((char)(0xC0 | (c >> 6)));
            out.push_back((char)(0x80 | (c & 0x3F)));
        } else {
            out.push_back((char)(0xE0 | (c >> 12)));
            out.push_back((char)(0x80 | ((c >> 6) & 0x3F)));
            out.push_back((char)(0x80 | (c & 0x3F)));
        }
    }
    return out;
}
