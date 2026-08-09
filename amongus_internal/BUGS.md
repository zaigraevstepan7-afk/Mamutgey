# Among Us Internal — bug review (2026.6.5)

## Download

| File | Link |
|------|------|
| **libau_internal.so** (arm64) | https://github.com/zaigraevstepan7-afk/Mamutgey/raw/cursor/amongus-internal-esp-0381/amongus_internal/libs/arm64-v8a/libau_internal.so |
| PR | https://github.com/zaigraevstepan7-afk/Mamutgey/pull/2 |
| Branch | https://github.com/zaigraevstepan7-afk/Mamutgey/tree/cursor/amongus-internal-esp-0381/amongus_internal |
| Game APK (dump source) | https://d.apkpure.net/b/XAPK/com.innersloth.spacemafia?version=latest |

```bash
curl -L -o libau_internal.so \
  "https://github.com/zaigraevstepan7-afk/Mamutgey/raw/cursor/amongus-internal-esp-0381/amongus_internal/libs/arm64-v8a/libau_internal.so"
adb push libau_internal.so /data/local/tmp/
adb shell am force-stop com.innersloth.spacemafia
# start game, then inject ONCE:
adb shell /data/local/tmp/AndKittyInjector \
  --package com.innersloth.spacemafia \
  --libs /data/local/tmp/libau_internal.so
```

Logcat marker for this build: `BUILD=20260809f bugfix`

## Bugs fixed in 20260809f

1. **RegisterNatives ABI break** — C++ registered `nativeEspFill([F)I` while embedded dex still had `([F)V` → overlay would fail to register. Java + dex updated; draw uses fill return count.
2. **ESP count/fill/label desync** — `nativeEspCount` + separate fill + soft cull dropped rows but labels/index still assumed count. Fill now returns written count and snapshots labels in the same pass.
3. **Murder ESP without box** — murder highlight now forces box bit even if Boxes toggle is off.
4. **Screen→overlay scale** — Unity `Screen` size vs View size mismatch corrected with scale factors.
5. **`CheatState` data race** — toggles are `std::atomic<bool>` (UI thread vs tick thread).
6. **Unsafe `siglongjmp` crash guard** — removed (UB with C++ destructors / vectors).
7. **Managed getters on worker thread** — dropped `get_PlayerName` / `get_DefaultOutfit`; read `Outfits` dictionary + `PlayerOutfit` fields instead.
8. **Activity local ref** — `UnityPlayer.currentActivity` kept as `NewGlobalRef` across async UI post.
9. **Dead ImGui path in build** — `Android.mk` only links overlay/game/main (Vulkan devices never hit `eglSwapBuffers`).

## Remaining risks

1. IL2CPP pointer reads from a worker thread can still hard-crash if objects are destroyed mid-frame.
2. Dictionary entry layout for `Outfits` is assumed (Entry size 24); if names show as `P#` only, layout may need a tweak.
3. Force-stop between injects; inject once per process.
4. Roles in lobby may be unset until `GameState == Started`.
