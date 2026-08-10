#include "overlay.h"
#include "Hooks.h"
#include "log.h"

// Embedded dex generated later — for first build use runtime path without dex:
// We register natives on a class loaded via InMemoryDex like amongus.
// Until dex is baked, Overlay_Start is a soft stub if load fails.

#include "overlay_dex.h"

#include <jni.h>
#include <pthread.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <vector>
#include <mutex>

static JavaVM* g_VM = nullptr;
static std::mutex g_LabelMu;
static std::vector<std::string> g_Labels;
static int g_ViewH = 0;

static JNIEnv* Env() {
    JNIEnv* e = nullptr;
    if (g_VM->GetEnv((void**)&e, JNI_VERSION_1_6) == JNI_OK) return e;
    if (g_VM->AttachCurrentThread(&e, nullptr) == 0) return e;
    return nullptr;
}

static jint J_count(JNIEnv*, jclass) {
    std::lock_guard<std::mutex> lk(g_EspMu);
    return (jint)g_EspList.size();
}

static jint J_fill(JNIEnv* env, jclass, jfloatArray arr) {
    std::vector<EspPlayer> snap;
    {
        std::lock_guard<std::mutex> lk(g_EspMu);
        snap = g_EspList;
    }
    std::vector<std::string> labels;
    labels.reserve(snap.size());
    jsize cap = env->GetArrayLength(arr);
    int written = 0;
    for (size_t i = 0; i < snap.size() && (written + 8) <= (int)cap; ++i) {
        auto& p = snap[i];
        jfloat v[8] = { p.sx, p.sy, p.width2, p.height2,
                        p.enemy ? 1.f : 0.f, p.distance, 0.f, 0.f };
        env->SetFloatArrayRegion(arr, written, 8, v);
        written += 8;
        labels.push_back(p.name);
    }
    {
        std::lock_guard<std::mutex> lk(g_LabelMu);
        g_Labels.swap(labels);
    }
    return (jint)(written / 8);
}

static jstring J_name(JNIEnv* env, jclass, jint i) {
    std::lock_guard<std::mutex> lk(g_LabelMu);
    if (i < 0 || i >= (jint)g_Labels.size()) return env->NewStringUTF("");
    return env->NewStringUTF(g_Labels[i].c_str());
}

static void J_setEsp(JNIEnv*, jclass, jboolean v) { g_Cheat.espEnabled = v; }
static jboolean J_getEsp(JNIEnv*, jclass) { return g_Cheat.espEnabled; }
static jstring J_status(JNIEnv* env, jclass) {
    char buf[128];
    snprintf(buf, sizeof(buf), "PG ESP | players:%d cam:%s",
             (int)g_EspList.size(), g_MainCamera ? "OK" : "?");
    return env->NewStringUTF(buf);
}

static JNINativeMethod g_Methods[] = {
    {"nativeEspCount", "()I", (void*)J_count},
    {"nativeEspFill", "([F)I", (void*)J_fill},
    {"nativeEspName", "(I)Ljava/lang/String;", (void*)J_name},
    {"nativeSetEsp", "(Z)V", (void*)J_setEsp},
    {"nativeGetEsp", "()Z", (void*)J_getEsp},
    {"nativeStatus", "()Ljava/lang/String;", (void*)J_status},
};

static jclass LoadDex(JNIEnv* env, jobject appCl) {
    jclass bbCls = env->FindClass("java/nio/ByteBuffer");
    jmethodID wrap = env->GetStaticMethodID(bbCls, "wrap", "([B)Ljava/nio/ByteBuffer;");
    jbyteArray arr = env->NewByteArray((jsize)pg_overlay_dex_len);
    env->SetByteArrayRegion(arr, 0, (jsize)pg_overlay_dex_len, (const jbyte*)pg_overlay_dex);
    jobject buf = env->CallStaticObjectMethod(bbCls, wrap, arr);
    jclass imdex = env->FindClass("dalvik/system/InMemoryDexClassLoader");
    jmethodID ctor = env->GetMethodID(imdex, "<init>", "(Ljava/nio/ByteBuffer;Ljava/lang/ClassLoader;)V");
    jobject loader = env->NewObject(imdex, ctor, buf, appCl);
    jclass clCls = env->FindClass("java/lang/ClassLoader");
    jmethodID load = env->GetMethodID(clCls, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
    jstring name = env->NewStringUTF("pg.overlay.PgOverlay");
    jobject cls = env->CallObjectMethod(loader, load, name);
    if (!cls || env->ExceptionCheck()) {
        env->ExceptionClear();
        LOGE("load PgOverlay failed");
        return nullptr;
    }
    return (jclass)env->NewGlobalRef(cls);
}

bool Overlay_Start(JavaVM* vm) {
    g_VM = vm;
    JNIEnv* env = Env();
    if (!env) return false;

    jclass at = env->FindClass("android/app/ActivityThread");
    jmethodID curApp = env->GetStaticMethodID(at, "currentApplication", "()Landroid/app/Application;");
    jobject app = env->CallStaticObjectMethod(at, curApp);
    if (!app) return false;
    jclass ctx = env->FindClass("android/content/Context");
    jmethodID getCl = env->GetMethodID(ctx, "getClassLoader", "()Ljava/lang/ClassLoader;");
    jobject appCl = env->CallObjectMethod(app, getCl);

    // Unity activity
    jclass clCls = env->FindClass("java/lang/ClassLoader");
    jmethodID load = env->GetMethodID(clCls, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
    jstring upName = env->NewStringUTF("com.unity3d.player.UnityPlayer");
    jobject upCls = env->CallObjectMethod(appCl, load, upName);
    if (!upCls || env->ExceptionCheck()) { env->ExceptionClear(); return false; }
    jfieldID fid = env->GetStaticFieldID((jclass)upCls, "currentActivity", "Landroid/app/Activity;");
    jobject act = env->GetStaticObjectField((jclass)upCls, fid);
    if (!act) return false;
    act = env->NewGlobalRef(act);

    jclass ovl = LoadDex(env, appCl);
    if (!ovl) return false;
    if (env->RegisterNatives(ovl, g_Methods, (jint)(sizeof(g_Methods)/sizeof(g_Methods[0]))) != 0) {
        env->ExceptionClear();
        LOGE("RegisterNatives failed");
        return false;
    }
    jmethodID start = env->GetStaticMethodID(ovl, "start", "(Landroid/app/Activity;)V");
    env->CallStaticVoidMethod(ovl, start, act);
    if (env->ExceptionCheck()) { env->ExceptionClear(); return false; }
    LOGI("PgOverlay started");
    return true;
}
