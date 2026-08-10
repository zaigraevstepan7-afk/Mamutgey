LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := pg_internal

LOCAL_SRC_FILES := \
    main.cpp \
    Resolve.cpp \
    Functions.cpp \
    Hooks.cpp \
    overlay.cpp \
    And64InlineHook.cpp \
    xdl/xdl.c \
    xdl/xdl_iterate.c \
    xdl/xdl_linker.c \
    xdl/xdl_lzma.c \
    xdl/xdl_util.c

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH) \
    $(LOCAL_PATH)/xdl \
    $(LOCAL_PATH)/xdl/include

LOCAL_CFLAGS   := -O2 -fvisibility=hidden -DANDROID
LOCAL_CPPFLAGS := -std=c++17 -O2 -fvisibility=hidden
LOCAL_LDLIBS   := -llog -landroid -ldl
LOCAL_LDFLAGS  := -Wl,--gc-sections

include $(BUILD_SHARED_LIBRARY)
