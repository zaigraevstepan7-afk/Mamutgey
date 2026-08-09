#include "ui.h"
#include "game.h"

#include "imgui/imgui.h"
#include "backends/imgui_impl_opengl3.h"

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <jni.h>
#include <dlfcn.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cstring>
#include <atomic>

struct HookRec {
    void* target = nullptr;
    void* replace = nullptr;
    void* trampoline = nullptr;
    uint32_t orig[4]{};
    bool active = false;
};

static size_t PageOf(uintptr_t addr) {
    static size_t ps = static_cast<size_t>(sysconf(_SC_PAGESIZE));
    return addr & ~(ps - 1);
}

static bool Unprotect(void* addr, size_t len) {
    uintptr_t start = PageOf(reinterpret_cast<uintptr_t>(addr));
    uintptr_t end = PageOf(reinterpret_cast<uintptr_t>(addr) + len - 1) + sysconf(_SC_PAGESIZE);
    return mprotect(reinterpret_cast<void*>(start), end - start, PROT_READ | PROT_WRITE | PROT_EXEC) == 0;
}

static void WriteAbsJump(void* at, void* to) {
    uint32_t* p = reinterpret_cast<uint32_t*>(at);
    p[0] = 0x58000050; // LDR X16, #8
    p[1] = 0xD61F0200; // BR X16
    *reinterpret_cast<uint64_t*>(p + 2) = reinterpret_cast<uint64_t>(to);
}

static bool InstallHook(HookRec& h, void* target, void* replace) {
    if (!target || !replace) return false;
    h.target = target;
    h.replace = replace;
    memcpy(h.orig, target, sizeof(h.orig));

    void* tri = mmap(nullptr, 4096, PROT_READ | PROT_WRITE | PROT_EXEC,
                     MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (tri == MAP_FAILED) return false;
    memcpy(tri, h.orig, 16);
    WriteAbsJump(reinterpret_cast<uint8_t*>(tri) + 16,
                 reinterpret_cast<uint8_t*>(target) + 16);
    h.trampoline = tri;

    if (!Unprotect(target, 16)) return false;
    WriteAbsJump(target, replace);
    __builtin___clear_cache(reinterpret_cast<char*>(target),
                            reinterpret_cast<char*>(target) + 16);
    h.active = true;
    return true;
}

using EglSwap_t = EGLBoolean (*)(EGLDisplay, EGLSurface);
static HookRec g_SwapHook;
static EglSwap_t g_OrigSwap = nullptr;
static std::atomic<bool> g_ImguiInit{false};
static int g_W = 0, g_H = 0;
static std::atomic<bool> g_Running{true};
static JavaVM* g_VM = nullptr;

static void EnsureImGui(EGLDisplay dpy, EGLSurface surf) {
    EGLint w = 0, h = 0;
    eglQuerySurface(dpy, surf, EGL_WIDTH, &w);
    eglQuerySurface(dpy, surf, EGL_HEIGHT, &h);
    if (w <= 0 || h <= 0) return;
    g_W = w;
    g_H = h;

    if (!g_ImguiInit.load()) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        ImGui::StyleColorsDark();
        ImGui_ImplOpenGL3_Init("#version 300 es");
        g_ImguiInit.store(true);
        LOGI("ImGui init %dx%d", w, h);
    }
}

