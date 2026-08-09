#include "game.h"
#include "overlay.h"

#include <jni.h>
#include <pthread.h>
#include <unistd.h>
#include <atomic>

static JavaVM* g_VM = nullptr;
static std::atomic<bool> g_HackStarted{false};

static void* HackThread(void*) {
    // Wait for game process to settle after Kitty inject
    usleep(1500 * 1000);

    for (int i = 0; i < 400; ++i) {
        if (Il2CppReady()) break;
        usleep(50 * 1000);
    }

    if (g_VM) {
        for (int i = 0; i < 120; ++i) {
            if (Overlay_Start(g_VM)) return nullptr;
            usleep(150 * 1000);
        }
        LOGE("overlay failed to start");
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
