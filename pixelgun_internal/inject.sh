#!/system/bin/sh
# Kitty inject — Pixel Gun 3D 26.10.2 ESP (stanuwu port)
PKG=com.pixel.gun3d
SO=/data/local/tmp/libpg_internal.so
KITTY=/data/local/tmp/AndKittyInjector

am force-stop "$PKG"
sleep 1
# Launch then inject after delay if your Kitty supports --launch; else start game manually first.
"$KITTY" --package "$PKG" --libs "$SO"
echo "logcat -s PGInternal:I A64_HOOK:E"
