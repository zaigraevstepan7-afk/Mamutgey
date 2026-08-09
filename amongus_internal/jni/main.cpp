#include "ui.h"
#include "game.h"

#include "imgui/imgui.h"
#include "backends/imgui_impl_opengl3.h"

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <jni.h>
#include <dlfcn.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>
#include <cstring>
#include <atomic>
#include <chrono>

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

// Returns false if first 16 bytes contain PC-relative ops that break a naive trampoline.
static bool InstructionsSafeForTrampoline(const uint32_t* ins, int count) {
    for (int i = 0; i < count; ++i) {
        uint32_t w = ins[i];
        // B / BL (unconditional branch imm)
        if ((w & 0x7C000000) == 0x14000000) return false;
        // CBZ / CBNZ
        if ((w & 0x7E000000) == 0x34000000) return false;
        // TBZ / TBNZ
        if ((w & 0x7E000000) == 0x36000000) return false;
        // B.cond
        if ((w & 0xFF000010) == 0x54000000) return false;
        // ADR / ADRP
        if ((w & 0x1F000000) == 0x10000000) return false;
        // LDR (literal) 0x18000000 / 0x58000000 / 0x98000000
        if ((w & 0x3B000000) == 0x18000000) return false;
    }
    return true;
}

static bool InstallHook(HookRec& h, void* target, void* replace) {
    if (!target || !replace) return false;
    h.target = target;
    h.replace = replace;
    memcpy(h.orig, target, sizeof(h.orig));

    if (!InstructionsSafeForTrampoline(h.orig, 4)) {
        LOGE("hook target has PC-relative ops in first 16 bytes — abort to avoid crash");
        return false;
    }

    void* tri = mmap(nullptr, 4096, PROT_READ | PROT_WRITE | PROT_EXEC,
                     MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (tri == MAP_FAILED) return false;
    memcpy(tri, h.orig, 16);
    WriteAbsJump(reinterpret_cast<uint8_t*>(tri) + 16,
                 reinterpret_cast<uint8_t*>(target) + 16);
    __builtin___clear_cache(reinterpret_cast<char*>(tri),
                            reinterpret_cast<char*>(tri) + 32);
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
static std::chrono::steady_clock::time_point g_LastFrame =
    std::chrono::steady_clock::now();

struct GLStateBackup {
    GLint viewport[4]{};
    GLint scissor[4]{};
    GLboolean scissorTest{};
    GLboolean blend{};
    GLboolean depthTest{};
    GLboolean cullFace{};
    GLint framebuffer{};
    GLint program{};
    GLint activeTexture{};
};

static void BackupGL(GLStateBackup& s) {
    glGetIntegerv(GL_VIEWPORT, s.viewport);
    glGetIntegerv(GL_SCISSOR_BOX, s.scissor);
    s.scissorTest = glIsEnabled(GL_SCISSOR_TEST);
    s.blend = glIsEnabled(GL_BLEND);
    s.depthTest = glIsEnabled(GL_DEPTH_TEST);
    s.cullFace = glIsEnabled(GL_CULL_FACE);
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &s.framebuffer);
    glGetIntegerv(GL_CURRENT_PROGRAM, &s.program);
    glGetIntegerv(GL_ACTIVE_TEXTURE, &s.activeTexture);
}

static void RestoreGL(const GLStateBackup& s) {
    glBindFramebuffer(GL_FRAMEBUFFER, s.framebuffer);
    glUseProgram(s.program);
    glActiveTexture(s.activeTexture);
    glViewport(s.viewport[0], s.viewport[1], s.viewport[2], s.viewport[3]);
    glScissor(s.scissor[0], s.scissor[1], s.scissor[2], s.scissor[3]);
    if (s.scissorTest) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
    if (s.blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
    if (s.depthTest) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    if (s.cullFace) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
}

static const char* PickGlslVersion() {
    const char* ver = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    if (ver && strstr(ver, "OpenGL ES 3")) return "#version 300 es";
    // GLES2 / GL ES 2.0 context
    return "#version 100";
}

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
        io.ConfigWindowsMoveFromTitleBarOnly = true;
        ImGui::StyleColorsDark();
        const char* glsl = PickGlslVersion();
        if (!ImGui_ImplOpenGL3_Init(glsl)) {
            LOGE("ImGui_ImplOpenGL3_Init failed (glsl=%s)", glsl);
            ImGui::DestroyContext();
            return;
        }
        g_LastFrame = std::chrono::steady_clock::now();
        g_ImguiInit.store(true);
        LOGI("ImGui init %dx%d glsl=%s", w, h, glsl);
    }
}

