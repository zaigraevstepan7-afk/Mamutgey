# Among Us Internal — bug review (2026.6.5)

## Download

| File | Link |
|------|------|
| **libau_internal.so** (arm64) | https://github.com/zaigraevstepan7-afk/Mamutgey/raw/cursor/amongus-internal-esp-0381/amongus_internal/libs/arm64-v8a/libau_internal.so |
| PR | https://github.com/zaigraevstepan7-afk/Mamutgey/pull/2 |
| Branch | https://github.com/zaigraevstepan7-afk/Mamutgey/tree/cursor/amongus-internal-esp-0381/amongus_internal |
| Game APK (dump source) | https://d.apkpure.net/b/XAPK/com.innersloth.spacemafia?version=latest |

```bash
# quick get
curl -L -o libau_internal.so \
  "https://github.com/zaigraevstepan7-afk/Mamutgey/raw/cursor/amongus-internal-esp-0381/amongus_internal/libs/arm64-v8a/libau_internal.so"
adb push libau_internal.so /data/local/tmp/
adb shell /data/local/tmp/AndKittyInjector \
  --package com.innersloth.spacemafia \
  --libs /data/local/tmp/libau_internal.so
```

## Bugs found & fixed in this pass

### Critical (fixed)
1. **DisplaySize set after NewFrame** — hit-testing / menu clicks wrong. Now set before `NewFrame`.
2. **DeltaTime never set** — ImGui timers/hover broken. Now computed each frame.
3. **GL state not saved/restored** — Unity render corruption / flicker. Backup+restore around ImGui.
4. **ImGui init ignored failure** — GLES2 games would soft-break. Check `ImGui_ImplOpenGL3_Init`, pick `#version 100` vs `300 es`.
5. **Wrong `Transform_get_position_Injected` in offsets.h** — was `0x44565A0` (get) instead of `0x44565FC`.
6. **Double HackThread** — separate atomics in ctor/`JNI_OnLoad` could start two threads. Shared `StartHackOnce()`.
7. **Naive ARM64 trampoline on PC-relative prologues** — would crash. Reject unsafe first-16-byte patterns.
8. **List OOB** — no check `size <= max_length`. Added.
9. **Dangling RoleBehaviour / Data** — read without `IsUnityAlive`. Added.

### Remaining risks (not fully eliminable in this design)
1. **IL2CPP calls from GL thread** — `eglSwapBuffers` ≠ Unity main thread; rare crashes possible under load.
2. **Touch hook may fail** — modern Unity can bind `nativeInjectEvent` only via `RegisterNatives` (no `Java_*` export) → MENU not clickable. Check logcat: `touch hook unavailable`.
3. **View vs EGL coordinate mismatch** — MotionEvent space can differ from surface pixels on some devices → ESP/menu offset.
4. **No SEH/signal guard** — bad pointer still hard-crashes the process.
5. **Murder ESP in lobby** — roles often unset until game start; expect false negatives until `GameState == Started`.

### Medium / logic notes
- Murder detection uses `RoleBehaviour.TeamType == Impostor` **or** known murder `RoleTypes` (correct for Phantom/Viper/SS).
- ESP when only Murder ESP on correctly hides crewmates.
- `FindLibBase` now prefers `r-xp` mapping.
