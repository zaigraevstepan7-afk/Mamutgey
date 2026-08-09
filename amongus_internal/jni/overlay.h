#pragma once
#include <jni.h>

// Android View overlay (works on GLES + Vulkan). Call from hack thread with JavaVM.
bool Overlay_Start(JavaVM* vm);
void Overlay_Shutdown();
bool Overlay_IsAlive();
