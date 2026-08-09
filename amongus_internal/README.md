# Among Us Internal — ESP / Murder ESP (2026.6.5)

Internal arm64 cheat for **Among Us Android 2026.6.5** (`com.innersloth.spacemafia`, versionCode **7045**).  
Inject with **AndKittyInjector** (Kitty). Offsets are synced from the dump in `amongus_dump_2026.6.5/`.

## Features
- **Player ESP** — box, snapline, name, role, distance
- **Murder ESP** — impostor team highlight (Impostor / Shapeshifter / Phantom / Viper / ImpGhost)
- **White bottom strip** — tap the glowing white bar at the bottom to open/close the menu sheet
- Android View overlay (works on Vulkan + GLES)

## Build (NDK)
```bash
export NDK=/path/to/ndk   # e.g. $ANDROID_HOME/ndk/26.1.10909125
cd amongus_internal
$NDK/ndk-build -C jni
# output: libs/arm64-v8a/libau_internal.so
```

## Inject (AndKittyInjector)
```bash
adb push libs/arm64-v8a/libau_internal.so /data/local/tmp/
adb push AndKittyInjector /data/local/tmp/
adb shell chmod 755 /data/local/tmp/AndKittyInjector

# App already running:
adb push amongus_internal/libs/arm64-v8a/libau_internal.so /data/local/tmp/
adb shell /data/local/tmp/AndKittyInjector \
  --package com.innersloth.spacemafia \
  --libs /data/local/tmp/libau_internal.so

# Or launch + inject:
adb shell /data/local/tmp/AndKittyInjector \
  --package com.innersloth.spacemafia \
  --libs /data/local/tmp/libau_internal.so \
  --launch --delay 3000000
```

Prebuilt `.so` is already in `amongus_internal/libs/arm64-v8a/libau_internal.so`.

Kitty injector: https://github.com/MJx0/AndKittyInjector (or forks such as tuyilmaz/AndKittyInjector)

## APK source used for dump
- APKPure XAPK: `https://d.apkpure.net/b/XAPK/com.innersloth.spacemafia?version=latest`
- Version **2026.6.5** / Unity **2022.3.62f3**

## Notes
- Works in a **started** (or joined) match once `PlayerControl` statics are initialized.
- Murder roles are detected via `RoleBehaviour.TeamType` + `RoleTypes`.
- Re-dump offsets if Innersloth ships a new build.
