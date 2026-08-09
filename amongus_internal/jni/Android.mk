LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE     := au_internal
LOCAL_SRC_FILES  := \
    main.cpp \
    game.cpp \
    overlay.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)

LOCAL_CFLAGS     := -O2 -fvisibility=hidden -ffunction-sections -fdata-sections
LOCAL_CPPFLAGS   := -std=c++17 -O2 -fvisibility=hidden -ffunction-sections -fdata-sections
LOCAL_LDLIBS     := -llog -landroid -ldl
LOCAL_LDFLAGS    := -Wl,--gc-sections

include $(BUILD_SHARED_LIBRARY)
