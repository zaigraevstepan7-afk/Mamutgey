#include "overlay.h"
#include "game.h"
#include "overlay_dex.h"

#include <android/log.h>
#include <dlfcn.h>
#include <jni.h>
#include <pthread.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <vector>
#include <mutex>
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <setjmp.h>
#include <csignal>
#include <atomic>

#define OLOGI(...) __android_log_print(ANDROID_LOG_INFO, "AUInternal", __VA_ARGS__)
#define OLOGE(...) __android_log_print(ANDROID_LOG_ERROR, "AUInternal", __VA_ARGS__)

static JavaVM* g_VM = nullptr;
static std::atomic<bool> g_Alive{false};
static std::atomic<bool> g_Stop{false};
static int g_ViewW = 0, g_ViewH = 0;
static std::mutex g_LabelMu;
static std::vector<std::string> g_Labels;

// ---- crash guard for IL2CPP reads on worker thread ----
static thread_local sigjmp_buf g_Jmp;
static thread_local volatile bool g_Guarding = false;
static struct sigaction g_OldSegv{};
static struct sigaction g_OldBus{};

static void GuardHandler(int sig, siginfo_t*, void*) {
    if (g_Guarding) siglongjmp(g_Jmp, sig);
    // fall through to previous if possible
}

static void InstallGuard() {
    struct sigaction sa{};
    sa.sa_sigaction = GuardHandler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGSEGV, &sa, &g_OldSegv);
    sigaction(SIGBUS, &sa, &g_OldBus);
}

static bool SafeTick() {
    g_Guarding = true;
    if (sigsetjmp(g_Jmp, 1) == 0) {
        Game_TickCollect();
        g_Guarding = false;
        return true;
    }
    g_Guarding = false;
    OLOGE("Game_TickCollect fault swallowed");
    return false;
}

static JNIEnv* GetEnv() {
    if (!g_VM) return nullptr;
    JNIEnv* env = nullptr;
    jint st = g_VM->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
    if (st == JNI_OK) return env;
    if (st == JNI_EDETACHED && g_VM->AttachCurrentThread(&env, nullptr) == 0) return env;
    return nullptr;
}

static jobject GetActivity(JNIEnv* env) {
    jclass up = env->FindClass("com/unity3d/player/UnityPlayer");
    if (!up) { env->ExceptionClear(); return nullptr; }
    jfieldID fid = env->GetStaticFieldID(up, "currentActivity", "Landroid/app/Activity;");
    if (!fid) { env->ExceptionClear(); return nullptr; }
    return env->GetStaticObjectField(up, fid);
}

// ---- JNI natives for AuOverlay ----
static jint J_nativeEspCount(JNIEnv*, jclass) {
    std::lock_guard<std::mutex> lk(g_EspMutex);
    int n = 0;
    for (auto& p : g_EspSnapshot) {
        if (p.isLocal) continue;
        if (!p.onScreen) continue;
        const bool showMurder = g_Cheat.murderEspEnabled && p.isMurder;
        const bool showNormal = g_Cheat.espEnabled;
        if (!showMurder && !showNormal) continue;
        ++n;
        if (n >= 16) break;
    }
    return n;
}

