#!/system/bin/sh
# Push & inject helper (run on PC with adb, or adapt for device shell)
PKG=com.innersloth.spacemafia
LIB=libau_internal.so
INJECTOR=AndKittyInjector

adb push "../libs/arm64-v8a/$LIB" /data/local/tmp/$LIB
adb shell chmod 644 /data/local/tmp/$LIB

if [ ! -f "/data/local/tmp/$INJECTOR" ]; then
  echo "Push AndKittyInjector to /data/local/tmp/$INJECTOR first"
  exit 1
fi

adb shell /data/local/tmp/$INJECTOR --package $PKG --libs /data/local/tmp/$LIB "$@"
