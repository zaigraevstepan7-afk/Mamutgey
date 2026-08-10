#include "Resolve.h"
#include "Functions.h"
#include "Hooks.h"
#include "overlay.h"
#include "log.h"

#include <jni.h>
#include <pthread.h>
#include <unistd.h>
#include <atomic>

static JavaVM* g_VM = nullptr;
static std::atomic<bool> g_Started{false};

static void* Worker(void*) {
    usleep(3000 * 1000); // let game / il2cpp init

    for (int i = 0; i < 60; ++i) {
        if (Resolve_All()) break;
        LOGW("resolve retry %d", i);
        usleep(1000 * 1000);
    }

    if (!Offsets::PlayerMoveCUpdate) {
        LOGE("ESP abort: PlayerMoveC.Update not resolved — check obfuscation / dump");
    } else {
        Functions::init(g_Il2Cpp);
        Hooks_Install();
    }

    if (g_VM) {
        for (int i = 0; i < 40; ++i) {
            if (Overlay_Start(g_VM)) break;
            usleep(250 * 1000);
        }
    }
    return nullptr;
}

static void StartOnce() {
    bool expected = false;
    if (!g_Started.compare_exchange_strong(expected, true)) return;
    pthread_t t;
    pthread_create(&t, nullptr, Worker, nullptr);
    pthread_detach(t);
}

extern "C" jint JNI_OnLoad(JavaVM* vm, void*) {
    g_VM = vm;
    LOGI("JNI_OnLoad — PG3D 26.10.2 ESP (stanuwu PixelGunCheatInternal port)");
    StartOnce();
    return JNI_VERSION_1_6;
}

__attribute__((constructor))
static void on_load() {
    LOGI("libpg_internal constructor");
}