static void J_nativeEspFill(JNIEnv* env, jclass, jfloatArray arr) {
    if (!arr) return;
    const int maxN = 16;
    float tmp[16 * 8]{};
    std::vector<std::string> labels;
    labels.reserve(16);

    int w = g_ViewW;
    int h = g_ViewH;
    {
        std::lock_guard<std::mutex> lk(g_EspMutex);
        int idx = 0;
        for (auto& p : g_EspSnapshot) {
            if (idx >= maxN) break;
            if (p.isLocal || !p.onScreen) continue;
            const bool showMurder = g_Cheat.murderEspEnabled && p.isMurder;
            const bool showNormal = g_Cheat.espEnabled;
            if (!showMurder && !showNormal) continue;

            float sx = p.screen.x;
            float sy = (h > 0) ? (h - p.screen.y) : p.screen.y;
            float scale = std::clamp(220.0f / std::max(p.distance, 0.35f), 28.0f, 140.0f);
            float top = sy - scale;
            float bot = sy;

            float r = 0.3f, g = 0.75f, b = 1.f, a = 1.f;
            float flags = 1.f;
            if (showMurder && p.isMurder) {
                r = 1.f; g = 0.15f; b = 0.15f; flags = 2.f;
            } else if (p.isDead) {
                r = g = b = 0.6f;
            }

            int o = idx * 8;
            tmp[o + 0] = sx;
            tmp[o + 1] = top;
            tmp[o + 2] = bot;
            tmp[o + 3] = r;
            tmp[o + 4] = g;
            tmp[o + 5] = b;
            tmp[o + 6] = a;
            tmp[o + 7] = (g_Cheat.espBox || g_Cheat.espLine || g_Cheat.espName) ? flags : 0.f;

            char buf[192];
            buf[0] = 0;
            if (g_Cheat.espName) {
                snprintf(buf, sizeof(buf), "%s", p.name.empty() ? "Player" : p.name.c_str());
            }
            if (g_Cheat.espRole) {
                char t[64];
                snprintf(t, sizeof(t), "%s[%s]", buf[0] ? " " : "", Offsets::RoleName(p.role));
                size_t used = strlen(buf);
                if (used + 1 < sizeof(buf)) strncat(buf, t, sizeof(buf) - used - 1);
            }
            if (showMurder && p.isMurder) {
                size_t used = strlen(buf);
                if (used + 1 < sizeof(buf)) strncat(buf, " *MURDER*", sizeof(buf) - used - 1);
            }
            labels.emplace_back(buf);
            ++idx;
        }
    }

    {
        std::lock_guard<std::mutex> lk(g_LabelMu);
        g_Labels.swap(labels);
    }

    jsize len = env->GetArrayLength(arr);
    jsize n = std::min(len, (jsize)(maxN * 8));
    env->SetFloatArrayRegion(arr, 0, n, tmp);
    (void)w;
}

static jstring J_nativeEspLabel(JNIEnv* env, jclass, jint index) {
    std::lock_guard<std::mutex> lk(g_LabelMu);
    if (index < 0 || index >= (jint)g_Labels.size()) return env->NewStringUTF("");
    return env->NewStringUTF(g_Labels[index].c_str());
}

static void J_nativeSetEsp(JNIEnv*, jclass, jboolean v) { g_Cheat.espEnabled = v; }
static void J_nativeSetMurderEsp(JNIEnv*, jclass, jboolean v) { g_Cheat.murderEspEnabled = v; }
static void J_nativeSetBox(JNIEnv*, jclass, jboolean v) { g_Cheat.espBox = v; }
static void J_nativeSetLine(JNIEnv*, jclass, jboolean v) { g_Cheat.espLine = v; }
static void J_nativeSetName(JNIEnv*, jclass, jboolean v) {
    g_Cheat.espName = v;
    g_Cheat.espRole = v;
}
static jboolean J_nativeGetEsp(JNIEnv*, jclass) { return g_Cheat.espEnabled; }
static jboolean J_nativeGetMurderEsp(JNIEnv*, jclass) { return g_Cheat.murderEspEnabled; }

static void J_nativeSetViewSize(JNIEnv*, jclass, jint w, jint h) {
    g_ViewW = w;
    g_ViewH = h;
}

static JNINativeMethod g_Methods[] = {
    {const_cast<char*>("nativeEspCount"), const_cast<char*>("()I"), (void*)J_nativeEspCount},
    {const_cast<char*>("nativeEspFill"), const_cast<char*>("([F)V"), (void*)J_nativeEspFill},
    {const_cast<char*>("nativeEspLabel"), const_cast<char*>("(I)Ljava/lang/String;"), (void*)J_nativeEspLabel},
    {const_cast<char*>("nativeSetEsp"), const_cast<char*>("(Z)V"), (void*)J_nativeSetEsp},
    {const_cast<char*>("nativeSetMurderEsp"), const_cast<char*>("(Z)V"), (void*)J_nativeSetMurderEsp},
    {const_cast<char*>("nativeSetBox"), const_cast<char*>("(Z)V"), (void*)J_nativeSetBox},
    {const_cast<char*>("nativeSetLine"), const_cast<char*>("(Z)V"), (void*)J_nativeSetLine},
    {const_cast<char*>("nativeSetName"), const_cast<char*>("(Z)V"), (void*)J_nativeSetName},
    {const_cast<char*>("nativeGetEsp"), const_cast<char*>("()Z"), (void*)J_nativeGetEsp},
    {const_cast<char*>("nativeGetMurderEsp"), const_cast<char*>("()Z"), (void*)J_nativeGetMurderEsp},
    {const_cast<char*>("nativeSetViewSize"), const_cast<char*>("(II)V"), (void*)J_nativeSetViewSize},
};

