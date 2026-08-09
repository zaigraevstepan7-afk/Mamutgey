#include "game.h"
#include "overlay.h"

#include <jni.h>
#include <pthread.h>
#include <unistd.h>
#include <atomic>

static JavaVM* g_VM = nullptr;
static std::atomic<bool> g_HackStarted{false};

static void* HackThread(void*) {
    LOGI("hack thread — waiting for libil2cpp + Unity activity");

    for (int i = 0; i < 400; ++i) {
        if (Il2CppReady()) break;
        usleep(50 * 1000);
    }
    if (!UBase) LOGE("libil2cpp.so not found yet (will keep retrying in tick)");

    // Primary UI path: Android View overlay (works on Vulkan + GLES)
    if (g_VM) {
        for (int i = 0; i < 100; ++i) {
            if (Overlay_Start(g_VM)) {
                LOGI("overlay OK — look for red MENU button on the left");
                return nullptr;
            }
            usleep(100 * 1000);
        }
        LOGE("overlay failed to start — check logcat AUInternal");
    } else {
        LOGE("JavaVM is null (JNI_OnLoad not called?)");
    }
    return nullptr;
}

static void StartHackOnce() {
    bool expected = false;
    if (!g_HackStarted.compare_exchange_strong(expected, true)) return;
    pthread_t t;
    pthread_create(&t, nullptr, HackThread, nullptr);
    pthread_detach(t);
}

extern "C" jint JNI_OnLoad(JavaVM* vm, void*) {
    g_VM = vm;
    LOGI("JNI_OnLoad — Among Us Internal 2026.6.5 (overlay build)");
    StartHackOnce();
    return JNI_VERSION_1_6;
}

__attribute__((constructor))
static void on_load() {
    LOGI("libau_internal constructor");
    // JNI_OnLoad preferred; constructor alone cannot get JavaVM reliably with Kitty
}
