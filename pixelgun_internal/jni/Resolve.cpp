#include "Resolve.h"
#include "Offsets.h"
#include "Functions.h"
#include "log.h"
#include "xdl.h"

#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <unistd.h>

static void* (*il2cpp_domain_get)() = nullptr;
static void* (*il2cpp_thread_attach)(void*) = nullptr;
static const void** (*il2cpp_domain_get_assemblies)(const void*, size_t*) = nullptr;
static const void* (*il2cpp_assembly_get_image)(const void*) = nullptr;
static size_t (*il2cpp_image_get_class_count)(const void*) = nullptr;
static const void* (*il2cpp_image_get_class)(const void*, size_t) = nullptr;
static const char* (*il2cpp_class_get_name)(const void*) = nullptr;
static const char* (*il2cpp_class_get_namespace)(const void*) = nullptr;
static void* (*il2cpp_class_get_method_from_name)(const void*, const char*, int) = nullptr;
static void* (*il2cpp_class_get_field_from_name)(const void*, const char*) = nullptr;
static size_t (*il2cpp_field_get_offset)(void*) = nullptr;

struct MethodInfo_Min { void* methodPointer; };

uintptr_t FindLibBase(const char* needle) {
    std::ifstream maps("/proc/self/maps");
    std::string line;
    uintptr_t best = 0;
    while (std::getline(maps, line)) {
        if (line.find(needle) == std::string::npos) continue;
        uintptr_t start = 0;
        std::stringstream ss(line);
        ss >> std::hex >> start;
        if (!start) continue;
        bool exec = line.find("r-xp") != std::string::npos;
        if (exec) {
            if (!best || start < best) best = start;
        } else if (!best) best = start;
    }
    return best;
}

static void* ClassByName(const char* namespaze, const char* name) {
    size_t n = 0;
    auto domain = il2cpp_domain_get();
    if (!domain) return nullptr;
    auto asms = il2cpp_domain_get_assemblies(domain, &n);
    if (!asms) return nullptr;
    for (size_t i = 0; i < n; ++i) {
        auto image = il2cpp_assembly_get_image(asms[i]);
        if (!image) continue;
        size_t cc = il2cpp_image_get_class_count(image);
        for (size_t j = 0; j < cc; ++j) {
            auto klass = il2cpp_image_get_class(image, j);
            if (!klass) continue;
            const char* cn = il2cpp_class_get_name(klass);
            if (!cn || std::strcmp(cn, name) != 0) continue;
            if (namespaze && namespaze[0]) {
                const char* ns = il2cpp_class_get_namespace(klass);
                if (!ns || std::strcmp(ns, namespaze) != 0) continue;
            }
            return const_cast<void*>(klass);
        }
    }
    return nullptr;
}

static uintptr_t MethodRva(void* klass, const char* name, int argc) {
    if (!klass || !g_Il2Cpp) return 0;
    auto mi = (MethodInfo_Min*)il2cpp_class_get_method_from_name(klass, name, argc);
    if (!mi || !mi->methodPointer) {
        for (int a = 0; a <= 3; ++a) {
            mi = (MethodInfo_Min*)il2cpp_class_get_method_from_name(klass, name, a);
            if (mi && mi->methodPointer) break;
        }
    }
    if (!mi || !mi->methodPointer) return 0;
    return (uintptr_t)mi->methodPointer - g_Il2Cpp;
}

static uintptr_t FieldOff(void* klass, const char* name) {
    if (!klass) return 0;
    void* f = il2cpp_class_get_field_from_name(klass, name);
    if (!f) return 0;
    return (uintptr_t)il2cpp_field_get_offset(f);
}

static void WriteOffsetsHeader(const char* path) {
    std::ofstream o(path);
    if (!o) { LOGW("cannot write %s", path); return; }
    o << std::hex;
    o << "#pragma once\n// Auto from live resolve — stanuwu PixelGunCheatInternal layout\n";
    o << "#include <cstdint>\nnamespace Offsets {\n";
    o << "inline uintptr_t PlayerMoveCUpdate=0x" << Offsets::PlayerMoveCUpdate << ";\n";
    o << "inline uintptr_t WorldToScreenPoint=0x" << Offsets::WorldToScreenPoint << ";\n";
    o << "inline uintptr_t CameraGetMain=0x" << Offsets::CameraGetMain << ";\n";
    o << "inline uintptr_t ComponentGetTransform=0x" << Offsets::ComponentGetTransform << ";\n";
    o << "inline uintptr_t TransformGetPosition=0x" << Offsets::TransformGetPosition << ";\n";
    o << "inline uintptr_t TextMeshGetText=0x" << Offsets::TextMeshGetText << ";\n";
    o << "inline uintptr_t TextMeshGetColor=0x" << Offsets::TextMeshGetColor << ";\n";
    o << "inline uintptr_t myPlayerTransform=0x" << Offsets::myPlayerTransform << ";\n";
    o << "inline uintptr_t nickLabel=0x" << Offsets::nickLabel << ";\n";
    o << "inline uintptr_t headCollider=0x" << Offsets::headCollider << ";\n";
    o << "}\n";
    LOGI("wrote %s", path);
}

