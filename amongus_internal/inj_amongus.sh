#!/system/bin/sh
# Among Us Kitty inject — package MUST be spacemafia, not standoff2
PKG=com.innersloth.spacemafia
LIB=/data/local/tmp/libau_internal.so
INJ=/data/local/tmp/AndKittyInjector

# Prefer whatever injector binary name you have
[ -x "$INJ" ] || INJ=/data/local/tmp/injector
[ -x "$INJ" ] || INJ=/data/local/tmp/kitty

if [ ! -f "$LIB" ]; then
  echo "E: missing $LIB — adb push new libau_internal.so first"
  exit 1
fi

echo "I: killing old Among Us (clean inject)..."
am force-stop "$PKG" 2>/dev/null
sleep 1
monkey -p "$PKG" -c android.intent.category.LAUNCHER 1 >/dev/null 2>&1
sleep 8

echo "I: injecting once into $PKG ..."
"$INJ" --package "$PKG" --libs "$LIB" --memfd --delay 2000000

echo "I: check toast 'AU CHEAT OK' and logcat:"
echo "    logcat -d -s AUInternal:I | tail -40"