static EGLBoolean Hooked_eglSwapBuffers(EGLDisplay dpy, EGLSurface surf) {
    if (dpy && surf != EGL_NO_SURFACE) {
        EnsureImGui(dpy, surf);
        if (g_ImguiInit.load()) {
            Game_TickCollect();
            Menu_ApplyTouches();
            ImGui_ImplOpenGL3_NewFrame();
            ImGui::NewFrame();
            Esp_Draw(static_cast<float>(g_W), static_cast<float>(g_H));
            Menu_Draw(static_cast<float>(g_W), static_cast<float>(g_H));
            ImGui::Render();
            g_ImguiWantMouse.store(ImGui::GetIO().WantCaptureMouse);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
    }
    return g_OrigSwap(dpy, surf);
}

// UnityPlayer.nativeInjectEvent — touch → ImGui
using NativeInject_t = jboolean (*)(JNIEnv*, jobject, jobject);
static NativeInject_t g_OrigNativeInject = nullptr;
static bool g_TouchHooked = false;

static jboolean Hooked_nativeInjectEvent(JNIEnv* env, jobject thiz, jobject event) {
    if (event && g_ImguiInit.load()) {
        jclass motionCls = env->FindClass("android/view/MotionEvent");
        if (motionCls && env->IsInstanceOf(event, motionCls)) {
            jmethodID getAction = env->GetMethodID(motionCls, "getActionMasked", "()I");
            jmethodID getX = env->GetMethodID(motionCls, "getX", "()F");
            jmethodID getY = env->GetMethodID(motionCls, "getY", "()F");
            jint action = env->CallIntMethod(event, getAction);
            jfloat x = env->CallFloatMethod(event, getX);
            jfloat y = env->CallFloatMethod(event, getY);

            // ACTION_DOWN=0, UP=1, MOVE=2, CANCEL=3, POINTER_DOWN=5, POINTER_UP=6
            bool down = (action == 0 || action == 2 || action == 5);
            if (action == 1 || action == 3 || action == 6) down = false;
            Menu_PushTouch(x, y, down);

            if (g_ImguiWantMouse.load()) {
                if (action == 0 || action == 5 || action == 1 || action == 6 || action == 3)
                    return JNI_TRUE; // swallow clicks on menu
            }
        }
    }
    if (g_OrigNativeInject) return g_OrigNativeInject(env, thiz, event);
    return JNI_FALSE;
}

static void* FindUnityNativeInject() {
    const char* names[] = {
        "Java_com_unity3d_player_UnityPlayer_nativeInjectEvent",
        "Java_com_unity3d_player_ReflectionHelper_nativeInjectEvent",
        nullptr};
    const char* libs[] = {"libunity.so", "libmain.so", "libil2cpp.so", nullptr};
    for (int L = 0; libs[L]; ++L) {
        void* h = dlopen(libs[L], RTLD_NOLOAD);
        if (!h) h = dlopen(libs[L], RTLD_NOW);
        if (!h) continue;
        for (int N = 0; names[N]; ++N) {
            void* s = dlsym(h, names[N]);
            if (s) return s;
        }
    }
    // last resort: RTLD_DEFAULT
    for (int N = 0; names[N]; ++N) {
        void* s = dlsym(RTLD_DEFAULT, names[N]);
        if (s) return s;
    }
    return nullptr;
}

static bool TryHookUnityTouch(JNIEnv* env) {
    if (g_TouchHooked) return true;
    (void)env;

    void* sym = FindUnityNativeInject();
    if (!sym) return false;

    static HookRec touchHook;
    if (!InstallHook(touchHook, sym, reinterpret_cast<void*>(Hooked_nativeInjectEvent))) {
        LOGE("inline hook nativeInjectEvent failed");
        return false;
    }
    g_OrigNativeInject = reinterpret_cast<NativeInject_t>(touchHook.trampoline);
    g_TouchHooked = true;
    LOGI("nativeInjectEvent inline-hooked @ %p", sym);
    return true;
}

static void* HackThread(void*) {
    LOGI("hack thread start");
    for (int i = 0; i < 300 && g_Running.load(); ++i) {
        Il2CppReady();
        if (g_VM) {
            JNIEnv* env = nullptr;
            if (g_VM->AttachCurrentThread(&env, nullptr) == 0 && env) {
                TryHookUnityTouch(env);
            }
        }
        if (UBase && g_TouchHooked) break;
        usleep(100 * 1000);
    }

    void* egl = dlopen("libEGL.so", RTLD_NOW);
    void* sym = egl ? dlsym(egl, "eglSwapBuffers") : nullptr;
    if (!sym) {
        LOGE("eglSwapBuffers missing");
        return nullptr;
    }
    if (!InstallHook(g_SwapHook, sym, reinterpret_cast<void*>(Hooked_eglSwapBuffers))) {
        LOGE("hook eglSwapBuffers failed");
        return nullptr;
    }
    g_OrigSwap = reinterpret_cast<EglSwap_t>(g_SwapHook.trampoline);
    LOGI("eglSwapBuffers hooked OK");

    // Keep retrying touch hook a bit longer
    for (int i = 0; i < 100 && g_Running.load() && !g_TouchHooked; ++i) {
        if (g_VM) {
            JNIEnv* env = nullptr;
            if (g_VM->AttachCurrentThread(&env, nullptr) == 0 && env)
                TryHookUnityTouch(env);
        }
        usleep(200 * 1000);
    }
    return nullptr;
}

extern "C" jint JNI_OnLoad(JavaVM* vm, void*) {
    g_VM = vm;
    LOGI("JNI_OnLoad — Among Us Internal 2026.6.5");
    return JNI_VERSION_1_6;
}

__attribute__((constructor))
static void on_load() {
    LOGI("libau_internal constructor");
    pthread_t t;
    pthread_create(&t, nullptr, HackThread, nullptr);
    pthread_detach(t);
}
