#include "overlay.h"
#include "game.h"
#include "overlay_dex.h"

#include <android/log.h>
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

static thread_local sigjmp_buf g_Jmp;
static thread_local volatile bool g_Guarding = false;

static void GuardHandler(int, siginfo_t*, void*) {
    if (g_Guarding) siglongjmp(g_Jmp, 1);
}

static void InstallGuard() {
    struct sigaction sa{};
    sa.sa_sigaction = GuardHandler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGSEGV, &sa, nullptr);
    sigaction(SIGBUS, &sa, nullptr);
}

static bool SafeTick() {
    g_Guarding = true;
    if (sigsetjmp(g_Jmp, 1) == 0) {
        Game_TickCollect();
        g_Guarding = false;
        return true;
    }
    g_Guarding = false;
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

/** App ClassLoader — CRITICAL for FindClass after Kitty inject. */
static jobject GetAppClassLoader(JNIEnv* env) {
    jclass at = env->FindClass("android/app/ActivityThread");
    if (!at) { env->ExceptionClear(); OLOGE("ActivityThread missing"); return nullptr; }
    jmethodID curApp = env->GetStaticMethodID(at, "currentApplication", "()Landroid/app/Application;");
    if (!curApp) { env->ExceptionClear(); return nullptr; }
    jobject app = env->CallStaticObjectMethod(at, curApp);
    if (!app) { OLOGE("currentApplication() null"); return nullptr; }
    jclass ctx = env->FindClass("android/content/Context");
    jmethodID getCl = env->GetMethodID(ctx, "getClassLoader", "()Ljava/lang/ClassLoader;");
    jobject cl = env->CallObjectMethod(app, getCl);
    return cl;
}

static jclass LoadAppClass(JNIEnv* env, jobject classLoader, const char* name) {
    if (!classLoader) return nullptr;
    jclass clCls = env->FindClass("java/lang/ClassLoader");
    jmethodID load = env->GetMethodID(clCls, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
    jstring jname = env->NewStringUTF(name);
    jobject cls = env->CallObjectMethod(classLoader, load, jname);
    env->DeleteLocalRef(jname);
    if (!cls || env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        OLOGE("loadClass failed: %s", name);
        return nullptr;
    }
    return (jclass)cls;
}

static jobject GetUnityActivity(JNIEnv* env, jobject classLoader) {
    jclass up = LoadAppClass(env, classLoader, "com.unity3d.player.UnityPlayer");
    if (!up) return nullptr;
    jfieldID fid = env->GetStaticFieldID(up, "currentActivity", "Landroid/app/Activity;");
    if (!fid) {
        env->ExceptionClear();
        OLOGE("UnityPlayer.currentActivity field missing");
        return nullptr;
    }
    jobject act = env->GetStaticObjectField(up, fid);
    if (!act) OLOGE("UnityPlayer.currentActivity is null (open the game fully first)");
    return act;
}

static void ShowToast(JNIEnv* env, jobject activity, const char* msg) {
    if (!activity) return;
    jclass toastCls = env->FindClass("android/widget/Toast");
    jmethodID make = env->GetStaticMethodID(toastCls, "makeText",
        "(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;");
    jmethodID show = env->GetMethodID(toastCls, "show", "()V");
    jstring jmsg = env->NewStringUTF(msg);
    jobject toast = env->CallStaticObjectMethod(toastCls, make, activity, jmsg, 1 /* LONG */);
    if (toast) env->CallVoidMethod(toast, show);
    env->DeleteLocalRef(jmsg);
}

// ---- JNI natives ----
static jint J_nativeEspCount(JNIEnv*, jclass) {
    std::lock_guard<std::mutex> lk(g_EspMutex);
    int n = 0;
    for (auto& p : g_EspSnapshot) {
        if (p.isLocal || !p.onScreen) continue;
        if (!(g_Cheat.espEnabled || (g_Cheat.murderEspEnabled && p.isMurder))) continue;
        if (++n >= 16) break;
    }
    return n;
}

static void J_nativeEspFill(JNIEnv* env, jclass, jfloatArray arr) {
    if (!arr) return;
    float tmp[16 * 8]{};
    std::vector<std::string> labels;
    int h = g_ViewH;
    {
        std::lock_guard<std::mutex> lk(g_EspMutex);
        int idx = 0;
        for (auto& p : g_EspSnapshot) {
            if (idx >= 16) break;
            if (p.isLocal || !p.onScreen) continue;
            bool showMurder = g_Cheat.murderEspEnabled && p.isMurder;
            if (!g_Cheat.espEnabled && !showMurder) continue;

            float sx = p.screen.x;
            float sy = (h > 0) ? (h - p.screen.y) : p.screen.y;
            float scale = std::clamp(220.0f / std::max(p.distance, 0.35f), 28.0f, 140.0f);
            float r = 0.3f, g = 0.75f, b = 1.f, a = 1.f, flags = 1.f;
            if (showMurder) { r = 1.f; g = 0.15f; b = 0.15f; flags = 2.f; }

            int o = idx * 8;
            tmp[o]=sx; tmp[o+1]=sy-scale; tmp[o+2]=sy;
            tmp[o+3]=r; tmp[o+4]=g; tmp[o+5]=b; tmp[o+6]=a; tmp[o+7]=flags;

            char buf[192]{};
            if (g_Cheat.espName)
                snprintf(buf, sizeof(buf), "%s", p.name.empty() ? "Player" : p.name.c_str());
            if (g_Cheat.espRole) {
                char t[64];
                snprintf(t, sizeof(t), "%s[%s]", buf[0]?" ":"", Offsets::RoleName(p.role));
                size_t u = strlen(buf);
                if (u+1 < sizeof(buf)) strncat(buf, t, sizeof(buf)-u-1);
            }
            if (showMurder) {
                size_t u = strlen(buf);
                if (u+1 < sizeof(buf)) strncat(buf, " *MURDER*", sizeof(buf)-u-1);
            }
            labels.emplace_back(buf);
            ++idx;
        }
    }
    {
        std::lock_guard<std::mutex> lk(g_LabelMu);
        g_Labels.swap(labels);
    }
    jsize n = std::min(env->GetArrayLength(arr), (jsize)(16 * 8));
    env->SetFloatArrayRegion(arr, 0, n, tmp);
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
static void J_nativeSetName(JNIEnv*, jclass, jboolean v) { g_Cheat.espName = v; g_Cheat.espRole = v; }
static jboolean J_nativeGetEsp(JNIEnv*, jclass) { return g_Cheat.espEnabled; }
static jboolean J_nativeGetMurderEsp(JNIEnv*, jclass) { return g_Cheat.murderEspEnabled; }
static void J_nativeSetViewSize(JNIEnv*, jclass, jint w, jint h) { g_ViewW = w; g_ViewH = h; }
static void J_nativeLog(JNIEnv* env, jclass, jstring msg) {
    if (!msg) return;
    const char* c = env->GetStringUTFChars(msg, nullptr);
    if (c) {
        OLOGI("[java] %s", c);
        env->ReleaseStringUTFChars(msg, c);
    }
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
    {const_cast<char*>("nativeLog"), const_cast<char*>("(Ljava/lang/String;)V"), (void*)J_nativeLog},
};

static jclass LoadOverlayClass(JNIEnv* env, jobject appCl) {
    jclass bbCls = env->FindClass("java/nio/ByteBuffer");
    jmethodID wrap = env->GetStaticMethodID(bbCls, "wrap", "([B)Ljava/nio/ByteBuffer;");
    jbyteArray arr = env->NewByteArray((jsize)au_overlay_dex_len);
    env->SetByteArrayRegion(arr, 0, (jsize)au_overlay_dex_len,
                            reinterpret_cast<const jbyte*>(au_overlay_dex));
    jobject buf = env->CallStaticObjectMethod(bbCls, wrap, arr);

    jclass imdex = env->FindClass("dalvik/system/InMemoryDexClassLoader");
    if (!imdex) {
        env->ExceptionClear();
        OLOGE("InMemoryDexClassLoader not available");
        return nullptr;
    }
    jmethodID ctor = env->GetMethodID(imdex, "<init>", "(Ljava/nio/ByteBuffer;Ljava/lang/ClassLoader;)V");
    jobject loader = env->NewObject(imdex, ctor, buf, appCl);
    if (!loader || env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        OLOGE("InMemoryDexClassLoader ctor failed");
        return nullptr;
    }

    jclass clCls = env->FindClass("java/lang/ClassLoader");
    jmethodID load = env->GetMethodID(clCls, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
    jstring name = env->NewStringUTF("au.overlay.AuOverlay");
    jobject clsObj = env->CallObjectMethod(loader, load, name);
    if (!clsObj || env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        OLOGE("loadClass au.overlay.AuOverlay failed");
        return nullptr;
    }
    OLOGI("AuOverlay class loaded OK");
    return (jclass)env->NewGlobalRef(clsObj);
}

static void* TickThread(void*) {
    InstallGuard();
    while (!g_Stop.load()) {
        SafeTick();
        usleep(16 * 1000);
    }
    return nullptr;
}

bool Overlay_IsAlive() { return g_Alive.load(); }
void Overlay_Shutdown() { g_Stop.store(true); g_Alive.store(false); }

bool Overlay_Start(JavaVM* vm) {
    g_VM = vm;
    JNIEnv* env = GetEnv();
    if (!env) {
        OLOGE("Overlay_Start: AttachCurrentThread failed");
        return false;
    }

    OLOGI("BUILD=20260809d package=com.innersloth.spacemafia (CONFIRMED)");

    jobject appCl = GetAppClassLoader(env);
    if (!appCl) return false;
    OLOGI("app ClassLoader OK");

    jobject activity = nullptr;
    for (int i = 0; i < 150; ++i) {
        activity = GetUnityActivity(env, appCl);
        if (activity) break;
        usleep(100 * 1000);
    }
    if (!activity) {
        OLOGE("no Unity activity after wait");
        return false;
    }
    OLOGI("Unity activity acquired");

    // Early toast from native (may need UI thread — try anyway + Java will toast too)
    // Defer toast to Java start()

    jclass ovl = LoadOverlayClass(env, appCl);
    if (!ovl) return false;

    if (env->RegisterNatives(ovl, g_Methods, (jint)(sizeof(g_Methods) / sizeof(g_Methods[0]))) != 0) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        OLOGE("RegisterNatives failed");
        return false;
    }
    OLOGI("RegisterNatives OK");

    jmethodID start = env->GetStaticMethodID(ovl, "start", "(Landroid/app/Activity;)V");
    if (!start) {
        env->ExceptionClear();
        OLOGE("AuOverlay.start method missing");
        return false;
    }
    env->CallStaticVoidMethod(ovl, start, activity);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        OLOGE("AuOverlay.start exception");
        return false;
    }

    g_Alive.store(true);
    g_Stop.store(false);
    pthread_t t;
    pthread_create(&t, nullptr, TickThread, nullptr);
    pthread_detach(t);
    OLOGI("overlay start() called — expect Toast + red MENU");
    return true;
}
