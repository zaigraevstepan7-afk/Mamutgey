LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE     := au_internal
LOCAL_SRC_FILES  := \
    main.cpp \
    game.cpp \
    ui.cpp \
    overlay.cpp \
    imgui/imgui.cpp \
    imgui/imgui_draw.cpp \
    imgui/imgui_tables.cpp \
    imgui/imgui_widgets.cpp \
    backends/imgui_impl_opengl3.cpp

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH) \
    $(LOCAL_PATH)/imgui \
    $(LOCAL_PATH)/backends

LOCAL_CFLAGS     := -O2 -fvisibility=hidden -ffunction-sections -fdata-sections
LOCAL_CPPFLAGS   := -std=c++17 -O2 -fvisibility=hidden -ffunction-sections -fdata-sections
LOCAL_LDLIBS     := -llog -landroid -lEGL -lGLESv3 -ldl
LOCAL_LDFLAGS    := -Wl,--gc-sections

include $(BUILD_SHARED_LIBRARY)