static jclass LoadOverlayClass(JNIEnv* env) {
    // ByteBuffer.wrap(dex)
    jclass bbCls = env->FindClass("java/nio/ByteBuffer");
    jmethodID wrap = env->GetStaticMethodID(bbCls, "wrap", "([B)Ljava/nio/ByteBuffer;");
    jbyteArray arr = env->NewByteArray((jsize)au_overlay_dex_len);
    env->SetByteArrayRegion(arr, 0, (jsize)au_overlay_dex_len,
                            reinterpret_cast<const jbyte*>(au_overlay_dex));
    jobject buf = env->CallStaticObjectMethod(bbCls, wrap, arr);

    // parent = activity classloader
    jobject activity = GetActivity(env);
    if (!activity) return nullptr;
    jclass ctxCls = env->FindClass("android/content/Context");
    jmethodID getCl = env->GetMethodID(ctxCls, "getClassLoader", "()Ljava/lang/ClassLoader;");
    jobject parent = env->CallObjectMethod(activity, getCl);

    jclass imdex = env->FindClass("dalvik/system/InMemoryDexClassLoader");
    if (!imdex) {
        env->ExceptionClear();
        OLOGE("InMemoryDexClassLoader missing");
        return nullptr;
    }
    jmethodID ctor = env->GetMethodID(imdex, "<init>", "(Ljava/nio/ByteBuffer;Ljava/lang/ClassLoader;)V");
    jobject loader = env->NewObject(imdex, ctor, buf, parent);
    if (!loader || env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return nullptr;
    }

    jclass clCls = env->FindClass("java/lang/ClassLoader");
    jmethodID load = env->GetMethodID(clCls, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
    jstring name = env->NewStringUTF("au.overlay.AuOverlay");
    jobject clsObj = env->CallObjectMethod(loader, load, name);
    if (!clsObj || env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        OLOGE("loadClass AuOverlay failed");
        return nullptr;
    }
    return (jclass)env->NewGlobalRef(clsObj);
}

static void* TickThread(void*) {
    InstallGuard();
    while (!g_Stop.load()) {
        SafeTick();
        // update view size from snapshot path using DisplayMetrics if needed
        usleep(16 * 1000);
    }
    return nullptr;
}

bool Overlay_IsAlive() { return g_Alive.load(); }

void Overlay_Shutdown() {
    g_Stop.store(true);
    g_Alive.store(false);
}

bool Overlay_Start(JavaVM* vm) {
    g_VM = vm;
    JNIEnv* env = GetEnv();
    if (!env) {
        OLOGE("Overlay_Start: no JNIEnv");
        return false;
    }

    // wait for activity
    jobject activity = nullptr;
    for (int i = 0; i < 200; ++i) {
        activity = GetActivity(env);
        if (activity) break;
        usleep(50 * 1000);
    }
    if (!activity) {
        OLOGE("Unity currentActivity is null");
        return false;
    }
    OLOGI("Unity activity OK, loading overlay dex (%u bytes)", au_overlay_dex_len);

    jclass ovl = LoadOverlayClass(env);
    if (!ovl) return false;

    if (env->RegisterNatives(ovl, g_Methods, (jint)(sizeof(g_Methods) / sizeof(g_Methods[0]))) != 0) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        OLOGE("RegisterNatives failed");
        return false;
    }

    jmethodID start = env->GetStaticMethodID(ovl, "start", "(Landroid/app/Activity;)V");
    if (!start) {
        env->ExceptionClear();
        OLOGE("AuOverlay.start missing");
        return false;
    }
    env->CallStaticVoidMethod(ovl, start, activity);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        OLOGE("AuOverlay.start threw");
        return false;
    }

    g_Alive.store(true);
    g_Stop.store(false);
    pthread_t t;
    pthread_create(&t, nullptr, TickThread, nullptr);
    pthread_detach(t);
    OLOGI("Android overlay started (MENU button on left)");
    return true;
}
