# Pixel Gun 3D Internal ESP — 26.10.2

Android arm64 port of ESP from **[stanuwu/PixelGunCheatInternal](https://github.com/stanuwu/PixelGunCheatInternal)**.

Same pipeline as the PC cheat:
1. Hook `PlayerMoveC.Update`
2. Local player = nick `#Player Nickname`
3. Enemy = `nickLabel` TextMesh color RGB(1,0,0)
4. Position from `myPlayerTransform` → W2S → box / name / tracer

## Target
- Package: `com.pixel.gun3d`
- Version: **26.10.2** (`versionCode` 626974)
- Inject: **AndKittyInjector**

## Download
`pixelgun_internal/libs/arm64-v8a/libpg_internal.so`

## Inject (root / Termux)
```bash
am force-stop com.pixel.gun3d
# start game, wait for lobby, then:
/data/local/tmp/AndKittyInjector \
  --package com.pixel.gun3d \
  --libs /data/local/tmp/libpg_internal.so
```

On success logcat (`PGInternal`) shows resolved RVAs and `hooked PlayerMoveC.Update`.
Offsets also written to:
- `/sdcard/Download/pg3d_26.10.2_offsets.h`
- `/data/local/tmp/pg3d_26.10.2_offsets.h`

## Notes
- APK `global-metadata.dat` is encrypted offline — offsets are resolved **at runtime** via `xdl` (same idea as Zygisk-Il2CppDumper) using class/field names from the stanuwu cheat (`PlayerMoveC`, `myPlayerTransform`, `nickLabel`).
- If Cubic obfuscated those names on Android, resolve fails and you need a live `dump.cs` — open an issue with the offsets header / logcat.
- Original PC source kept under `PixelGunCheatInternal/` for reference.

## Build
```bash
ndk-build -C pixelgun_internal/jni
```