static EGLBoolean Hooked_eglSwapBuffers(EGLDisplay dpy, EGLSurface surf) {
    if (dpy && surf != EGL_NO_SURFACE) {
        EnsureImGui(dpy, surf);
        if (g_ImguiInit.load()) {
            GLStateBackup glbak{};
            BackupGL(glbak);

            Game_TickCollect();

            ImGuiIO& io = ImGui::GetIO();
            io.DisplaySize = ImVec2(static_cast<float>(g_W), static_cast<float>(g_H));
            auto now = std::chrono::steady_clock::now();
            float dt = std::chrono::duration<float>(now - g_LastFrame).count();
            g_LastFrame = now;
            if (dt <= 0.f || dt > 1.f) dt = 1.f / 60.f;
            io.DeltaTime = dt;

            Menu_ApplyTouches();
            ImGui_ImplOpenGL3_NewFrame();
            ImGui::NewFrame();
            Esp_Draw(static_cast<float>(g_W), static_cast<float>(g_H));
            Menu_Draw(static_cast<float>(g_W), static_cast<float>(g_H));
            ImGui::Render();
            g_ImguiWantMouse.store(ImGui::GetIO().WantCaptureMouse);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            RestoreGL(glbak);
        }
    }
    return g_OrigSwap(dpy, surf);
}

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
            jmethodID getPtrCount = env->GetMethodID(motionCls, "getPointerCount", "()I");
            if (getAction && getX && getY) {
                jint action = env->CallIntMethod(event, getAction);
                jfloat x = env->CallFloatMethod(event, getX);
                jfloat y = env->CallFloatMethod(event, getY);

                // Scale view coords → EGL surface if they differ (Unity often mismatches)
                // Without Activity access we approximate: if touch coords exceed surface,
                // leave as-is; ImGui DisplaySize uses EGL size.

                bool down = (action == 0 || action == 2 || action == 5);
                if (action == 1 || action == 3 || action == 6) down = false;
                Menu_PushTouch(x, y, down);

                // Multi-touch: still forward to game if >1 pointer (joystick+button)
                int ptrs = getPtrCount ? env->CallIntMethod(event, getPtrCount) : 1;
                if (g_ImguiWantMouse.load() && ptrs <= 1) {
                    if (action == 0 || action == 5 || action == 1 || action == 6 || action == 3 || action == 2)
                        return JNI_TRUE;
                }
            }
        }
    }
    if (g_OrigNativeInject) return g_OrigNativeInject(env, thiz, event);
    return JNI_FALSE;
}

static void* FindUnityNativeInject() {
    const char* names[] = {
        "Java_com_unity3d_player_UnityPlayer_nativeInjectEvent",
        nullptr};
    const char* libs[] = {"libunity.so", "libmain.so", nullptr};
    for (int L = 0; libs[L]; ++L) {
        void* h = dlopen(libs[L], RTLD_NOLOAD);
        if (!h) h = dlopen(libs[L], RTLD_NOW);
        if (!h) continue;
        for (int N = 0; names[N]; ++N) {
            void* s = dlsym(h, names[N]);
            if (s) {
                LOGI("found %s in %s", names[N], libs[L]);
                return s;
            }
        }
    }
    for (int N = 0; names[N]; ++N) {
        void* s = dlsym(RTLD_DEFAULT, names[N]);
        if (s) return s;
    }
    return nullptr;
}

static bool TryHookUnityTouch() {
    if (g_TouchHooked) return true;
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

    // Wait for libil2cpp + try touch hook
    for (int i = 0; i < 400 && g_Running.load(); ++i) {
        Il2CppReady();
        TryHookUnityTouch();
        if (UBase) break;
        usleep(50 * 1000);
    }
    if (!UBase) LOGE("libil2cpp.so not found after wait — ESP will retry each frame");

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

    for (int i = 0; i < 150 && g_Running.load() && !g_TouchHooked; ++i) {
        TryHookUnityTouch();
        usleep(200 * 1000);
    }
    if (!g_TouchHooked)
        LOGE("touch hook unavailable — MENU button may not receive input (symbol missing / RegisterNatives-only)");
    return nullptr;
}

static std::atomic<bool> g_HackStarted{false};

static void StartHackOnce() {
    bool expected = false;
    if (!g_HackStarted.compare_exchange_strong(expected, true)) return;
    pthread_t t;
    pthread_create(&t, nullptr, HackThread, nullptr);
    pthread_detach(t);
}

extern "C" jint JNI_OnLoad(JavaVM* vm, void*) {
    g_VM = vm;
    LOGI("JNI_OnLoad — Among Us Internal 2026.6.5");
    StartHackOnce();
    return JNI_VERSION_1_6;
}

__attribute__((constructor))
static void on_load() {
    LOGI("libau_internal constructor");
    // Fallback if injector skips JNI_OnLoad
    StartHackOnce();
}