bool Resolve_All() {
    g_Il2Cpp = FindLibBase("libil2cpp.so");
    if (!g_Il2Cpp) { LOGE("libil2cpp.so not mapped"); return false; }
    LOGI("libil2cpp @ %p", (void*)g_Il2Cpp);

    void* handle = xdl_open("libil2cpp.so", XDL_DEFAULT);
    if (!handle) { LOGE("xdl_open failed"); return false; }

#define SYM(n) do { n = (decltype(n))xdl_sym(handle, #n, nullptr); if (!n) LOGW("missing %s", #n); } while(0)
    SYM(il2cpp_domain_get);
    SYM(il2cpp_thread_attach);
    SYM(il2cpp_domain_get_assemblies);
    SYM(il2cpp_assembly_get_image);
    SYM(il2cpp_image_get_class_count);
    SYM(il2cpp_image_get_class);
    SYM(il2cpp_class_get_name);
    SYM(il2cpp_class_get_namespace);
    SYM(il2cpp_class_get_method_from_name);
    SYM(il2cpp_class_get_field_from_name);
    SYM(il2cpp_field_get_offset);
#undef SYM

    if (!il2cpp_domain_get || !il2cpp_domain_get_assemblies || !il2cpp_image_get_class ||
        !il2cpp_class_get_method_from_name) {
        LOGE("critical APIs missing");
        return false;
    }

    auto domain = il2cpp_domain_get();
    if (!domain) { LOGE("domain null"); return false; }
    if (il2cpp_thread_attach) il2cpp_thread_attach(domain);

    void* cam = ClassByName("UnityEngine", "Camera");
    void* comp = ClassByName("UnityEngine", "Component");
    void* tr = ClassByName("UnityEngine", "Transform");
    void* tm = ClassByName("UnityEngine", "TextMesh");

    bool w2s_inj = false, pos_inj = false;
    if (cam) {
        Offsets::CameraGetMain = MethodRva(cam, "get_main", 0);
        uintptr_t inj = MethodRva(cam, "WorldToScreenPoint_Injected", 2);
        if (!inj) inj = MethodRva(cam, "WorldToScreenPoint_Injected", 3);
        if (inj) { Offsets::WorldToScreenPoint = inj; w2s_inj = true; }
        else Offsets::WorldToScreenPoint = MethodRva(cam, "WorldToScreenPoint", 2);
    }
    if (comp) Offsets::ComponentGetTransform = MethodRva(comp, "get_transform", 0);
    if (tr) {
        uintptr_t inj = MethodRva(tr, "get_position_Injected", 1);
        if (inj) { Offsets::TransformGetPosition = inj; pos_inj = true; }
        else Offsets::TransformGetPosition = MethodRva(tr, "get_position", 0);
    }
    if (tm) {
        Offsets::TextMeshGetText = MethodRva(tm, "get_text", 0);
        Offsets::TextMeshGetColor = MethodRva(tm, "get_color", 0);
    }

    void* pmc = ClassByName("", "PlayerMoveC");
    if (!pmc) {
        size_t n = 0;
        auto asms = il2cpp_domain_get_assemblies(domain, &n);
        for (size_t i = 0; i < n && !pmc; ++i) {
            auto image = il2cpp_assembly_get_image(asms[i]);
            if (!image) continue;
            size_t cc = il2cpp_image_get_class_count(image);
            for (size_t j = 0; j < cc; ++j) {
                auto klass = const_cast<void*>(il2cpp_image_get_class(image, j));
                if (!klass) continue;
                if (il2cpp_class_get_field_from_name(klass, "myPlayerTransform") &&
                    il2cpp_class_get_field_from_name(klass, "nickLabel")) {
                    pmc = klass;
                    LOGI("PlayerMoveC-like: %s", il2cpp_class_get_name(klass));
                    break;
                }
            }
        }
    }

    if (pmc) {
        Offsets::PlayerMoveCUpdate = MethodRva(pmc, "Update", 0);
        if (auto t = FieldOff(pmc, "myPlayerTransform")) Offsets::myPlayerTransform = t;
        if (auto n = FieldOff(pmc, "nickLabel")) Offsets::nickLabel = n;
        if (auto h = FieldOff(pmc, "headCollider")) Offsets::headCollider = h;
    } else {
        LOGE("PlayerMoveC not found (obfuscated?)");
    }

    WriteOffsetsHeader("/sdcard/Download/pg3d_26.10.2_offsets.h");
    WriteOffsetsHeader("/data/local/tmp/pg3d_26.10.2_offsets.h");

    Functions::init(g_Il2Cpp);
    Functions::SetInjectedFlags(w2s_inj, pos_inj);

    LOGI("RVA Update=0x%lx main=0x%lx W2S=0x%lx inj=%d/%d",
         (unsigned long)Offsets::PlayerMoveCUpdate,
         (unsigned long)Offsets::CameraGetMain,
         (unsigned long)Offsets::WorldToScreenPoint,
         w2s_inj ? 1 : 0, pos_inj ? 1 : 0);

    return Offsets::PlayerMoveCUpdate && Offsets::CameraGetMain && Offsets::WorldToScreenPoint &&
           Offsets::TransformGetPosition && Offsets::ComponentGetTransform;
}
